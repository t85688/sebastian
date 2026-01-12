
#include <QtTest/QtTest>

#include "gmock/gmock.h"  // Brings in gMock.

using ::testing::_;

class Turtle {
  // virtual ~Turtle() {}
  virtual void PenUp() = 0;
  virtual void PenDown() = 0;
  virtual void Forward(int distance) = 0;
  virtual void Turn(int degrees) = 0;
  virtual void GoTo(int x, int y) = 0;
  virtual int GetX() const = 0;
  virtual int GetY() const = 0;
};

class MockTurtle : public Turtle {
 public:
  MOCK_METHOD(void, PenUp, (), (override));
  MOCK_METHOD(void, PenDown, (), (override));
  MOCK_METHOD(void, Forward, (int distance), (override));
  MOCK_METHOD(void, Turn, (int degrees), (override));
  MOCK_METHOD(void, GoTo, (int x, int y), (override));
  MOCK_METHOD(int, GetX, (), (const, override));
  MOCK_METHOD(int, GetY, (), (const, override));
};

TEST(PainterTest, CanDrawSomething) {
  testing::StrictMock<MockTurtle> turtle;                       // #2
  EXPECT_CALL(turtle, Forward(_)).Times(testing::AnyNumber());  // #1
  // EXPECT_CALL(turtle, Forward(_));                              // #1
  EXPECT_CALL(turtle, Forward(10))  // #2
      .Times(2);
  turtle.Forward(10);
  turtle.Forward(10);
  turtle.Forward(30);
}
// int main(int argc, char **argv) {
//   testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }