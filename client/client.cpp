#include <asio.hpp>
#include <thread>
#include <iostream>
#include <vector>
#include <array>
#include <cstring>
#include "common/protocol.hpp"
#include "common/messages.hpp"

int main(int argc, char **argv) {
  using asio::ip::tcp;
  if (argc < 4) {
    std::cerr << "usage: chatc <host> <port> <room> [user]\n";
    return 1;
  }
  std::string host = argv[1], port = argv[2], room = argv[3];
  std::string user = (argc > 4) ? argv[4] : "anon";

  asio::io_context io;
  tcp::resolver res(io);
  auto ep = res.resolve(host, port);
  tcp::socket sock(io);
  asio::connect(sock, ep);

  // reader thread
  std::thread reader([&] {
    std::array<std::byte, 8> hdr{};
    for (;;) {
      asio::read(sock, asio::buffer(hdr));
      FrameHeader h = parse_header(std::span<const std::byte, 8>(hdr.data(), 8));
      uint32_t len = header_len(h);
      uint16_t type = header_type(h);
      std::vector<std::byte> payload(len);
      asio::read(sock, asio::buffer(payload));
      if (type == ChatLine::type_id) {
        ChatLine msg = from_bytes<ChatLine>(
            std::span<const std::byte>(payload.data(), payload.size()));
        std::cout << "[" << msg.room << "] " << msg.user << ": " << msg.text << "\n";
      } else {
        std::cout << "(unknown message type " << type << ")\n";
      }
    }
  });

  // stdin loop
  for (std::string line; std::getline(std::cin, line);) {
    ChatLine m{room, user, line};
    auto frame = make_frame(m);
    asio::write(sock, asio::buffer(frame));
  }

  reader.join();
  return 0;
}
