#include <signal.h>
#include <webserver/thread/EventLoopThread.h>

using namespace webserver;

EventLoopThread::EventLoopThread(EventLoopThreadInitCallback& init_callback)
    : loop_(nullptr),
      existed_(false),
      thread_(nullptr),
      init_callback_(init_callback),
      mutex_set_loop_(),
      cond_wait_set_loop_() {}

EventLoopThread::~EventLoopThread() {
  existed_ = true;
  if (loop_ != nullptr) {
    loop_->quit();
    if (thread_->joinable()) {
      thread_->join();
    }
  }
  LOG_DEBUG << "EventLoopThread " << this << " destructed";
}

EventLoop* EventLoopThread::start_loop() {
  auto t = new std::thread(std::bind(&EventLoopThread::thread_func, this));
  thread_ = std::unique_ptr<std::thread>(std::move(t));

  // wait new thread create `EventLoop` assigning to loop_
  {
    std::unique_lock<std::mutex> lock(mutex_set_loop_);
    cond_wait_set_loop_.wait(lock, [this] { return this->loop_ != nullptr; });
  }

  return loop_;
}

void EventLoopThread::thread_func() {
  // ignore all signal
  sigset_t sigset;
  sigfillset(&sigset);
  pthread_sigmask(SIG_BLOCK, &sigset, nullptr);

  EventLoop loop;
  if (init_callback_) {
    init_callback_(&loop);
  }

  // notify old thread, loop_ is set.
  {
    std::unique_lock<std::mutex> lock(mutex_set_loop_);
    loop_ = &loop;
    cond_wait_set_loop_.notify_all();
  }

  loop.loop();

  loop_ = nullptr;
}