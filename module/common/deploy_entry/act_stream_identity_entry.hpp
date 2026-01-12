/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
// #include <QtGlobal>

#include "act_json.hpp"

/**
 * @brief Root identity for all stream identification types
 *
 */
enum ActIdentificationType {
  kNull_stream_identification = 1,       // Null Stream Identification
  kSmac_vlan_stream_identification = 2,  // Source MAC and VLAN Stream Identification
  kDmac_vlan_stream_identification = 3,  // Active Destination MAC and VLAN Stream Identification
  kIp_stream_identification = 4          // IP Stream Identification
};

/**
 * @brief The QMap for Frame type class enum mapping
 *
 */
static const QMap<QString, ActIdentificationType> kActIdentificationTypeMap = {
    {"dot1cb-stream-identification-types:null-stream-identification",
     ActIdentificationType::kNull_stream_identification},
    {"dot1cb-stream-identification-types:smac-vlan-stream-identification",
     ActIdentificationType::kSmac_vlan_stream_identification},
    {"dot1cb-stream-identification-types:dmac-vlan-stream-identification",
     ActIdentificationType::kDmac_vlan_stream_identification},
    {"dot1cb-stream-identification-types:ip-stream-identification", ActIdentificationType::kIp_stream_identification}};

/**
 * @brief next protocol
 *
 */
enum ActNextProtocol {
  kNone = 0,  // No protocol is specified
  kUdp = 1,   // UDP is specified as the next protocol.
  kTcp = 2,   // TCP is specified as the next protocol.
  kSctp = 3   // SCTP is specified as the next protocol.
};

/**
 * @brief The QMap for Frame type class enum mapping
 *
 */
static const QMap<QString, ActNextProtocol> kActNextProtocolMap = {{"none", ActNextProtocol::kNone},
                                                                   {"udp", ActNextProtocol::kUdp},
                                                                   {"tcp", ActNextProtocol::kTcp},
                                                                   {"sctp", ActNextProtocol::kSctp}};

/**
 * @brief Enumeration describing how a Stream can be identified using the VLAN tag.
 *
 */
enum ActVlanTagIdentificationType {
  kTagged = 1,    // A frame must have a VLAN tag to be recognized as belonging to the Stream.
  kPriority = 2,  // A frame must be untagged, or have a VLAN tag with a VLAN ID = 0 to be recognized as belonging to
                  // the Stream.
  kAll = 3        // A frame is recognized as belonging to the Stream whether tagged or not.
};

/**
 * @brief The QMap for Frame type class enum mapping
 *
 */
static const QMap<QString, ActVlanTagIdentificationType> kActVlanTagIdentificationTypeMap = {
    {"tagged", ActVlanTagIdentificationType::kTagged},
    {"priority", ActVlanTagIdentificationType::kPriority},
    {"all", ActVlanTagIdentificationType::kAll}};

/**
 * @brief in facing
 *
 */
class ActInFacing : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* The list of ports on which an in-facing Stream
  identification function (6.2) using this identification
  method (9.1.1.6, 9.1.1.7) is to be placed for this Stream
  (9.1.1.1) in the input (coming from the system forwarding
  function) direction. Any number of tsnStreamIdEntry objects
  can list the same port for the same tsnStreamIdHandle in
  its tsnStreamIdInFacInputPortList. */
  ACT_JSON_QT_SET(QString, input_port_list, InputPortList);  // list of interface name

  /* The list of ports on which an in-facing Stream
  identification function (6.2) using this identification
  method (9.1.1.6, 9.1.1.7) is to be placed for this Stream
  (9.1.1.1) in the output (towards the system forwarding
  function) direction. At most one tsnStreamIdEntry can list
  a given port for a given tsnStreamIdHandle in its
  tsnStreamIdInFacOutputPortList. */
  ACT_JSON_QT_SET(QString, output_port_list, OutputPortList);  // list of interface name

 public:
  ActInFacing() {}
};

/**
 * @brief out facing
 *
 */
class ActOutFacing : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* The list of ports on which an out-facing Stream
  identification function (6.2) using this identification
  method (9.1.1.6, 9.1.1.7) is to be placed for this Stream
  (9.1.1.1) in the input (coming from the physical interface)
  direction. Any number of tsnStreamIdEntry objects can list
  the same port for the same tsnStreamIdHandle in its
  tsnStreamIdOutFacInputPortList. */
  ACT_JSON_QT_SET(QString, input_port_list, InputPortList);  // list of interface name

  /* The list of ports on which an out-facing Stream
  identification function (6.2) using this identification
  method (9.1.1.6, 9.1.1.7) is to be placed for this Stream
  (9.1.1.1) in the output (towards the physical interface)
  direction. At most one tsnStreamIdEntry can list a given
  port for a given tsnStreamIdHandle in its
  tsnStreamIdOutFacOutputPortList. */
  ACT_JSON_QT_SET(QString, output_port_list, OutputPortList);  // list of interface name

 public:
  ActOutFacing() {}
};

/**
 * @brief null stream identification group
 *
 */
class ActNullStreamIdentificationGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Specifies the destination_address that identifies a packet in
  an EISS indication primitive, to the Null Stream
  identification function */
  ACT_JSON_FIELD(QString, destination_mac, DestinationMac);  // pattern "[0-9a-fA-F]{2}(-[0-9a-fA-F]{2}){5}"

  /* An enumerated value indicating whether a packet in an EISS
  indication primitive to the Null Stream identification
  function is permitted to have a VLAN tag. */
  ACT_JSON_ENUM(ActVlanTagIdentificationType, tagged, Tagged);

  /* Specifies the vlan_identifier parameter that identifies a
  packet in an EISS indication primitive to the Null Stream
  identification function. A value of 0 indicates that the
  vlan_identifier parameter is ignored on EISS indication
  primitives. */
  ACT_JSON_FIELD(quint16, vlan, Vlan);  // range "0 .. 4095"

 public:
  ActNullStreamIdentificationGroup() : destination_mac_(""), vlan_(0), tagged_(ActVlanTagIdentificationType::kTagged) {}
};

/**
 * @brief smac vlan stream identification group
 *
 */
class ActSmacVlanStreamIdentificationGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Specifies the source_address that identifies a packet in an
  EISS indication primitive, to the Source MAC and VLAN Stream
  identification function. */
  ACT_JSON_FIELD(QString, source_mac, SourceMac);  // pattern "[0-9a-fA-F]{2}(-[0-9a-fA-F]{2}){5}"

  /* An enumerated value indicating whether a packet in an EISS
  indication primitive to the Source MAC and VLAN Stream
  identification function is permitted to have a VLAN tag. */
  ACT_JSON_ENUM(ActVlanTagIdentificationType, tagged, Tagged);

  /* Specifies the vlan_identifier parameter that identifies a
  packet in an EISS indication primitive to the Null Stream
  identification function. A value of 0 indicates that the
  vlan_identifier parameter is ignored on EISS indication
  primitives. */
  ACT_JSON_FIELD(quint16, vlan, Vlan);  // range "0 .. 4095"

 public:
  ActSmacVlanStreamIdentificationGroup() : source_mac_(""), tagged_(ActVlanTagIdentificationType::kTagged), vlan_(0) {}
};

/**
 * @brief down
 *
 */
class ActDown : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Specifies the destination_address parameter to use in the
  EISS request primitive for output packets sent to lower
  layers by the Active Destination MAC and VLAN Stream
  identification function, and the destination_address that
  identifies an input packet in an EISS indication primitive
  to the Active Destination MAC and VLAN Stream
  identification function. */
  ACT_JSON_FIELD(QString, destination_mac, DestinationMac);  // pattern "[0-9a-fA-F]{2}(-[0-9a-fA-F]{2}){5}"

  /* An enumerated value indicating whether a packet in an EISS
  indication or request primitive between the Active
  Destination MAC and VLAN Stream identification function and
  the lower layers is to have a VLAN tag. This variable is
  not used in an FRER C-component. See 8.4. */
  ACT_JSON_ENUM(ActVlanTagIdentificationType, tagged, Tagged);

  /* Specifies the vlan_identifier parameter to use in the EISS
  request primitive for output packets sent to lower layers
  by the Active Destination MAC and VLAN Stream
  identification function, and the vlan_identifier that
  identifies an input packet in an EISS indication primitive
  to the Active Destination MAC and VLAN Stream
  identification function. A value of 0 indicates that the
  vlan_identifier parameter is ignored on EISS indication
  primitives. */
  ACT_JSON_FIELD(quint16, vlan, Vlan);  // range "0 .. 4095"

  /* Specifies the priority parameter to use in the EISS request
  primitive for output packets sent to lower layers by the
  Active Destination MAC and VLAN Stream identification
  function for all packets in a particular Stream. */
  ACT_JSON_FIELD(quint8, priority, Priority);  // range "0..7"

 public:
  ActDown() : destination_mac_(""), tagged_(ActVlanTagIdentificationType::kTagged), vlan_(0), priority_(0) {}
};

/**
 * @brief up
 *
 */
class ActUp : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Specifies the destination_address parameter to use in the
  EISS indication primitive for input packets offered to upper
  layers by the Active Destination MAC and VLAN Stream
  identification layer. This address replaces the address that
  was used to identify the packet (tsnCpeDmacVlanDownDestMac,
  9.1.4.1). */
  ACT_JSON_FIELD(QString, destination_mac, DestinationMac);  // pattern "[0-9a-fA-F]{2}(-[0-9a-fA-F]{2}){5}"

  /* An enumerated value indicating whether a packet in an EISS
  indication or request primitive between the Active
  Destination MAC and VLAN Stream identification function and
  the upper layers is to have a VLAN
  tag. This variable is used only by an end system and not by
  a relay system. See 8.4. */
  ACT_JSON_ENUM(ActVlanTagIdentificationType, tagged, Tagged);

  /* Specifies the vlan_identifier parameter to use in the EISS
  indication primitive for packets offered to upper layers, or
  the VLAN ID field for an IEEE 802.1Q tag in an
  ISS mac_service_data_unit. This address replaces the VLAN ID
  that was used to identify the packet (tsnCpeDmacVlanDownVlan,
  9.1.4.3). */
  ACT_JSON_FIELD(quint16, vlan, Vlan);  // range "0 .. 4095"

  /* Specifies the priority parameter to use in the EISS
  indication primitive for packets offered to upper layers. */
  ACT_JSON_FIELD(quint8, priority, Priority);  // range "0..7"

 public:
  ActUp() : destination_mac_(""), tagged_(ActVlanTagIdentificationType::kTagged), vlan_(0), priority_(0) {}
};

/**
 * @brief dmac vlan stream identification group
 *
 */
class ActDmacVlanStreamIdentificationGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Container for all parameters which are sent to lower layers. */
  ACT_JSON_OBJECT(ActDown, down, Down);

  /* Container for all parameters which are offered to higher layers. */
  ACT_JSON_OBJECT(ActUp, up, Up);

 public:
  ActDmacVlanStreamIdentificationGroup() {}
};

/**
 * @brief ip stream identification group
 *
 */
class ActIpStreamIdentificationGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* Specifies the destination_address parameter that identifies a
  packet in an EISS indication primitive. */
  ACT_JSON_FIELD(QString, destination_mac, DestinationMac);

  /* An enumerated value indicating whether a packet in an EISS
  indication or request primitive to the IP Stream identification
  function is to have a VLAN tag. */
  ACT_JSON_ENUM(ActVlanTagIdentificationType, tagged, Tagged);

  /* Specifies the vlan_identifier parameter that identifies a
  packet in an EISS indication primitive. A value of 0 indicates
  that the frame is not to have a VLAN tag. */
  ACT_JSON_FIELD(quint16, vlan, Vlan);  // range "0 .. 4095"

  /* Specifies the IPv4 (RFC 791) or IPv6 (RFC 2460) source address
  parameter that must be matched to identify packets coming up
  from lower layers. An address of all 0 indicates that the
  IP  source address is to be ignored on packets received from
  lower layers. */
  ACT_JSON_FIELD(QString, ip_source, IpSource);  // pattern
                                                 //   '(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.){3}'
                                                 // +  '([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])'
                                                 // + '(%[\p{N}\p{L}]+)?'

  /* Specifies the IPv4 (RFC 791) or IPv6 (RFC 2460) destination
  address parameter that must be matched to identify packets
  coming up from lower layers. */
  ACT_JSON_FIELD(QString, ip_destination,
                 IpDestination);  // pattern
                                  //   '(([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])\.){3}'
                                  // +  '([0-9]|[1-9][0-9]|1[0-9][0-9]|2[0-4][0-9]|25[0-5])'
                                  // + '(%[\p{N}\p{L}]+)?'

  /* Specifies the IPv4 (RFC 791) or IPv6 (RFC 2460) differentiated
  services codepoint (DSCP, RFC 2474) that must be matched to
  identify packets coming up from the lower layers. A value of
  64 decimal indicates that the DSCP is to be ignored on packets
  received from lower layers. */
  ACT_JSON_FIELD(quint8, dscp, Dscp);  // range "0..63"

  /* Specifies the IP next protocol parameter that must be matched
  to identify packets coming up from lower layers. The value of
  this parameter must specify either none, UDP (RFC 768),
  TCP (RFC 793), or SCTP (RFC 4960). If “none,” then the
  tsnCpeIpIdSourcePort (9.1.5.8) and tsnCpeIpIdDestinationPort
  (9.1.5.9) managed objects are not used. */
  ACT_JSON_ENUM(ActNextProtocol, next_protocol, NextProtocol);

  /* Specifies the TCP or UDP Source Port parameter that must be
  matched to identify packets coming up from lower layers. A
  value of 0 indicates that the Source Port number of the packet
  is to be ignored on packets received from lower layers. */
  ACT_JSON_FIELD(quint16, source_port, SourcePort);  // range "0..65535"

  /* Specifies the TCP or UDP Destination Port parameter that must
  be matched to identify packets coming up from lower layers.
  A value of 0 indicates that the Destination Port number of the
  packet is to be ignored on packets received from
  lower layers. */
  ACT_JSON_FIELD(quint16, destination_port, DestinationPort);  // range "0..65535"

 public:
  ActIpStreamIdentificationGroup()
      : destination_mac_(""),
        tagged_(ActVlanTagIdentificationType::kTagged),
        vlan_(0),
        ip_source_(""),
        ip_destination_(""),
        dscp_(0),
        next_protocol_(ActNextProtocol::kNone),
        source_port_(0),
        destination_port_(0) {}
};

/**
 * @brief stream identity entry
 *
 */
class ActTsnStreamIdEntryGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* The objects in a given entry of the Stream identity table are
  used to control packets whose stream_handle subparameter is
  equal to the entry’s tsnStreamIdHandle object. The specific
  values used in the tsnStreamIdHandle object are not
  necessarily used in the system; they are used only to relate
  the various management objects in Clause 9 and Clause 10. */
  ACT_JSON_FIELD(qint32, handle, Handle);

  /* Container for in-facing Stream identification functions. */
  ACT_JSON_OBJECT(ActInFacing, in_facing, InFacing);

  /* Container for out-facing Stream identification functions. */
  ACT_JSON_OBJECT(ActOutFacing, out_facing, OutFacing);

  /* An enumerated value indicating the method used to identify
  packets belonging to the Stream. The enumeration includes an
  Organizationally Unique Identifier (OUI) or Company ID (CID)
  to identify the organization defining the enumerated type. */
  ACT_JSON_ENUM(ActIdentificationType, identification_type, IdentificationType);

  /* When instantiating an instance of the Null Stream
  identification function (6.4) for a particular input
  Stream, the managed objects in the following subclauses
  serve as the tsnStreamIdParameters managed object
  (9.1.1.7). */
  ACT_JSON_OBJECT(ActNullStreamIdentificationGroup, null_stream_identification, NullStreamIdentification);

  /* When instantiating an instance of the Source MAC and VLAN
  Stream identification function (6.5) for a particular input
  Stream, the managed objects in the following subclauses
  serve as the tsnStreamIdParameters managed object
  (9.1.1.7). */
  ACT_JSON_OBJECT(ActSmacVlanStreamIdentificationGroup, smac_vlan_stream_identification, SmacVlanStreamIdentification);

  /* When instantiating an instance of the Active Destination
  MAC and VLAN Stream identification function (6.6) for a
  particular output Stream, the managed objects in the
  following subclauses, along with those listed in 9.1.2,
  serve as the tsnStreamIdParameters managed object
  (9.1.1.7). */
  ACT_JSON_OBJECT(ActDmacVlanStreamIdentificationGroup, dmac_vlan_stream_identification, DmacVlanStreamIdentification);

  /* When instantiating an instance of the IP Stream
  identification function (6.7), the parameters in the
  following subclauses replace the tsnStreamIdParameters
  managed object (9.1.1.7). */
  ACT_JSON_OBJECT(ActIpStreamIdentificationGroup, ip_stream_identification, IpStreamIdentification);

 public:
  ActTsnStreamIdEntryGroup() : handle_(0), identification_type_(ActIdentificationType::kNull_stream_identification) {}
};

/**
 * @brief stream identity list
 *
 */
class ActStreamIdentityEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  /* If a single Stream has multiple identification methods,
  perhaps (but not necessarily) on different ports, then there
  can be multiple tsnStreamIdEntry objects with the same value
  for the tsnStreamIdHandle */
  ACT_JSON_FIELD(quint32, index, Index);

  /* A set of managed objects, all applying to the Stream specified
  by tsnStreamIdHandle (9.1.1.1), and all using the same Stream
  identification types and parameters (9.1.1.6, 9.1.1.7).
  See 10.2 for additional managed objects that are present in
  the tsnStreamIdEntry only if Autoconfiguration (7.11) is used. */
  ACT_JSON_OBJECT(ActTsnStreamIdEntryGroup, tsn_stream_id_entry_group, TsnStreamIdEntryGroup);

 public:
  ActStreamIdentityEntry() : index_(0) {}

  ActStreamIdentityEntry(qint32 handle, ActIdentificationType identification_type) : ActStreamIdentityEntry() {
    this->tsn_stream_id_entry_group_.SetHandle(handle);
    this->tsn_stream_id_entry_group_.SetIdentificationType(identification_type);
  }

  /**
   * @brief The "==" comparison for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActStreamIdentityEntry &x, const ActStreamIdentityEntry &y) {
    return x.tsn_stream_id_entry_group_.GetHandle() == y.tsn_stream_id_entry_group_.GetHandle() &&
           x.tsn_stream_id_entry_group_.GetIdentificationType() == y.tsn_stream_id_entry_group_.GetIdentificationType();
  }

  /**
   * @brief The comparison operator for QMap
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator<(const ActStreamIdentityEntry &x, const ActStreamIdentityEntry &y) {
    return x.index_ < y.index_;
  }
};