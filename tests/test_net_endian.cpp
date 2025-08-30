#include "../common/net_endian.hpp"
#include <gtest/gtest.h>

class NetEndianTest : public ::testing::Test {
protected:
  void SetUp() override {}
  void TearDown() override {}
};

TEST_F(NetEndianTest, ToBe16RoundTrip) {
  // Test that to_be16 and from_be16 are inverse operations
  uint16_t original = 0x1234;
  uint16_t converted = to_be16(original);
  uint16_t back = from_be16(converted);

  EXPECT_EQ(original, back);
}

TEST_F(NetEndianTest, ToBe32RoundTrip) {
  // Test that to_be32 and from_be32 are inverse operations
  uint32_t original = 0x12345678;
  uint32_t converted = to_be32(original);
  uint32_t back = from_be32(converted);

  EXPECT_EQ(original, back);
}

TEST_F(NetEndianTest, ToBe16KnownValues) {
  // Test specific known values
  EXPECT_EQ(to_be16(0x1234), 0x3412);
  EXPECT_EQ(to_be16(0x0001), 0x0100);
  EXPECT_EQ(to_be16(0x00FF), 0xFF00);
  EXPECT_EQ(to_be16(0xFFFF), 0xFFFF);
}

TEST_F(NetEndianTest, ToBe32KnownValues) {
  // Test specific known values
  EXPECT_EQ(to_be32(0x12345678), 0x78563412);
  EXPECT_EQ(to_be32(0x00000001), 0x01000000);
  EXPECT_EQ(to_be32(0x000000FF), 0xFF000000);
  EXPECT_EQ(to_be32(0xFFFFFFFF), 0xFFFFFFFF);
}

TEST_F(NetEndianTest, FromBe16KnownValues) {
  // Test specific known values
  EXPECT_EQ(from_be16(0x3412), 0x1234);
  EXPECT_EQ(from_be16(0x0100), 0x0001);
  EXPECT_EQ(from_be16(0xFF00), 0x00FF);
  EXPECT_EQ(from_be16(0xFFFF), 0xFFFF);
}

TEST_F(NetEndianTest, FromBe32KnownValues) {
  // Test specific known values
  EXPECT_EQ(from_be32(0x78563412), 0x12345678);
  EXPECT_EQ(from_be32(0x01000000), 0x00000001);
  EXPECT_EQ(from_be32(0xFF000000), 0x000000FF);
  EXPECT_EQ(from_be32(0xFFFFFFFF), 0xFFFFFFFF);
}

TEST_F(NetEndianTest, ZeroValues) {
  // Test zero values
  EXPECT_EQ(to_be16(0), 0);
  EXPECT_EQ(from_be16(0), 0);
  EXPECT_EQ(to_be32(0), 0);
  EXPECT_EQ(from_be32(0), 0);
}

TEST_F(NetEndianTest, MaxValues) {
  // Test maximum values
  EXPECT_EQ(to_be16(0xFFFF), 0xFFFF);
  EXPECT_EQ(from_be16(0xFFFF), 0xFFFF);
  EXPECT_EQ(to_be32(0xFFFFFFFF), 0xFFFFFFFF);
  EXPECT_EQ(from_be32(0xFFFFFFFF), 0xFFFFFFFF);
}