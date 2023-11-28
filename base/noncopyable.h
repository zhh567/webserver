#ifndef WEBSERVER_BASE_NONCOPYABLE_H
#define WEBSERVER_BASE_NONCOPYABLE_H

namespace webserver {

class noncopyable {
 public:
  noncopyable() = default;
  ~noncopyable() = default;

 protected:
  noncopyable(const noncopyable&) = delete;
  noncopyable operator=(const noncopyable&) = delete;
};

}  // namespace server

#endif  // WEBSERVER_BASE_NONCOPYABLE_H
