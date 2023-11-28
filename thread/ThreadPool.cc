#include <webserver/logger/Logger.h>
#include <webserver/thread/ThreadPool.h>

#include <chrono>

using namespace webserver;

ThreadPool::ThreadPool(size_t init_thread_num, size_t max_thread_num,
                       size_t max_queue_size, size_t time_wait)
    : running_(false),
      thread_num_(init_thread_num),
      max_thread_num_(max_thread_num),
      max_queue_size_(max_queue_size),
      time_wait_(time_wait) {
  if (init_thread_num > max_thread_num) {
    LOG_FATAL << "init_thread_num > max_thread_num";
  }
  if (max_queue_size == 0) {
    LOG_FATAL << "max_queue_size == 0";
  }
  threads_.reserve(max_thread_num);
}

ThreadPool::~ThreadPool() {
  if (running_) {
    stop();
  }
}

void ThreadPool::start() {
  if (!threads_.empty() || running_) {
    return;
  }
  running_ = true;

  for (auto i = 0; i < thread_num_; ++i) {
    std::thread t(std::bind(&ThreadPool::runInThread, this));
    threads_.push_back(std::move(t));
  }
}

void ThreadPool::stop() {
  {
    std::lock_guard<std::mutex> lock(mutex_);
    running_ = false;
    not_empty_.notify_all();
  }
  for (auto& t : threads_) {
    if (t.joinable()) {
      t.join();
    }
  }
}

void ThreadPool::run(const Task& task) {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!running_) {
    return;
  }

  // if ((tasks_.size() >= max_queue_size_) && thread_num_ < max_thread_num_) {
  //   threads_.push_back(
  //       std::move(std::thread(std::bind(&ThreadPool::runInThread, this))));
  //   ++thread_num_;
  // }
  if (tasks_.size() >= max_queue_size_) {
    not_full_.wait(lock, [this] { return (tasks_.size() < max_queue_size_); });
  }
  tasks_.push(task);
  not_empty_.notify_all();
}

void ThreadPool::runInThread() {
  try {
    while (running_) {
      Task task(take());
      if (task) {
        task();
      } else if (task == nullptr) {
        return;
      }
    }
  } catch (std::exception e) {
    abort();
  }
}

ThreadPool::Task ThreadPool::take() {
  std::unique_lock<std::mutex> lock(mutex_);
  if (!not_empty_.wait_for(lock, std::chrono::seconds(time_wait_),
                           [this] { return !tasks_.empty() && running_; })) {
    return nullptr;
  }

  Task task;
  if (!tasks_.empty()) {
    task = tasks_.front();
    tasks_.pop();
    if (max_queue_size_ > 0) {
      not_full_.notify_all();
    }
  }
  return task;
}
