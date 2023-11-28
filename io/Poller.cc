#include <errno.h>
#include <signal.h>
#include <webserver/base/Alias.h>
#include <webserver/base/helper.h>
#include <webserver/io/Poller.h>
#include <webserver/logger/Logger.h>

using namespace webserver;

const int Poller::kNew = 0;
const int Poller::kAdded = 1;
const int Poller::kDeleted = 2;

Poller::Poller(EventLoop* loop)
    : epollfd_(epoll_create1(EPOLL_CLOEXEC)),
      loop_(loop),
      events_(kInitEventListSize) {
  LOG_INFO << "create new Poller: " << epollfd_
           << " in thread: " /*<< loop_->get_thread_id()*/;
}
Poller::~Poller() { ::close(epollfd_); }

/// @brief wait epoll_event and set corresponding channel, don't call CallBack
/// functino
/// @param timeout
/// @param active_channels
/// @return timestamp receiving event
Timestamp Poller::poll(int timeout, ChannelList* active_channels) {
  int num_recv_events =
      ::epoll_wait(epollfd_, &*(events_.begin()), events_.size(), timeout);
  Timestamp now = std::chrono::system_clock::now();

  if (num_recv_events == 0) {
    // LOG_DEBUG << "epoll_pwait timeout";
    return now;
  } else if (num_recv_events < 0) {
    LOG_ERROR << "epoll_pwait error: " << errno /*<< loop_->get_thread_id()*/;
    return now;
  }

  // when events occur
  if (num_recv_events == events_.size()) {
    events_.resize(num_recv_events * 2);
  }

  // active channels which receive events
  for (size_t i = 0; i < num_recv_events; i++) {
    struct epoll_event event = events_[i];
    Channel* channel = channels_[event.data.fd];
    channel->set_recv_events(event.events);
    active_channels->push_back(channel);
  }

  return now;
}

// FIXME delete me
void Poller::add_channel(Channel* channel) {
  channels_[channel->fd()] = channel;
}

void Poller::update_channel(Channel* channel) {
  int status = channel->get_status();
  LOG_DEBUG << "fd = " << channel->fd() << " events = " << channel->get_events()
            << " status = " << status;

  if (status == kNew || status == kDeleted) {
    if (status == kNew) {  // when this channel is newly created, Poller's
                           // channel vector don't have it.
      channels_[channel->fd()] = channel;
    }
    update(EPOLL_CTL_ADD, channel);
    channel->set_status(kAdded);
  }
  // when channel is added
  else {
    if (channel->is_none_event()) {
      update(EPOLL_CTL_DEL, channel);
      channel->set_status(kDeleted);
    } else {
      update(EPOLL_CTL_MOD, channel);
    }
  }
}

void Poller::remove_channel(Channel* channel) {
  LOG_DEBUG << " channel = " << channel->get_name();
  channels_.erase(channel->fd());
  if (channel->get_status() == kAdded) {
    update(EPOLL_CTL_DEL, channel);
  }
  channel->set_status(kDeleted);
}

void Poller::update(int operation, Channel* channel) {
  struct epoll_event event = {0};
  event.events = channel->get_events();
  event.data.fd = channel->fd();

  std::string op("UNKOWN");
  if (operation == EPOLL_CTL_ADD) {
    op = "ADD";
  } else if (operation == EPOLL_CTL_DEL) {
    op = "DEL";
  } else if (operation == EPOLL_CTL_MOD) {
    op = "MOD";
  }

  std::string event_str("UNKOWN");
  if (event.events & EPOLLIN) {
    event_str = "EPOLLIN";
  } else if (event.events & EPOLLPRI) {
    event_str = "EPOLLPRI";
  } else if (event.events & EPOLLOUT) {
    event_str = "EPOLLOUT";
  } else if (event.events & EPOLLERR) {
    event_str = "EPOLLERR";
  } else if (event.events & EPOLLHUP) {
    event_str = "EPOLLHUP";
  } else if (event.events & EPOLLRDHUP) {
    event_str = "EPOLLRDHUP";
  }

  LOG_DEBUG << "epoll_ctl op = " << op << " name = " << channel->get_name()
            << " event = { " << event_str << " }";

  // TODO there may occur logistic error
  ::epoll_ctl(epollfd_, operation, channel->fd(), &event);

}
