#ifndef WEBSERVER_BASE_ALIAS_H
#define WEBSERVER_BASE_ALIAS_H

#include <chrono>
#include <functional>
#include <memory>

namespace webserver {

class EventLoop;
class Buffer;

using Timestamp = std::chrono::system_clock::time_point;

// class Channel

using ReadEventCallback =
    std::function<void(std::chrono::system_clock::time_point)>;
using EventCallback = std::function<void()>;

using VoidFunctor = std::function<void()>;

using EventLoopThreadInitCallback = std::function<void(EventLoop*)>;

// used by TcpConnection
class TcpConnection;
using TcpConnectionPtr = std::shared_ptr<TcpConnection>;

using ReadCallback =
    std::function<void(const TcpConnectionPtr&, Buffer*, Timestamp)>;
using WriteCompleteCallback = std::function<void(const TcpConnectionPtr&)>;
using ConnectionCallback = std::function<void(const TcpConnectionPtr&)>;
using CloseCallback = std::function<void(const TcpConnectionPtr&)>;
using HightWaterCallback = std::function<void(const TcpConnectionPtr&)>;

}  // namespace webserver

#endif  // WEBSERVER_BASE_ALIAS_H
