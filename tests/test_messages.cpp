#include <gtest/gtest.h>
#include "../common/messages.hpp"
#include "../common/serialize.hpp"

class MessagesTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(MessagesTest, ChatLineTypeId) {
  // Test that ChatLine has the correct type_id
  EXPECT_EQ(ChatLine::type_id, 1);
}

TEST_F(MessagesTest, ChatLineSerializationBasic) {
  // Test basic serialization
  ChatLine msg;
  msg.room = "lobby";
  msg.user = "alice";
  msg.text = "Hello, world!";

  auto bytes = to_bytes(msg);

  // Check that we have the expected size
  // 3 * 4 bytes for string lengths + actual string data
  size_t expected_size = 4 + msg.room.size() + 4 + msg.user.size() + 4 + msg.text.size();
  EXPECT_EQ(bytes.size(), expected_size);
}

TEST_F(MessagesTest, ChatLineSerializationEmpty) {
  // Test serialization with empty strings
  ChatLine msg;
  msg.room = "";
  msg.user = "";
  msg.text = "";

  auto bytes = to_bytes(msg);

  // Should have 3 * 4 bytes for empty string lengths
  EXPECT_EQ(bytes.size(), 12);
}

TEST_F(MessagesTest, ChatLineDeserializationBasic) {
  // Test basic deserialization
  ChatLine original;
  original.room = "general";
  original.user = "bob";
  original.text = "How are you?";

  auto bytes = to_bytes(original);
  ChatLine deserialized = from_bytes<ChatLine>(bytes);

  EXPECT_EQ(original.room, deserialized.room);
  EXPECT_EQ(original.user, deserialized.user);
  EXPECT_EQ(original.text, deserialized.text);
}

TEST_F(MessagesTest, ChatLineRoundTrip) {
  // Test round-trip serialization/deserialization
  ChatLine original;
  original.room = "dev";
  original.user = "charlie";
  original.text = "Working on the new feature";

  auto bytes = to_bytes(original);
  ChatLine deserialized = from_bytes<ChatLine>(bytes);

  EXPECT_EQ(original.room, deserialized.room);
  EXPECT_EQ(original.user, deserialized.user);
  EXPECT_EQ(original.text, deserialized.text);
}

TEST_F(MessagesTest, ChatLineSpecialCharacters) {
  // Test with special characters
  ChatLine msg;
  msg.room = "test-room_123";
  msg.user = "user@domain.com";
  msg.text = "Hello 世界! 🌍 Special chars: @#$%^&*()";

  auto bytes = to_bytes(msg);
  ChatLine deserialized = from_bytes<ChatLine>(bytes);

  EXPECT_EQ(msg.room, deserialized.room);
  EXPECT_EQ(msg.user, deserialized.user);
  EXPECT_EQ(msg.text, deserialized.text);
}

TEST_F(MessagesTest, ChatLineLongStrings) {
  // Test with long strings
  std::string long_room(1000, 'A');
  std::string long_user(1000, 'B');
  std::string long_text(1000, 'C');

  ChatLine msg;
  msg.room = long_room;
  msg.user = long_user;
  msg.text = long_text;

  auto bytes = to_bytes(msg);
  ChatLine deserialized = from_bytes<ChatLine>(bytes);

  EXPECT_EQ(msg.room, deserialized.room);
  EXPECT_EQ(msg.user, deserialized.user);
  EXPECT_EQ(msg.text, deserialized.text);
}

TEST_F(MessagesTest, ChatLineNewlinesAndTabs) {
  // Test with newlines and tabs
  ChatLine msg;
  msg.room = "room\nwith\ttabs";
  msg.user = "user\nwith\ttabs";
  msg.text = "text\nwith\ttabs\nand\nmultiple\nlines";

  auto bytes = to_bytes(msg);
  ChatLine deserialized = from_bytes<ChatLine>(bytes);

  EXPECT_EQ(msg.room, deserialized.room);
  EXPECT_EQ(msg.user, deserialized.user);
  EXPECT_EQ(msg.text, deserialized.text);
}

TEST_F(MessagesTest, ChatLineNullBytes) {
  // Test with null bytes (should be preserved)
  ChatLine msg;
  msg.room = "room\0with\0nulls";
  msg.user = "user\0with\0nulls";
  msg.text = "text\0with\0nulls";

  auto bytes = to_bytes(msg);
  ChatLine deserialized = from_bytes<ChatLine>(bytes);

  EXPECT_EQ(msg.room, deserialized.room);
  EXPECT_EQ(msg.user, deserialized.user);
  EXPECT_EQ(msg.text, deserialized.text);
}

TEST_F(MessagesTest, SerializableConcept) {
  // Test that ChatLine satisfies the Serializable concept
  static_assert(Serializable<ChatLine>, "ChatLine should satisfy Serializable concept");

  ChatLine msg;
  msg.room = "test";
  msg.user = "test";
  msg.text = "test";

  // Test that to_bytes works
  auto bytes = to_bytes(msg);
  EXPECT_FALSE(bytes.empty());

  // Test that from_bytes works
  ChatLine deserialized = from_bytes<ChatLine>(bytes);
  EXPECT_EQ(msg.room, deserialized.room);
}

TEST_F(MessagesTest, DeserializationErrorHandling) {
  // Test error handling for malformed data
  std::vector<std::byte> empty_data;
  EXPECT_THROW(from_bytes<ChatLine>(empty_data), std::runtime_error);

  // Test with insufficient data (less than 4 bytes for first length)
  std::vector<std::byte> short_data = {std::byte{0x01}, std::byte{0x02}};
  EXPECT_THROW(from_bytes<ChatLine>(short_data), std::runtime_error);

  // Test with length that exceeds available data
  std::vector<std::byte> bad_length_data = {
      std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
      std::byte{0xFF}, // length = 255
      std::byte{0x00}, std::byte{0x00}, std::byte{0x00},
      std::byte{0x00}, // user length = 0
      std::byte{0x00}, std::byte{0x00}, std::byte{0x00}, std::byte{0x00} // text length = 0
      // Missing 255 bytes of room data
  };
  EXPECT_THROW(from_bytes<ChatLine>(bad_length_data), std::runtime_error);
}

TEST_F(MessagesTest, SerializationConsistency) {
  // Test that multiple serializations of the same message produce the same
  // result
  ChatLine msg;
  msg.room = "consistent";
  msg.user = "test";
  msg.text = "message";

  auto bytes1 = to_bytes(msg);
  auto bytes2 = to_bytes(msg);

  EXPECT_EQ(bytes1.size(), bytes2.size());
  EXPECT_EQ(bytes1, bytes2);
}