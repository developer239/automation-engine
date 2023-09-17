#pragma once

#include <chrono>
#include <future>
class ParallelTaskManager {
 public:
  template<typename Func>
  void Execute(Func&& f) {
    // Only execute if no task is in progress and
    // the previous task (if any) has completed.
    if (!isTaskInProgress &&
        (futureTask.valid() ? futureTask.wait_for(std::chrono::seconds(0)) == std::future_status::ready : true)) {
      isTaskInProgress = true;
      futureTask = std::async(std::launch::async, [this, &f]() {
        auto start = std::chrono::high_resolution_clock::now();
        f();
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Task took: " << duration.count() << "ms" << std::endl;
        isTaskInProgress = false;
      });
    }
  }

  // Returns true if there's a task in progress
  bool IsTaskInProgress() const { return isTaskInProgress; }

 private:
  std::future<void> futureTask;
  std::atomic<bool> isTaskInProgress = false;
};
