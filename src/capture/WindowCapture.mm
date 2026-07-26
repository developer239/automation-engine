#include "capture/WindowCapture.h"

#import <CoreMedia/CoreMedia.h>
#import <ScreenCaptureKit/ScreenCaptureKit.h>

#include <atomic>
#include <chrono>
#include <memory>
#include <mutex>

#include <spdlog/spdlog.h>

namespace ae::capture {

// Shared between WindowCapture and the ObjC stream output. The delegate keeps
// its own shared_ptr, so late callbacks after stop() (the sample queue is not
// drained synchronously) find a deactivated channel instead of dangling
// pointers. The mailbox itself must outlive the capture - the composition
// root guarantees that.
struct StreamChannel {
  FrameMailbox* mailbox = nullptr;
  std::uint32_t windowId = 0;
  std::atomic<bool> active{false};
};

}  // namespace ae::capture

// Receives frames on the stream's sample handler queue. The handler only
// retains the pixel buffer and swaps it into the mailbox - all conversion,
// scripting, and rendering happens on other threads.
@interface AEStreamOutput : NSObject <SCStreamOutput, SCStreamDelegate> {
 @public
  std::shared_ptr<ae::capture::StreamChannel> channel;
}
@end

@implementation AEStreamOutput

- (void)stream:(SCStream*)stream
    didOutputSampleBuffer:(CMSampleBufferRef)sampleBuffer
                   ofType:(SCStreamOutputType)type {
  const auto chan = self->channel;
  if (chan == nullptr || !chan->active.load()) return;
  if (type != SCStreamOutputTypeScreen) return;
  if (!CMSampleBufferIsValid(sampleBuffer)) return;

  // ScreenCaptureKit delivers idle/blank samples too; only complete frames
  // carry new content. The same attachment dictionary carries the scale and
  // content-rect metadata needed to map capture pixels back to window points.
  double scaleFactor = 0.0;
  double contentScale = 0.0;
  CGRect contentRect = CGRectZero;
  CFArrayRef attachments =
      CMSampleBufferGetSampleAttachmentsArray(sampleBuffer, false);
  if (attachments != nullptr && CFArrayGetCount(attachments) > 0) {
    const auto* attachment = static_cast<CFDictionaryRef>(
        CFArrayGetValueAtIndex(attachments, 0)
    );
    const auto* statusNumber = static_cast<CFNumberRef>(CFDictionaryGetValue(
        attachment, (__bridge CFStringRef)SCStreamFrameInfoStatus
    ));
    if (statusNumber != nullptr) {
      std::int32_t status = 0;
      CFNumberGetValue(statusNumber, kCFNumberSInt32Type, &status);
      if (status != SCFrameStatusComplete) return;
    }
    const auto* scaleNumber = static_cast<CFNumberRef>(CFDictionaryGetValue(
        attachment, (__bridge CFStringRef)SCStreamFrameInfoScaleFactor
    ));
    if (scaleNumber != nullptr) {
      CFNumberGetValue(scaleNumber, kCFNumberFloat64Type, &scaleFactor);
    }
    const auto* contentScaleNumber =
        static_cast<CFNumberRef>(CFDictionaryGetValue(
            attachment, (__bridge CFStringRef)SCStreamFrameInfoContentScale
        ));
    if (contentScaleNumber != nullptr) {
      CFNumberGetValue(
          contentScaleNumber, kCFNumberFloat64Type, &contentScale
      );
    }
    const auto* contentRectDict =
        static_cast<CFDictionaryRef>(CFDictionaryGetValue(
            attachment, (__bridge CFStringRef)SCStreamFrameInfoContentRect
        ));
    if (contentRectDict != nullptr) {
      CGRectMakeWithDictionaryRepresentation(contentRectDict, &contentRect);
    }
  }

  CVPixelBufferRef pixels = CMSampleBufferGetImageBuffer(sampleBuffer);
  if (pixels == nullptr) return;

  // Stamp the frame with when it was presented, not when this callback ran -
  // a sample delayed inside ScreenCaptureKit must not look fresh to the
  // stale-frame input gate. Presentation time and the host clock share the
  // same time base, so the age subtraction is exact.
  auto capturedAt = std::chrono::steady_clock::now();
  const CMTime presentation =
      CMSampleBufferGetPresentationTimeStamp(sampleBuffer);
  if (CMTIME_IS_VALID(presentation)) {
    const CMTime hostNow = CMClockGetTime(CMClockGetHostTimeClock());
    const double ageSeconds =
        CMTimeGetSeconds(CMTimeSubtract(hostNow, presentation));
    capturedAt -= std::chrono::duration_cast<std::chrono::steady_clock::duration>(
        std::chrono::duration<double>(ageSeconds)
    );
  }

  chan->mailbox->publish(ae::capture::CapturedFrame{
      .pixels = ae::capture::FrameHandle(pixels),
      .windowId = chan->windowId,
      .width = static_cast<std::int32_t>(CVPixelBufferGetWidth(pixels)),
      .height = static_cast<std::int32_t>(CVPixelBufferGetHeight(pixels)),
      .scaleFactor = scaleFactor,
      .contentScale = contentScale,
      .contentX = contentRect.origin.x,
      .contentY = contentRect.origin.y,
      .contentWidth = contentRect.size.width,
      .contentHeight = contentRect.size.height,
      .capturedAt = capturedAt,
  });
}

- (void)stream:(SCStream*)stream didStopWithError:(NSError*)error {
  spdlog::error(
      "[Capture] stream stopped: {}", error.localizedDescription.UTF8String
  );
  const auto chan = self->channel;
  if (chan != nullptr) chan->active.store(false);
}

@end

namespace ae::capture {

struct WindowCapture::Impl {
  SCStream* stream = nil;
  AEStreamOutput* output = nil;
  dispatch_queue_t queue = nil;
  std::shared_ptr<StreamChannel> channel;
  std::optional<WindowInfo> target;
  mutable std::mutex targetMutex;
};

namespace {

std::expected<SCShareableContent*, Error> fetchShareableContent() {
  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
  __block SCShareableContent* content = nil;
  __block NSError* error = nil;
  [SCShareableContent
      getShareableContentExcludingDesktopWindows:YES
                             onScreenWindowsOnly:YES
                               completionHandler:^(
                                   SCShareableContent* result, NSError* err
                               ) {
                                 content = result;
                                 error = err;
                                 dispatch_semaphore_signal(semaphore);
                               }];

  const auto deadline =
      dispatch_time(DISPATCH_TIME_NOW, 5 * static_cast<int64_t>(NSEC_PER_SEC));
  if (dispatch_semaphore_wait(semaphore, deadline) != 0) {
    return std::unexpected(
        Error::captureFailed("timed out enumerating shareable content")
    );
  }
  if (error != nil) {
    if (error.code == SCStreamErrorUserDeclined) {
      return std::unexpected(Error::permissionDenied(
          "Screen Recording permission is not granted (System Settings > "
          "Privacy & Security > Screen Recording)"
      ));
    }
    return std::unexpected(
        Error::captureFailed(error.localizedDescription.UTF8String)
    );
  }
  return content;
}

WindowInfo toWindowInfo(SCWindow* window) {
  return WindowInfo{
      .windowId = window.windowID,
      .ownerPid = window.owningApplication != nil
                      ? window.owningApplication.processID
                      : 0,
      .appName = window.owningApplication != nil
                     ? window.owningApplication.applicationName.UTF8String
                     : "",
      .title = window.title != nil ? window.title.UTF8String : "",
      .frame =
          robot::LogicalRect{
              .origin = {.x = window.frame.origin.x, .y = window.frame.origin.y},
              .size =
                  {.width = window.frame.size.width,
                   .height = window.frame.size.height},
          },
  };
}

}  // namespace

WindowCapture::WindowCapture() : impl_(std::make_unique<Impl>()) {}

WindowCapture::~WindowCapture() { stop(); }

std::expected<std::vector<WindowInfo>, Error> WindowCapture::listWindows() {
  auto contentResult = fetchShareableContent();
  if (!contentResult) return std::unexpected(contentResult.error());

  std::vector<WindowInfo> windows;
  for (SCWindow* window in (*contentResult).windows) {
    if (window.windowLayer != 0) continue;
    if (window.title == nil || window.title.length == 0) continue;
    if (window.frame.size.width < 64 || window.frame.size.height < 64) continue;
    windows.push_back(toWindowInfo(window));
  }

  return windows;
}

std::expected<void, Error> WindowCapture::start(
    const WindowInfo& target, FrameMailbox& mailbox, const double maxFps
) {
  stop();

  auto contentResult = fetchShareableContent();
  if (!contentResult) return std::unexpected(contentResult.error());

  SCWindow* scWindow = nil;
  for (SCWindow* window in (*contentResult).windows) {
    if (window.windowID == target.windowId) {
      scWindow = window;
      break;
    }
  }
  if (scWindow == nil) {
    return std::unexpected(Error::windowNotFound(target.windowId));
  }

  SCContentFilter* filter =
      [[SCContentFilter alloc] initWithDesktopIndependentWindow:scWindow];
  SCStreamConfiguration* config = [[SCStreamConfiguration alloc] init];
  const float scale = filter.pointPixelScale;
  config.width =
      static_cast<size_t>(filter.contentRect.size.width * scale);
  config.height =
      static_cast<size_t>(filter.contentRect.size.height * scale);
  config.pixelFormat = kCVPixelFormatType_32BGRA;
  config.minimumFrameInterval =
      CMTimeMake(1, static_cast<std::int32_t>(maxFps));
  config.queueDepth = 3;
  config.showsCursor = YES;

  auto channel = std::make_shared<StreamChannel>();
  channel->mailbox = &mailbox;
  channel->windowId = target.windowId;
  channel->active.store(true);

  impl_->output = [[AEStreamOutput alloc] init];
  impl_->output->channel = channel;
  impl_->queue =
      dispatch_queue_create("ae.capture.frames", DISPATCH_QUEUE_SERIAL);
  impl_->stream = [[SCStream alloc] initWithFilter:filter
                                     configuration:config
                                          delegate:impl_->output];

  NSError* addError = nil;
  [impl_->stream addStreamOutput:impl_->output
                            type:SCStreamOutputTypeScreen
              sampleHandlerQueue:impl_->queue
                           error:&addError];
  if (addError != nil) {
    impl_->stream = nil;
    impl_->output = nil;
    impl_->queue = nil;
    return std::unexpected(
        Error::captureFailed(addError.localizedDescription.UTF8String)
    );
  }

  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
  __block NSError* startError = nil;
  [impl_->stream startCaptureWithCompletionHandler:^(NSError* err) {
    startError = err;
    dispatch_semaphore_signal(semaphore);
  }];
  const auto deadline =
      dispatch_time(DISPATCH_TIME_NOW, 5 * static_cast<int64_t>(NSEC_PER_SEC));
  if (dispatch_semaphore_wait(semaphore, deadline) != 0) {
    // Startup may still complete later; deactivate the channel and issue a
    // fire-and-forget stop so a late success never publishes frames.
    // startError is deliberately not read here - the completion handler may
    // still be about to write it.
    channel->active.store(false);
    [impl_->stream stopCaptureWithCompletionHandler:^(NSError*) {
    }];
    impl_->stream = nil;
    impl_->output = nil;
    impl_->queue = nil;
    return std::unexpected(Error::captureFailed("timed out starting capture"));
  }
  if (startError != nil) {
    channel->active.store(false);
    impl_->stream = nil;
    impl_->output = nil;
    impl_->queue = nil;
    return std::unexpected(
        Error::captureFailed(startError.localizedDescription.UTF8String)
    );
  }

  impl_->channel = channel;
  {
    std::scoped_lock lock(impl_->targetMutex);
    impl_->target = target;
  }

  spdlog::info(
      "[Capture] streaming '{}' ({}) at {}x{} px", target.title,
      target.appName, config.width, config.height
  );
  return {};
}

void WindowCapture::stop() {
  if (impl_->channel != nullptr) impl_->channel->active.store(false);
  if (impl_->stream == nil) return;

  dispatch_semaphore_t semaphore = dispatch_semaphore_create(0);
  [impl_->stream stopCaptureWithCompletionHandler:^(NSError* err) {
    (void)err;  // already stopping; nothing to do differently on error
    dispatch_semaphore_signal(semaphore);
  }];
  const auto deadline =
      dispatch_time(DISPATCH_TIME_NOW, 2 * static_cast<int64_t>(NSEC_PER_SEC));
  dispatch_semaphore_wait(semaphore, deadline);

  impl_->stream = nil;
  impl_->output = nil;
  impl_->queue = nil;
  impl_->channel.reset();
  {
    std::scoped_lock lock(impl_->targetMutex);
    impl_->target.reset();
  }

  spdlog::info("[Capture] stopped");
}

bool WindowCapture::isRunning() const noexcept {
  return impl_->channel != nullptr && impl_->channel->active.load();
}

std::optional<WindowInfo> WindowCapture::target() const {
  std::scoped_lock lock(impl_->targetMutex);
  return impl_->target;
}

}  // namespace ae::capture
