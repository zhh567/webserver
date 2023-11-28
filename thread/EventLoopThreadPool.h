#ifndef WEBSERVER_THREAD_EVENTLOOPTHREADPOOL_H
#define WEBSERVER_THREAD_EVENTLOOPTHREADPOOL_H

#include <webserver/thread/EventLoopThread.h>

#include <memory>
#include <vector>

namespace webserver {

class EventLoopThreadPool {
 public:
  EventLoopThreadPool(int thread_num, EventLoop* baseloop);
  ~EventLoopThreadPool();

  void set_thread_num(int num_threads) { threads_num_ = num_threads; }
  EventLoop* get_next_loop();
  void start(EventLoopThreadInitCallback& init_callback);

 private:
  EventLoop* baseloop_;  // in main thread, attributed to Acceptor

  int threads_num_;
  std::vector<std::unique_ptr<EventLoopThread>> threads_;
  std::vector<EventLoop*> loops_;

  int next_;

  bool started_;
};  // class EventLoopThreadPool

}  // namespace webserver

#endif  // WEBSERVER_THREAD_EVENTLOOPTHREADPOOL_H
