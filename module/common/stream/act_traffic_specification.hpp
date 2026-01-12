/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"
#include "act_system.hpp"

/**
 * @brief The FRER type enum class
 *
 */
enum class ActFrerTypeEnum { kSplit, kForward, kMerge, kUndefined };

/**
 * @brief The FRER type enum mapping
 *
 */
static const QMap<QString, ActFrerTypeEnum> kActFrerTypeEnumMap = {{"Split", ActFrerTypeEnum::kSplit},
                                                                   {"Forward", ActFrerTypeEnum::kForward},
                                                                   {"Merge", ActFrerTypeEnum::kMerge},
                                                                   {"Undefined", ActFrerTypeEnum::kUndefined}};

/**
 * @brief The ActTimeAware class
 *
 */
class ActTimeAware : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE
  ACT_JSON_FIELD(quint32, earliest_transmit_offset, EarliestTransmitOffset);  ///< EarliestTransmitOffset item
  ACT_JSON_FIELD(quint32, latest_transmit_offset, LatestTransmitOffset);      ///< LatestTransmitOffset item
  ACT_JSON_FIELD(quint32, jitter, Jitter);                                    ///< Jitter item (nanosecond)

 public:
  /**
   * @brief Construct a new Time Aware object
   *
   */
  ActTimeAware()
      : jitter_(ACT_JITTER_MIN),
        earliest_transmit_offset_(ACT_TIME_AWARE_MIN),
        latest_transmit_offset_(ACT_TIME_AWARE_MAX) {}

  /**
   * @brief Construct a new Time Aware object
   *
   * @param earliest_transmit_offset
   * @param latest_transmit_offset
   * @param jitter
   */
  ActTimeAware(const quint32 &earliest_transmit_offset, const quint32 &latest_transmit_offset, const quint32 &jitter)
      : earliest_transmit_offset_(earliest_transmit_offset),
        latest_transmit_offset_(latest_transmit_offset),
        jitter_(jitter) {}
};

/**
 * @brief The ActInterval rational number class
 *
 */
class ActInterval : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, denominator, Denominator);  ///< Denominator item
  ACT_JSON_FIELD(quint32, numerator, Numerator);      ///< Numerator item
 public:
  // static const quint32 kNanosecond = 1000000000L;
  // static const quint32 kNsMax = 1000000000L;
  // static const quint32 kNsMin = 30000L;

  /**
   * @brief Construct a new ActInterval object
   *
   */
  ActInterval() : numerator_(ACT_INTERVAL_MIN), denominator_(1000000000) {}

  ActInterval(const quint32 &numerator) : ActInterval() { this->numerator_ = numerator; }

  /**
   * @brief Construct a new ActInterval object
   *
   * @param denominator
   * @param numerator
   */
  ActInterval(const quint32 &denominator, const quint32 &numerator)
      : denominator_(denominator), numerator_(numerator) {}
};

/**
 * @brief The Stream's TrafficSpecification class
 * 8021qcc 2018: 46.2.3.5
 *
 */
class ActTrafficSpecification : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint32, max_frame_size, MaxFrameSize);                   ///< MaxFrameSize item
  ACT_JSON_FIELD(quint32, max_frames_per_interval, MaxFramesPerInterval);  ///< MaxFramesPerInterval item
  ACT_JSON_OBJECT(ActInterval, interval, Interval);                        ///< Interval object item
  ACT_JSON_OBJECT(ActTimeAware, time_aware, TimeAware);                    ///< TimeAware object item
  ACT_JSON_FIELD(bool, enable_max_bytes_per_interval,
                 EnableMaxBytesPerInterval);  ///< EnableMaxBytesPerInterval item (not in 8021qcc-2018)
  ACT_JSON_FIELD(quint32, max_bytes_per_interval,
                 MaxBytesPerInterval);  ///< MaxBytesPerInterval item (not in 8021qcc-2018)

  // Tom: 2022/03/17
  // Moxa TSN Switch only support strict priority
  // ACT_JSON_FIELD(quint8, transmission_selection, TransmissionSelection);   ///< TransmissionSelection item

 public:
  ActTrafficSpecification() {
    this->SetMaxFrameSize(ACT_FRAME_SIZE_MIN);
    this->SetMaxFramesPerInterval(ACT_FRAME_PER_INTERVAL_MIN);
    this->SetEnableMaxBytesPerInterval(false);
    this->SetMaxBytesPerInterval(0);
  }
};
