/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The Stream's ActUserToNetworkRequirement class
 * 8021qcc 2018: 46.2.3.6
 *
 */
class ActUserToNetworkRequirement : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, max_latency, MaxLatency);               ///< MaxLatency item (46.2.3.6.2)
  ACT_JSON_FIELD(quint8, num_seamless_trees, NumSeamlessTrees);   ///< NumSeamlessTrees item
                                                                  // 1 for 802.1CB redundancy (46.2.3.6.1)
  ACT_JSON_FIELD(quint32, min_receive_offset, MinReceiveOffset);  ///< MinReceiveOffset item (From OPCUA)
  ACT_JSON_FIELD(quint32, max_receive_offset, MaxReceiveOffset);  ///< MaxReceiveOffset item (From OPCUA)

 public:
  /**
   * @brief Construct a new User To Network Requirement object
   *
   */
  ActUserToNetworkRequirement()
      : max_latency_(ACT_LATENCY_MIN),
        num_seamless_trees_(0),
        min_receive_offset_(ACT_RECEIVE_OFFSET_MIN),
        max_receive_offset_(ACT_RECEIVE_OFFSET_MAX) {}

  /**
   * @brief Construct a new User To Network Requirement object
   *
   * @param max_latency
   * @param num_seamless_trees
   */
  ActUserToNetworkRequirement(const quint32 &max_latency, const quint8 &num_seamless_trees)
      : max_latency_(max_latency), num_seamless_trees_(num_seamless_trees) {}
};

// /**
//  * @brief Check the value in the ranges
//  *
//  * @param low
//  * @param high
//  * @param value
//  * @return true
//  * @return false
//  */
// bool inRange(const quint32 &low, const quint32 &high, const quint32 &value) {
//   return (low <= value && value <= high);
// }

// /**
//  * @brief Check the value of the max_latency & num_seamless_trees in the ranges
//  *
//  */
// void CheckValue() {
//   bool max_latency_checked = this->inRange(1000, 1000000000, this->max_latency_);
//   bool num_seamless_trees_checked = this->inRange(0, 1, this->num_seamless_trees_);

//   // if (!(max_latency_checked && num_seamless_trees_checked)) {
//   //   // TODO: throw ACT error
//   // }
// }