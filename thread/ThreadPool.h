#ifndef WEB_SERVER_BASE_THREADPOOL_H
#define WEB_SERVER_BASE_THREADPOOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "webserver/base/noncopyable.h"

namespace webserver {

class ThreadPool : public noncopyable {
 public:
  using Task = std::function<void()>;

  ThreadPool(size_t init_thread_num, size_t max_thread_num,
             size_t max_queue_size, size_t time_wait);

  ~ThreadPool();

  void start();
  void stop();

  void run(const Task& task);

  void set_thread_num(int num) { thread_num_ = num; }

 private:
  void runInThread();

  Task take();

  // bool isFull();

  // member variables:

  std::atomic_bool running_;

  std::vector<std::thread> threads_;
  std::atomic_int thread_num_;
  const size_t max_thread_num_;
  const size_t time_wait_;

  std::queue<Task> tasks_;
  const size_t max_queue_size_;
  std::mutex mutex_;  // this mutex is used to protect `tasks_`
  std::condition_variable not_empty_;
  std::condition_variable not_full_;
};

}  // namespace webserver

#endif  // WEB_SERVER_BASE_THREADPOOL_H