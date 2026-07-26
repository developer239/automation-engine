#pragma once

#include <cstdint>

namespace ae::capture {

// Brings the application owning pid to the foreground. Returns false when the
// application is no longer running or activation was refused by the OS.
bool activateApplication(std::int32_t pid);

}  // namespace ae::capture
