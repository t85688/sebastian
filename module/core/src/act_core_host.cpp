
#include <QNetworkInterface>

#include "act_core.hpp"

namespace act {
namespace core {

ACT_STATUS ActCore::GetHostAdapters(ActHostAdapterList &adapter_list) {
  ACT_STATUS_INIT();

  adapter_list.GetAdapters().clear();
  try {
    // QNetworkInterface
    // ref: https://www.cnblogs.com/liushui-sky/p/6479110.html
    // ref:
    // https://stackoverflow.com/questions/5572263/get-current-qnetworkinterface-active-and-connected-to-the-internet
    const QList<QNetworkInterface> netinterfaces = QNetworkInterface::allInterfaces();

    for (auto netinterface : netinterfaces) {
      if (!netinterface.isValid()) {  // check valid
        continue;
      }

      QNetworkInterface::InterfaceFlags flags = netinterface.flags();
      if ((!flags.testFlag(QNetworkInterface::IsRunning)) ||
          flags.testFlag(QNetworkInterface::IsLoopBack)) {  // check flags(IsRunning & not LoopBack)
        continue;
      }

      if (netinterface.type() != QNetworkInterface::Ethernet) {  // check Ethernet type
        continue;
      }

      for (auto entry : netinterface.addressEntries()) {
        if (entry.ip() == QHostAddress::LocalHost) {  // check not local host
          continue;
        }

        if (entry.ip().protocol() != QAbstractSocket::IPv4Protocol) {  // check is ipv4 addresses
          continue;
        }
        // Insert entry
        ActHostAdapter host_adapter;
        // host_adapter.SetAdapterName(netinterface.name());
        host_adapter.SetAdapterName(netinterface.humanReadableName());
        host_adapter.SetIpAddress(entry.ip().toString());
        host_adapter.SetNetMask(entry.netmask().toString());
        adapter_list.GetAdapters().append(host_adapter);
      }
    }
  } catch (std::exception &e) {
    qCritical() << __func__ << "Get Host Adapter failed. Error:" << e.what();
    return std::make_shared<ActStatusInternalError>("Core");
  }

  return act_status;
}

}  // namespace core
}  // namespace act
