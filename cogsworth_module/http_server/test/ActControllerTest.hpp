#ifndef ActControllerTest_hpp
#define ActControllerTest_hpp

#include "oatpp-test/UnitTest.hpp"

class ActControllerTest : public oatpp::test::UnitTest {
 public:
  ActControllerTest() : UnitTest("TEST[ActControllerTest]") {}
  void onRun() override;
};

#endif  // ActControllerTest_hpp
