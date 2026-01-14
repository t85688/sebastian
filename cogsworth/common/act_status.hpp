/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <memory>

#include "act_device_event_log.hpp"
#include "act_json.hpp"

#define ACT_STATUS std::shared_ptr<ActStatusBase>
#define ACT_STATUS_INIT() ACT_STATUS act_status = std::make_shared<ActStatusBase>()
#define ACT_STATUS_STOP std::make_shared<ActStatusBase>(ActStatusType::kStop)
#define ACT_STATUS_SUCCESS std::make_shared<ActStatusBase>(ActStatusType::kSuccess)

#define ACT_SKIP (1)
#define ACT_SUCCESS (0)
#define ACT_FAILED (-1)

/**
 * @brief The ACT status type
 *
 */
enum class ActStatusType {

  kSuccess = 200,
  kCreated = 201,
  kNoContent = 204,
  kBadRequest = 400,
  kUnauthorized = 401,
  kForbidden = 403,
  kNotFound = 404,
  kConflict = 409,
  kUnProcessable = 422,
  kInternalError = 500,
  kServiceUnavailable = 503,

  kSkip = 1002,
  kRunning = 1003,
  kStop = 1004,
  kFinished = 1005,
  kFailed = 1006,
  kDuplicated = 1007,
  kFull = 1008,

  kLicenseContentFailed = 1100,
  kLicenseSizeFailed = 1101,
  kLicenseNotActive = 1102,
  kLicenseNotSupport = 1103,

  kDeviceProfileIsUsedFailed = 1200,

  kRoutingDestinationUnreachable = 2000,
  kPcpInsufficientForTimeSlot = 2001,
  kTimeSyncPcpNotConsistentWithDevice = 2002,
  kRoutingDeviceTypeIncapable = 2003,

  kSchedulingFailed = 2100,
  kFeasibilityCheckFailed = 2101,
  kCalculateTimeout = 2200,

  kSetConfigFailed = 3000,
  kGetDeviceDataFailed = 3002,

  kCompareFailed = 3003,
  kCompareTopologyFailed = 3004,
  kDeployFailed = 3005,
  kSyncFailed = 3006,
  kCheckFeatureFailed = 3007,

  kSouthboundFailed = 4000,
  kAutoScanFailed = 5000,
  kUpdateProjectTopologyFailed = 5001,

  kServicePlatformUnauthorized = 6001,

  kOpcUaErrorCodeStart = 0x8000,
  kOpcUaErrorCodeEnd = 0x8fff
};

/**
 * @brief The mapping table for ActStatusType
 *
 */
static const QMap<QString, ActStatusType> kActStatusTypeMap = {

    {"Success", ActStatusType::kSuccess},
    {"Created", ActStatusType::kCreated},
    {"NoContent", ActStatusType::kNoContent},
    {"BadRequest", ActStatusType::kBadRequest},
    {"Unauthorized", ActStatusType::kUnauthorized},
    {"Forbidden", ActStatusType::kForbidden},
    {"NotFound", ActStatusType::kNotFound},
    {"Conflict", ActStatusType::kConflict},
    {"UnProcessable", ActStatusType::kUnProcessable},
    {"InternalError", ActStatusType::kInternalError},
    {"ServiceUnavailable", ActStatusType::kServiceUnavailable},

    {"Skip", ActStatusType::kSkip},
    {"Running", ActStatusType::kRunning},
    {"Stop", ActStatusType::kStop},
    {"Finished", ActStatusType::kFinished},
    {"Failed", ActStatusType::kFailed},
    {"Duplicated", ActStatusType::kDuplicated},
    {"Full", ActStatusType::kFull},

    {"LicenseContentFailed", ActStatusType::kLicenseContentFailed},
    {"LicenseSizeFailed", ActStatusType::kLicenseSizeFailed},
    {"LicenseNotActive", ActStatusType::kLicenseNotActive},
    {"LicenseNotSupport", ActStatusType::kLicenseNotSupport},

    {"DeviceProfileIsUsedFailed", ActStatusType::kDeviceProfileIsUsedFailed},

    {"RoutingDestinationUnreachable", ActStatusType::kRoutingDestinationUnreachable},
    {"SchedulingFailed", ActStatusType::kSchedulingFailed},
    {"PcpInsufficientForTimeSlot", ActStatusType::kPcpInsufficientForTimeSlot},
    {"RoutingDeviceTypeIncapable", ActStatusType::kRoutingDeviceTypeIncapable},
    {"TimeSyncPcpNotConsistentWithDevice", ActStatusType::kTimeSyncPcpNotConsistentWithDevice},
    {"FeasibilityCheckFailed", ActStatusType::kFeasibilityCheckFailed},
    {"CalculateTimeout", ActStatusType::kCalculateTimeout},

    {"SetConfigFailed", ActStatusType::kSetConfigFailed},
    {"GetDeviceDataFailed", ActStatusType::kGetDeviceDataFailed},

    {"CompareFailed", ActStatusType::kCompareFailed},
    {"CompareTopologyFailed", ActStatusType::kCompareTopologyFailed},
    {"DeployFailed", ActStatusType::kDeployFailed},
    {"SyncFailed", ActStatusType::kSyncFailed},
    {"CheckFeatureFailed", ActStatusType::kCheckFeatureFailed},

    {"SouthboundFailed", ActStatusType::kSouthboundFailed},
    {"AutoScanFailed", ActStatusType::kAutoScanFailed},
    {"UpdateProjectTopologyFailed", ActStatusType::kUpdateProjectTopologyFailed},

    {"ServicePlatformUnauthorized", ActStatusType::kServicePlatformUnauthorized},

    {"OpcUaErrorCodeStart", ActStatusType::kOpcUaErrorCodeStart},
    {"OpcUaErrorCodeEnd", ActStatusType::kOpcUaErrorCodeEnd}};

/**
 * @brief The severity of ACT Status
 *
 */
enum class ActSeverity { kFatal = 0x00, kCritical = 0x01, kWarning = 0x02, kInformation = 0x03, kDebug = 0x04 };

/**
 * @brief The mapping table for ActSeverity ENUM type
 *
 */
static const QMap<QString, ActSeverity> kActSeverityMap = {{"Fatal", ActSeverity::kFatal},
                                                           {"Critical", ActSeverity::kCritical},
                                                           {"Warning", ActSeverity::kWarning},
                                                           {"Information", ActSeverity::kInformation},
                                                           {"Debug", ActSeverity::kDebug}};

/**
 * @brief The base ACT status class
 *
 */
class ActStatusBase : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, status_code, StatusCode);       ///< The status code of this return entry
  ACT_JSON_ENUM(ActStatusType, status, Status);          ///< The status of this return entry
  ACT_JSON_ENUM(ActSeverity, severity, Severity);        ///< The severity of this return entry
  ACT_JSON_FIELD(QString, error_message, ErrorMessage);  ///< The error message of this return entry

 public:
  QList<QString> key_order_;

 public:
  /**
   * @brief Construct a new Act Status Base object
   *
   */
  ActStatusBase() {
    this->SetStatus(ActStatusType::kSuccess);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSuccess));
    this->SetSeverity(ActSeverity::kDebug);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kSuccess));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Base object
   *
   * @param status
   */
  ActStatusBase(const ActStatusType &status) : ActStatusBase() {
    this->SetStatus(status);
    this->SetStatusCode(static_cast<qint64>(status));
    this->SetErrorMessage(kActStatusTypeMap.key(status));
  }

  /**
   * @brief Construct a new Act Status Base object
   *
   * @param status
   * @param severity
   * @param code
   */
  ActStatusBase(const ActStatusType &status, const ActSeverity &severity) : status_(status), severity_(severity) {
    this->SetStatusCode(static_cast<qint64>(status));
    this->SetErrorMessage(kActStatusTypeMap.key(status));
  }

  /**
   * @brief Set the Act Status object
   *
   * @param status
   */
  void SetActStatus(const ActStatusType &status) {
    this->SetStatus(status);
    this->SetStatusCode(static_cast<qint64>(status));
  }

  /**
   * @brief Set the Act Status object
   *
   * @param status
   * @param severity
   */
  void SetActStatus(const ActStatusType &status, const ActSeverity &severity) {
    this->SetStatus(status);
    this->SetStatusCode(static_cast<qint64>(status));
    this->SetSeverity(severity);
    this->SetErrorMessage(kActStatusTypeMap.key(status));
  }
};

/**
 * @brief The parameter of ActBadRequest
 *
 */
class ActBadRequestParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, error_message, ErrorMessage);

 public:
  ActBadRequestParameter() {}
  ActBadRequestParameter(const QString &message) : error_message_(message) {}
};

/**
 * @brief The errors that are not caused by user input
 *
 */
class ActBadRequest : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActBadRequestParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActBadRequest() {
    this->SetStatus(ActStatusType::kBadRequest);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kBadRequest));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kBadRequest));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   * @param message
   */
  ActBadRequest(const QString &message) : ActBadRequest() {
    this->SetErrorMessage(message);
    this->SetParameter(ActBadRequestParameter(message));
  }
};

/**
 * @brief The parameter of ActProgressStatus
 *
 */
class ActProgressStatusParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);

 public:
  ActProgressStatusParameter() : progress_(0) {}
  ActProgressStatusParameter(const quint8 &progress) : progress_(progress) {}
};

/**
 * @brief The Status include the progress
 *
 */
class ActProgressStatus : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, progress, Progress);
  ACT_JSON_OBJECT(ActProgressStatusParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActProgressStatus() {
    this->SetStatus(ActStatusType::kRunning);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kRunning));
    this->SetSeverity(ActSeverity::kInformation);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kRunning));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
    this->SetProgress(0);
    this->SetParameter(ActProgressStatusParameter(0));
  }

  ActProgressStatus(const ActStatusBase &status_base, const quint8 &progress) : ActProgressStatus() {
    this->SetStatus(status_base.GetStatus());
    this->SetStatusCode(static_cast<qint64>(status_base.GetStatus()));
    this->SetSeverity(status_base.GetSeverity());
    this->SetErrorMessage(kActStatusTypeMap.key(status_base.GetStatus()));
    this->SetProgress(progress);
    this->SetParameter(ActProgressStatusParameter(progress));
  }

  ActProgressStatus(const quint8 &progress) : ActProgressStatus() {
    this->SetProgress(progress);
    this->SetParameter(ActProgressStatusParameter(progress));
  }
};

/**
 * @brief The status of the Device Configure result
 *
 */
class ActDeviceConfigureResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(quint8, progress, Progress);    ///< Progress item
  ACT_JSON_ENUM(ActStatusType, status, Status);  ///< The config status type of this device
  ACT_JSON_FIELD(QString, response, Response);
  ACT_JSON_FIELD(QString, error_message, ErrorMessage);  ///< The error message of this return entry
  ACT_JSON_FIELD(QString, error_detail, ErrorDetail);    ///< The error message of this return entry

 public:
  /**
   * @brief Construct a new Act Ipv 4 object
   *
   */
  ActDeviceConfigureResult() {
    id_ = -1;
    progress_ = 0;
    status_ = ActStatusType::kFailed;
    response_ = "";
    error_message_ = "";
    error_detail_ = "";
  }

  ActDeviceConfigureResult(const qint64 &id, const ActStatusType &status) : ActDeviceConfigureResult() {
    id_ = id;
    status_ = status;
  }

  ActDeviceConfigureResult(const qint64 &id, const ActStatusType &status, const quint8 &progress,
                           const QString &response)
      : ActDeviceConfigureResult() {
    id_ = id;
    status_ = status;
    progress_ = progress;
    response_ = response;
  }

  // For Deploy module success
  ActDeviceConfigureResult(const qint64 &id, const quint8 &progress, const ActStatusType &status)
      : ActDeviceConfigureResult() {
    id_ = id;
    progress_ = progress;
    status_ = status;
  }

  // For Deploy module failed
  ActDeviceConfigureResult(const qint64 &id, const quint8 &progress, const ActStatusType &status,
                           const QString &error_message)
      : ActDeviceConfigureResult() {
    id_ = id;
    progress_ = progress;
    status_ = status;
    error_message_ = error_message;
  }

  // For Deploy module failed
  ActDeviceConfigureResult(const qint64 &id, const quint8 &progress, const ActStatusType &status,
                           const QString &error_message, const QString &error_detail)
      : ActDeviceConfigureResult() {
    id_ = id;
    progress_ = progress;
    status_ = status;
    error_message_ = error_message;
    error_detail_ = error_detail;
  }
};

class ActDeviceEventLogResult : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_ENUM(ActStatusType, status, Status);  ///< The config status type of this device
  ACT_JSON_FIELD(quint8, progress, Progress);    ///< Progress item
  ACT_JSON_OBJECT(ActDeviceEventLog, response, Response);

 public:
  /**
   * @brief Construct a new Act Ipv 4 object
   *
   */
  ActDeviceEventLogResult() {
    id_ = -1;
    status_ = ActStatusType::kFailed;
  }

  ActDeviceEventLogResult(const qint64 &id, const quint8 &progress, const ActStatusType &status,
                          const ActDeviceEventLog &response)
      : ActDeviceEventLogResult() {
    id_ = id;
    progress_ = progress;
    status_ = status;
    response_ = response;
  }
};
/**
 * @brief The Status include the progress
 *
 */
class ActDeviceConfigurationStatus : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceConfigureResult, device_config_result, DeviceConfigResult);

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActDeviceConfigurationStatus() {
    this->SetStatus(ActStatusType::kRunning);
    this->SetSeverity(ActSeverity::kDebug);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kRunning));
  }

  ActDeviceConfigurationStatus(const ActStatusBase &status, const ActDeviceConfigureResult &dev_config_result) {
    this->SetStatus(status.GetStatus());
    this->SetSeverity(status.GetSeverity());
    this->SetDeviceConfigResult(dev_config_result);
    this->SetErrorMessage(kActStatusTypeMap.key(status.GetStatus()));
  }
};

// class ActDeviceConfigurationStatus : public ActProgressStatus {
//   Q_GADGET
//   QS_SERIALIZABLE

//   ACT_JSON_OBJECT(ActDeviceConfigureResult, device_config_result, DeviceConfigResult);

//  public:
//   /**
//    * @brief Construct a new Act JSON Parse Error object
//    *
//    */
//   ActDeviceConfigurationStatus() {
//     this->SetStatus(ActStatusType::kRunning);
//     this->SetSeverity(ActSeverity::kDebug);
//     this->SetProgress(0);
//   }

//   ActDeviceConfigurationStatus(const ActProgressStatus& progress_status,
//                                const ActDeviceConfigureResult& dev_config_result) {
//     this->SetStatus(progress_status.GetStatus());
//     this->SetSeverity(progress_status.GetSeverity());
//     this->SetProgress(progress_status.GetProgress());
//     this->SetDeviceConfigResult(dev_config_result);
//   }
// };

/**
 * @brief The parameter of ActDuplicatedError
 *
 */
class ActDuplicatedErrorParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);

 public:
  ActDuplicatedErrorParameter() {}
  ActDuplicatedErrorParameter(const QString &item) : item_(item) {}
};

/**
 * @brief The errors that are not caused by user input
 *
 */
class ActDuplicatedError : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_OBJECT(ActDuplicatedErrorParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act JSON Duplicated Error object
   *
   */
  ActDuplicatedError() {
    this->SetStatus(ActStatusType::kDuplicated);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kDuplicated));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kDuplicated));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act JSON Duplicated Error object
   *
   * @param item
   */
  ActDuplicatedError(const QString &item) : ActDuplicatedError() {
    this->SetErrorMessage(QString("%1 is duplicated").arg(item));
    this->SetParameter(ActDuplicatedErrorParameter(item));
  }
};

/**
 * @brief The parameter of ActStatusRoutingDestinationUnreachable
 *
 */
class ActStatusRoutingDestinationUnreachableParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, stream_name, StreamName);
  ACT_JSON_FIELD(QString, destination_ip, DestinationIp);

 public:
  ActStatusRoutingDestinationUnreachableParameter() {}
  ActStatusRoutingDestinationUnreachableParameter(const QString &stream_name, const QString &destination_ip)
      : stream_name_(stream_name), destination_ip_(destination_ip) {}
};

/**
 * @brief Routing failed: the destination node cannot be reached
 *
 */
class ActStatusRoutingDestinationUnreachable : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_OBJECT(ActStatusRoutingDestinationUnreachableParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Routing Destination Unreachable object
   *
   */
  ActStatusRoutingDestinationUnreachable() {
    this->SetStatus(ActStatusType::kRoutingDestinationUnreachable);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kRoutingDestinationUnreachable));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kRoutingDestinationUnreachable));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Routing Destination Unreachable object
   *
   * @param stream_name
   * @param destination_ip
   */
  ActStatusRoutingDestinationUnreachable(const QString &stream_name, const QString &destination_ip)
      : ActStatusRoutingDestinationUnreachable() {
    this->SetParameter(ActStatusRoutingDestinationUnreachableParameter(stream_name, destination_ip));
    this->SetErrorMessage(QString("%1: Cannot reach to %2").arg(stream_name).arg(destination_ip));
  }
};

class ActStatusRoutingDeviceTypeIncapableParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, stream_name, StreamName);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);

 public:
  ActStatusRoutingDeviceTypeIncapableParameter() {}
  ActStatusRoutingDeviceTypeIncapableParameter(const QString &stream_name, const QString &device_ip)
      : stream_name_(stream_name), device_ip_(device_ip) {}
};

/**
 * @brief Routing failed: the destination node cannot be reached
 *
 */
class ActStatusRoutingDeviceTypeIncapable : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_OBJECT(ActStatusRoutingDeviceTypeIncapableParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Routing Destination Unreachable object
   *
   */
  ActStatusRoutingDeviceTypeIncapable() {
    this->SetStatus(ActStatusType::kRoutingDeviceTypeIncapable);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kRoutingDeviceTypeIncapable));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kRoutingDeviceTypeIncapable));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Routing Destination Unreachable object
   *
   * @param stream_name
   * @param destination_ip
   */
  ActStatusRoutingDeviceTypeIncapable(const QString &stream_name, const QString &device_ip)
      : ActStatusRoutingDeviceTypeIncapable() {
    this->SetParameter(ActStatusRoutingDeviceTypeIncapableParameter(stream_name, device_ip));
    this->SetErrorMessage(
        QString("%1: Has the incapable routing device at the path(%2)").arg(stream_name).arg(device_ip));
  }
};

/**
 * @brief Scheduling failed status class
 *
 */
class ActStatusSchedulingFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

 public:
  /**
   * @brief Construct a new Act Status Scheduling Failed object
   *
   */
  ActStatusSchedulingFailed() {
    this->SetStatus(ActStatusType::kSchedulingFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSchedulingFailed));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kSchedulingFailed));
  }

  ActStatusSchedulingFailed(const QString &message) : ActStatusSchedulingFailed() { this->SetErrorMessage(message); }
};

/**
 * @brief The parameter of ActStatusPcpInsufficientForTimeSlot
 *
 */
class ActStatusPcpInsufficientForTimeSlotParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, time_slot_index, TimeSlotIndex);

 public:
  ActStatusPcpInsufficientForTimeSlotParameter() {}
  ActStatusPcpInsufficientForTimeSlotParameter(const quint8 &time_slot_index) : time_slot_index_(time_slot_index) {}
};

/**
 * @brief The PCP is not enough for time slot
 *
 */
class ActStatusPcpInsufficientForTimeSlot : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusPcpInsufficientForTimeSlotParameter, parameter, Parameter);

 public:
  ActStatusPcpInsufficientForTimeSlot() {
    this->SetStatus(ActStatusType::kPcpInsufficientForTimeSlot);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kPcpInsufficientForTimeSlot));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kPcpInsufficientForTimeSlot));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status PCP Insufficient For Time Slot object
   *
   * @param time_slot_index
   */
  ActStatusPcpInsufficientForTimeSlot(const quint8 &time_slot_index) : ActStatusPcpInsufficientForTimeSlot() {
    this->SetParameter(ActStatusPcpInsufficientForTimeSlotParameter(time_slot_index));
    this->SetErrorMessage(QString("The PCP is insufficient for time slot %1").arg(QString::number(time_slot_index)));
  }
};

/**
 * @brief The parameter of ActStatusTimeSyncPcpNotConsistentWithDevice
 *
 */
class ActStatusTimeSyncPcpNotConsistentWithDeviceParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint8, time_sync_pcp, TimeSyncPCP);
  ACT_JSON_FIELD(QString, device_ip, DeviceIp);

 public:
  ActStatusTimeSyncPcpNotConsistentWithDeviceParameter() {}
  ActStatusTimeSyncPcpNotConsistentWithDeviceParameter(const quint8 &time_sync_pcp, const QString &device_ip)
      : time_sync_pcp_(time_sync_pcp), device_ip_(device_ip) {}
};

/**
 * @brief The PCP is not enough for time slot
 *
 */
class ActStatusTimeSyncPcpNotConsistentWithDevice : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusTimeSyncPcpNotConsistentWithDeviceParameter, parameter, Parameter);

 public:
  ActStatusTimeSyncPcpNotConsistentWithDevice() {
    this->SetStatus(ActStatusType::kTimeSyncPcpNotConsistentWithDevice);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kTimeSyncPcpNotConsistentWithDevice));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kTimeSyncPcpNotConsistentWithDevice));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status PCP Insufficient For Time Slot object
   *
   * @param time_sync_pcp
   */
  ActStatusTimeSyncPcpNotConsistentWithDevice(const quint8 &time_sync_pcp, const QString &device_ip)
      : ActStatusTimeSyncPcpNotConsistentWithDevice() {
    this->SetParameter(ActStatusTimeSyncPcpNotConsistentWithDeviceParameter(time_sync_pcp, device_ip));
    this->SetErrorMessage(QString("The time sync PCP %1 in the device %2 is not consistent with project setting")
                              .arg(QString::number(time_sync_pcp))
                              .arg(device_ip));
  }
};

/**
 * @brief The parameter of ActStatusFeasibilityCheckFailedParameter
 *
 */
class ActStatusFeasibilityCheckFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, reason, Reason);

 public:
  ActStatusFeasibilityCheckFailedParameter() {}
  ActStatusFeasibilityCheckFailedParameter(const QString &reason) : reason_(reason) {}
};

/**
 * @brief The error occurs in scheduling feasibility check (TODO: should be divided into several errors)
 *
 */
class ActStatusFeasibilityCheckFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusFeasibilityCheckFailedParameter, parameter, Parameter);

 public:
  ActStatusFeasibilityCheckFailed() {
    this->SetStatus(ActStatusType::kFeasibilityCheckFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kFeasibilityCheckFailed));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kFeasibilityCheckFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Feasibility Check Failed object
   *
   * @param reason
   */
  ActStatusFeasibilityCheckFailed(const QString &reason) : ActStatusFeasibilityCheckFailed() {
    this->SetErrorMessage(reason);
    this->SetParameter(ActStatusFeasibilityCheckFailedParameter(reason));
  }
};

/**
 * @brief The error occurs in calculate timeout
 *
 */
class ActStatusCalculateTimeout : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

 public:
  ActStatusCalculateTimeout() {
    this->SetStatus(ActStatusType::kCalculateTimeout);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kCalculateTimeout));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(QString("Calculating timeout"));
  }
};

/**
 * @brief Southbound failed status class
 *
 */
class ActStatusSouthboundFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

 public:
  ActStatusSouthboundFailed() {
    this->SetStatus(ActStatusType::kSouthboundFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSouthboundFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kSouthboundFailed));
  }

  ActStatusSouthboundFailed(const QString &error_message) : ActStatusSouthboundFailed() {
    this->SetErrorMessage(error_message);
  }
};

/**
 * @brief Southbound failed status class
 *
 */
class ActStatusCheckFeatureFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

 public:
  ActStatusCheckFeatureFailed() {
    this->SetStatus(ActStatusType::kCheckFeatureFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kCheckFeatureFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kCheckFeatureFailed));
  }

  ActStatusCheckFeatureFailed(const QString &error_message) : ActStatusCheckFeatureFailed() {
    this->SetErrorMessage(error_message);
  }
};

/**
 * @brief The parameter of ActStatusInternalError
 *
 */
class ActStatusInternalErrorParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, module, Module);

 public:
  ActStatusInternalErrorParameter() {}
  ActStatusInternalErrorParameter(const QString &module) : module_(module) {}
};

/**
 * @brief The errors that are not caused by user input
 *
 */
class ActStatusInternalError : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusInternalErrorParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Internal Error object
   *
   */
  ActStatusInternalError() {
    this->SetStatus(ActStatusType::kInternalError);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kInternalError));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kInternalError));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Internal Error object
   *
   * @param module
   */
  ActStatusInternalError(const QString &module) : ActStatusInternalError() {
    this->SetParameter(ActStatusInternalErrorParameter(module));
    this->SetErrorMessage(QString("Internal error in %1 module").arg(module));
  }
};

/**
 * @brief The parameter of ActStatusSetConfigFailed
 *
 */
class ActStatusSetConfigFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);
  ACT_JSON_FIELD(QString, device, Device);

 public:
  ActStatusSetConfigFailedParameter() {}
  ActStatusSetConfigFailedParameter(const QString &item, const QString &device) : item_(item), device_(device) {}
};

/**
 * @brief The set configuration failed status object
 *
 */
class ActStatusSetConfigFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusSetConfigFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Set Config Failed object
   *
   */
  ActStatusSetConfigFailed() {
    this->SetStatus(ActStatusType::kSetConfigFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSetConfigFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kSetConfigFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Set Config Failed object
   *
   * @param item
   * @param device
   */
  ActStatusSetConfigFailed(const QString &item, const QString &device) : ActStatusSetConfigFailed() {
    this->SetParameter(ActStatusSetConfigFailedParameter(item, device));
    this->SetErrorMessage(QString("Set %1 configuration failed. Device: %2").arg(item).arg(device));
  }
};

/**
 * @brief The parameter of ActStatusGetDeviceDataFailed
 *
 */
class ActStatusGetDeviceDataFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);
  ACT_JSON_FIELD(QString, device, Device);

 public:
  ActStatusGetDeviceDataFailedParameter() {}
  ActStatusGetDeviceDataFailedParameter(const QString &item, const QString &device) : item_(item), device_(device) {}
};

/**
 * @brief The get configuration failed status object
 *
 */
class ActStatusGetDeviceDataFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusGetDeviceDataFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Get Config Failed object
   *
   */
  ActStatusGetDeviceDataFailed() {
    this->SetStatus(ActStatusType::kGetDeviceDataFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kGetDeviceDataFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kGetDeviceDataFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Get Config Failed object
   *
   * @param item
   * @param device
   */
  ActStatusGetDeviceDataFailed(const QString &item, const QString &device) : ActStatusGetDeviceDataFailed() {
    this->SetParameter(ActStatusGetDeviceDataFailedParameter(item, device));
    this->SetErrorMessage(QString("Get %1 device data failed. Device: %2").arg(item).arg(device));
  }
};

/**
 * @brief The parameter of ActStatusNotFound
 *
 */
class ActStatusNotFoundParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);

 public:
  ActStatusNotFoundParameter() {}
  ActStatusNotFoundParameter(const QString &item) : item_(item) {}
};

class ActStatusNotFound : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusNotFoundParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Failed object
   *
   */
  ActStatusNotFound() {
    this->SetStatus(ActStatusType::kNotFound);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kNotFound));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kNotFound));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Failed object
   *
   * @param item
   * @param devices
   */
  ActStatusNotFound(const QString &item) : ActStatusNotFound() {
    this->SetParameter(ActStatusNotFoundParameter(item));
    this->SetErrorMessage(QString("%1 is not found").arg(item));
  }
};

/**
 * @brief The parameter of ActStatusDeployFailedParameter
 *
 */
class ActStatusDeployFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET(QString, failed_devices, FailedDevices)

 public:
  ActStatusDeployFailedParameter() {}
  ActStatusDeployFailedParameter(const QSet<QString> &failed_devices) : failed_devices_(failed_devices) {}
};

/**
 * @brief The deploy failed status object
 *
 */
class ActStatusDeployFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusDeployFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Failed object
   *
   */
  ActStatusDeployFailed() {
    this->SetStatus(ActStatusType::kDeployFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kDeployFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kDeployFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Failed object
   *
   * @param item
   * @param devices
   */
  ActStatusDeployFailed(const QSet<QString> &failed_devices) : ActStatusDeployFailed() {
    this->SetParameter(ActStatusDeployFailedParameter(failed_devices));

    QString failed_devices_set("{");
    for (QString dev : failed_devices) {
      failed_devices_set.append(dev);
      failed_devices_set.append(",");
    }
    failed_devices_set.back() = '}';

    this->SetErrorMessage(QString("Deploy failed. Failed Devices: %1").arg(failed_devices_set));
  }
};

/**
 * @brief The parameter of ActStatusCompareFailed
 *
 */
class ActStatusCompareFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);
  ACT_JSON_FIELD(QString, topology_item_type, TopologyItemType);
  ACT_JSON_QT_SET(QString, topology_items, TopologyItems)

 public:
  ActStatusCompareFailedParameter() {}
  ActStatusCompareFailedParameter(const QString &item, const QString &topology_item_type,
                                  const QSet<QString> &topology_items)
      : item_(item), topology_item_type_(topology_item_type), topology_items_(topology_items) {}
};

/**
 * @brief The compare failed status object
 *
 */
class ActStatusCompareFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusCompareFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Compare Failed object
   *
   */
  ActStatusCompareFailed() {
    this->SetStatus(ActStatusType::kCompareFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kCompareFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kCompareFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Compare Failed object
   *
   * @param item
   * @param devices
   */
  ActStatusCompareFailed(const QString &item, const QString &topology_item_type, const QSet<QString> &topology_items)
      : ActStatusCompareFailed() {
    this->SetParameter(ActStatusCompareFailedParameter(item, topology_item_type, topology_items));

    QString items("{");
    for (QString topology_item : topology_items) {
      items.append(topology_item);
      items.append(',');
    }
    items.back() = '}';

    this->SetErrorMessage(QString("Compare %1 failed. %2: %3").arg(item).arg(topology_item_type).arg(items));
  }
};

/**
 * @brief The parameter of ActStatusCompareTopologyFailed
 *
 */
class ActStatusCompareTopologyFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET(QString, not_found_links, NotFoundLinks);
  ACT_JSON_QT_SET(QString, extra_links, ExtraLinks)

 public:
  ActStatusCompareTopologyFailedParameter() {}
  ActStatusCompareTopologyFailedParameter(const QSet<QString> &not_found_links, const QSet<QString> &extra_links)
      : not_found_links_(not_found_links), extra_links_(extra_links) {}
};

/**
 * @brief The compare failed status object
 *
 */
class ActStatusCompareTopologyFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusCompareTopologyFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Compare Failed object
   *
   */
  ActStatusCompareTopologyFailed() {
    this->SetStatus(ActStatusType::kCompareTopologyFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kCompareTopologyFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kCompareTopologyFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Compare Failed object
   *
   * @param item
   * @param devices
   */
  ActStatusCompareTopologyFailed(const QSet<QString> &not_found_links, const QSet<QString> &extra_links)
      : ActStatusCompareTopologyFailed() {
    this->SetParameter(ActStatusCompareTopologyFailedParameter(not_found_links, extra_links));

    QString not_found_links_set("{");
    for (QString not_found_link : not_found_links) {
      not_found_links_set.append(not_found_link);
      not_found_links_set.append(",");
    }
    not_found_links_set.back() = '}';

    QString extra_links_set("{");
    for (QString extra_link : extra_links) {
      extra_links_set.append(extra_link);
      extra_links_set.append(",");
    }
    extra_links_set.back() = '}';

    this->SetErrorMessage(QString("Compare topology consistent failed. Link not found. Links: %1; Extra links: %2")
                              .arg(not_found_links_set)
                              .arg(extra_links_set));
  }
};

class ActStatusUpdateProjectTopologyFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET(qint64, devices, Devices);
  ACT_JSON_QT_SET(qint64, links, Links)

 public:
  ActStatusUpdateProjectTopologyFailedParameter() {}
  ActStatusUpdateProjectTopologyFailedParameter(const QSet<qint64> &devices, const QSet<qint64> &links)
      : ActStatusUpdateProjectTopologyFailedParameter() {
    this->devices_ = devices;
    this->links_ = links;
  }
};

/**
 * @brief The compare failed status object
 *
 */
class ActStatusUpdateProjectTopologyFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActStatusUpdateProjectTopologyFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Status Compare Failed object
   *
   */
  ActStatusUpdateProjectTopologyFailed() {
    this->SetStatus(ActStatusType::kUpdateProjectTopologyFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kUpdateProjectTopologyFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kUpdateProjectTopologyFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act Status Compare Failed object
   *
   * @param item
   * @param devices
   */
  ActStatusUpdateProjectTopologyFailed(const QSet<qint64> &devices, const QSet<qint64> &links)
      : ActStatusUpdateProjectTopologyFailed() {
    this->SetParameter(ActStatusUpdateProjectTopologyFailedParameter(devices, links));

    QString devices_str = "";
    for (auto it = devices.constBegin(); it != devices.constEnd(); it++) {
      devices_str.append(QString::number(*it));
      if (std::next(it) != devices.constEnd()) {  // not last
        devices_str.append(",");
      }
    }

    QString links_str = "";
    for (auto it = links.constBegin(); it != links.constEnd(); it++) {
      links_str.append(QString::number(*it));
      if (std::next(it) != links.constEnd()) {  // not last
        links_str.append(",");
      }
    }

    QString error_msg("Update Project topology failed.");
    if (!devices_str.isEmpty()) {
      error_msg.append(QString(" Failed Devices: %1.").arg(devices_str));
    }
    if (!links_str.isEmpty()) {
      error_msg.append(QString(" Failed Links: %1.").arg(links_str));
    }

    this->SetErrorMessage(error_msg);
  }
};

/**
 * @brief The parameter of ActLicenseSizeFailedRequest
 *
 */
class ActLicenseSizeFailedRequestParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);
  ACT_JSON_FIELD(quint16, size, Size);

 public:
  ActLicenseSizeFailedRequestParameter() {}
  ActLicenseSizeFailedRequestParameter(const QString &item, const quint16 &size) : item_(item), size_(size) {}
};

class ActLicenseSizeFailedRequest : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActLicenseSizeFailedRequestParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act License Size Failed Request object
   *
   */
  ActLicenseSizeFailedRequest() {
    this->SetStatus(ActStatusType::kLicenseSizeFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kLicenseSizeFailed));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kLicenseSizeFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act License Size Failed Request object
   *
   * @param item
   * @param size
   */
  ActLicenseSizeFailedRequest(const QString &item, const quint16 &size) : ActLicenseSizeFailedRequest() {
    this->SetParameter(ActLicenseSizeFailedRequestParameter(item, size));
    this->SetErrorMessage(QString("The %1 exceeds the limit: %2").arg(item).arg(size));
  }
};

class ActLicenseNotSupportRequestParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);

 public:
  ActLicenseNotSupportRequestParameter() {}
  ActLicenseNotSupportRequestParameter(const QString &item) : item_(item) {}
};

class ActLicenseNotSupport : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActLicenseNotSupportRequestParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act License Not Support object
   *
   */
  ActLicenseNotSupport() {
    this->SetStatus(ActStatusType::kLicenseNotSupport);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kLicenseNotSupport));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kLicenseNotSupport));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act License Not Support object
   *
   * @param item
   */
  ActLicenseNotSupport(const QString &item) : ActLicenseNotSupport() {
    this->SetParameter(ActLicenseNotSupportRequestParameter(item));
    this->SetErrorMessage(QString("The project profile %1 is not supported by the license").arg(item));
  }
};

/**
 * @brief The parameter of ActLicenseNotActiveFailed
 *
 */
class ActLicenseNotActiveFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);

 public:
  ActLicenseNotActiveFailedParameter() {}
  ActLicenseNotActiveFailedParameter(const QString &item) : item_(item) {}
};

/**
 * @brief The errors that are not caused by user input
 *
 */
class ActLicenseNotActiveFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActLicenseNotActiveFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act License Not Active Failed object
   *
   */
  ActLicenseNotActiveFailed() {
    this->SetStatus(ActStatusType::kLicenseNotActive);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kLicenseNotActive));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kLicenseNotActive));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  /**
   * @brief Construct a new Act License Not Active Failed object
   *
   * @param item
   */
  ActLicenseNotActiveFailed(const QString &item) : ActLicenseNotActiveFailed() {
    this->SetParameter(ActLicenseNotActiveFailedParameter(item));
    this->SetErrorMessage(QString("The %1 feature is not active").arg(item));
  }
};

/**
 * @brief The used DeviceList class in project
 *
 */
class ActProjectUsedDeviceIpList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, project_name, ProjectName);  ///< The project name that used the device
  ACT_JSON_QT_SET(QString, devices, Devices);          ///< The specific device ip list
};

/**
 * @brief The used DeviceList class in topology
 *
 */
class ActTopologyUsedDeviceIpList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, topology_name, TopologyName);  ///< The topology name that used the device
  ACT_JSON_QT_SET(QString, devices, Devices);            ///< The specific device ip list
};

/** The used DeviceList class in baseline */
class ActBaselineUsedDeviceIpList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, baseline_name, BaselineName);  ///< The baseline name that used the device
  ACT_JSON_QT_SET(QString, devices, Devices);            ///< The specific device ip list
};

/**
 * @brief The parameter of ActDeviceProfileIsUsedFailed
 *
 */
class ActDeviceProfileIsUsedFailedParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActProjectUsedDeviceIpList, used_projects,
                              UsedProjects);  ///< The project list which specifies the device profile is used
  ACT_JSON_COLLECTION_OBJECTS(QList, ActTopologyUsedDeviceIpList, used_topologies,
                              UsedTopologies);  ///< The topology list which specifies the device profile is used
  ACT_JSON_COLLECTION_OBJECTS(QList, ActBaselineUsedDeviceIpList, used_baselines,
                              UsedBaselines);  ///< The baseline list which specifies the device profile is used

 public:
  ActDeviceProfileIsUsedFailedParameter() {}
  // ActDeviceProfileIsUsedFailedParameter(const QString& item, const quint16& size) : item_(item), size_(size) {}
};

/**
 * @brief The errors that are not caused by user input
 *
 */
class ActDeviceProfileIsUsedFailed : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActDeviceProfileIsUsedFailedParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActDeviceProfileIsUsedFailed() {
    this->SetStatus(ActStatusType::kDeviceProfileIsUsedFailed);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kDeviceProfileIsUsedFailed));
    this->SetSeverity(ActSeverity::kCritical);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kDeviceProfileIsUsedFailed));
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  // /**
  //  * @brief Construct a new Act JSON Parse Error object
  //  *
  //  * @param item
  //  */
  // ActDeviceProfileIsUsedFailed(const QString& item, const quint16& size) : ActDeviceProfileIsUsedFailed() {
  //   this->SetParameter(ActDeviceProfileIsUsedFailedParameter(item, size));
  //   this->SetErrorMessage(QString("The %1 exceeds the limit: %2").arg(item).arg(size));
  // }
};

/**
 * @brief The parameter of ActCopyTopologyResponse
 *
 */
class ActCopyTopologyResponseParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, device_ids,
                      DeviceIds);  ///< The copied device ids
  ACT_JSON_COLLECTION(QList, qint64, link_ids,
                      LinkIds);  ///< The copied link ids

 public:
  ActCopyTopologyResponseParameter() {}
  ActCopyTopologyResponseParameter(const QList<qint64> &dev_ids, const QList<qint64> &link_ids)
      : device_ids_(dev_ids), link_ids_(link_ids) {}
};

/**
 * @brief A response for return dev_ids and link_ids in CopyTopology
 *
 */
class ActCopyTopologyResponse : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActCopyTopologyResponseParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Copy Topology Response object
   *
   */
  ActCopyTopologyResponse() {
    this->SetStatus(ActStatusType::kSuccess);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSuccess));
    this->SetSeverity(ActSeverity::kInformation);
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  ActCopyTopologyResponse(const QList<qint64> &dev_ids, const QList<qint64> &link_ids) : ActCopyTopologyResponse() {
    this->SetParameter(ActCopyTopologyResponseParameter(dev_ids, link_ids));
  }
};

/**
 * @brief The parameter of ActBatchCreateLinkResponse
 *
 */
class ActBatchCreateLinkResponseParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, link_ids,
                      LinkIds);  ///< The copied link ids

 public:
  ActBatchCreateLinkResponseParameter() {}
  ActBatchCreateLinkResponseParameter(const QList<qint64> &link_ids) : link_ids_(link_ids) {}
};

/**
 * @brief A response for return link_ids in BatchCreateLinks
 *
 */
class ActBatchCreateLinkResponse : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActBatchCreateLinkResponseParameter, parameter, Parameter);

 public:
  /**
   * @brief Construct a new Act Batch Create Link Response object
   *
   */
  ActBatchCreateLinkResponse() {
    this->SetStatus(ActStatusType::kSuccess);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSuccess));
    this->SetSeverity(ActSeverity::kInformation);
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  ActBatchCreateLinkResponse(const QList<qint64> &link_ids) : ActBatchCreateLinkResponse() {
    this->SetParameter(ActBatchCreateLinkResponseParameter(link_ids));
  }
};

/**
 * @brief The parameter of ActBatchCreateDeviceResponse
 *
 */
class ActBatchCreateDeviceResponseParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, qint64, device_ids, DeviceIds);  ///< The created device ids

 public:
  ActBatchCreateDeviceResponseParameter() {}
  ActBatchCreateDeviceResponseParameter(const QList<qint64> &device_ids) : device_ids_(device_ids) {}
};

/**
 * @brief A response for return device_ids in BatchCreateDevices
 *
 */
class ActBatchCreateDeviceResponse : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActBatchCreateDeviceResponseParameter, parameter, Parameter);

 public:
  ActBatchCreateDeviceResponse() {
    this->SetStatus(ActStatusType::kSuccess);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSuccess));
    this->SetSeverity(ActSeverity::kInformation);
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  ActBatchCreateDeviceResponse(const QList<qint64> &device_ids) : ActBatchCreateDeviceResponse() {
    this->SetParameter(ActBatchCreateDeviceResponseParameter(device_ids));
  }
};

/**
 * @brief The parameter of ActBatchCreateStreamResponse
 *
 */
class ActBatchCreateStreamResponseParameter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_SET(qint64, stream_ids, StreamIds);  ///< The created stream ids

 public:
  ActBatchCreateStreamResponseParameter() {}
  ActBatchCreateStreamResponseParameter(const QSet<qint64> &stream_ids) : stream_ids_(stream_ids) {}
};

/**
 * @brief A response for return stream_ids in BatchCreateStreams
 *
 */
class ActBatchCreateStreamResponse : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActBatchCreateStreamResponseParameter, parameter, Parameter);

 public:
  ActBatchCreateStreamResponse() {
    this->SetStatus(ActStatusType::kSuccess);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kSuccess));
    this->SetSeverity(ActSeverity::kInformation);
    this->key_order_ = QList<QString>({QString("StatusCode"), QString("Parameter"), QString("ErrorMessage")});
  }

  ActBatchCreateStreamResponse(const QSet<qint64> &stream_ids) : ActBatchCreateStreamResponse() {
    this->SetParameter(ActBatchCreateStreamResponseParameter(stream_ids));
  }
};

/**
 * @brief The Status include the progress
 *
 */
class ActUnauthorized : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActUnauthorized() {
    this->SetStatus(ActStatusType::kUnauthorized);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kUnauthorized));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kUnauthorized));
  }
};

/**
 * @brief The Status include the progress
 *
 */
class ActServiceUnavailable : public ActStatusBase {
  Q_GADGET
  QS_SERIALIZABLE

 public:
  /**
   * @brief Construct a new Act JSON Parse Error object
   *
   */
  ActServiceUnavailable() {
    this->SetStatus(ActStatusType::kServiceUnavailable);
    this->SetStatusCode(static_cast<qint64>(ActStatusType::kServiceUnavailable));
    this->SetSeverity(ActSeverity::kWarning);
    this->SetErrorMessage(kActStatusTypeMap.key(ActStatusType::kServiceUnavailable));
  }
};

/**
 * @brief The undo/redo status class
 *
 */
class ActUndoRedoStatus : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, status, Status);  ///< The status of the undo/redo function
};

/**
 * @brief Check if act_status is success
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusSuccess(ACT_STATUS &act_status) { return act_status->GetStatus() == ActStatusType::kSuccess; }

/**
 * @brief Check if act_status is skip
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusSkip(ACT_STATUS &act_status) { return act_status->GetStatus() == ActStatusType::kSkip; }

/**
 * @brief Check if act_status is not found
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusNotFound(ACT_STATUS &act_status) { return act_status->GetStatus() == ActStatusType::kNotFound; }

/**
 * @brief Check if act_status is feasibility check failed
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusFeasibilityCheckFailed(ACT_STATUS &act_status) {
  return act_status->GetStatus() == ActStatusType::kFeasibilityCheckFailed;
}

/**
 * @brief Check if act_status is bad request
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusBadRequest(ACT_STATUS &act_status) {
  return act_status->GetStatus() == ActStatusType::kBadRequest;
}

/**
 * @brief Check if act_status is internalError
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusInternalError(ACT_STATUS &act_status) {
  return act_status->GetStatus() == ActStatusType::kInternalError;
}

/**
 * @brief Check if act_status is failed or not
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusFailed(ACT_STATUS &act_status) { return act_status->GetStatus() >= ActStatusType::kFailed; }

/**
 * @brief Check if act_status is running
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusRunning(ACT_STATUS &act_status) { return act_status->GetStatus() == ActStatusType::kRunning; }

/**
 * @brief Check if act_status is Finished
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusFinished(ACT_STATUS &act_status) { return act_status->GetStatus() == ActStatusType::kFinished; }

/**
 * @brief Check if act_status is stop
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusStop(ACT_STATUS &act_status) { return act_status->GetStatus() == ActStatusType::kStop; }

/**
 * @brief Check if act_status is Unauthorized
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusUnauthorized(ACT_STATUS &act_status) {
  return act_status->GetStatus() == ActStatusType::kUnauthorized;
}

/**
 * @brief Check if act_status is ServiceUnavailable
 *
 * @param act_status
 * @return true
 * @return false
 */
inline bool IsActStatusServiceUnavailable(ACT_STATUS &act_status) {
  return act_status->GetStatus() == ActStatusType::kServiceUnavailable;
}
