-- Minimal example: draws a crosshair overlay and reports frame stats.
-- Load it from the Script window after picking a window to capture.

local tickCount = 0

main = {
  onFrame = function(frame)
    tickCount = tickCount + 1

    local cx = frame.width / 2
    local cy = frame.height / 2
    overlay.line({ x1 = cx - 40, y1 = cy, x2 = cx + 40, y2 = cy, color = { r = 255, g = 64, b = 64 } })
    overlay.line({ x1 = cx, y1 = cy - 40, x2 = cx, y2 = cy + 40, color = { r = 255, g = 64, b = 64 } })
    overlay.box({
      x = cx - 60,
      y = cy - 60,
      w = 120,
      h = 120,
      color = { r = 64, g = 255, b = 64, a = 200 },
      thickness = 2,
    })
    overlay.text({
      x = 12,
      y = 12,
      text = string.format("tick %d | age %.1fms | focused: %s", tickCount, frame.ageMs, tostring(frame.isFocused)),
      color = { r = 255, g = 255, b = 0 },
    })

    -- Input example (only runs while the target window is frontmost):
    -- local ok, err = input.tap(keys.SPACE)
    -- if not ok then log.warn(err.code .. ": " .. err.message) end
  end,
}
