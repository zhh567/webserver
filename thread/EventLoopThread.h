#ifndef WEBSERVER_THREAD_EVENTLOOPTHREAD_H
#define WEBSERVER_THREAD_EVENTLOOPTHREAD_H

#include <webserver/io/EventLoop.h>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <thread>

namespace webserver {

class EventLoopThread {
 public:
  EventLoopThread(EventLoopThreadInitCallback& init_callback);
  ~EventLoopThread();

  EventLoop* start_loop();

 private:
  void thread_func();
  EventLoop* loop_;
  std::atomic_bool existed_;

  std::unique_ptr<std::thread> thread_;
  EventLoopThreadInitCallback init_callback_;

  // create `EventLoop` object in new thread, wait set loop_ equal new
  // `EventLoop`.
  std::mutex mutex_set_loop_;
  std::condition_variable cond_wait_set_loop_;
};

}  // namespace webserver

#endif  // WEBSERVER_THREAD_EVENTLOOPTHREAD_H
