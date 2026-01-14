/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
// #include <QtGlobal>

#include "act_json.hpp"

#define RECOVERY_TIMEOUT (2000)
#define HISTORY_LENGTH (10)

/**
 * @brief There is one Sequence identification table per system, and one
       entry in the Sequence identification table for each port and
       direction for which an instance of the Sequence encode/decode
       function (7.6) is to be created.
 *
 */
enum ActSequenceEncodeDecodeTypes {
  kRTag = 1,                // R-TAG
  kHSRSequenceTag = 2,      // HSR sequence tag
  kPRPSequenceTrailer = 3,  // PRP sequence trailer
};

/**
 * @brief The QMap for Frame type class enum mapping
 *
 */
static const QMap<QString, ActSequenceEncodeDecodeTypes> kActSequenceEncodeDecodeTypesMap = {
    {"r-tag", ActSequenceEncodeDecodeTypes::kRTag},
    {"hsr-sequence-tag", ActSequenceEncodeDecodeTypes::kHSRSequenceTag},
    {"prp-sequence-trailer", ActSequenceEncodeDecodeTypes::kPRPSequenceTrailer}};

enum ActSequenceRecoveryAlgorithm { kVector = 0, kMatch = 1 };

/**
 * @brief The QMap for Frame type class enum mapping
 *
 */
static const QMap<QString, ActSequenceRecoveryAlgorithm> kActSequenceRecoveryAlgorithmMap = {
    {"vector", ActSequenceRecoveryAlgorithm::kVector}, {"match", ActSequenceRecoveryAlgorithm::kMatch}};

/**
 * @brief sequence identification entry
 *
 */
class ActSequenceIdentificationEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* A list of stream_handles, corresponding to the values of the
  tsnStreamIdHandle objects (9.1.1.1) in the Stream identity
  table (9.1), for which the system is to use the same
  encapsulation (10.5.1.5) for the Sequence encode/decode
  function. */
  ACT_JSON_QT_SET(qint32, stream_list, StreamList);  // min-elements 1

  /* The port on which the system is to place an instance of the
  Sequence encode/decode function (7.6). */
  ACT_JSON_FIELD(QString, port, Port);  // interface name

  /* An object indicating whether the Sequence recovery function
  (7.4.2) or Individual recovery function (7.5) is to be placed
  on the out-facing (True) or in-facing (False) side of the port
  (Figure 6-6). */
  ACT_JSON_FIELD(bool, direction, Direction);

  /* A Boolean value specifying whether this frerSeqEncEntry is
  passive (False), and therefore is used only to decode (extract
  information from) input packets passing up the protocol stack,
  or active (True), and therefore is used both for recognizing
  input packets and for encoding output packets being passed
  down the protocol stack. */
  ACT_JSON_FIELD(bool, active, Active);

  /* An enumerated value indicating the type of encapsulation used
  for this instance of the Sequence encode/decode function (7.6).
  The type includes an OUI or CID. The values defined by this
  standard are shown in Table 10-2. */
  ACT_JSON_ENUM(ActSequenceEncodeDecodeTypes, encapsulation, Encapsulation);

  /* A 4-bit integer value to be placed in the PathId field of an
  HSR sequence tag (7.9) or the LanId field of a PRP sequence
  trailer (7.10) added to an output packet. This managed object
  is used only if:
  a)  The HSR sequence tag or the PRP sequence trailer is
      selected by the frerSeqEncEncapsType object (10.5.1.5); and
  b)  frerSeqEncActive (10.5.1.4) is False (passive) */
  ACT_JSON_FIELD(qint8, path_id_lan_id, PathIdLanId);

 public:
  ActSequenceIdentificationEntry()
      : port_(""),
        direction_(true),
        active_(false),
        encapsulation_(ActSequenceEncodeDecodeTypes::kRTag),
        path_id_lan_id_(0) {}
};

/**
 * @brief sequence generation entry
 *
 */
class ActSequenceGenerationEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* A list of stream_handle values, corresponding to the values of
  the tsnStreamIdHandle objects (9.1.1.1) in the Stream identity
  table (9.1), on which this instance of the Sequence generation
  function (7.4.1) is to operate. The single instance of the
  Sequence generation function created by this frerSeqGenEntry
  operates every packet belonging to this Stream, regardless of
  the port on which it is received. */
  ACT_JSON_QT_SET(qint32, stream_list, StreamList);  // min-elements 1;

  /* An object indicating whether the Sequence generation function
  (7.4.1) is to be placed on the out-facing (True) or
  in-facing (False) side of the port (Figure 6-6). */
  ACT_JSON_FIELD(bool, direction, Direction);

  /* A Boolean object indicating that the Sequence generation
  function (7.4.1) is to be reset by calling its corresponding
  SequenceGenerationReset function (7.4.1.3). Writing the value
  True to frerSeqGenReset triggers a reset; writing the value
  False has no effect. When read, frerSeqGenReset always
  returns the value False. */
  ACT_JSON_FIELD(bool, reset, Reset);

 public:
  ActSequenceGenerationEntry() : direction_(false), reset_(false) {}
};

/**
 * @brief latent error detection parameters
 *
 */
class ActLatentErrorDetectionParameters : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* An integer specifying the maximum difference between
  frerCpsSeqRcvyDiscardedPackets (10.8.6), and the product of
  frerCpsSeqRcvyPassedPackets (10.8.5) and
  (frerSeqRcvyLatentErrorPaths – 1) (10.4.1.12.3) that is
  allowed. Any larger difference will trigger the detection of
  a latent error by the LatentErrorTest function (7.4.4.4). */
  ACT_JSON_FIELD(qint32, difference, Difference);

  /* The integer number of milliseconds that are to elapse between
  instances of running the LatentErrorTest function (7.4.4.4).
  An implementation can have a minimum value for
  frerSeqRcvyLatentErrorPeriod, below which it cannot be set,
  but this minimum shall be no larger than 1000 ms (1 s).
  Default value 2000 (2 s). */
  ACT_JSON_FIELD(qint32, period, Period);  // default "2000"

  /* The integer number of paths over which FRER is operating for
  this instance of the Base recovery function (7.4.3) and
  Latent error detection function (7.4.4). */
  ACT_JSON_FIELD(quint16, paths, Paths);

  /* The integer number of milliseconds that are to elapse between
  instances of running the LatentErrorReset function (7.4.4.3).
  An implementation can have a minimum value for
  LatentErrorReset, below which it cannot be set, but this
  minimum shall be no larger than 1000 ms (1 s).
  Default value 30000 (30 s). */
  ACT_JSON_FIELD(quint32, reset_period, ResetPeriod);  // default "30000"

 public:
  ActLatentErrorDetectionParameters() : difference_(0), period_(2000), paths_(0), reset_period_(30000) {}
};

/**
 * @brief sequence recovery entry
 *
 */
class ActSequenceRecoveryEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* A list of the stream_handle values, corresponding to the
  values of the tsnStreamIdHandle objects (9.1.1.1) in the
  Stream identity table (9.1), to which the system is to apply
  the instance of the Sequence recovery function (7.4.2) or
  Individual recovery function (7.5). */
  ACT_JSON_QT_SET(qint32, stream_list, StreamList);  // min-elements 1

  /* The list of ports on each of which the system is to
  instantiate the Sequence recovery function (7.4.2), or from
  which received packets are to be fed to a single instance of
  the Individual recovery function (7.5). */
  ACT_JSON_QT_SET(QString, port_list, PortList);  // min-elements 1

  /* An object indicating whether the Sequence recovery function
  (7.4.2) or Individual recovery function (7.5) is to be placed
  on the out-facing (True) or in-facing (False) side of the port
  (Figure 6-6). */
  ACT_JSON_FIELD(bool, direction, Direction);

  /* A Boolean object indicating that the Sequence recovery
  function (7.4.2) or Individual recovery function (7.5) is to
  be reset by calling its corresponding SequenceGenerationReset
  function (7.4.1.3). Writing the value True to frerSeqRcvyReset
  triggers a reset; writing the value False has no effect. When
  read, frerSeqRcvyReset always returns the value False. */
  ACT_JSON_FIELD(bool, reset, Reset);

  /* This object is an enumerated value specifying which sequence
  recovery algorithm is to be used for this instance of the
  Sequence recovery function (7.4.2). The enumeration uses an
  OUI or CID as shown in Table 10-1. The default value for
  frerSeqRcvyAlgorithm is Vector_Alg (00-80-C2, 0). */
  ACT_JSON_ENUM(ActSequenceRecoveryAlgorithm, algorithm, Algorithm);  // default "vector"

  /* An integer specifying how many bits of the SequenceHistory
  variable (7.4.3.2.2) are to be used. The minimum and the
  default value is 2, maximum is the maximum allowed by the
  implementation. [Not used if
  frerSeqRcvyAlgorithm (10.4.1.5) = Match_Alg (00-80-C2, 1).] */
  ACT_JSON_FIELD(quint32, history_length, HistoryLength);  // range "2..max", default 2, when "../algorithm != 'match'"

  /* An unsigned integer specifying the timeout period in
  milliseconds for the RECOVERY_TIMEOUT event (item c in 7.4.3.1). */
  ACT_JSON_FIELD(quint32, reset_timeout, ResetTimeout);

  /* A read-only unsigned integer value that cannot be encoded in a
  packet as a value for the sequence_number subparameter
  (item b in 6.1), i.e., frerSeqRcvyInvalidSequenceValue is
  larger than or equal to RecovSeqSpace (7.4.3.2.1). */
  ACT_JSON_FIELD(quint32, invalid_sequence_value, InvalidSequenceValue);

  /* A Boolean value specifying whether packets with no
  sequence_number subparameter are to be accepted (True) or not
  (False). Default value False. See item i in 7.1.1. */
  ACT_JSON_FIELD(bool, take_no_sequence, TakeNoSequence);  // default "false"

  /* A Boolean value specifying whether this entry describes a
  Sequence recovery function (7.4.2) or Individual recovery
  function (7.5).
  a) True:  The entry describes an Individual recovery function
    (7.5). Packets discarded by the SequenceGenerationAlgorithm
    (7.4.1.4) will cause the variable RemainingTicks (7.4.3.2.4)
    to be reset. There is no Latent error detection function
    (7.4.4) associated with this entry, so
    frerSeqRcvyLatentErrorDetection (10.4.1.11) cannot also
    be True.
  b) False: The entry describes a Sequence recovery function
    (7.4.2). Packets discarded by the
    SequenceGenerationAlgorithm (7.4.1.4) will not cause the
    variable RemainingTicks (7.4.3.2.4) to be reset. */
  ACT_JSON_FIELD(bool, individual_recovery, IndividualRecovery);

  /* A Boolean value indicating whether an instance of the Latent
  error detection function (7.4.4) is to be instantiated along
  with the Base recovery function (7.4.3) in this Sequence
  recovery function (7.4.2) or Individual recovery function
  (7.5). frerSeqRcvyLatentErrorDetection cannot be set True if
  frerSeqRcvyIndividualRecovery (10.4.1.10) is also True; an
  Individual recovery function does not include a Latent error
  detection function. */
  ACT_JSON_FIELD(bool, latent_error_detection, LatentErrorDetection);  // when "../individual-recovery = 'false'"

  /* The objects in the following subclauses are present if and
  only if frerSeqRcvyIndividualRecovery
  (10.4.1.10) is False. */
  ACT_JSON_OBJECT(ActLatentErrorDetectionParameters, latent_error_detection_parameters,
                  LatentErrorDetectionParameters);  // when "../individual-recovery = 'false'"

 public:
  ActSequenceRecoveryEntry()
      : direction_(false),
        reset_(false),
        algorithm_(ActSequenceRecoveryAlgorithm::kVector),
        history_length_(2),
        reset_timeout_(0),
        invalid_sequence_value_(0),
        take_no_sequence_(false),
        individual_recovery_(false),
        latent_error_detection_(false) {}
};

/**
 * @brief sequence identification list
 *
 */
class ActSequenceIdentificationList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Each entry in the Sequence identification table specifies a port
  (10.5.1.2) and direction (10.5.1.3) on which an instance of the
  Sequence encode/decode function is to be instantiated for a
  list of Streams (10.5.1.1). */
  ACT_JSON_OBJECT(ActSequenceIdentificationEntry, sequence_identification_entry, SequenceIdentificationEntry);

 public:
  ActSequenceIdentificationList() {}
  ActSequenceIdentificationList(QString port, bool active) : ActSequenceIdentificationList() {
    sequence_identification_entry_.SetPort(port);
    sequence_identification_entry_.SetActive(active);
  }

  /**
   * @brief The "==" comparison for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSequenceIdentificationList &x, const ActSequenceIdentificationList &y) {
    return x.sequence_identification_entry_.GetPort() == y.sequence_identification_entry_.GetPort() &&
           x.sequence_identification_entry_.GetActive() == y.sequence_identification_entry_.GetActive();
  }

  void SetStreamList(qint32 stream_handle) {
    QSet<qint32> &stream_list = sequence_identification_entry_.GetStreamList();
    stream_list.insert(stream_handle);
  }
};

/**
 * @brief sequence generation list
 *
 */
class ActSequenceGenerationList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* There is one Sequence generation table in a system, and one
  entry in the Sequence generation table for each Sequence
  generation function (7.4.1). */
  ACT_JSON_FIELD(quint32, index, Index);

  /* Each frerSeqGenEntry lists the Streams (10.3.1.1) and direction
  (10.3.1.2) for which a single instance of the Sequence
  generation function (7.4.1) is to be placed. */
  ACT_JSON_OBJECT(ActSequenceGenerationEntry, sequence_generation_entry, SequenceGenerationEntry);

 public:
  ActSequenceGenerationList() : index_(0) {}
  ActSequenceGenerationList(quint32 index, bool direction, bool reset) {
    this->index_ = index;
    this->sequence_generation_entry_.SetDirection(direction);
    this->sequence_generation_entry_.SetReset(reset);
  }

  void SetStreamList(qint32 stream_handle) {
    QSet<qint32> &stream_list = this->sequence_generation_entry_.GetStreamList();
    stream_list.insert(stream_handle);
  }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActSequenceGenerationList &x) { return qHash(x.GetIndex(), 0); }

  /**
   * @brief The "==" comparison for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSequenceGenerationList &x, const ActSequenceGenerationList &y) {
    return x.index_ == y.index_;
  }

  /**
   * @brief The comparison operator for QMap
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActSequenceGenerationList &x, const ActSequenceGenerationList &y) {
    return x.index_ < y.index_;
  }
};

/**
 * @brief sequence recovery list
 *
 */
class ActSequenceRecoveryList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Clause 10 of IEEE Std 802.1CB-2017 states that the same
  stream handle can be present multiple times in the sequence
  recovery table. Therefore this index leaf is being used to
  uniquely identify an entry in the sequence recovery table. */
  ACT_JSON_FIELD(quint32, index, Index);

  /** Each frerSeqRcvyEntry lists the Streams (10.4.1.1), ports
  (10.4.1.2), and direction (10.4.1.3) for which instances of a
  Sequence recovery function (7.4.2) or Individual recovery
  function (7.5) are to be instantiated. */
  ACT_JSON_OBJECT(ActSequenceRecoveryEntry, sequence_recovery_entry, SequenceRecoveryEntry);

 public:
  ActSequenceRecoveryList() : index_(0) {
    this->sequence_recovery_entry_.SetDirection(false);
    this->sequence_recovery_entry_.SetReset(false);
    this->sequence_recovery_entry_.SetAlgorithm(ActSequenceRecoveryAlgorithm::kMatch);
    this->sequence_recovery_entry_.SetResetTimeout(RECOVERY_TIMEOUT);
    this->sequence_recovery_entry_.SetTakeNoSequence(false);
    this->sequence_recovery_entry_.SetIndividualRecovery(true);
    this->sequence_recovery_entry_.SetLatentErrorDetection(false);
  }
  ActSequenceRecoveryList(quint32 index) : ActSequenceRecoveryList() { this->index_ = index; }
  ActSequenceRecoveryList(QString port, qint32 stream_handle, ActSequenceRecoveryAlgorithm algorithm)
      : ActSequenceRecoveryList() {
    sequence_recovery_entry_.GetPortList().insert(port);
    sequence_recovery_entry_.GetStreamList().insert(stream_handle);
    sequence_recovery_entry_.SetAlgorithm(algorithm);
  }

  /**
   * @brief The "==" comparison for QList
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActSequenceRecoveryList &x, const ActSequenceRecoveryList &y) {
    return x.sequence_recovery_entry_.GetPortList().intersects(y.sequence_recovery_entry_.GetPortList()) &&
           x.sequence_recovery_entry_.GetStreamList().intersects(y.sequence_recovery_entry_.GetStreamList()) &&
           x.sequence_recovery_entry_.GetAlgorithm() == y.sequence_recovery_entry_.GetAlgorithm();
  }

  void SetStreamList(qint32 stream_handle) {
    QSet<qint32> &stream_list = sequence_recovery_entry_.GetStreamList();
    stream_list.insert(stream_handle);
  }

  void SetPortList(QString port) {
    QSet<QString> &port_list = sequence_recovery_entry_.GetPortList();
    port_list.insert(port);
  }
};

/**
 * @brief FRER params
 *
 */
class ActFREREntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* There is one Sequence identification table per system, and one
  entry in the Sequence identification table for each port and
  direction for which an instance of the Sequence encode/decode
  function (7.6) is to be created. */
  ACT_JSON_COLLECTION_OBJECTS(QList, ActSequenceIdentificationList, sequence_identification_lists,
                              SequenceIdentificationLists);

  /* There is one Sequence generation table in a system, and one
  entry in the Sequence generation table for each Sequence
  generation function (7.4.1). */
  ACT_JSON_COLLECTION_OBJECTS(QList, ActSequenceGenerationList, sequence_generation_lists, SequenceGenerationLists);

  /** There is one Sequence recovery table in a system, and one entry
  in the Sequence recovery table for each Sequence recovery
  function (7.4.2) or Individual recovery function (7.5) that can
  also be present. The entry describes a set of managed objects
  for the single instance of a Base recovery function (7.4.3)
  and Latent error detection function (7.4.4) included in the
  Sequence recovery function or Individual recovery function. */
  ACT_JSON_COLLECTION_OBJECTS(QList, ActSequenceRecoveryList, sequence_recovery_lists, SequenceRecoveryLists);

 private:
  quint32 sequence_generation_index_;
  quint32 sequence_recovery_index_;

 public:
  ActFREREntry() {
    sequence_generation_index_ = 0;
    sequence_recovery_index_ = 0;
  }

  quint32 GetSequenceGenerationIndex() { return sequence_generation_index_++; }

  quint32 SequenceRecoveryIndexIncrease() { return sequence_recovery_index_++; }

  quint32 SequenceRecoveryIndexDecrease() { return --sequence_recovery_index_; }
};