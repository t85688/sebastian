/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_OFFLINE_CONFIG_HPP
#define ACT_OFFLINE_CONFIG_HPP

#include <QDate>
#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QList>
#include <QProcess>
#include <QString>

#include "act_device.hpp"
#include "act_maf_client_handler.hpp"
#include "act_project.hpp"
#include "act_status.hpp"
#include "act_traffic.hpp"
#include "agents/act_moxa_iei_client_agent.hpp"
#include "client/act_moxa_iei_client.hpp"

namespace act {
namespace offline_config {

const QString INPUT_JSON_FILE = "./tmp/offline_config/offline_config_input.json";
const int EXECUTE_AF_TIMEOUT = 5000;  // (ms) 5 second

/**
 * @brief Generate device config by ActDeviceConfig, the generated config file will in tmp/device_config
 *
 * @param project
 * @param device_id
 * @param result_file_id
 * @return ACT_STATUS
 */
ACT_STATUS GenerateOfflineConfig(const ActProject &project, const qint64 &device_id, QString &result_file_id);

/**
 * @brief Remove the json file (for offline config tool used) to initialize
 *
 * @return ACT_STATUS
 */
ACT_STATUS RemoveJsonFile();
/**
 * @brief Generate the json file (for offline config tool used) by ActDeviceConfig in the project
 *
 * @param project
 * @param device_id
 * @param feature_list
 *
 * @return ACT_STATUS
 */
ACT_STATUS ConvertToJson(const ActProject &project, const qint64 &device_id, QJsonArray &feature_list);

/**
 * @brief Get formatted (1/x, 2/x) interface name map
 *
 * @param device
 * @return QMap<qint64, QString>
 */
QMap<qint64, QString> GetInterfaceNameMap(ActDevice &device);
/**
 * @brief Get formatted (1/x, 2/x) interface name map (map key is interface name)
 *
 * @param device
 * @return QMap<QString, QString>
 */
QMap<QString, QString> GetInterfaceNameMapFromString(ActDevice &device);
/**
 * @brief Generate a element section (one field of feature) and append to the list
 *
 * @param elemets
 * @param element_name
 * @param element_data_list
 */
void AddElementToList(QList<QJsonObject> &elements, const QString &element_name, const QJsonArray &element_data_list);

/**
 * @brief Generate a element section (one field of feature) and append to the list
 *
 * @param array
 * @param port_id
 * @param value
 */
void AppendElementData(QJsonArray &array, const QString &port_id, const QString &value);

/**
 * @brief Generate a element section (one field of feature) and append to the list
 *
 * @param elemets
 * @param element_name
 * @tparam values
 * @return QString
 */
template <typename... Args>
void AddSingleValueElementToList(QList<QJsonObject> &elements, const QString &element_name, Args &&...values) {
  QJsonArray values_list = ToQJsonArray(std::forward<Args>(values)...);

  QJsonObject element = CreateSingleValueElement(element_name, values_list);
  elements.push_back(element);
}

/**
 * @brief Convert arguments to QString
 *
 * @tparam args
 * @return QStringList
 */
template <typename... Args>
QJsonArray ToQJsonArray(Args &&...args) {
  QStringList list;
  (list.append(ToQString(std::forward<Args>(args))), ...);

  QJsonArray json_list;
  // QStringList -> QJsonArray
  for (auto item : list) {
    json_list.append(item);
  }
  return json_list;
}
/**
 * @brief Convert bool or value (int, double, ...) to QString
 *
 * @tparam T
 * @return QString
 */
template <typename T>
QString ToQString(const T &value) {
  if constexpr (std::is_same<T, bool>::value) {
    // is bool
    return value ? "enable" : "disable";
  } else if constexpr (std::is_same<T, QString>::value) {
    // is QString
    return value;
  } else {
    // is number - qint64, qint32
    return QString::number(value);
  }
}

/**
 * @brief Generate a element section (one field of feature)
 *
 * @param element_name
 * @param values
 * @return QJsonObject
 */
inline QJsonObject CreateSingleValueElement(const QString &element_name, const QJsonArray &values) {
  QJsonObject element;
  element["elementName"] = element_name;

  QJsonObject value_object;
  value_object["value"] = values;

  QJsonArray element_data_array;
  element_data_array.append(value_object);
  element["elementData"] = element_data_array;

  return element;
}

/**
 * @brief Generate feature section
 *
 * @param element_name
 * @param values
 * @return QJsonObject
 */
QJsonObject CreateFeature(const QString &feature_name, QList<QJsonObject> &element_data_list);
/**
 * @brief Save the json file (for offline config tool used)
 *
 * @param doc
 * @return ACT_STATUS
 */
ACT_STATUS SaveJsonToFile(const QJsonDocument &doc);

/**
 * @brief Generate API request for gen offline config MAF API
 *
 * @param device
 * @param api_request
 * @param snmp_setting
 * @return ACT_STATUS
 */
ACT_STATUS GenerateApiRequest(const ActDevice &device, MafGenOfflineConfigRequest &api_request,
                              QJsonObject &secret_setting);

/**
 * @brief Call AF API to generate the config file
 *
 * @param api_request
 * @param secret_setting
 * @param feature_list
 * @param file_id
 * @return ACT_STATUS
 */
ACT_STATUS GenerateConfigFileByAPI(const MafGenOfflineConfigRequest &api_request, QJsonObject &secret_setting,
                                   const QJsonArray &feature_list, QString &file_id);

/**
 * @brief save offline config (.zip) to tmp folder
 *
 * @param dev_id_list
 * @return ACT_STATUS
 */
ACT_STATUS SaveOfflineConfigToTmpFolder(const QList<qint64> &dev_id_list);

/**
 * @brief clear MAF file DB
 *
 * @return ACT_STATUS
 */
ACT_STATUS ClearMafFileDb();

static QString GetInputJsonFilePath() {
  // INPUT_JSON_FILE
  QString tempPath = qEnvironmentVariable("CHAMBERLAIN_COGSWORTH_TMP_FOLDER", "tmp");
  return tempPath + QDir::separator() + "offline_config" + QDir::separator() + "offline_config_input.json";
}

// features

static const QMap<ActSnmpTrapModeEnum, QString> kSnmpTrapModeConfigStringEnum = {
    {ActSnmpTrapModeEnum::kTrapV1, "trap-v1"},
    {ActSnmpTrapModeEnum::kTrapV2c, "trap-v2c"},
    {ActSnmpTrapModeEnum::kInformV2c, "inform-v2c"},
    {ActSnmpTrapModeEnum::kNotSupport, "not-support"}};

static const QMap<ActClockSourceEnum, QString> kClockSourceConfigStringEnum = {{ActClockSourceEnum::kLocal, "local"},
                                                                               {ActClockSourceEnum::kSNTP, "sntp"},
                                                                               {ActClockSourceEnum::kNTP, "ntp"},
                                                                               {ActClockSourceEnum::kPTP, "ptp"}};

static const QMap<ActTimeZoneEnum, QString> kTimeZoneConfigStringEnum = {
    {ActTimeZoneEnum::kIDLW, "-12"},  {ActTimeZoneEnum::kSST, "-11"},     {ActTimeZoneEnum::kHST, "-10"},
    {ActTimeZoneEnum::kMIT, "-9:30"}, {ActTimeZoneEnum::kAKST, "-9"},     {ActTimeZoneEnum::kPST, "-8"},
    {ActTimeZoneEnum::kMST, "-7"},    {ActTimeZoneEnum::kCST, "-6"},      {ActTimeZoneEnum::kEST, "-5"},
    {ActTimeZoneEnum::kAST, "-4"},    {ActTimeZoneEnum::kNST, "-3:30"},   {ActTimeZoneEnum::kBRT, "-3"},
    {ActTimeZoneEnum::kFNT, "-2"},    {ActTimeZoneEnum::kCVT, "-1"},      {ActTimeZoneEnum::kGMT, "0"},
    {ActTimeZoneEnum::kCET, "1"},     {ActTimeZoneEnum::kEET, "2"},       {ActTimeZoneEnum::kMSK, "3"},
    {ActTimeZoneEnum::kIRST, "3:30"}, {ActTimeZoneEnum::kGST, "4"},       {ActTimeZoneEnum::kAFT, "4:30"},
    {ActTimeZoneEnum::kPKT, "5"},     {ActTimeZoneEnum::kIST, "5:30"},    {ActTimeZoneEnum::kNPT, "5:45"},
    {ActTimeZoneEnum::kBHT, "6"},     {ActTimeZoneEnum::kMMT, "6:30"},    {ActTimeZoneEnum::kICT, "7"},
    {ActTimeZoneEnum::kBJT, "8"},     {ActTimeZoneEnum::kJST, "9"},       {ActTimeZoneEnum::kACST, "9:30"},
    {ActTimeZoneEnum::kAEST, "10"},   {ActTimeZoneEnum::kLHST, "10:30"},  {ActTimeZoneEnum::kVUT, "11"},
    {ActTimeZoneEnum::kNZST, "12"},   {ActTimeZoneEnum::kCHAST, "12:45"}, {ActTimeZoneEnum::kPHOT, "13"},
    {ActTimeZoneEnum::kLINT, "14"}};

static const QMap<quint16, QString> kTimeWeekConfigStringEnum = {
    {1, "1st"}, {2, "2nd"}, {3, "3rd"}, {4, "4th"}, {5, "last"}};

static const QMap<ActVlanPortTypeEnum, QString> kVlanPortTypeEnum = {{ActVlanPortTypeEnum::kAccess, "access"},
                                                                     {ActVlanPortTypeEnum::kTrunk, "trunk"},
                                                                     {ActVlanPortTypeEnum::kHybrid, "hybrid"}};

static const QMap<ActStreamPriorityTypeEnum, QString> kStreamPriorityTypeEnum = {
    {ActStreamPriorityTypeEnum::kInactive, "inactive"},
    {ActStreamPriorityTypeEnum::kEthertype, "ethertype"},
    {ActStreamPriorityTypeEnum::kTcp, "tcp"},
    {ActStreamPriorityTypeEnum::kUdp, "udp"}};

static const QMap<ActSpanningTreeVersionEnum, QString> kSpanningTreeVersionEnum = {
    {ActSpanningTreeVersionEnum::kSTP, "stp"},
    {ActSpanningTreeVersionEnum::kRSTP, "rstp"},
    {ActSpanningTreeVersionEnum::kNotSupport, "not-support"}};

static const QMap<ActRstpLinkTypeEnum, QString> kSpanningTreeLinkTypeEnum = {
    {ActRstpLinkTypeEnum::kPointToPoint, "point-to-point"},
    {ActRstpLinkTypeEnum::kShared, "shared"},
    {ActRstpLinkTypeEnum::kAuto, "auto"}};

static const QMap<ActIdentificationType, QString> kIdentificationTypeEnum = {
    {ActIdentificationType::kNull_stream_identification, "null"},
    {ActIdentificationType::kDmac_vlan_stream_identification, "dmac-vlan"}};

static const QMap<Act1588ClockTypeEnum, QString> k1588ClockTypeEnum = {
    {Act1588ClockTypeEnum::kBoundaryClock, "boundary"}, {Act1588ClockTypeEnum::kTransparentClock, "transparent"}};

static const QMap<Act1588DelayMechanismEnum, QString> k1588DelayMechanismTypeEnum = {
    {Act1588DelayMechanismEnum::kEndToEnd, "e2e"}, {Act1588DelayMechanismEnum::kPeerToPeer, "p2p"}};

static const QMap<Act1588TransportTypeEnum, QString> k1588TransportTypeEnum = {
    {Act1588TransportTypeEnum::kUDPIPv4, "ipv4"}, {Act1588TransportTypeEnum::kIEEE802Dot3Ethernet, "ethernet"}};

static const QMap<ActMgmtSnmpServiceModeEnum, QString> kSnmpServiceModeEnum = {
    {ActMgmtSnmpServiceModeEnum::kEnabled, "enable"},
    {ActMgmtSnmpServiceModeEnum::kDisabled, "disable"},
    {ActMgmtSnmpServiceModeEnum::kReadOnly, "read-only"}};

static const QMap<ActMgmtSnmpServiceTransLayerProtoEnum, QString> kSnmpTransLayerEnum = {
    {ActMgmtSnmpServiceTransLayerProtoEnum::kUDP, "udp"}, {ActMgmtSnmpServiceTransLayerProtoEnum::kTCP, "tcp"}};

static const QMap<ActUserAccountRoleEnum, QString> kUserAccountRoleEnum = {
    {ActUserAccountRoleEnum::kAdmin, "admin"},
    {ActUserAccountRoleEnum::kSupervisor, "supervisor"},
    {ActUserAccountRoleEnum::kUser, "user"}};

ACT_STATUS GenL2NetworkSetting(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenLoginPolicy(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenLoopProtection(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenSNMPTrapSetting(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenSyslogSetting(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenNosTimeSetting(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenTsnTimeSetting(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenInformationSetting(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenPortSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                          const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenNosVlanSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                             const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenTsnVlanSetting(const bool supportHybrid, const ActDeviceConfig &device_config, const qint64 &device_id,
                             const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenStaticForwardSetting(const ActDeviceConfig &device_config, const qint64 &device_id,
                                   const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenPortDefaultPriority(const ActDeviceConfig &device_config, const qint64 &device_id,
                                  const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenStreamAdapterV1(const ActDeviceConfig &device_config, const qint64 &device_id,
                              const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenStreamAdapterV2(const ActDeviceConfig &device_config, const qint64 &device_id,
                              const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenGCL(const ActDeviceConfig &device_config, const qint64 &device_id,
                  const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenCB(const ActDeviceConfig &device_config, const qint64 &device_id,
                 const QMap<QString, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenNosRstp(const ActDeviceConfig &device_config, const qint64 &device_id,
                      const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenTsnRstp(const ActDeviceConfig &device_config, const qint64 &device_id,
                      const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
ACT_STATUS GenTimeSync(const ActTimeSyncSettingItem &feature_group, const ActDeviceConfig &device_config,
                       const qint64 &device_id, const QMap<qint64, QString> &interface_names, QJsonArray *root_array);
void HandleTimeSyncAS(const QMap<qint64, QString> &interface_names, ActTimeSyncProfileEnum profile,
                      ActTimeSync802Dot1ASConfig &as, QList<QJsonObject> *elements);
void HandleTimeSyncPTP2008(const ActTimeSyncSettingItem &feature_group, const QMap<qint64, QString> &interface_names,
                           ActTimeSyncProfileEnum profile, ActTimeSync1588Config &ptp, QList<QJsonObject> *elements);
void HandleTimeSync61850(const QMap<qint64, QString> &interface_names, ActTimeSyncProfileEnum profile,
                         ActTimeSyncIec61850Config &power_profile_61850, QList<QJsonObject> *elements);
void HandleTimeSyncC37(const QMap<qint64, QString> &interface_names, ActTimeSyncProfileEnum profile,
                       ActTimeSyncC37Dot238Config &power_profile_c37, QList<QJsonObject> *elements);
ACT_STATUS GenMgmtInterface(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);
ACT_STATUS GenUserAccount(const ActDeviceConfig &device_config, const qint64 &device_id, QJsonArray *root_array);

}  // namespace offline_config
}  // namespace act
#endif  // ACT_OFFLINE_CONFIG_HPP