/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The Field's Ieee802VlanTag class
 *
 */
class ActIeee802VlanTag : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, vlan_id, VlanId);                        ///< VlanId item
  ACT_JSON_FIELD(quint8, priority_code_point, PriorityCodePoint);  ///< PriorityCodePoint item (0~7)

 public:
  /**
   * @brief Construct a new Ieee 8 0 2 Vlan Tag object
   *
   */
  ActIeee802VlanTag() : priority_code_point_(0), vlan_id_(0) {}

  /**
   * @brief Construct a new Ieee 8 0 2 Vlan Tag object
   *
   * @param vid
   * @param pcp
   */
  ActIeee802VlanTag(const quint16 &vid, const quint8 &pcp) : vlan_id_(vid), priority_code_point_(pcp) {}

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActIeee802VlanTag &x, const ActIeee802VlanTag &y) {
    return (x.GetVlanId() == y.GetVlanId() && x.GetPriorityCodePoint() == y.GetPriorityCodePoint());
  }
};
