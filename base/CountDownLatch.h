#ifndef WEBBER_BASE_COUNTDOWNLATCH_H
#define WEBBER_BASE_COUNTDOWNLATCH_H

#include <condition_variable>
#include <mutex>

namespace webserver {

class CountDownLatch {
 public:
  CountDownLatch(int count) : mutex_(), condition_(), count_(count) {}

  void wait() {
    std::unique_lock<std::mutex> lock(mutex_);
    condition_.wait(lock, [this] { return count_ == 0; });
  }

  void countDown() {
    std::lock_guard<std::mutex> lock(mutex_);
    --count_;
    if (count_ == 0) {
      condition_.notify_all();
    }
  }

  int getCount() {
    std::lock_guard<std::mutex> lock(mutex_);
    return count_;
  }

 private:
  std::mutex mutex_;
  std::condition_variable condition_;
  int count_;
};

}  // namespace server

#endif  // WEBBER_BASE_COUNTDOWNLATCH_H