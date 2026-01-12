/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <stdint.h>

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QList>
#include <QMap>
#include <QSet>
#include <QString>
#include <map>
#include <set>
#include <string>

#include "act_json.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
#include "simplecrypt.h"

static QString stub_err_msg = QString();

/**
 * @brief The ACT size license class
 *
 */
class ActSizeLicense : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, project_size, ProjectSize);
  ACT_JSON_FIELD(quint16, device_qty, DeviceQty);

 public:
  QList<QString> key_order_;
  ActSizeLicense() {
    this->project_size_ = 0;
    this->device_qty_ = 0;
    this->key_order_.append(QList<QString>({QString("ProjectSize"), QString("DeviceQty")}));
  }
};

/**
 * @brief The ACT device config license class
 *
 */
class ActDeviceConfigLicense : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enabled, Enabled);  ///< The main license control item of device configuration

 public:
  QList<QString> key_order_;
  ActDeviceConfigLicense() {
    this->enabled_ = true;
    this->key_order_.append(QList<QString>({QString("Enabled")}));
  }
};

/**
 * @brief The ACT TSN license class
 *
 */
class ActTSNLicense : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, enabled, Enabled);  ///< The main license control item of TSN feature
  ACT_JSON_FIELD(bool, scheduling, Scheduling);
  ACT_JSON_FIELD(bool, frer, FRER);

 public:
  QList<QString> key_order_;
  ActTSNLicense() {
    this->enabled_ = true;
    this->scheduling_ = true;
    this->frer_ = true;
    this->key_order_.append(QList<QString>({QString("Enabled"), QString("Scheduling"), QString("FRER")}));
  }
};

/**
 * @brief The ACT stage license class
 *
 */
class ActStageLicense : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, design, Design);
  ACT_JSON_FIELD(bool, operation, Operation);
  ACT_JSON_FIELD(bool, manufacture, Manufacture);

 public:
  QList<QString> key_order_;
  ActStageLicense() {
    this->design_ = true;
    this->operation_ = true;
    this->manufacture_ = true;
    this->key_order_.append(QList<QString>({QString("Design"), QString("Operation"), QString("Manufacture")}));
  }
};

/**
 * @brief The ACT feature license class
 *
 */
class ActFeatureLicense : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, https, Https);
  ACT_JSON_FIELD(bool, opcua, Opcua);
  ACT_JSON_FIELD(bool, intelligent, Intelligent);
  ACT_JSON_FIELD(bool, auto_probe, AutoProbe);
  ACT_JSON_OBJECT(ActDeviceConfigLicense, device_config, DeviceConfig);
  ACT_JSON_OBJECT(ActTSNLicense, tsn, TSN);
  ACT_JSON_OBJECT(ActStageLicense, stage, Stage);

 public:
  QList<QString> key_order_;
  ActFeatureLicense() {
    this->https_ = true;
    this->opcua_ = true;
    this->intelligent_ = true;
    this->auto_probe_ = false;
    this->key_order_.append(
        QList<QString>({QString("Https"), QString("Opcua"), QString("Intelligent"), QString("AutoProbe"),
                        QString("DeviceConfig"), QString("TSN"), QString("Stage")}));
  }
};

/**
 * @brief The ACT license class
 *
 */
class ActLicense : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(QDate, expire_date, ExpireDate);  ///< expire date of the system in QDate format
  ACT_JSON_FIELD(quint8, major_version, MajorVersion);
  ACT_JSON_FIELD(QString, bound_mac_address, BoundMacAddress);
  ACT_JSON_OBJECT(ActSizeLicense, size, Size);
  ACT_JSON_COLLECTION_ENUM(QList, ActServiceProfileForLicenseEnum, profiles, Profiles);
  ACT_JSON_OBJECT(ActFeatureLicense, feature, Feature);
  ACT_JSON_ENUM(ActDeploymentTypeEnum, deployment_type, DeploymentType);

 public:
  QList<QString> key_order_;
  ActLicense() {
    this->major_version_ = ACT_MAJOR_VERSION;
    this->expire_date_ = QDate::fromString(ACT_EVALUATION_EXPIRE_DATE, "yyyy/MM/dd");
    this->profiles_.append(ActServiceProfileForLicenseEnum::kSelfPlanning);
    this->profiles_.append(ActServiceProfileForLicenseEnum::kGeneral);
    this->profiles_.append(ActServiceProfileForLicenseEnum::kFoxboro);
    this->deployment_type_ = ActDeploymentTypeEnum::kLocal;
    this->key_order_.append(
        QList<QString>({QString("ExpireDate"), QString("MajorVersion"), QString("MacAddress"),
                        QString("DeploymentType"), QString("Size"), QString("Profiles"), QString("Feature")}));
  }

  /**
   * @brief Read the plain license json
   *
   * @param lic_str
   * @param err_msg
   * @return ACT_STATUS
   */
  ACT_STATUS ReadPlainLicense(QString lic_str, QString &err_msg = stub_err_msg) {
    ACT_STATUS_INIT();

    QByteArray ba = lic_str.toUtf8();  // QString to QByteArray
    QJsonParseError e;
    QJsonDocument json_doc = QJsonDocument::fromJson(ba, &e);
    if (e.error != QJsonParseError::NoError) {
      err_msg = e.errorString();
      qCritical() << err_msg;
      act_status->SetActStatus(ActStatusType::kLicenseContentFailed, ActSeverity::kCritical);
      return act_status;
    }

    if (json_doc.isNull()) {
      err_msg = "The JSON is empty";
      qCritical() << err_msg;
      act_status->SetActStatus(ActStatusType::kLicenseContentFailed, ActSeverity::kCritical);
      return act_status;
    }

    if (!json_doc.isObject()) {
      err_msg = "The JSON is not an object";
      qCritical() << err_msg;
      act_status->SetActStatus(ActStatusType::kLicenseContentFailed, ActSeverity::kCritical);
      return act_status;
    }

    // Convert QJsonDocument to QJsonObject
    QJsonObject root_obj = json_doc.object();

    // Parse Json to object
    this->fromJson(root_obj);

    return act_status;
  }

  /**
   * @brief Check license file exist
   *
   * @return ACT_STATUS
   */
  ACT_STATUS CheckLicenseExist(bool &exist) {
    ACT_STATUS_INIT();

    // kene+
    /*
    QFile file(ACT_ENCRYPTED_LICENSE_FILE_NAME);
    */
    QFile file(GetEncryptedLicenseFilePath());
    // kene-
    exist = file.exists();

    return act_status;
  }

  /**
   * @brief Read encrypted license from database
   *
   * @return ACT_STATUS
   */
  ACT_STATUS ReadEncryptedLicense() {
    ACT_STATUS_INIT();

    // Open the license file
    QString content;
    // kene+
    QString encryptedLicenseFilePath = GetEncryptedLicenseFilePath();
    /*
    QFile file(ACT_ENCRYPTED_LICENSE_FILE_NAME);
    */
    QFile file(encryptedLicenseFilePath);
    if (!file.open(QIODevice::ReadOnly)) {
      // The license.txt file doesn't exist at beginning
      /*
      qCritical("License file doesn't exists: %s", ACT_ENCRYPTED_LICENSE_FILE_NAME);
      */
      qCritical("License file doesn't exists: %s", encryptedLicenseFilePath);
      // kene-
      act_status->SetActStatus(ActStatusType::kNotFound, ActSeverity::kCritical);
      return act_status;
    }

    // Read all contents at one time
    content = file.readAll();

    // Close the license file
    file.close();

    // Decrypt the content
    QString lic_str = this->Decrypt(content);

    // Verify the license content
    act_status = ReadPlainLicense(lic_str);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "ReadPlainLicense(): failed";
      return act_status;
    }

    return act_status;
  }

  /**
   * @brief Read encrypted license from input string
   *
   * @param content
   * @return ACT_STATUS
   */
  ACT_STATUS ReadEncryptedLicense(QString content) {
    ACT_STATUS_INIT();

    // Decrypt the content
    QString lic_str = this->Decrypt(content);

    // Verify the license content
    act_status = ReadPlainLicense(lic_str);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "ReadPlainLicense(): failed";
      return act_status;
    }

    return act_status;
  }

  /**
   * @brief Write encrypted license file
   *
   * @return ACT_STATUS
   */
  ACT_STATUS WriteEncryptedLicense(QString &encrypted) {
    ACT_STATUS_INIT();

    // kene+
    /*
    QFile file(ACT_ENCRYPTED_LICENSE_FILE_NAME);
    */
    QFile file(GetEncryptedLicenseFilePath());
    // kene-
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qCritical() << "Cannot open file for writing:" << qPrintable(file.errorString());
      return std::make_shared<ActStatusInternalError>("License");
    }

    file.write(encrypted.toStdString().c_str());

    file.close();
    return act_status;
  }

  /**
   * @brief Write encrypted license file
   *
   * @return ACT_STATUS
   */
  ACT_STATUS WriteEncryptedLicense() {
    ACT_STATUS_INIT();

    // 2022/01/07: we choose this format to dump the json document
    QString encrypted = this->Encrypt();

    // kene+
    /*
    QFile file(ACT_ENCRYPTED_LICENSE_FILE_NAME);
    */
    QFile file(GetEncryptedLicenseFilePath());
    // kene-
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qCritical() << "Cannot open file for writing:" << qPrintable(file.errorString());
      return std::make_shared<ActStatusInternalError>("License");
    }

    file.write(encrypted.toStdString().c_str());

    file.close();
    return act_status;
  }

  /**
   * @brief Read License.json from system
   *
   * @return ACT_STATUS
   */
  ACT_STATUS ReadLicenseJsonFile(QString lic_file_name) {
    ACT_STATUS_INIT();

    QFile file(lic_file_name);

    // Open the license file
    if (!file.open(QIODevice::ReadWrite)) {
      qCritical() << "open json file failed:" << lic_file_name;
      act_status->SetActStatus(ActStatusType::kLicenseContentFailed, ActSeverity::kCritical);
      return act_status;
    }

    // Read all contents at one time
    QByteArray ba = file.readAll();

    // Close the license file
    file.close();

    return ReadPlainLicense(QString(ba));
  }

  /**
   * @brief Write license file
   *
   * @return ACT_STATUS
   */
  ACT_STATUS WriteLicenseJsonFile(QString output_file_name) {
    ACT_STATUS_INIT();

    // 2022/01/07: we choose this format to dump the json document

    QFile file(output_file_name);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
      qCritical() << "Cannot open file for writing:" << qPrintable(file.errorString());
      return std::make_shared<ActStatusInternalError>("License");
    }

    // QTextStream out(&file);
    QByteArray lic_json_ba = this->toRawJson();
    QJsonDocument lic_json_doc = QJsonDocument::fromJson(lic_json_ba);
    // out << lic_json_doc.toJson(QJsonDocument::Indented).toStdString().c_str();
    file.write(lic_json_doc.toJson(QJsonDocument::Indented));

    file.close();
    return act_status;
  }

  /**
   * @brief Encrypt the license
   *
   * @return QString
   */
  QString Encrypt() {
    SimpleCrypt lic_crypto;
    lic_crypto.setKey(ACT_LICENSE_SECRET_KEY);

    QString tmp = this->ToString();
    QString encrypted = lic_crypto.encryptToString(tmp);
    return encrypted;
  }

  /**
   * @brief Decrypt the encrypted license string
   *
   * @param encrypted the encrypted license string
   * @return QString
   */
  QString Decrypt(QString &encrypted) {
    SimpleCrypt lic_crypto;
    lic_crypto.setKey(ACT_LICENSE_SECRET_KEY);

    QString decrypted = lic_crypto.decryptToString(encrypted);
    return decrypted;
  }

  /**
   * @brief Transfer license content to logger message
   *
   * Hide the real format of the license content
   *
   * @return QString
   */
  QString ToLogString() {
    QString message = "\n";
    QString date = this->GetExpireDate().toString("yyyy/MM/dd");
    message.append(QString("Expire Date: %1\n").arg(date));
    message.append(QString("Major Version: %1\n").arg(this->GetMajorVersion()));
    message.append(QString("Bound MAC Address: %1\n").arg(this->GetBoundMacAddress()));

    message.append(
        QString("Deployment Type: %1\n")
            .arg(GetStringFromEnum<ActDeploymentTypeEnum>(this->GetDeploymentType(), kActDeploymentTypeEnumMap)));

    message.append(QString("Size:\n"));
    message.append(QString("\tProject Size: %1\n").arg(this->GetSize().GetProjectSize()));
    message.append(QString("\tDevice Quantity: %1\n").arg(this->GetSize().GetDeviceQty()));
    message.append(QString("Profiles:\n"));
    for (auto profile : this->GetProfiles()) {
      message.append(QString("\t%1\n").arg(
          GetStringFromEnum<ActServiceProfileForLicenseEnum>(profile, kActServiceProfileForLicenseEnumMap)));
    }
    message.append(QString("Feature:\n"));
    message.append(QString("\tHttps State: %1\n").arg((this->GetFeature().GetHttps() ? "Enabled" : "Disabled")));
    message.append(QString("\tOpcua State: %1\n").arg((this->GetFeature().GetOpcua() ? "Enabled" : "Disabled")));
    message.append(
        QString("\tIntelligent State: %1\n").arg((this->GetFeature().GetIntelligent() ? "Enabled" : "Disabled")));
    message.append(QString("\tAuto Probe: %1\n").arg((this->GetFeature().GetAutoProbe() ? "Enabled" : "Disabled")));
    message.append(QString("\tDevice Config: %1\n")
                       .arg((this->GetFeature().GetDeviceConfig().GetEnabled() ? "Enabled" : "Disabled")));
    message.append(QString("\tTSN: %1\n").arg((this->GetFeature().GetTSN().GetEnabled() ? "Enabled" : "Disabled")));
    message.append(
        QString("\t\tScheduling: %1\n").arg((this->GetFeature().GetTSN().GetScheduling() ? "Enabled" : "Disabled")));
    message.append(QString("\t\tFRER: %1\n").arg((this->GetFeature().GetTSN().GetFRER() ? "Enabled" : "Disabled")));
    message.append(QString("\tStage:\n"));
    message.append(
        QString("\t\tDesign: %1\n").arg((this->GetFeature().GetStage().GetDesign() ? "Enabled" : "Disabled")));
    message.append(
        QString("\t\tOperation: %1\n").arg((this->GetFeature().GetStage().GetOperation() ? "Enabled" : "Disabled")));
    message.append(QString("\t\tManufacture: %1\n")
                       .arg((this->GetFeature().GetStage().GetManufacture() ? "Enabled" : "Disabled")));

    return message;
  }
};
