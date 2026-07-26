// Type definitions for the automation-engine Lua API.
//
// Write scripts in TypeScript and compile them with TypeScriptToLua
// (https://typescripttolua.github.io/). Multi-value returns use
// LuaMultiReturn from @typescript-to-lua/language-extensions.
//
// Treat this file like a header: no npm package, just reference it from your
// script project's tsconfig.

/** @noSelfInFile */

/** Per-frame view passed to main.onFrame. All coordinates and sizes are in
 * capture pixels. */
interface Frame {
  /** Monotonic frame sequence number; increases by more than 1 when frames
   * were dropped. */
  seq: number;
  width: number;
  height: number;
  /** Capture-to-tick-start latency in milliseconds. */
  ageMs: number;
  /** Capture-timestamp gap to the previously processed frame. 0 on the first
   * frame. */
  deltaMs: number;
  /** Whether the captured window's application was frontmost when the tick
   * began. Input calls re-check focus at injection time. */
  isFocused: boolean;
}

interface ApiError {
  /** Stable machine-readable reason: outside_tick, no_target, stale_frame,
   * target_not_focused, out_of_bounds, missing_scale, window_gone,
   * unknown_key, unknown_button, invalid_overlay, activation_failed,
   * input_failed. */
  code: string;
  message: string;
}

/** true on success, or nil plus a structured error. Never a silent no-op. */
type ApiResult = LuaMultiReturn<[true] | [undefined, ApiError]>;

/** Components 0-255; every component optional, defaults to opaque white. */
interface Color {
  r?: number;
  g?: number;
  b?: number;
  a?: number;
}

type MouseButton = 'left' | 'right' | 'middle';

/** Per-frame overlay drawing in capture pixel space. Commands are cleared
 * every tick and only published when onFrame completes without an error. */
declare const overlay: {
  box: (spec: {
    x: number;
    y: number;
    w: number;
    h: number;
    color?: Color;
    thickness?: number;
    filled?: boolean;
  }) => ApiResult;
  line: (spec: {
    x1: number;
    y1: number;
    x2: number;
    y2: number;
    color?: Color;
    thickness?: number;
  }) => ApiResult;
  text: (spec: { x: number; y: number; text: string; color?: Color }) => ApiResult;
};

/** Keyboard/mouse injection via robot-cpp. Coordinates are capture pixels;
 * the engine converts to desktop coordinates through the frame's scale
 * metadata and the window's current bounds. Refused (with an ApiError) when
 * the target window is not frontmost, the frame is stale, or coordinates are
 * out of bounds. */
declare const input: {
  /** Instant cursor warp. */
  move: (x: number, y: number) => ApiResult;
  /** Interpolated cursor move (~300 ms). */
  moveSmooth: (x: number, y: number) => ApiResult;
  /** Move to (x, y) and click. Button defaults to 'left'. */
  click: (x: number, y: number, button?: MouseButton) => ApiResult;
  /** Hold a mouse button at the current position. Idempotent while held. */
  pressButton: (button: MouseButton) => ApiResult;
  releaseButton: (button: MouseButton) => ApiResult;
  /** Press and release a physical key (positional, layout-independent). */
  tap: (key: string) => ApiResult;
  /** Hold a physical key. Idempotent while held. Held keys are auto-released
   * on pause, reload, script error, focus loss, and shutdown. */
  press: (key: string) => ApiResult;
  release: (key: string) => ApiResult;
  /** Inject Unicode text (the only correct way to produce characters). */
  typeText: (text: string) => ApiResult;
};

declare const target: {
  /** Bring the captured window's application to the foreground. */
  focus: () => ApiResult;
};

/** Messages land in the Logs panel and stdout. */
declare const log: {
  info: (message: string) => void;
  warn: (message: string) => void;
  error: (message: string) => void;
};

/** Validated key-name constants for input.tap/press/release. Using these
 * instead of raw strings makes typos fail at compile time. */
declare const keys: {
  A: 'a'; B: 'b'; C: 'c'; D: 'd'; E: 'e'; F: 'f'; G: 'g'; H: 'h'; I: 'i';
  J: 'j'; K: 'k'; L: 'l'; M: 'm'; N: 'n'; O: 'o'; P: 'p'; Q: 'q'; R: 'r';
  S: 's'; T: 't'; U: 'u'; V: 'v'; W: 'w'; X: 'x'; Y: 'y'; Z: 'z';
  '0': '0'; '1': '1'; '2': '2'; '3': '3'; '4': '4'; '5': '5'; '6': '6';
  '7': '7'; '8': '8'; '9': '9';
  ENTER: 'enter'; ESCAPE: 'escape'; BACKSPACE: 'backspace'; TAB: 'tab';
  SPACE: 'space'; UP: 'up'; DOWN: 'down'; LEFT: 'left'; RIGHT: 'right';
  SHIFT: 'shift'; CTRL: 'ctrl'; ALT: 'alt'; CMD: 'cmd';
  F1: 'f1'; F2: 'f2'; F3: 'f3'; F4: 'f4'; F5: 'f5'; F6: 'f6'; F7: 'f7';
  F8: 'f8'; F9: 'f9'; F10: 'f10'; F11: 'f11'; F12: 'f12';
};

/** The script contract: define a global `main` table. onFrame runs once per
 * captured frame on the control thread. A tick longer than 100 ms is aborted;
 * a tick that errors releases all held input and pauses the script. */
declare var main: {
  onFrame: (frame: Frame) => void;
};
