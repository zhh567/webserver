#include <sys/epoll.h>
#include <unistd.h>
#include <webserver/io/Channel.h>
#include <webserver/io/EventLoop.h>

#include <sstream>

using namespace webserver;

const int Channel::kNoneEvent = 0;
const int Channel::kReadEvent = EPOLLIN;
const int Channel::kWriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd)
    : fd_(fd),
      loop_(loop),
      events_(kNoneEvent),
      recv_events_(kNoneEvent),
      status_(Poller::kNew){};
Channel::~Channel() { ::close(fd_); }

// As callback in Poller, EventLoop
void Channel::handle_event(Timestamp timestamp) {
  std::ostringstream oss;
  if (recv_events_ & EPOLLIN) oss << "EPOLLIN ";
  if (recv_events_ & EPOLLPRI) oss << "EPOLLPRI ";
  if (recv_events_ & EPOLLOUT) oss << "EPOLLOUT ";
  if (recv_events_ & EPOLLERR) oss << "EPOLLERR ";
  if (recv_events_ & EPOLLHUP) oss << "EPOLLHUP ";
  if (recv_events_ & EPOLLRDHUP) oss << "EPOLLRDHUP ";

  LOG_DEBUG << "Channel: " << get_name() << " receive events: " << recv_events_
            << " " << oss.str();

  if (recv_events_ & EPOLLHUP && !(recv_events_ & EPOLLIN)) {
    if (close_callback_) close_callback_();
  }
  if (recv_events_ & EPOLLERR) {
    if (error_callback_) error_callback_();
  }
  if (recv_events_ & (EPOLLIN | EPOLLPRI | EPOLLRDHUP)) {
    if (read_callback_) read_callback_(timestamp);
  }
  if (recv_events_ & EPOLLOUT) {
    if (write_callback_) write_callback_();
  }
}

void Channel::add() { loop_->add_channel(this); }
void Channel::update() { loop_->update_channel(this); }
void Channel::remove() { loop_->remove_channel(this); }
