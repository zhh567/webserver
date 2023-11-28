#include <webserver/thread/EventLoopThreadPool.h>

using namespace webserver;

EventLoopThreadPool::EventLoopThreadPool(int thread_num, EventLoop* baseloop)
    : baseloop_(baseloop),
      threads_num_(thread_num),
      threads_(),
      loops_(),
      next_(0),
      started_(false) {}
EventLoopThreadPool::~EventLoopThreadPool() {}

/// @brief Start thread pool. If thread num equal zero, current thread which
/// call this function run `init_callback`. Otherwise, each sub-thread run it.
/// @param init_callback
void EventLoopThreadPool::start(
    EventLoopThreadInitCallback& init_callback) {
  started_ = true;

  for (int i = 0; i < threads_num_; i++) {
    EventLoopThread* t = new EventLoopThread(init_callback);
    threads_.push_back(std::unique_ptr<EventLoopThread>(t));
    loops_.push_back(t->start_loop());
  }
  if (threads_num_ == 0 && init_callback != nullptr) {
    init_callback(baseloop_);
  }
}

EventLoop* EventLoopThreadPool::get_next_loop() {
  if (loops_.empty()) {
    return baseloop_;
  }
  EventLoop* loop = loops_[next_];
  next_ = (next_ + 1) % loops_.size();
  return loop;
}