#include <asio.hpp>
#include <iostream>
#include <memory>
#include "common/thread_pool.hpp"
#include "server/session.hpp"
#include "server/hub.hpp"

int main(int argc, char** argv) {
  using asio::ip::tcp;
  uint16_t port = (argc>1)? static_cast<uint16_t>(std::stoi(argv[1])) : 5555;

  asio::io_context io;
  auto hub = std::make_shared<Hub>();
  ThreadPool workers{4};

  tcp::acceptor acceptor(io, tcp::endpoint(tcp::v4(), port));

  std::function<void()> do_accept;
  do_accept = [&]{
    acceptor.async_accept([&](std::error_code ec, tcp::socket sock){
      if (!ec) {
        auto s = std::make_shared<Session>(std::move(sock), hub);
        s->start();
      }
      do_accept();
    });
  };
  do_accept();
  std::cout << "chatd listening on " << port << std::endl;
  io.run();
  return 0;
}
