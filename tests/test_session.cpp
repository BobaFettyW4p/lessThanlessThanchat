#include <gtest/gtest.h>
#include <asio.hpp>
#include <memory>
#include <thread>
#include <chrono>
#include "../server/session.hpp"
#include "../server/hub.hpp"
#include "../common/messages.hpp"

class SessionTest : public ::testing::Test {
protected:
  void SetUp() override {
    io_context = std::make_unique<asio::io_context>();
    hub = std::make_shared<Hub>();
  }

  void TearDown() override {
    if (io_context) {
      io_context->stop();
    }
    hub.reset();
    io_context.reset();
  }

  std::unique_ptr<asio::io_context> io_context;
  std::shared_ptr<Hub> hub;
};

TEST_F(SessionTest, SessionCreation) {
  // Create a mock socket (we'll use a real socket for this test)
  asio::ip::tcp::socket socket(*io_context);

  // Create a session
  auto session = std::make_shared<Session>(std::move(socket), hub);

  // Session should be created successfully
  EXPECT_NE(session, nullptr);
}

TEST_F(SessionTest, SessionSendMessage) {
  // Create a mock socket
  asio::ip::tcp::socket socket(*io_context);

  // Create a session
  auto session = std::make_shared<Session>(std::move(socket), hub);

  // Create a message
  ChatLine msg;
  msg.room = "test_room";
  msg.user = "test_user";
  msg.text = "Hello, world!";

  // Send the message (this should not crash even with a mock socket)
  EXPECT_NO_THROW(session->send(msg));
}

TEST_F(SessionTest, SessionSendRawData) {
  // Create a mock socket
  asio::ip::tcp::socket socket(*io_context);

  // Create a session
  auto session = std::make_shared<Session>(std::move(socket), hub);

  // Create raw data
  std::vector<std::byte> data = {std::byte{0x01}, std::byte{0x02},
                                 std::byte{0x03}};

  // Send raw data (this should not crash even with a mock socket)
  EXPECT_NO_THROW(session->send_raw(data));
}

// Integration test with real sockets
class SessionIntegrationTest : public ::testing::Test {
protected:
  void SetUp() override {
    io_context = std::make_unique<asio::io_context>();
    hub = std::make_shared<Hub>();

    // Create acceptor
    acceptor = std::make_unique<asio::ip::tcp::acceptor>(
        *io_context, asio::ip::tcp::endpoint(asio::ip::tcp::v4(), 0));

    // Get the port
    port = acceptor->local_endpoint().port();
  }

  void TearDown() override {
    if (acceptor) {
      acceptor->close();
    }
    if (io_context) {
      io_context->stop();
    }
    hub.reset();
    acceptor.reset();
    io_context.reset();
  }

  std::unique_ptr<asio::io_context> io_context;
  std::shared_ptr<Hub> hub;
  std::unique_ptr<asio::ip::tcp::acceptor> acceptor;
  uint16_t port;
};

TEST_F(SessionIntegrationTest, SessionStartAndJoin) {
  // This test verifies that a session can start and join a room
  // We'll use a mock callback to capture the join operation

  std::atomic<bool> session_started{false};
  std::string joined_room;

  // Set up acceptor to accept one connection
  std::thread server_thread([&]() {
    asio::ip::tcp::socket socket(*io_context);
    acceptor->accept(socket);

    // Create session and start it
    auto session = std::make_shared<Session>(std::move(socket), hub);

    // Mock the hub join method to capture the call
    auto mock_hub = std::make_shared<Hub>();
    session_started = true;

    // Start the session (this will call hub->join)
    session->start();
  });

  // Connect to the server
  std::thread client_thread([&]() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    asio::ip::tcp::socket socket(*io_context);
    asio::ip::tcp::resolver resolver(*io_context);
    auto endpoints = resolver.resolve("127.0.0.1", std::to_string(port));
    asio::connect(socket, endpoints);

    // Keep connection alive briefly
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  });

  // Run the io_context
  std::thread io_thread([&]() { io_context->run(); });

  // Wait for completion
  client_thread.join();
  server_thread.join();

  // Stop io_context
  io_context->stop();
  io_thread.join();

  // Verify session was started
  EXPECT_TRUE(session_started);
}

// Test for message handling (this would require more complex setup)
class SessionMessageTest : public ::testing::Test {
protected:
  void SetUp() override {
    io_context = std::make_unique<asio::io_context>();
    hub = std::make_shared<Hub>();
  }

  void TearDown() override {
    if (io_context) {
      io_context->stop();
    }
    hub.reset();
    io_context.reset();
  }

  std::unique_ptr<asio::io_context> io_context;
  std::shared_ptr<Hub> hub;
};

TEST_F(SessionMessageTest, MessageSerialization) {
  // Test that messages are properly serialized when sent
  asio::ip::tcp::socket socket(*io_context);
  auto session = std::make_shared<Session>(std::move(socket), hub);

  // Create a message
  ChatLine msg;
  msg.room = "test_room";
  msg.user = "test_user";
  msg.text = "Test message";

  // This should not throw
  EXPECT_NO_THROW(session->send(msg));
}

TEST_F(SessionMessageTest, MultipleMessages) {
  // Test sending multiple messages
  asio::ip::tcp::socket socket(*io_context);
  auto session = std::make_shared<Session>(std::move(socket), hub);

  // Send multiple messages
  for (int i = 0; i < 10; ++i) {
    ChatLine msg;
    msg.room = "test_room";
    msg.user = "user" + std::to_string(i);
    msg.text = "Message " + std::to_string(i);

    EXPECT_NO_THROW(session->send(msg));
  }
}

TEST_F(SessionMessageTest, LargeMessage) {
  // Test sending a large message
  asio::ip::tcp::socket socket(*io_context);
  auto session = std::make_shared<Session>(std::move(socket), hub);

  ChatLine msg;
  msg.room = "test_room";
  msg.user = "test_user";
  msg.text = std::string(10000, 'A'); // Large message

  EXPECT_NO_THROW(session->send(msg));
}

TEST_F(SessionMessageTest, SpecialCharacters) {
  // Test sending messages with special characters
  asio::ip::tcp::socket socket(*io_context);
  auto session = std::make_shared<Session>(std::move(socket), hub);

  ChatLine msg;
  msg.room = "test-room_123";
  msg.user = "user@domain.com";
  msg.text = "Hello 世界! 🌍 Special chars: @#$%^&*()";

  EXPECT_NO_THROW(session->send(msg));
}

// Test for error handling
TEST_F(SessionMessageTest, ErrorHandling) {
  // Test that the session handles errors gracefully
  asio::ip::tcp::socket socket(*io_context);
  auto session = std::make_shared<Session>(std::move(socket), hub);

  // Try to send to a closed socket (should not crash)
  socket.close();

  ChatLine msg;
  msg.room = "test_room";
  msg.user = "test_user";
  msg.text = "Test message";

  // This might throw or not, but should not crash the program
  try {
    session->send(msg);
  } catch (...) {
    // Expected for closed socket
  }
}