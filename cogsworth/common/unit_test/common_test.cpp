// // #include <gtest/gtest.h>
// #include <iostream>

// #include "act_unit_test.hpp"

// // We derive a fixture named IntegerFunctionTest from the QuickTest
// // fixture.  All tests using this fixture will be automatically
// // required to be quick.
// class IntegerFunctionTest : public ActQuickTest {
//   // We don't need any more logic than already in the QuickTest fixture.
//   // Therefore the body is empty.
// };

// // Demonstrate some basic assertions.
// TEST_F(IntegerFunctionTest, InputIsZero) {
//   this->SetExpireTime(0);
//   // Expect two strings not to be equal.
//   EXPECT_STRNE("hello", "world");

//   // Expect equality.
//   EXPECT_EQ(7 * 6, 42);
// }

// TEST_F(IntegerFunctionTest, InputIsPositive) {
//   // Expect two strings not to be equal.

//   // Expect equality.
//   EXPECT_EQ(7 * 6, 42);
// }

// TEST_F(IntegerFunctionTest, BasicAssertions) {
//   // Expect two strings not to be equal.
//   EXPECT_STRNE("hello", "world");

//   // Expect equality.
//   EXPECT_EQ(7 * 6, 42);
// }

// // int main(int argc, char **argv) {
// //   testing::InitGoogleTest(&argc, argv);
// //   return RUN_ALL_TESTS();
// // }

#include <QString>
#include <QtTest/QtTest>
//

class TestQString : public QObject {
  Q_OBJECT
 private slots:
  void toUpper();
};

void TestQString::toUpper() {
  QString str = "Hello";
  QVERIFY(str.toUpper() == "HELLO");
}

QTEST_MAIN(TestQString)
#include "common_test.moc"
