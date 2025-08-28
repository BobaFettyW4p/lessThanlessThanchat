#include <gtest/gtest.h>

TEST(SimpleTest, BasicAssertion) {
    EXPECT_EQ(1, 1);
    EXPECT_TRUE(true);
    EXPECT_FALSE(false);
}

TEST(SimpleTest, MathOperations) {
    EXPECT_EQ(2 + 2, 4);
    EXPECT_NE(3, 4);
    EXPECT_LT(1, 2);
    EXPECT_GT(3, 2);
}

TEST(SimpleTest, StringOperations) {
    std::string hello = "Hello";
    std::string world = "World";
    std::string combined = hello + " " + world;
    
    EXPECT_EQ(combined, "Hello World");
    EXPECT_NE(combined, "Hello");
}