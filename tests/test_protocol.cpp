#include "../common/messages.hpp"
#include "../common/net_endian.hpp"
#include "../common/protocol.hpp"
#include <gtest/gtest.h>

class ProtocolTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(ProtocolTest, FrameHeaderStructure) {
  // Test that FrameHeader has the expected size and layout
  FrameHeader header;
  EXPECT_EQ(sizeof(header), 8);

  // Test that we can set and get values
  header.len_be = to_be32(100);
  header.type_be = to_be16(1);
  header.flags_be = to_be16(0);

  EXPECT_EQ(header_len(header), 100);
  EXPECT_EQ(header_type(header), 1);
}

TEST_F(ProtocolTest, ParseHeader) {
  // Create a test header
  FrameHeader original;
  original.len_be = to_be32(42);
  original.type_be = to_be16(123);
  original.flags_be = to_be16(456);

  // Convert to bytes
  std::array<std::byte, 8> header_bytes;
  std::memcpy(header_bytes.data(), &original, sizeof(original));

  // Parse it back
  FrameHeader parsed = parse_header(header_bytes);

  EXPECT_EQ(parsed.len_be, original.len_be);
  EXPECT_EQ(parsed.type_be, original.type_be);
  EXPECT_EQ(parsed.flags_be, original.flags_be);
}

TEST_F(ProtocolTest, MakeFrameBasic) {
  // Create a simple ChatLine message
  ChatLine msg;
  msg.room = "test_room";
  msg.user = "test_user";
  msg.text = "Hello, world!";

  // Create frame
  auto frame = make_frame(msg);

  // Check frame size
  EXPECT_GE(frame.size(), sizeof(FrameHeader));

  // Parse header
  std::span<const std::byte, 8> header_span(frame.data(), 8);
  FrameHeader header = parse_header(header_span);

  // Check header values
  EXPECT_EQ(header_type(header), ChatLine::type_id);
  EXPECT_EQ(header_len(header), frame.size() - sizeof(FrameHeader));
  EXPECT_EQ(header_len(header),
            msg.room.size() + msg.user.size() + msg.text.size() + 12); // 3 * 4 bytes for lengths
}

TEST_F(ProtocolTest, MakeFrameEmptyMessage) {
  // Create an empty ChatLine message
  ChatLine msg;
  msg.room = "";
  msg.user = "";
  msg.text = "";

  // Create frame
  auto frame = make_frame(msg);

  // Check frame size
  EXPECT_EQ(frame.size(),
            sizeof(FrameHeader) + 12); // 3 * 4 bytes for empty string lengths

  // Parse header
  std::span<const std::byte, 8> header_span(frame.data(), 8);
  FrameHeader header = parse_header(header_span);

  // Check header values
  EXPECT_EQ(header_type(header), ChatLine::type_id);
  EXPECT_EQ(header_len(header), 12);
}

TEST_F(ProtocolTest, MakeFrameLargeMessage) {
  // Create a large ChatLine message
  ChatLine msg;
  msg.room = std::string(1000, 'A');
  msg.user = std::string(1000, 'B');
  msg.text = std::string(1000, 'C');

  // Create frame
  auto frame = make_frame(msg);

  // Check frame size
  EXPECT_EQ(frame.size(),
            sizeof(FrameHeader) + 12 + 3000); // 3 * 4 bytes for lengths + 3000 chars

  // Parse header
  std::span<const std::byte, 8> header_span(frame.data(), 8);
  FrameHeader header = parse_header(header_span);

  // Check header values
  EXPECT_EQ(header_type(header), ChatLine::type_id);
  EXPECT_EQ(header_len(header), 12 + 3000);
}

TEST_F(ProtocolTest, HeaderLenTypeHelpers) {
  FrameHeader header;

  // Test various lengths
  header.len_be = to_be32(0);
  EXPECT_EQ(header_len(header), 0);

  header.len_be = to_be32(1);
  EXPECT_EQ(header_len(header), 1);

  header.len_be = to_be32(0xFFFFFFFF);
  EXPECT_EQ(header_len(header), 0xFFFFFFFF);

  // Test various types
  header.type_be = to_be16(0);
  EXPECT_EQ(header_type(header), 0);

  header.type_be = to_be16(1);
  EXPECT_EQ(header_type(header), 1);

  header.type_be = to_be16(0xFFFF);
  EXPECT_EQ(header_type(header), 0xFFFF);
}

TEST_F(ProtocolTest, FrameRoundTrip) {
  // Create a ChatLine message
  ChatLine original;
  original.room = "lobby";
  original.user = "alice";
  original.text = "Hello, everyone!";

  // Create frame
  auto frame = make_frame(original);

  // Extract payload
  std::span<const std::byte> payload(frame.data() + sizeof(FrameHeader),
                                     frame.size() - sizeof(FrameHeader));

  // Deserialize message
  ChatLine deserialized = from_bytes<ChatLine>(payload);

  // Compare
  EXPECT_EQ(original.room, deserialized.room);
  EXPECT_EQ(original.user, deserialized.user);
  EXPECT_EQ(original.text, deserialized.text);
}

TEST_F(ProtocolTest, FrameWithSpecialCharacters) {
  // Test with special characters and unicode
  ChatLine msg;
  msg.room = "test-room_123";
  msg.user = "user@domain.com";
  msg.text = "Hello 世界! 🌍";

  auto frame = make_frame(msg);

  // Extract and deserialize
  std::span<const std::byte> payload(frame.data() + sizeof(FrameHeader),
                                     frame.size() - sizeof(FrameHeader));
  ChatLine deserialized = from_bytes<ChatLine>(payload);

  EXPECT_EQ(msg.room, deserialized.room);
  EXPECT_EQ(msg.user, deserialized.user);
  EXPECT_EQ(msg.text, deserialized.text);
}