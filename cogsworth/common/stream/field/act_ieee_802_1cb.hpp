/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

/**
 * @brief The Field's ActSequenceGeneration class
 *
 */
class ActSequenceGeneration : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, min, Min);
  ACT_JSON_FIELD(quint16, max, Max);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new sequence generation object
   *
   */
  ActSequenceGeneration() {
    this->min_ = 0;
    this->max_ = 1023;
    this->key_order_.append(QList<QString>({QString("Min"), QString("Max")}));
  }
};

/**
 * @brief The Field's ActSequenceRecovery class
 *
 */
class ActSequenceRecovery : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, min, Min);
  ACT_JSON_FIELD(quint16, max, Max);
  ACT_JSON_FIELD(quint16, min_history_length, MinHistoryLength);
  ACT_JSON_FIELD(quint16, max_history_length, MaxHistoryLength);
  ACT_JSON_FIELD(quint16, reset_timeout, ResetTimeout);

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new sequence recovery object
   *
   */
  ActSequenceRecovery() {
    this->min_ = 0;
    this->max_ = 1535;
    this->min_history_length_ = 2;
    this->max_history_length_ = 15;
    this->reset_timeout_ = 4096;
    this->key_order_.append(QList<QString>({QString("Min"), QString("Max"), QString("MinHistoryLength"),
                                            QString("MaxHistoryLength"), QString("ResetTimeout")}));
  }
};

/**
 * @brief The Field's ActSequenceIdentification class
 *
 */
class ActSequenceIdentification : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, min, Min);
  ACT_JSON_FIELD(quint16, max, Max);

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new sequence identification object
   *
   */
  ActSequenceIdentification() {
    this->min_ = 0;
    this->max_ = 1023;
    this->key_order_.append(QList<QString>({QString("Min"), QString("Max")}));
  }
};

/**
 * @brief The Field's ActStreamIdentity class
 *
 */
class ActStreamIdentity : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint16, min, Min);
  ACT_JSON_FIELD(quint16, max, Max);
  ACT_JSON_FIELD(quint64, min_handle, MinHandle);
  ACT_JSON_FIELD(quint64, max_handle, MaxHandle);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new stream identity object
   *
   */
  ActStreamIdentity() {
    this->min_ = 0;
    this->max_ = 1535;
    this->min_handle_ = 0;
    this->max_handle_ = 2147483647;
    this->key_order_.append(
        QList<QString>({QString("Min"), QString("Max"), QString("MinHandle"), QString("MaxHandle")}));
  }
};

/**
 * @brief The Field's ActIeee802Dot1cb class
 *
 */
class ActIeee802Dot1cb : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActSequenceGeneration, sequence_generation, SequenceGeneration);
  ACT_JSON_OBJECT(ActSequenceRecovery, sequence_recovery, SequenceRecovery);
  ACT_JSON_OBJECT(ActSequenceIdentification, sequence_identification, SequenceIdentification);
  ACT_JSON_OBJECT(ActStreamIdentity, stream_identity, StreamIdentity);

 public:
  QList<QString> key_order_;

  /**
   * @brief Construct a new Act Ieee 8 0 2 Dot 1cb object
   *
   */
  ActIeee802Dot1cb() {
    this->key_order_.append(QList<QString>({QString("SequenceGeneration"), QString("SequenceRecovery"),
                                            QString("SequenceIdentification"), QString("StreamIdentity")}));
  }
};
