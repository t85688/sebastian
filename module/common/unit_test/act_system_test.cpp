#include "act_system.hpp"

#include <QtTest/QtTest>

#include "act_unit_test.hpp"

// We derive a fixture named IntegerFunctionTest from the ActQuickTest
// fixture.  All tests using this fixture will be automatically
// required to be quick.
class ActSystemTest : public ActQuickTest {
 protected:  // You should make the members protected s.t. they can be
             // accessed from sub-classes.
  // virtual void SetUp() will be called before each test is run.  You
  // should define it if you need to initialize the variables.
  // Otherwise, this can be skipped.
  // void SetUp() override {}

  // virtual void TearDown() will be called after each test is run.
  // You should define it if there is cleanup work to do.  Otherwise,
  // you don't have to provide it.
  // void TearDown() override {}

  // Declares the variables your tests want to use.
  ActSystem dummy;
};

// When you have a test fixture, you define a test using TEST_F
// instead of TEST.

// Demonstrate some basic assertions.
TEST_F(ActSystemTest, TestActSystemConstructor) {
  EXPECT_FALSE(dummy.GetOpcua());
  EXPECT_FALSE(dummy.GetHttps());
}

TEST_F(ActSystemTest, TestToString) {
  QString str = dummy.ToString();
  EXPECT_NE(0, str.length());
}

// int main(int argc, char **argv) {
//   testing::InitGoogleTest(&argc, argv);
//   return RUN_ALL_TESTS();
// }