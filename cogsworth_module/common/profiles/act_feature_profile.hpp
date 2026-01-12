/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"
#include "act_status.hpp"
#include "act_system.hpp"
// #include "act_utilities.hpp"
// #include "json_utils.hpp"

/**
 * @brief The SNMP version class enum class
 *
 */
enum class ActFeatureEnum { kBase = 1, kAutoScan = 2, kOperation = 3, kConfiguration = 4, kMonitor = 5 };

/**
 * @brief The QMap SNMP version class enum mapping
 *
 */
static const QMap<QString, ActFeatureEnum> kActFeatureEnumMap = {{"Base", ActFeatureEnum::kBase},
                                                                 {"AutoScan", ActFeatureEnum::kAutoScan},
                                                                 {"Operation", ActFeatureEnum::kOperation},
                                                                 {"Configuration", ActFeatureEnum::kConfiguration},
                                                                 {"Monitor", ActFeatureEnum::kMonitor}};

/**
 * @brief The ActConnectProtocol type enum class
 *
 */
enum class ActConnectProtocolTypeEnum { kEmpty, kMOXAcommand, kSNMP, kNETCONF, kRESTful };

/**
 * @brief The QMap for ActSouthboundProtocol type enum mapping
 *
 */
static const QMap<QString, ActConnectProtocolTypeEnum> kActConnectProtocolTypeEnumMap = {
    {"", ActConnectProtocolTypeEnum::kEmpty},
    {"MOXAcommand", ActConnectProtocolTypeEnum::kMOXAcommand},
    {"SNMP", ActConnectProtocolTypeEnum::kSNMP},
    {"NETCONF", ActConnectProtocolTypeEnum::kNETCONF},
    {"RESTful", ActConnectProtocolTypeEnum::kRESTful}};

/**
 * @brief The Feature's item class
 *
 */
class ActMethodAction : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, source, Source);
  ACT_JSON_FIELD(QString, path, Path);

 public:
  /**
   * @brief Construct a new Act Feature Action object
   *
   */
  ActMethodAction() {
    this->source_ = "";
    this->path_ = "";
  }
};

class ActFeatureMethodProtocol : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActMethodAction, actions, Actions);  ///< The Items map
};

class ActFeatureMethod : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActFeatureMethodProtocol, protocols, Protocols);  ///< The Items map
};

class ActFeatureSubItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActFeatureMethod, methods, Methods);  ///< The Items map
  ACT_JSON_COLLECTION(QList, QString, probe_sequence, ProbeSequence);

 public:
  QList<QString> key_order_;
  ActFeatureSubItem() { this->key_order_ = QList<QString>({QString("Methods")}); }
};

class ActFeatureItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActFeatureSubItem, sub_items, SubItems);  ///< The Items map
};

class ActFeatureSubItemFindParameters : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, device_profile_id, DeviceProfileID);  ///< The DeviceProfile ID
  ACT_JSON_ENUM(ActFeatureEnum, feature, Feature);             ///< Feature item
  ACT_JSON_FIELD(QString, item, Item);
  ACT_JSON_FIELD(QString, sub_item, SubItem);

 public:
  ActFeatureSubItemFindParameters() {
    this->device_profile_id_ = -1;
    this->item_ = "";
    this->sub_item_ = "";
  }
};

/**
 * @brief The FeatureProfile class
 *
 */
class ActFeatureProfile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);  ///< The unique id
  ACT_JSON_FIELD(QString, data_version, DataVersion);
  ACT_JSON_ENUM(ActFeatureEnum, feature, Feature);                        ///< Feature item
  ACT_JSON_QT_DICT_OBJECTS(QMap, QString, ActFeatureItem, items, Items);  ///< The Items map

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Feature Profile object
   *
   */
  ActFeatureProfile() {
    this->id_ = -1;
    this->data_version_ = ACT_FEATURE_PROFILE_DATA_VERSION;
    this->key_order_.append(
        QList<QString>({QString("Id"), QString("DataVersion"), QString("Feature"), QString("Items")}));
  }

  /**
   * @brief Construct a new Act Feature Profile object
   *
   * @param feature
   */
  ActFeatureProfile(const ActFeatureEnum &feature) : ActFeatureProfile() {
    this->feature_ = feature;
    this->id_ = static_cast<qint64>(feature);
  }

  /**
   * @brief Get the Feature Sub-Item object
   *
   * @param item_key
   * @param sub_item_key
   * @param result_feat_sub_item
   * @return ACT_STATUS
   */
  ACT_STATUS GetFeatureSubItem(const QString &item_key, const QString &sub_item_key,
                               ActFeatureSubItem &result_feat_sub_item) {
    ACT_STATUS_INIT();

    // Get Item element
    if (!this->items_.contains(item_key)) {
      QString not_found_elem =
          QString("Item(%1) of the Feature(%2)").arg(item_key).arg(kActFeatureEnumMap.key(this->feature_));

      qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
      return std::make_shared<ActStatusNotFound>(not_found_elem);
    }
    auto feature_item = this->items_[item_key];

    // Get Sub-Item element
    if (!feature_item.GetSubItems().contains(sub_item_key)) {
      QString not_found_elem = QString("Sub-Item(%1) of the Feature(%2) > Item(%3)")
                                   .arg(sub_item_key)
                                   .arg(kActFeatureEnumMap.key(this->feature_))
                                   .arg(item_key);

      qCritical() << __func__ << QString("The %1 is not found").arg(not_found_elem);
      return std::make_shared<ActStatusNotFound>(not_found_elem);
    }
    result_feat_sub_item = feature_item.GetSubItems()[sub_item_key];

    return act_status;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActFeatureProfile &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActFeatureProfile &x, const ActFeatureProfile &y) {
    return (x.id_ == y.id_) || (x.feature_ == y.feature_);
  }
};

// TODO: remove

/**
 * @brief The Method's item class
 *
 */
class ActMethodItem : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, item, Item);  ///< Item item
  ACT_JSON_FIELD(QString, path, Path);  ///< Path item

 public:
  /**
   * @brief Construct a new Act Method Item object
   *
   */
  ActMethodItem() {
    this->item_ = "";
    this->path_ = "";
  }

  /**
   * @brief Construct a new Act Method Item object
   *
   */
  ActMethodItem(const QString &item) : ActMethodItem() { this->item_ = item; }

  /**
   * @brief Construct a new Act Method Item object
   *
   */
  ActMethodItem(const QString &item, const QString &path) {
    this->item_ = item;
    this->path_ = path;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActMethodItem &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActMethodItem &x, const ActMethodItem &y) { return x.item_ == y.item_; }
};

/**
 * @brief The Action's method class
 *
 */
class ActActionMethod : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActConnectProtocolTypeEnum, protocol, Protocol);  ///< Protocol type enum item
  ACT_JSON_FIELD(QString, key, Key);                              ///< Key item
  // ACT_JSON_COLLECTION_OBJECTS(QList, ActMethodItem, items, Items);   ///< Items list
  ACT_JSON_QT_SET_OBJECTS(ActMethodItem, items, Items);  ///< Items set

 public:
  /**
   * @brief Construct a new Act Action Method object
   *
   */
  ActActionMethod() {
    this->key_ = "";
    this->protocol_ = ActConnectProtocolTypeEnum::kEmpty;
  }

  /**
   * @brief Construct a new Act Action Method object
   *
   * @param key
   */
  ActActionMethod(const QString &key) {
    this->key_ = key;
    this->protocol_ = ActConnectProtocolTypeEnum::kEmpty;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActActionMethod &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActActionMethod &x, const ActActionMethod &y) { return x.key_ == y.key_; }
};

/**
 * @brief The Feature's action class
 *
 */
class ActFeatureAction : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, action, Action);                                ///< Action item
  ACT_JSON_FIELD(bool, must, Must);                                       ///< Must flag
  ACT_JSON_COLLECTION_OBJECTS(QList, ActActionMethod, methods, Methods);  ///< Methods list

 public:
  /**
   * @brief Construct a new Act Feature Action object
   *
   */
  ActFeatureAction() {
    this->action_ = "";
    this->must_ = true;
  }

  /**
   * @brief Construct a new Act Feature Action object
   *
   * @param action
   */
  ActFeatureAction(const QString &action) : ActFeatureAction() { this->action_ = action; }

  /**
   * @brief Get the Method By Std Find object
   *
   * @param target_action
   * @return ACT_STATUS
   */
  ACT_STATUS GetMethodByStdFind(ActActionMethod &target_method) {
    ACT_STATUS_INIT();

    auto method_it = std::find(this->methods_.begin(), this->methods_.end(), target_method);
    if (method_it == this->methods_.end()) {  // not found
      qCritical() << __func__
                  << QString("Method not found. Method(%1) in %2 Action")
                         .arg(target_method.GetKey())
                         .arg(this->action_)
                         .toStdString()
                         .c_str();
      return std::make_shared<ActStatusNotFound>(QString("Method(%1)").arg(target_method.GetKey()));
    }

    target_method = ActActionMethod(*method_it);
    return act_status;
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActFeatureAction &x) {
    // no effect on the program's behavior
    static_cast<void>(x);
    return 0;
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActFeatureAction &x, const ActFeatureAction &y) { return x.action_ == y.action_; }
};