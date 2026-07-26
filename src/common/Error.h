#pragma once

#include <cstdlib>
#include <format>
#include <string>
#include <string_view>

namespace ae {

enum class ErrorCode {
  CaptureFailed,
  WindowNotFound,
  PermissionDenied,
  ScriptLoadFailed,
  ScriptRuntimeError,
  UiInitFailed,
};

constexpr std::string_view toString(const ErrorCode code) {
  switch (code) {
    case ErrorCode::CaptureFailed: return "CaptureFailed";
    case ErrorCode::WindowNotFound: return "WindowNotFound";
    case ErrorCode::PermissionDenied: return "PermissionDenied";
    case ErrorCode::ScriptLoadFailed: return "ScriptLoadFailed";
    case ErrorCode::ScriptRuntimeError: return "ScriptRuntimeError";
    case ErrorCode::UiInitFailed: return "UiInitFailed";
  }
  std::abort();
}

struct Error {
  ErrorCode code;
  std::string message;

  static Error captureFailed(std::string_view details) {
    return {ErrorCode::CaptureFailed, std::format("Capture failed: {}", details)};
  }

  static Error windowNotFound(std::uint32_t windowId) {
    return {ErrorCode::WindowNotFound,
            std::format("Window {} no longer exists on screen", windowId)};
  }

  static Error permissionDenied(std::string_view details) {
    return {ErrorCode::PermissionDenied,
            std::format("Permission denied: {}", details)};
  }

  static Error scriptLoadFailed(std::string_view path, std::string_view details) {
    return {ErrorCode::ScriptLoadFailed,
            std::format("Failed to load script '{}': {}", path, details)};
  }

  static Error scriptRuntimeError(std::string_view details) {
    return {ErrorCode::ScriptRuntimeError,
            std::format("Script error: {}", details)};
  }

  static Error uiInitFailed(std::string_view details) {
    return {ErrorCode::UiInitFailed, std::format("UI init failed: {}", details)};
  }
};

}  // namespace ae
