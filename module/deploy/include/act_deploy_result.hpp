/* Copyright (C) MOXA Inc. All rights reserved.
 This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
 See the file MOXA-SOFTWARE-NOTICE for details.
 */

#ifndef ACT_DEPLOY_RESULT_HPP
#define ACT_DEPLOY_RESULT_HPP
#include "act_json.hpp"

namespace act {
namespace deploy {

// /**
//  * @brief The Deploy result status enum class
//  *
//  */
// enum class DeployResultStatus {
//   kUndefined,
//   kDeployFail,
//   kDeploySuccess,
// };

// /**
//  * @brief The QMap for Deploy result status enum mapping
//  *
//  */
// static const QMap<QString, DeployResultStatus> kDeployResultStatusMap = {
//     {"Undefined", DeployResultStatus::kUndefined},
//     {"DeployFail", DeployResultStatus::kDeployFail},
//     {"DeploySuccess", DeployResultStatus::kDeploySuccess}};

// /**
//  * @brief The Deploy error code enum class
//  *
//  */
// enum class DeployErrorCode { kDeployFail1, kDeployFail2 };

// /**
//  * @brief The QMap for  Deploy error code enum mapping
//  *
//  */
// static const QMap<QString, DeployErrorCode> kDeployErrorCodeMap = {{"DeployFail1", DeployErrorCode::kDeployFail1},
//                                                                    {"DeployFail2", DeployErrorCode::kDeployFail2}};

/**
 * @brief The Deploy result template derives from a variety of result types
 *
 * @tparam T The derives class
 */
template <typename T>
class ActDeployResult : public T {
 public:
  ActDeployResult() : T() {};
  ActDeployResult(QString device_ip) : T(device_ip) {};
};

/**
 * @brief The Deploy result base class
 *
 */
class ActDeployResultBase : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_ip, DeviceIp);  ///< The device IP
  // ACT_JSON_FIELD(QString, result_body, ResultBody);  ///< The qstring result body
  // ACT_JSON_FIELD(QString, error_message, ErrorMessage);            ///< The qstring error message
  // ACT_JSON_ENUM(DeployErrorCode, error_code, ErrorCode);           ///< The error code
  // ACT_JSON_ENUM(DeployResultStatus, result_status, ResultStatus);  ///< The enum result status

 public:
  /**
   * @brief Construct a new Deploy Result object
   *
   */
  // DeployResultBase() : error_code_(DeployErrorCode::kDeployFail1), result_status_(DeployResultStatus::kUndefined) {}
  // DeployResultBase() : result_status_(DeployResultStatus::kUndefined) {}
  ActDeployResultBase() {}

  /**
   * @brief Construct a new Deploy Result Base object
   *
   * @param device_ip The device IP
   */
  ActDeployResultBase(const QString &device_ip) : device_ip_(device_ip) {}
};

class ActTypeQSetGateControl : public ActDeployResultBase {
  Q_GADGET
  QS_SERIALIZABLE

  // ACT_JSON_QT_SET_OBJECTS(PortGateParameters, result_body, ResultBody);  ///< The qset result body

 public:
  ActTypeQSetGateControl(const QString &device_ip) : ActDeployResultBase(device_ip) {};
};
}  // namespace deploy
}  // namespace act
#endif /* ACT_DEPLOY_RESULT_HPP */
