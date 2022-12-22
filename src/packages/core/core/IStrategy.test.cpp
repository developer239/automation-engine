// TODO: review warnings and double check that everything works
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include "./IStrategy.h"

using namespace testing;
using namespace Core;

class MockWindow : public Window {
  public:
    MOCK_METHOD0(get, std::shared_ptr<SDL_Window>());
};

class MockRenderer : public Renderer {
  public:
    MockRenderer(const std::shared_ptr<SDL_Window>& window) : Renderer(window) {}
    MOCK_METHOD0(Render, void());
    MOCK_METHOD0(get, std::shared_ptr<SDL_Renderer>());
};

class MockStrategy : public IStrategy {
  public:
    MOCK_METHOD2(Init, void(Window& window, Renderer& renderer));
    MOCK_METHOD1(HandleEvent, void(SDL_Event& event));
    MOCK_METHOD2(OnRender, void(Window& window, Renderer& renderer));
};

TEST(IStrategyTest, TestInitMethod) {
  MockWindow mockWindow;
  MockRenderer mockRenderer(mockWindow.get());
  MockStrategy mockStrategy;

  EXPECT_CALL(mockStrategy, Init(Ref(mockWindow), Ref(mockRenderer)));

  mockStrategy.Init(mockWindow, mockRenderer);
}

TEST(IStrategyTest, TestHandleEventMethod) {
  MockStrategy mockStrategy;
  SDL_Event event{};

  EXPECT_CALL(mockStrategy, HandleEvent(Ref(event)));

  mockStrategy.HandleEvent(event);
}

TEST(IStrategyTest, TestOnRenderMethod) {
  MockWindow mockWindow;
  MockRenderer mockRenderer(mockWindow.get());
  MockStrategy mockStrategy;

  EXPECT_CALL(mockStrategy, OnRender(Ref(mockWindow), Ref(mockRenderer)));

  mockStrategy.OnRender(mockWindow, mockRenderer);
}
