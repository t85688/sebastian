/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_project.hpp"
#include "topology/act_device.hpp"

enum class ActBaselineModeEnum { kDesign = 1, kOperation = 2 };

static const QMap<QString, ActBaselineModeEnum> kActBaselineModeEnumMap = {
    {"Design", ActBaselineModeEnum::kDesign}, {"Operation", ActBaselineModeEnum::kOperation}};

class ActNetworkBaselineDevice : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_id, DeviceId);
  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(QString, firmware_version, FirmwareVersion);
  ACT_JSON_FIELD(QString, configuration, Configuration);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act NetworkBaseline object
   *
   */
  ActNetworkBaselineDevice() {
    this->key_order_.append(QList<QString>({QString("DeviceId"), QString("IpAddress"), QString("ModelName"),
                                            QString("FirmwareVersion"), QString("Configuration")}));

    this->device_id_ = -1;
    this->ip_address_ = "";
    this->model_name_ = "";
    this->firmware_version_ = "";
    this->configuration_ = "";
  }

  /**
   * @brief Construct a new Act NetworkBaseline object
   *
   * @param id
   */
  ActNetworkBaselineDevice(const qint64 &device_id, const QString &ip_address, const QString &model_name,
                           const QString &firmware_version)
      : ActNetworkBaselineDevice() {
    this->device_id_ = device_id;
    this->ip_address_ = ip_address;
    this->model_name_ = model_name;
    this->firmware_version_ = firmware_version;
  }

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActNetworkBaselineDevice &x, const ActNetworkBaselineDevice &y) {
    return x.device_id_ == y.device_id_;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActNetworkBaselineDevice &obj) {
    return qHash(obj.device_id_, 0);  // arbitrary value is 0
  }
};

class ActNetworkBaseline : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_FIELD(QString, name, Name);
  ACT_JSON_FIELD(quint64, date, Date);
  ACT_JSON_FIELD(QString, created_user, CreatedUser);
  ACT_JSON_FIELD(qint64, project_id, ProjectId);
  ACT_JSON_FIELD(bool, activate, Activate);
  ACT_JSON_FIELD(QString, activated_user, ActivatedUser);
  ACT_JSON_FIELD(quint64, activated_date, ActivatedDate);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_ENUM(ActBaselineModeEnum, mode, Mode);

  ACT_JSON_QT_SET_OBJECTS(ActNetworkBaselineDevice, devices, Devices);

  // For DB
  ACT_JSON_OBJECT(ActProject, project, Project);

 public:
  QList<QString> key_order_;
  QList<QString> write_db_key_order_;

  /**
   * @brief Construct a new Act NetworkBaseline object
   *
   */
  ActNetworkBaseline() {
    this->key_order_.append(
        QList<QString>({QString("Id"), QString("DataVersion"), QString("Name"), QString("Date"), QString("CreatedUser"),
                        QString("ProjectId"), QString("Activate"), QString("ActivatedUser"), QString("ActivatedDate"),
                        QString("Description"), QString("Mode"), QString("Devices")}));
    this->write_db_key_order_.append(
        QList<QString>({QString("Id"), QString("DataVersion"), QString("Name"), QString("Date"), QString("CreatedUser"),
                        QString("ProjectId"), QString("Activate"), QString("ActivatedUser"), QString("ActivatedDate"),
                        QString("Description"), QString("Mode"), QString("Project")}));

    this->id_ = -1;
    this->data_version_ = ACT_BASELINE_DATA_VERSION;
    this->project_id_ = -1;
    this->name_ = "";
    this->date_ = 0;
    this->created_user_ = "";

    this->activate_ = false;
    this->activated_user_ = "";
    this->activated_date_ = 0;
    this->description_ = "";
    this->mode_ = ActBaselineModeEnum::kDesign;
  }

  /**
   * @brief Construct a new Act NetworkBaseline object
   *
   * @param id
   */
  ActNetworkBaseline(const qint64 &id) : ActNetworkBaseline() { this->id_ = id; }

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActNetworkBaseline &x, const ActNetworkBaseline &y) { return x.id_ == y.id_; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActNetworkBaseline &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }

  /**
   * @brief Hide the password
   *
   * @return QString
   */
  ACT_STATUS HidePassword() {
    ACT_STATUS_INIT();

    this->project_.HidePassword();
    return act_status;
  }

  /**
   * @brief Encrypt the password
   *
   * @return QString
   */
  ACT_STATUS EncryptPassword() {
    ACT_STATUS_INIT();

    this->project_.EncryptPassword();

    return act_status;
  }

  /**
   * @brief Decrypt the encrypted password
   *
   * @return QString
   */
  ACT_STATUS DecryptPassword() {
    ACT_STATUS_INIT();

    this->project_.DecryptPassword();
    return act_status;
  }
};

class ActSimpleNetworkBaseline : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, name, Name);
  ACT_JSON_FIELD(quint64, date, Date);
  ACT_JSON_FIELD(QString, created_user, CreatedUser);
  ACT_JSON_FIELD(qint64, project_id, ProjectId);
  ACT_JSON_FIELD(bool, activate, Activate);
  ACT_JSON_FIELD(QString, activated_user, ActivatedUser);
  ACT_JSON_FIELD(quint64, activated_date, ActivatedDate);
  ACT_JSON_FIELD(QString, description, Description);
  ACT_JSON_ENUM(ActBaselineModeEnum, mode, Mode);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act SimpleNetworkBaseline object
   *
   */
  ActSimpleNetworkBaseline() {
    this->key_order_.append(
        QList<QString>({QString("Id"), QString("Name"), QString("Date"), QString("ProjectId"), QString("Activate"),
                        QString("ActivatedUser"), QString("ActivatedDate"), QString("Description"), QString("Mode")}));

    this->id_ = -1;
    this->name_ = "";
    this->date_ = 0;
    this->created_user_ = "";
    this->project_id_ = -1;
    this->activate_ = false;
    this->activated_user_ = "";
    this->activated_date_ = 0;
    this->description_ = "";
    this->mode_ = ActBaselineModeEnum::kDesign;
  }

  /**
   * @brief Construct a new Act SimpleNetworkBaseline object
   *
   * @param id
   */
  ActSimpleNetworkBaseline(const ActNetworkBaseline &network_baseline) : ActSimpleNetworkBaseline() {
    this->id_ = network_baseline.GetId();
    this->name_ = network_baseline.GetName();
    this->date_ = network_baseline.GetDate();
    this->created_user_ = network_baseline.GetCreatedUser();
    this->project_id_ = network_baseline.GetProjectId();
    this->activate_ = network_baseline.GetActivate();
    this->activated_user_ = network_baseline.GetActivatedUser();
    this->activated_date_ = network_baseline.GetActivatedDate();
    this->description_ = network_baseline.GetDescription();
    this->mode_ = network_baseline.GetMode();
  }

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActSimpleNetworkBaseline &x, const ActSimpleNetworkBaseline &y) {
    return x.id_ == y.id_;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActSimpleNetworkBaseline &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }
};

class ActNetworkBaselineList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActSimpleNetworkBaseline, network_baseline_list, NetworkBaselineList);
};

class ActNetworkBaselineInfo : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, name, Name);
  ACT_JSON_FIELD(QString, description, Description);

 public:
  /**
   * @brief Construct a new Act ActNetworkBaselineInfo object
   *
   */
  ActNetworkBaselineInfo() {
    this->name_ = "";
    this->description_ = "";
  }
};

class ActBaselineBOMDetail : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, name, Name);
  ACT_JSON_FIELD(quint64, date, Date);
  ACT_JSON_FIELD(QString, created_user, CreatedUser);
  ACT_JSON_FIELD(qint64, project_id, ProjectId);
  ACT_JSON_FIELD(QString, total_price, TotalPrice);
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActSkuQuantity, sku_quantities_map, SkuQuantitiesMap);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Baseline BOM Detail object
   *
   */
  ActBaselineBOMDetail() {
    this->key_order_.append(QList<QString>({QString("Id"), QString("Name"), QString("Date"), QString("CreatedUser"),
                                            QString("ProjectId"), QString("TotalPrice"), QString("SkuQuantitiesMap")}));

    this->id_ = -1;
    this->project_id_ = -1;
    this->name_ = "";
    this->date_ = 0;
    this->created_user_ = "";
    this->total_price_ = "N/A";
  }

  /**
   * @brief Construct a new Baseline BOM Detail object
   *
   * @param network_baseline
   */
  ActBaselineBOMDetail(const ActNetworkBaseline &network_baseline) : ActBaselineBOMDetail() {
    this->id_ = network_baseline.GetId();
    this->project_id_ = network_baseline.GetProjectId();
    this->name_ = network_baseline.GetName();
    this->date_ = network_baseline.GetDate();
    this->created_user_ = network_baseline.GetCreatedUser();
    this->sku_quantities_map_ = network_baseline.GetProject().GetSkuQuantitiesMap();

    // Set Total Price
    double total_price = 0.0;
    for (auto key : network_baseline.GetProject().GetSkuQuantitiesMap().keys()) {
      auto sku_quantity = network_baseline.GetProject().GetSkuQuantitiesMap()[key];
      total_price += sku_quantity.GetQuantity() * sku_quantity.GetPrice().toDouble();
    }

    if (total_price != 0.0) {
      ActCurrencyEnum currency = network_baseline.GetProject().GetSkuQuantitiesMap().first().GetCurrency();
      this->total_price_ =
          QString("%1 %2").arg(kActCurrencyEnumMap.key(currency)).arg(QString::number(total_price, 'f', 2));
    }
  }

  /**
   * @brief The equal operator
   *
   * @param source The copied source
   */
  friend bool operator==(const ActBaselineBOMDetail &x, const ActBaselineBOMDetail &y) { return x.id_ == y.id_; }

  /**
   * @brief The hash function for QSet
   *
   * @param obj
   * @return uint
   */
  friend uint qHash(const ActBaselineBOMDetail &obj) {
    return qHash(obj.id_, 0);  // arbitrary value is 0
  }
};

class ActBaselineProjectDiffDetail : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, bom, BOM);
  ACT_JSON_FIELD(bool, device_config, DeviceConfig);
  ACT_JSON_FIELD(bool, topology_device, TopologyDevice);
  ACT_JSON_FIELD(bool, topology_link, TopologyLink);
  ACT_JSON_FIELD(bool, project_setting, ProjectSetting);

 public:
  /**
   * @brief Construct a new Act ActBaselineProjectDiffDetail object
   *
   */
  ActBaselineProjectDiffDetail() {
    this->bom_ = false;
    this->device_config_ = false;
    this->topology_device_ = false;
    this->topology_link_ = false;
    this->project_setting_ = false;
  }
};

class ActBaselineProjectDiffReport : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(qint64, project_id, ProjectId);
  ACT_JSON_FIELD(bool, has_diff, HasDiff);
  ACT_JSON_OBJECT(ActBaselineProjectDiffDetail, diff_detail, DiffDetail);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Baseline BOM Detail object
   *
   */
  ActBaselineProjectDiffReport() {
    this->key_order_.append(
        QList<QString>({QString("Id"), QString("ProjectId"), QString("HasDiff"), QString("DiffDetail")}));

    this->id_ = -1;
    this->project_id_ = -1;
    this->has_diff_ = false;
  }
};
