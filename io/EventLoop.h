#ifndef WEBSERVER_IO_EVENTLOOP_H
#define WEBSERVER_IO_EVENTLOOP_H

#include <webserver/io/Channel.h>
#include <webserver/io/Poller.h>
#include <webserver/logger/Logger.h>

#include <atomic>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

namespace webserver {

class Channel;

/// @brief  wait epoll_event using Poller，then call corresponding channel's
/// CallBack functions
class EventLoop {
 public:
  EventLoop();
  ~EventLoop();

  // loop forever
  void loop();
  void quit();

  Timestamp get_poll_return_time() const { return poll_return_time_; }
  void add_channel(Channel* channel) { poller_->add_channel(channel); }
  void update_channel(Channel* channel) { poller_->update_channel(channel); }
  void remove_channel(Channel* channel) { poller_->remove_channel(channel); }

  //  when call by loop thread, run Functor directly. otherwhise, overcome queue
  void run_in_loop(const VoidFunctor& cb);

  void wakeup();

  bool is_in_loop_thread() { return thread_id_ == std::this_thread::get_id(); }
  void abort_not_in_loop_thread();

  void set_is_net_loop(bool is_net_loop) { is_net_loop_ = is_net_loop; }

 private:
  void do_pending_functors();
  void handle_wakeup_read();

  bool is_net_loop_;

  std::thread::id thread_id_;
  std::atomic_bool looping_;
  std::atomic_bool quit_;

  Timestamp poll_return_time_;

  std::unique_ptr<Poller> poller_;
  ChannelList active_channels_;

  int wakeup_fd_;
  std::unique_ptr<Channel> wakeup_channel_;

  // protect pending_functors_ when call internal functions
  std::mutex mutex_pending_functors_;
  std::vector<VoidFunctor> pending_functors_;
};

}  // namespace webserver

#endif  // WEBSERVER_IO_EVENTLOOP_H
