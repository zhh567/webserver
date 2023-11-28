#include <sys/eventfd.h>
#include <webserver/io/EventLoop.h>
#include <webserver/logger/Logger.h>

#include <ostream>
#include <sstream>
#include <thread>

using namespace webserver;

namespace {

int create_event_fd() {
  int event_fd = ::eventfd(0, 0);
  if (event_fd < 0) {
    LOG_ERROR << "Failed in eventfd";
    abort();
  }
  return event_fd;
}

// void sigquit_handler(EventLoop* loop, int sig) { loop->quit(); }

}  // namespace

EventLoop::EventLoop()
    : thread_id_(std::this_thread::get_id()),
      looping_(false),
      quit_(false),
      poll_return_time_(std::chrono::system_clock::now()),
      poller_(new Poller(this)),
      active_channels_(),
      wakeup_fd_(create_event_fd()),
      wakeup_channel_(new Channel(this, wakeup_fd_)),
      pending_functors_() {
  wakeup_channel_->set_read_callback(
      std::bind(&EventLoop::handle_wakeup_read, this));
  wakeup_channel_->enable_reading();
  std::string s("EventLoop wakeup_cahnnel");
  wakeup_channel_->set_name(s);
}
EventLoop::~EventLoop() {
  ::close(wakeup_fd_);
  LOG_DEBUG << "EventLoop " << this << " destruct";
}

void EventLoop::loop() {
  if (!is_in_loop_thread()) {
    LOG_FATAL << "EventLoop::loop() was called by another thread";
  }

  looping_ = true;
  quit_ = false;
  LOG_DEBUG << "EventLoop " << this << " start looping";

  while (!quit_) {
    active_channels_.clear();
    Timestamp poll_return_time_ = poller_->poll(10000, &active_channels_);

    for (auto& channel : active_channels_) {
      channel->handle_event(poll_return_time_);
    }

    do_pending_functors();
  }

  looping_ = false;
  LOG_DEBUG << "EventLoop " << this << " stop looping";
}

void EventLoop::quit() {
  quit_.store(true);
  if (!is_in_loop_thread()) {
    wakeup();
  }
}

void EventLoop::run_in_loop(const VoidFunctor& cb) {
  if (is_in_loop_thread()) {
    cb();
  } else {  // not in loop thread
    {
      std::lock_guard<std::mutex> lock(mutex_pending_functors_);
      pending_functors_.push_back(cb);
    }
    wakeup();  // wake up loop thread
  }
}

// wakeup EventLoop from wait event to do something, like do_pending_functor;

void EventLoop::wakeup() {
  uint64_t one = 1;
  int n = ::write(wakeup_fd_, &one, sizeof(one));
  if (n != 8) {
    LOG_ERROR << "EventLoop::wakeup() writes " << n
              << " bytes instead of 1, errno = " << errno;
  }
}

void EventLoop::handle_wakeup_read() {
  uint64_t one = 0;
  int n = ::read(wakeup_fd_, &one, sizeof(one));
  if (n != 8) {
    LOG_ERROR << "EventLoop::handle_wakeup_read() reads " << n
              << " bytes instead of 1";
  }
}

void EventLoop::do_pending_functors() {
  std::vector<VoidFunctor> functors;
  {
    std::lock_guard<std::mutex> lock(mutex_pending_functors_);
    pending_functors_.swap(functors);
  }

  for (auto& func : functors) {
    func();
  }
}

void EventLoop::abort_not_in_loop_thread() {
  if (thread_id_ != std::this_thread::get_id()) {
    std::ostringstream oss;
    oss << "EventLoop::abort_not_in_loop_thread - EventLoop " << this
        << " was created in threadId_ = " << thread_id_
        << ", current thread id = " << std::this_thread::get_id();
    LOG_FATAL << oss.str();
  }
}
