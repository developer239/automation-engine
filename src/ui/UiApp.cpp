#include "ui/UiApp.h"

#include <algorithm>
#include <filesystem>

#include <SDL3/SDL.h>
#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_sdlrenderer3.h>
#include <spdlog/spdlog.h>

#include "capture/FrameHandle.h"

namespace ae::ui {

namespace {

ImU32 toImColor(const pipeline::OverlayColor color) {
  return IM_COL32(color.r, color.g, color.b, color.a);
}

std::int64_t fileMtime(const std::string& path) {
  std::error_code errorCode;
  const auto time = std::filesystem::last_write_time(path, errorCode);
  if (errorCode) return 0;
  return time.time_since_epoch().count();
}

}  // namespace

UiApp::~UiApp() {
  if (streamTexture_ != nullptr) SDL_DestroyTexture(streamTexture_);
  if (renderer_ != nullptr) {
    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
    SDL_DestroyRenderer(renderer_);
  }
  if (window_ != nullptr) SDL_DestroyWindow(window_);
  SDL_Quit();
}

std::expected<void, Error> UiApp::init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    return std::unexpected(Error::uiInitFailed(SDL_GetError()));
  }

  window_ = SDL_CreateWindow(
      "automation-engine", 1600, 1000,
      SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIGH_PIXEL_DENSITY
  );
  if (window_ == nullptr) {
    return std::unexpected(Error::uiInitFailed(SDL_GetError()));
  }

  renderer_ = SDL_CreateRenderer(window_, nullptr);
  if (renderer_ == nullptr) {
    return std::unexpected(Error::uiInitFailed(SDL_GetError()));
  }
  SDL_SetRenderVSync(renderer_, 1);

  IMGUI_CHECKVERSION();
  ImGui::CreateContext();
  ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
  ImGui::StyleColorsDark();
  ImGui_ImplSDL3_InitForSDLRenderer(window_, renderer_);
  ImGui_ImplSDLRenderer3_Init(renderer_);

  return {};
}

void UiApp::run() {
  bool quit = false;
  while (!quit) {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
      ImGui_ImplSDL3_ProcessEvent(&event);
      if (event.type == SDL_EVENT_QUIT) quit = true;
    }

    if (const auto stats = stats_.poll(seenStatsSeq_)) {
      seenStatsSeq_ = stats->seq;
      statsSnapshot_ = stats->value;
    }
    if (const auto overlay = overlays_.poll(seenOverlaySeq_)) {
      seenOverlaySeq_ = overlay->seq;
      overlay_ = overlay->value;
    }
    updateStreamTexture();
    pollAutoReload();

    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();
    ImGui::DockSpaceOverViewport();

    drawPickerWindow();
    drawStreamWindow();
    drawScriptWindow();
    drawStatsWindow();
    drawLogWindow();

    ImGui::Render();
    // ImGui works in points; the Retina backbuffer is in pixels. Without this
    // scale the whole UI renders at half size with mismatched clip rects and
    // hit-testing.
    SDL_SetRenderScale(
        renderer_, ImGui::GetIO().DisplayFramebufferScale.x,
        ImGui::GetIO().DisplayFramebufferScale.y
    );
    SDL_SetRenderDrawColor(renderer_, 24, 24, 26, 255);
    SDL_RenderClear(renderer_);
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), renderer_);
    SDL_RenderPresent(renderer_);
  }
}

void UiApp::updateStreamTexture() {
  const auto frame = frames_.poll(seenFrameSeq_);
  if (!frame) return;
  seenFrameSeq_ = frame->seq;

  const capture::CapturedFrame& captured = frame->value;
  CVPixelBufferRef pixels = captured.pixels.get();
  if (pixels == nullptr) return;

  if (streamTexture_ == nullptr || textureWidth_ != captured.width ||
      textureHeight_ != captured.height) {
    if (streamTexture_ != nullptr) SDL_DestroyTexture(streamTexture_);
    streamTexture_ = SDL_CreateTexture(
        renderer_, SDL_PIXELFORMAT_BGRA32, SDL_TEXTUREACCESS_STREAMING,
        captured.width, captured.height
    );
    if (streamTexture_ == nullptr) {
      spdlog::error("[UI] failed to create stream texture: {}", SDL_GetError());
      return;
    }
    textureWidth_ = captured.width;
    textureHeight_ = captured.height;
  }

  CVPixelBufferLockBaseAddress(pixels, kCVPixelBufferLock_ReadOnly);
  SDL_UpdateTexture(
      streamTexture_, nullptr, CVPixelBufferGetBaseAddress(pixels),
      static_cast<int>(CVPixelBufferGetBytesPerRow(pixels))
  );
  CVPixelBufferUnlockBaseAddress(pixels, kCVPixelBufferLock_ReadOnly);
}

void UiApp::drawPickerWindow() {
  ImGui::Begin("Windows");

  if (ImGui::Button("Refresh")) {
    if (auto windows = capture::WindowCapture::listWindows()) {
      windows_ = std::move(*windows);
      pickerError_.clear();
    } else {
      pickerError_ = windows.error().message;
    }
  }
  ImGui::SameLine();
  if (ImGui::Button("Stop capture")) {
    capture_.stop();
    commands_.push(control::SetTarget{.target = std::nullopt});
  }
  if (!pickerError_.empty()) {
    ImGui::TextColored(ImVec4(1.0F, 0.4F, 0.4F, 1.0F), "%s", pickerError_.c_str());
  }

  const auto current = capture_.target();
  if (current) {
    ImGui::Text("Capturing: %s - %s", current->appName.c_str(), current->title.c_str());
  } else {
    ImGui::TextDisabled("Not capturing");
  }
  ImGui::Separator();

  for (const auto& window : windows_) {
    const std::string label =
        std::format("{} - {}##{}", window.appName, window.title, window.windowId);
    if (ImGui::Selectable(label.c_str())) {
      if (auto started = capture_.start(window, frames_); started) {
        commands_.push(control::SetTarget{.target = window});
        pickerError_.clear();
      } else {
        pickerError_ = started.error().message;
      }
    }
  }

  ImGui::End();
}

void UiApp::drawStreamWindow() {
  ImGui::Begin("Stream");

  if (streamTexture_ == nullptr) {
    ImGui::TextDisabled("Pick a window to start streaming.");
    ImGui::End();
    return;
  }

  const ImVec2 available = ImGui::GetContentRegionAvail();
  const float frameWidth = static_cast<float>(textureWidth_);
  const float frameHeight = static_cast<float>(textureHeight_);
  const float scale = std::min(
      available.x / frameWidth, available.y / frameHeight
  );
  const ImVec2 drawSize{frameWidth * scale, frameHeight * scale};
  const ImVec2 origin = ImGui::GetCursorScreenPos();

  ImGui::Image(
      reinterpret_cast<ImTextureID>(streamTexture_), drawSize
  );

  // Overlays describe a specific frame; when the script stops publishing
  // (paused, unloaded, erroring) old commands must fade out rather than
  // stick to newer frames forever. The proper frame-exact pairing arrives
  // with the vision milestone's AnalyzedFrame.
  constexpr std::uint64_t kOverlayMaxLagFrames = 30;
  if (overlay_.frameSeq + kOverlayMaxLagFrames < seenFrameSeq_) {
    ImGui::End();
    return;
  }

  ImDrawList* drawList = ImGui::GetWindowDrawList();
  const auto toScreen = [&](const float x, const float y) {
    return ImVec2{origin.x + x * scale, origin.y + y * scale};
  };
  for (const auto& command : overlay_.commands) {
    std::visit(
        [&](const auto& shape) {
          using Shape = std::decay_t<decltype(shape)>;
          if constexpr (std::is_same_v<Shape, pipeline::OverlayBox>) {
            const ImVec2 min = toScreen(shape.x, shape.y);
            const ImVec2 max =
                toScreen(shape.x + shape.width, shape.y + shape.height);
            if (shape.filled) {
              drawList->AddRectFilled(min, max, toImColor(shape.color));
            } else {
              drawList->AddRect(
                  min, max, toImColor(shape.color), 0.0F, 0, shape.thickness
              );
            }
          } else if constexpr (std::is_same_v<Shape, pipeline::OverlayLine>) {
            drawList->AddLine(
                toScreen(shape.x1, shape.y1), toScreen(shape.x2, shape.y2),
                toImColor(shape.color), shape.thickness
            );
          } else if constexpr (std::is_same_v<Shape, pipeline::OverlayText>) {
            drawList->AddText(
                toScreen(shape.x, shape.y), toImColor(shape.color),
                shape.text.c_str()
            );
          }
        },
        command
    );
  }

  ImGui::End();
}

void UiApp::drawScriptWindow() {
  ImGui::Begin("Script");

  ImGui::InputText("Path", scriptPathBuffer_, sizeof(scriptPathBuffer_));
  if (ImGui::Button("Load")) {
    commands_.push(control::LoadScript{.path = scriptPathBuffer_});
    loadedScriptMtime_ = fileMtime(scriptPathBuffer_);
  }
  ImGui::SameLine();
  if (ImGui::Button("Reload") && !statsSnapshot_.scriptPath.empty()) {
    commands_.push(control::LoadScript{.path = statsSnapshot_.scriptPath});
    loadedScriptMtime_ = fileMtime(statsSnapshot_.scriptPath);
  }
  ImGui::SameLine();
  if (ImGui::Button("Unload")) {
    commands_.push(control::UnloadScript{});
  }

  bool paused = statsSnapshot_.scriptPaused;
  if (ImGui::Checkbox("Paused", &paused)) {
    commands_.push(control::SetPaused{.paused = paused});
  }
  ImGui::SameLine();
  if (ImGui::Checkbox("Require focus for input", &requireFocus_)) {
    commands_.push(control::SetRequireFocus{.require = requireFocus_});
  }
  ImGui::SameLine();
  ImGui::Checkbox("Auto-reload on change", &autoReload_);

  ImGui::Separator();
  if (statsSnapshot_.scriptLoaded) {
    ImGui::Text("Loaded: %s", statsSnapshot_.scriptPath.c_str());
  } else {
    ImGui::TextDisabled("No script loaded");
  }
  if (!statsSnapshot_.lastScriptError.empty()) {
    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0F, 0.4F, 0.4F, 1.0F));
    ImGui::TextWrapped("%s", statsSnapshot_.lastScriptError.c_str());
    ImGui::PopStyleColor();
  }

  ImGui::End();
}

void UiApp::pollAutoReload() {
  if (!autoReload_ || !statsSnapshot_.scriptLoaded) return;

  const double now = ImGui::GetTime();
  if (now - lastReloadCheckAt_ < 1.0) return;
  lastReloadCheckAt_ = now;

  const auto mtime = fileMtime(statsSnapshot_.scriptPath);
  if (mtime != 0 && mtime != loadedScriptMtime_) {
    loadedScriptMtime_ = mtime;
    commands_.push(control::LoadScript{.path = statsSnapshot_.scriptPath});
    spdlog::info("[UI] script changed on disk, reloading");
  }
}

void UiApp::drawStatsWindow() {
  ImGui::Begin("Stats");

  ImGui::Text("UI: %.1f fps", ImGui::GetIO().Framerate);
  ImGui::Text("Capture: %s", capture_.isRunning() ? "running" : "stopped");
  ImGui::Text("Frame seq: %llu", statsSnapshot_.frameSeq);
  ImGui::Text("Dropped frames: %llu", statsSnapshot_.droppedFrames);
  ImGui::Text("Frame age at tick: %.1f ms", statsSnapshot_.frameAgeAtTickMs);
  ImGui::Text("Tick duration: %.2f ms", statsSnapshot_.tickDurationMs);
  ImGui::Text(
      "Target focused: %s", statsSnapshot_.targetFocused ? "yes" : "no"
  );

  ImGui::End();
}

void UiApp::drawLogWindow() {
  ImGui::Begin("Logs");

  if (const auto version = logSink_.version(); version != seenLogVersion_) {
    seenLogVersion_ = version;
    logEntries_ = logSink_.snapshot();
  }

  ImGui::BeginChild("log-scroll");
  for (const auto& entry : logEntries_) {
    ImVec4 color{0.85F, 0.85F, 0.85F, 1.0F};
    if (entry.level == spdlog::level::warn) color = {1.0F, 0.8F, 0.3F, 1.0F};
    if (entry.level >= spdlog::level::err) color = {1.0F, 0.4F, 0.4F, 1.0F};
    ImGui::PushStyleColor(ImGuiCol_Text, color);
    ImGui::TextUnformatted(entry.text.c_str());
    ImGui::PopStyleColor();
  }
  if (ImGui::GetScrollY() >= ImGui::GetScrollMaxY() - 1.0F) {
    ImGui::SetScrollHereY(1.0F);
  }
  ImGui::EndChild();

  ImGui::End();
}

}  // namespace ae::ui
