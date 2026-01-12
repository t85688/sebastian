/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_NEW_MOXA_COMMAND_HANDLER_H
#define ACT_NEW_MOXA_COMMAND_HANDLER_H

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <Windows.h>
#include <iphlpapi.h>
#include <winsock2.h>
#else
#include <arpa/inet.h>
#include <netinet/in.h>  // IP related structs
#endif

#include <QString>

#include "act_feature_profile.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "deploy_entry/act_deploy_table.hpp"
#include "topology/act_device.hpp"

extern "C" {
#include "ieidll.h"
#include "moxa_cmd_defines.h"
#include "moxa_device_config.h"

// typedef struct MOXA_IEI_DEV_S {
//   uint32_t ip;
//   char modelname[MODEL_LENGTH + 1];
//   char mac[RAW_MAC_LENGTH + 1];
//   uint32_t host;
//   char serial[SERIAL_LENGTH + 1];
//   char firmware_version[FIRMVER_LENGTH + 1];

//  uint32_t netmask;
//  uint32_t gateway;
//  uint32_t dns1;
//  uint32_t dns2;

//  uint8_t firm_svrflag;  // password flag, bit 0,0 --> No password set
//  // bit 0,1 --> password is set
//  // bit 1 --> Installed
//  // bit 2 --> IP conflict
//  uint32_t ipflag;
//  uint16_t config_port;
//  char name[SVRNAME_LENGTH + 1];
//  char location[SVRLOCATION_LENGTH + 1];

//  struct MOXA_IEI_DEV_S *next;
//} MOXA_IEI_DEV_T;

// typedef int (*SCAN_CALLBACK)(MOXA_IEI_DEV_T *device);
// IEIDLL_API int dll_search_device_by_network_interfaces(SCAN_CALLBACK callback, unsigned int *ip_list, int count,
//                                                        int port, int timeout);
// IEIDLL_API int dll_set_arp_table(char *ip, char *mac, char *host);
// IEIDLL_API int dll_delete_arp_table(char *ip);
// IEIDLL_API int dll_verify_password(char *ip, int port, char *mac, char *host, char *username, char *password,
//                                    int timeout);
}

class ActNewMoxaCommandHandler {
 private:
  // QMap<QString, QString> mac_host_map_;

  /**
   * @brief Get the NewMoxaCommand error message
   *
   * @param err_num
   * @param err_msg
   * @return ACT_STATUS
   */
  ACT_STATUS GetNewMoxaCmdErrMsg(const int &err_num, QString &err_msg);

  /**
   * @brief Update private member mac_host_map_
   *
   * @return ACT_STATUS
   */
  ACT_STATUS UpdateMacHostMap();

 public:
  /**
   * @brief NewMoxaCommand's search_devices command
   *
   * @param action_key
   * @param protocol_elem
   * @param dev_list
   * @param mac_host_map
   * @return ACT_STATUS
   */
  ACT_STATUS SearchDevices(const QString &action_key, const ActFeatureMethodProtocol &protocol_elem,
                           QList<ActDevice> &dev_list, QMap<QString, QString> &mac_host_map);

  /**
   * @brief NewMoxaCommand's set Arp Table command
   *
   * @param dev_ip
   * @param dev_mac
   * @param act_host_ip
   * @return ACT_STATUS
   */
  ACT_STATUS SetArpTable(const ActDevice &device, const QString &act_host_ip);

  /**
   * @brief NewMoxaCommand's delete Arp entry command
   *
   * @param device
   * @return ACT_STATUS
   */
  ACT_STATUS DeleteArpEntry(const ActDevice &device);

  /**
   * @brief NewMoxaCommand's UpgradeFirmware command
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS UpgradeFirmware(const ActDevice &device, const QString &action_key,
                             const ActFeatureMethodProtocol &protocol_elem, const QString &file_path);

  /**
   * @brief NewMoxaCommand's change NetworkSettings command
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param network_setting_table
   * @return ACT_STATUS
   */
  ACT_STATUS ChangeNetworkSettings(const ActDevice &device, const QString &action_key,
                                   const ActFeatureMethodProtocol &protocol_elem,
                                   const ActNetworkSettingTable &network_setting_table);

  /**
   * @brief NewMoxaCommand's verify account command
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @return ACT_STATUS
   */
  ACT_STATUS VerifyAccount(const ActDevice &device, const QString &action_key,
                           const ActFeatureMethodProtocol &protocol_elem);

  /**
   * @brief NewMoxaCommand's ExportConfig command
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS ExportConfig(const ActDevice &device, const QString &action_key,
                          const ActFeatureMethodProtocol &protocol_elem, const QString &file_path);

  /**
   * @brief NewMoxaCommand's ImportConfig command
   *
   * @param device
   * @param action_key
   * @param protocol_elem
   * @param file_path
   * @return ACT_STATUS
   */
  ACT_STATUS ImportConfig(const ActDevice &device, const QString &action_key,
                          const ActFeatureMethodProtocol &protocol_elem, const QString &file_path);
};

#endif /* ACT_NEW_MOXA_COMMAND_HANDLER_H */
