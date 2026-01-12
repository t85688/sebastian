/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "stream/field/act_ieee_802_mac_addresses.hpp"
#include "stream/field/act_ieee_802_vlan_tag.hpp"
#include "stream/field/act_ipv4_tuple.hpp"
#include "stream/field/act_ipv6_tuple.hpp"

/**
 * @brief The Field type enum class
 *
 */
enum class ActFieldTypeEnum {
  kIeee802MacAddresses,  // 46.2.3.4.1
  kIeee802VlanTag        // 46.2.3.4.2
  // kIpv4Tuple,            // 46.2.3.4.3
  // kIpv6Tuple             // 46.2.3.4.4
  // kFieldTypeTimeAwareOffset // not in 8021qcc-2018
};

/**
 * @brief The QMap for Field type enum mapping
 *
 */
static const QMap<QString, ActFieldTypeEnum> kActFieldTypeEnumMap = {
    {"Ieee802MacAddresses", ActFieldTypeEnum::kIeee802MacAddresses},
    {"Ieee802VlanTag", ActFieldTypeEnum::kIeee802VlanTag}
    // {"Ipv4Tuple", ActFieldTypeEnum::kIpv4Tuple},
    // {"Ipv6Tuple", ActFieldTypeEnum::kIpv6Tuple}
    // {"TimeAwareOffset", ActFieldTypeEnum::kTimeAwareOffset}
};

/**
 * @brief The Stream's Field class
 *
 */
class ActField : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActFieldTypeEnum, type, Type);                                          ///< Field type enum
  ACT_JSON_OBJECT(ActIeee802MacAddresses, ieee802_mac_addresses, Ieee802MacAddresses);  ///< Ieee802MacAddresses item
  ACT_JSON_OBJECT(ActIeee802VlanTag, ieee802_vlan_tag, Ieee802VlanTag);                 ///< Ieee802VlanTag item
  // ACT_JSON_OBJECT(ActIpv4Tuple, ipv4_tuple, Ipv4Tuple);                                 ///< Ipv4Tuple item
  // ACT_JSON_OBJECT(ActIpv6Tuple, ipv6_tuple, Ipv6Tuple);

 public:
  /**
   * @brief Construct a new Field object
   *
   */
  ActField() { this->type_ = ActFieldTypeEnum::kIeee802VlanTag; }

  /**
   * @brief Construct a new Field object
   *
   * @param ieee802_mac_addresses
   */
  ActField(const ActIeee802MacAddresses &ieee802_mac_addresses)
      : type_(ActFieldTypeEnum::kIeee802MacAddresses), ieee802_mac_addresses_(ieee802_mac_addresses) {}

  /**
   * @brief Construct a new Field object
   *
   * @param ieee802_vlan_tag
   */
  ActField(const ActIeee802VlanTag &ieee802_vlan_tag)
      : type_(ActFieldTypeEnum::kIeee802VlanTag), ieee802_vlan_tag_(ieee802_vlan_tag) {}

  /**
   * @brief Construct a new Field object
   *
   * @param ipv4_tuple
   */
  // ActField(const ActIpv4Tuple& ipv4_tuple) : type_(ActFieldTypeEnum::kIpv4Tuple), ipv4_tuple_(ipv4_tuple) {}

  /**
   * @brief Construct a new Field object
   *
   * @param ipv6_tuple
   */
  // ActField(const ActIpv6Tuple& ipv6_tuple) : type_(ActFieldTypeEnum::kIpv6Tuple), ipv6_tuple_(ipv6_tuple) {}
};

// /**
//  * @brief The Stream's Field class
//  *
//  * @tparam T The derives class
//  */
// template <typename T>
// class Field : public T {
//  public:
//   ActField() : T(){};
//   // DeployResult(QString device_ip) : T(device_ip){};
// };

// /**
//  * @brief The Stream's Field class
//  *
//  */
// class FieldBase : public QSerializer {
//   Q_GADGET
//   QS_SERIALIZABLE

//   ACT_JSON_ENUM(ActFieldTypeEnum, type, Type);  ///< Field type enum
// };
