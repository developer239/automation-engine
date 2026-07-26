#pragma once

#include <cstdint>
#include <expected>
#include <string>
#include <vector>

#include "capture/WindowCapture.h"
#include "common/Error.h"
#include "control/Commands.h"
#include "pipeline/Mailbox.h"
#include "pipeline/Overlay.h"
#include "pipeline/Stats.h"
#include "ui/LogRingSink.h"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Texture;

namespace ae::ui {

// The main-thread shell: SDL3 window, ImGui docking UI, and the stream
// texture. Strictly an observer of the pipeline - it polls the frame, overlay,
// and stats mailboxes and never blocks on them, and it talks to the control
// thread only through the command queue.
class UiApp {
 public:
  UiApp(
      capture::FrameMailbox& frames, pipeline::OverlayMailbox& overlays,
      pipeline::StatsMailbox& stats, control::CommandQueue& commands,
      capture::WindowCapture& capture, LogRingSink& logSink
  )
      : frames_(frames),
        overlays_(overlays),
        stats_(stats),
        commands_(commands),
        capture_(capture),
        logSink_(logSink) {}

  ~UiApp();
  UiApp(const UiApp&) = delete;
  UiApp& operator=(const UiApp&) = delete;
  UiApp(UiApp&&) = delete;
  UiApp& operator=(UiApp&&) = delete;

  std::expected<void, Error> init();

  // Runs the render loop until the user quits.
  void run();

 private:
  void drawPickerWindow();
  void drawStreamWindow();
  void drawScriptWindow();
  void drawStatsWindow();
  void drawLogWindow();
  void updateStreamTexture();
  void pollAutoReload();

  capture::FrameMailbox& frames_;
  pipeline::OverlayMailbox& overlays_;
  pipeline::StatsMailbox& stats_;
  control::CommandQueue& commands_;
  capture::WindowCapture& capture_;
  LogRingSink& logSink_;

  SDL_Window* window_ = nullptr;
  SDL_Renderer* renderer_ = nullptr;
  SDL_Texture* streamTexture_ = nullptr;
  std::int32_t textureWidth_ = 0;
  std::int32_t textureHeight_ = 0;

  std::uint64_t seenFrameSeq_ = 0;
  std::uint64_t seenOverlaySeq_ = 0;
  std::uint64_t seenStatsSeq_ = 0;
  pipeline::OverlaySnapshot overlay_;
  pipeline::ControlStats statsSnapshot_;

  std::vector<capture::WindowInfo> windows_;
  std::string pickerError_;

  char scriptPathBuffer_[1024] = {};
  bool requireFocus_ = true;
  bool autoReload_ = false;
  std::int64_t loadedScriptMtime_ = 0;
  double lastReloadCheckAt_ = 0.0;

  std::uint64_t seenLogVersion_ = 0;
  std::vector<LogRingSink::Entry> logEntries_;
};

}  // namespace ae::ui
