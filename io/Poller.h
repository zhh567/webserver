#ifndef WEBSERVER_IO_POLLER_H
#define WEBSERVER_IO_POLLER_H

#include <sys/epoll.h>
#include <webserver/io/Channel.h>

#include <map>
#include <vector>

namespace webserver {

using ChannelList = std::vector<Channel*>;
using ChannelMap = std::map<int, Channel*>;
using EventList = std::vector<struct epoll_event>;

class EventLoop;

class Poller {
 public:
  static const int kNew;
  static const int kAdded;
  static const int kDeleted;
  // enum class ChannelStatus {
  //   NEW = 0,
  //   ADDED = 1,
  //   DELETED = 2,
  // };

  Poller(EventLoop* loop);
  ~Poller();

  // epoll_wait
  Timestamp poll(int timeout, ChannelList* active_channels);

  void add_channel(Channel* channel);
  void update_channel(Channel* channel);
  void remove_channel(Channel* channel);

 private:
  // 直接写到 poll 函数中，不需要单独写一个
  // void set_channel_recv_events(int num_events, ChannelList* channels);

  void update(int operation, Channel* channel);

  const int kInitEventListSize = 16;

  int epollfd_;
  EventLoop* loop_;
  EventList events_;
  ChannelMap channels_;
};

}  // namespace webserver

#endif  // WEBSERVER_IO_POLLER_H
