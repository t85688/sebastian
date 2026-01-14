/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The AlgorithmConfiguration class
 *
 */
class ActAlgorithmConfiguration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, media_specific_overhead_bytes,
                 MediaSpecificOverheadBytes);  ///< MediaSpecificOverheadBytes item
  // ACT_JSON_FIELD(bool, qbv_after_cb_merge, QbvAfterCbMerge);       ///< QbvAfterCbMerge bool item
  ACT_JSON_FIELD(quint32, time_sync_delay, TimeSyncDelay);  ///< TimeSyncDelay item
  ACT_JSON_FIELD(quint32, timeout, Timeout);                ///< Timeout in seconds
  // ACT_JSON_FIELD(quint32, stage, Stage);                           ///< Stage of the incremental mode (0 for batch)
  ACT_JSON_FIELD(quint32, best_effort_bandwidth, BestEffortBandwidth);  ///< Best Effort bandwidth
  ACT_JSON_FIELD(quint32, time_sync_bandwidth, TimeSyncBandwidth);      ///< Time Sync bandwidth
  ACT_JSON_FIELD(bool, keep_previous_result, KeepPreviousResult);       ///< KeepPreviousResult bool item (optional)

 public:
  /**
   * @brief Construct a new Algorithm Configuration object
   *
   */
  ActAlgorithmConfiguration() {
    // Default is 43 bytes
    // - 8-byte preamble
    // - 14-byte IEEE 802.3 header
    // - 4-byte IEEE 802.1Q priority/VlanId Tag
    // - 4-byte CRC
    // - 12-byte inter-frame gap
    // - 1-byte tolerance
    // - (Optional) 6-bytes R-tag for CB
    this->media_specific_overhead_bytes_ = 43;
    // this->qbv_after_cb_merge_ = false;
    this->time_sync_delay_ = 200;
    this->timeout_ = 9999;
    this->best_effort_bandwidth_ = 35;
    this->time_sync_bandwidth_ = 35;
    // this->stage_ = 0;
    this->keep_previous_result_ = false;
  }
};
