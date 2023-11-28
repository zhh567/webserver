#ifndef WEBSERVER_IO_CHANNEL_H
#define WEBSERVER_IO_CHANNEL_H

#include <webserver/base/Alias.h>
#include <webserver/logger/Logger.h>

#include <string>

namespace webserver {

class EventLoop;

class Channel {
 public:
  Channel(EventLoop* loop, int fd);
  ~Channel();

  // call callback to deal events
  void handle_event(Timestamp timestamp);
  // set callbacks
  void set_read_callback(const ReadEventCallback& cb) { read_callback_ = cb; }
  void set_write_callback(const EventCallback& cb) { write_callback_ = cb; }
  void set_close_callback(const EventCallback& cb) { close_callback_ = cb; }
  void set_error_callback(const EventCallback& cb) { error_callback_ = cb; }

  int fd() const { return fd_; }

  // events
  uint32_t get_events() const { return events_; }
  void enable_reading() {
    events_ |= kReadEvent;
    update();
  }
  void enable_writing() {
    events_ |= kWriteEvent;
    LOG_DEBUG << " Channel "
              << " enable writing";
    update();
  }
  void disable_reading() {
    events_ &= ~kReadEvent;
    update();
  }
  void disable_writing() {
    events_ &= ~kWriteEvent;
    update();
  }
  void disable_all() {
    events_ = kNoneEvent;
    update();
  }
  bool is_reading() { return events_ & kReadEvent; }
  bool is_writing() { return events_ & kWriteEvent; }
  bool is_none_event() { return events_ == kNoneEvent; }

  void set_recv_events(uint32_t recv_events) { recv_events_ = recv_events; }

  // poller
  int get_status() { return status_; }
  void set_status(int status) { status_ = status; }

  std::string get_name() { return name_; }
  void set_name(const std::string& name) { name_ = name; }

  // These call methods in `EventLoop`.
  // And `EventLoop` call `Poller` which implement specific operations.
  void add();
  void remove();
  void update();

 private:
  static const int kNoneEvent;
  static const int kReadEvent;
  static const int kWriteEvent;

  EventLoop* loop_;
  int fd_;
  uint32_t events_;
  uint32_t recv_events_;
  // static const int kNew;
  // static const int kAdded;
  // static const int kDeleted;
  int status_;

  std::string name_;

  // callbacks which handle events
  ReadEventCallback read_callback_;
  EventCallback write_callback_;
  EventCallback close_callback_;
  EventCallback error_callback_;
};

}  // namespace webserver

#endif  // WEBSERVER_IO_CHANNEL_H
