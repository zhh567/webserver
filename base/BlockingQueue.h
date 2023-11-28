#ifndef WEB_SERVER_BASE_BLOCKINGQUEUE_H
#define WEB_SERVER_BASE_BLOCKINGQUEUE_H

#include <webserver/base/noncopyable.h>

#include <condition_variable>
#include <mutex>
#include <queue>
#include <tuple>

namespace webserver {

template <typename T>
class BlockingQueue : noncopyable {
 public:
  BlockingQueue() : mutex_(), not_empty_(), queue_() {}
  ~BlockingQueue() {}

  void put(const T& x) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(x);
    not_empty_.notify_all();
  }

  void put(T&& x) {
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(std::move(x));
    not_empty_.notify_all();
  }

  T take() {
    std::unique_lock<std::mutex> lock(mutex_);
    not_empty_.wait(lock, [this] {
      return !queue_.empty();
    });  // equal to `while(queue_.empty()) { not_empty_.wait(lock); }`

    T front(std::move(queue_.front()));
    queue_.pop();
    return front;
  }

  std::tuple<T, bool> take(uint32_t duration) {
    std::unique_lock<std::mutex> lock(mutex_);
    bool ret = not_empty_.wait_for(lock, std::chrono::milliseconds(duration),
                                   [this] { return !queue_.empty(); });
    if (!ret) {
      return std::make_tuple(T(), true);
    }

    T front(std::move(queue_.front()));
    queue_.pop();
    return std::make_tuple(front, false);
  }

  size_t size() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.size();
  }

 private:
  mutable std::mutex mutex_;
  std::condition_variable not_empty_;
  std::queue<T> queue_;
};

}  // namespace server

#endif  // WEB_SERVER_BASE_BLOCKINGQUEUE_H
