#include <cstdlib>
#include <memory>

#include <spdlog/cfg/env.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

#include "capture/WindowCapture.h"
#include "control/ControlThread.h"
#include "control/InputController.h"
#include "pipeline/Mailbox.h"
#include "robot/Session.h"
#include "scripting/ScriptRuntime.h"
#include "ui/LogRingSink.h"
#include "ui/UiApp.h"

int main() {
  auto ringSink = std::make_shared<ae::ui::LogRingSink>();
  auto stdoutSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
  spdlog::set_default_logger(std::make_shared<spdlog::logger>(
      "ae", spdlog::sinks_init_list{stdoutSink, ringSink}
  ));
  spdlog::cfg::load_env_levels();  // SPDLOG_LEVEL=debug etc.

  auto session = robot::Session::create();
  if (!session) {
    spdlog::error("robot-cpp unavailable: {}", session.error().message);
    return EXIT_FAILURE;
  }
  const auto& capabilities = (*session)->capabilities();
  spdlog::info(
      "robot-cpp session ready (keyboard: {}, mouse: {}, unicode text: {})",
      capabilities.canInjectKeyboard, capabilities.canInjectMouse,
      capabilities.canTypeUnicode
  );

  ae::capture::FrameMailbox frames;
  ae::pipeline::OverlayMailbox overlays;
  ae::pipeline::StatsMailbox stats;
  ae::capture::WindowCapture capture;
  ae::control::InputController input(**session);
  ae::scripting::ScriptRuntime script(input);
  ae::control::ControlThread control(frames, overlays, stats, input, script);
  ae::ui::UiApp ui(frames, overlays, stats, control.commands(), capture, *ringSink);

  if (auto initialized = ui.init(); !initialized) {
    spdlog::error("{}", initialized.error().message);
    return EXIT_FAILURE;
  }

  control.start();
  ui.run();

  // Shutdown order matters: stop producing frames, then stop the control
  // thread (its exit path releases all held input), then everything else
  // tears down via destructors in reverse declaration order.
  capture.stop();
  control.stop();

  return EXIT_SUCCESS;
}
