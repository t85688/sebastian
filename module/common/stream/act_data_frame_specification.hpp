/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "stream/field/act_field.hpp"
#include "stream/field/act_ieee_802_mac_addresses.hpp"
#include "stream/field/act_ieee_802_vlan_tag.hpp"
#include "stream/field/act_ipv4_tuple.hpp"
#include "stream/field/act_ipv6_tuple.hpp"

/**
 * @brief The Stream's DataFrameSpecification class
 * 8021qcc 2018: 46.2.3.4
 *
 */
class ActDataFrameSpecification : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_ENUM(ActFieldTypeEnum, type, Type);                                          ///< Field type enum
  ACT_JSON_OBJECT(ActIeee802MacAddresses, ieee802_mac_addresses, Ieee802MacAddresses);  ///< Ieee802MacAddresses item
  ACT_JSON_OBJECT(ActIeee802VlanTag, ieee802_vlan_tag, Ieee802VlanTag);                 ///< Ieee802VlanTag item
  // ACT_JSON_OBJECT(ActIpv4Tuple, ipv4_tuple, Ipv4Tuple);                                 ///< Ipv4Tuple item
  // ACT_JSON_OBJECT(ActIpv6Tuple, ipv6_tuple, Ipv6Tuple);                                 ///< Ipv6Tuple item

  // ACT_JSON_OBJECT(Field<FieldBase>, field, Field);  ///< Field item
  // ACT_JSON_OBJECT(Field, field, Field);  ///< Field item

 public:
  /**
   * @brief Construct a new Interface Capability object
   *
   */
  ActDataFrameSpecification() { type_ = ActFieldTypeEnum::kIeee802MacAddresses; }

  ActDataFrameSpecification(const ActIeee802MacAddresses &ieee802_mac_addresses) : ActDataFrameSpecification() {
    type_ = ActFieldTypeEnum::kIeee802MacAddresses;
    ieee802_mac_addresses_ = ieee802_mac_addresses;
  }
  ActDataFrameSpecification(const ActIeee802VlanTag &ieee802_vlan_tag) : ActDataFrameSpecification() {
    type_ = ActFieldTypeEnum::kIeee802VlanTag;
    ieee802_vlan_tag_ = ieee802_vlan_tag;
  }
  // ActDataFrameSpecification(const ActIpv4Tuple& ipv4_tuple) : ActDataFrameSpecification() {
  //   type_ = ActFieldTypeEnum::kIpv4Tuple;
  //   ipv4_tuple_ = ipv4_tuple;
  // }
  // ActDataFrameSpecification(const ActIpv6Tuple& ipv6_tuple) : ActDataFrameSpecification() {
  //   type_ = ActFieldTypeEnum::kIpv6Tuple;
  //   ipv6_tuple_ = ipv6_tuple;
  // }
};
