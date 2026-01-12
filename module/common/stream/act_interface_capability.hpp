/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once
#include "act_json.hpp"

/**
 * @brief The Stream's InterfaceCapability class
 * 8021qcc 2018: 46.2.3.7
 *
 */
class ActInterfaceCapability : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, cb_capable, CbCapable);             ///< CbCapable item (? not in 8021qcc)
  ACT_JSON_FIELD(bool, vlan_tag_capable, VlanTagCapable);  ///< VlanTagCapable item
  // ACT_JSON_COLLECTION(QList, qint64, cb_sequence_type_list, CbSequenceTypeList);  ///< CbSequenceTypeList item
  // ACT_JSON_COLLECTION(QList, qint64, cb_stream_iden_type_list, CbStreamIdenTypeList);  ///< CbStreamIdenTypeList item

 public:
  /**
   * @brief Construct a new Interface Capability object
   *
   */
  ActInterfaceCapability() : cb_capable_(false), vlan_tag_capable_(true) {}
};
