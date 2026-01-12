
#include "act_snmp_handler.h"

#ifdef _WIN32
#include <windows.h>
#define SLEEP_MS(ms) Sleep(ms)
#else
#include <unistd.h>  // for sleep
#define SLEEP_MS(ms) sleep(ms / 1000)
#endif

// #include "act_snmp_result.hpp"
// #include "act_snmp_trap_server.h"
// #include "act_status.hpp"
// #include "act_system.hpp"

// #include "act_unit_test.hpp"

// class ActSnmpTest : public ActQuickTest {
//  protected:
//   ACT_STATUS_INIT();

//   void SetUp() override {}
//   void TearDown() override {}
// };
// TEST_F(ActSnmpTest, ActSnmpTest1) { EXPECT_EQ(1, 1); }

int main(void) {
  ActSnmpHandler snmp_handler;
  snmp_handler.StartSnmpTrapReceiver();

  // SnmpTrapReceiver receiver;
  // receiver.start();

  qint8 times = 0;
  while ((times < 120)) {
    times++;
    SLEEP_MS(2000);
    qDebug() << "test times:" << times;

    // act_status = monitor.GetStatus();
    // qDebug() << QString("act_status(%1)::%2")
    //                 .arg(QTime::currentTime().toString("hh:mm:ss"))
    //                 .arg(act_status->ToString())
    //                 .toStdString()
    //                 .c_str();

    // qDebug() << "Current Queue size(in queue):" << monitor.result_queue_.size();
    // while (!monitor.result_queue_.isEmpty()) {
    //   auto queue_result_status = monitor.result_queue_.dequeue();
    //   qDebug() << queue_result_status.ToString().toStdString().c_str();
    //   qDebug() << "Current Queue size(dequeue after):" << monitor.result_queue_.size();
    // }
  }

  return 0;
}
