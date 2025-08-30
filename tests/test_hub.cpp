#include "../common/messages.hpp"
#include "../common/protocol.hpp"
#include "../server/hub.hpp"
#include <functional>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <thread>
#include <vector>

class HubTest : public ::testing::Test {
protected:
  void SetUp() override { // NOLINT(misc-unused-function)
    hub = std::make_shared<Hub>();
  }

  void TearDown() override { // NOLINT(misc-unused-function)
    hub.reset();
  }

  std::shared_ptr<Hub> hub;
};

TEST_F(HubTest, HubCreation) {
  // Test that we can create a Hub
  EXPECT_NE(hub, nullptr);
}

TEST_F(HubTest, HubSharedFromThis) {
  // Test that Hub can be used with shared_from_this
  auto hub_copy = hub->shared_from_this();
  EXPECT_EQ(hub, hub_copy);
}

// Test the broadcast functionality without real sessions
TEST_F(HubTest, BroadcastToEmptyRoom) {
  // Test broadcasting to a room with no members
  ChatLine msg;
  msg.room = "empty_room";
  msg.user = "alice";
  msg.text = "Hello, empty room!";

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("empty_room", msg));
}

TEST_F(HubTest, BroadcastToNonexistentRoom) {
  // Test broadcasting to a room that doesn't exist
  ChatLine msg;
  msg.room = "nonexistent";
  msg.user = "alice";
  msg.text = "Hello, nonexistent room!";

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("nonexistent", msg));
}

TEST_F(HubTest, MultipleBroadcasts) {
  // Test multiple broadcasts to different rooms
  ChatLine msg1;
  msg1.room = "room1";
  msg1.user = "alice";
  msg1.text = "Hello, room1!";

  ChatLine msg2;
  msg2.room = "room2";
  msg2.user = "bob";
  msg2.text = "Hello, room2!";

  // Both should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("room1", msg1));
  EXPECT_NO_THROW(hub->broadcast("room2", msg2));
}

TEST_F(HubTest, LargeMessageBroadcast) {
  // Test broadcasting a large message
  ChatLine msg;
  msg.room = "test_room";
  msg.user = "test_user";
  msg.text = std::string(10000, 'A'); // Large message

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("test_room", msg));
}

TEST_F(HubTest, SpecialCharactersBroadcast) {
  // Test broadcasting messages with special characters
  ChatLine msg;
  msg.room = "test-room_123";
  msg.user = "user@domain.com";
  msg.text = "Hello 世界! 🌍 Special chars: @#$%^&*()";

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("test_room", msg));
}

TEST_F(HubTest, ConcurrentBroadcasts) {
  // Test concurrent broadcasts (thread safety)
  std::vector<std::thread> threads;

  for (int i = 0; i < 10; ++i) {
    threads.emplace_back([this, i]() {
      ChatLine msg;
      msg.room = "concurrent_room";
      msg.user = "user" + std::to_string(i);
      msg.text = "Message " + std::to_string(i);

      // This should not crash or throw
      EXPECT_NO_THROW(hub->broadcast("concurrent_room", msg));
    });
  }

  // Wait for all threads to complete
  for (auto &thread : threads) {
    thread.join();
  }
}

TEST_F(HubTest, DifferentMessageTypes) {
  // Test that the Hub can handle different message types
  // (Currently only ChatLine is supported, but this tests the template)

  ChatLine msg;
  msg.room = "test_room";
  msg.user = "test_user";
  msg.text = "Test message";

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("test_room", msg));
}

TEST_F(HubTest, RoomNameVariations) {
  // Test different room name formats
  std::vector<std::string> room_names = {"lobby",
                                         "general",
                                         "dev",
                                         "test-room",
                                         "room_123",
                                         "room@domain.com",
                                         "room with spaces",
                                         "room-with-dashes",
                                         "room_with_underscores",
                                         "room.with.dots"};

  for (const auto &room_name : room_names) {
    ChatLine msg;
    msg.room = room_name;
    msg.user = "test_user";
    msg.text = "Test message for " + room_name;

    // This should not crash or throw
    EXPECT_NO_THROW(hub->broadcast(room_name, msg));
  }
}

TEST_F(HubTest, EmptyMessageContent) {
  // Test broadcasting messages with empty content
  ChatLine msg;
  msg.room = "test_room";
  msg.user = "";
  msg.text = "";

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("test_room", msg));
}

TEST_F(HubTest, VeryLongRoomName) {
  // Test with very long room names
  std::string long_room_name(1000, 'A');

  ChatLine msg;
  msg.room = long_room_name;
  msg.user = "test_user";
  msg.text = "Test message";

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast(long_room_name, msg));
}

TEST_F(HubTest, VeryLongUserAndText) {
  // Test with very long user names and text
  std::string long_user(1000, 'B');
  std::string long_text(1000, 'C');

  ChatLine msg;
  msg.room = "test_room";
  msg.user = long_user;
  msg.text = long_text;

  // This should not crash or throw
  EXPECT_NO_THROW(hub->broadcast("test_room", msg));
}