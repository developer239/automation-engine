#include "Keyboard.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MockKeyboard : public Keyboard {
 public:
  MOCK_METHOD(void, Click, (char keyASCII));
  MOCK_METHOD(void, Type, (const std::string& query));
};

TEST(KeyboardTest, TypeSimulatesTypingEachCharacterInString) {
  MockKeyboard mockKeyboard;
  std::string query = "hello world";
  std::vector<int> expectedQuery = {4, 14, 37, 37, 31, 49, 13, 31, 15, 37, 2};

  // Note: That this is not ideal because we are mocking Click method and
  // PressAndRelease is not tested at all.
  std::vector<int> typedCharacters;
  EXPECT_CALL(mockKeyboard, Click(::testing::_))
      .WillRepeatedly(::testing::Invoke([&](char c) {
        int key = MockKeyboard::MapASCIIToVirtualKey(c);
        typedCharacters.push_back(key);
      }));

  // Note: That this is even worse because we are overriding how Type method
  // works.
  EXPECT_CALL(mockKeyboard, Type(::testing::_))
      .WillRepeatedly(::testing::Invoke([&](const std::string& query) {
        for (char c : query) {
          mockKeyboard.Click(c);
        }
      }));

  mockKeyboard.Type(query);

  // Technically this is basically glorified MapASCIIToVirtualKey test. 😅
  EXPECT_EQ(typedCharacters, expectedQuery);
}
