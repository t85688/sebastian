#include <QRegExp>
#include <QRegExpValidator>
#include <QSet>
#include <sstream>  // std::istringstream

#include "act_algorithm.hpp"
#include "act_core.hpp"

ACT_STATUS ParseMac(const QString &mac_str, quint64 &mac) {
  ACT_STATUS_INIT();
  std::istringstream iss(mac_str.toStdString().c_str());
  uint64_t nibble;
  iss >> std::hex;
  while (iss >> nibble) {
    mac = (mac << 8) + nibble;
    iss.get();
  }
  return act_status;
}

namespace act {
namespace core {

ACT_STATUS ActCore::CheckStream(ActProject &project, ActStream &stream, bool check_feasibility) {
  ACT_STATUS_INIT();

  if (stream.GetStreamTrafficType() != ActStreamTrafficTypeEnum::kCyclic) {
    QString error_msg = QString("Stream (%1) - Only support cyclic stream").arg(stream.GetStreamName());
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // The length should be 1 ~ 64 characters.
  if (stream.GetStreamName().length() < ACT_STRING_LENGTH_MIN ||
      stream.GetStreamName().length() > ACT_STRING_LENGTH_MAX) {
    QString error_msg = QString("Stream (%1) - The length of stream name should be %2 ~ %3 characters")
                            .arg(stream.GetStreamName())
                            .arg(ACT_STRING_LENGTH_MIN)
                            .arg(ACT_STRING_LENGTH_MAX);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Check FRER license
  if (stream.GetCB().GetActive()) {
    /* if (!this->GetLicense().GetFeature().GetTSN().GetEnabled() ||
    !this->GetLicense().GetFeature().GetTSN().GetFRER()) { QString error_msg = QString("Stream (%1) - The license of TSN
    FRER function is disabled").arg(stream.GetStreamName()); qDebug() << error_msg.toStdString().c_str();
      stream.GetCB().SetActive(false);
    } */
  }

  // if user defined VLAN, check the VLAN ID is valid in the project setting
  if (stream.GetUserDefinedVlan()) {
    // get VLAN or assign unique VLAN id
    ActIeee802VlanTag tag;
    act_status = stream.GetUserDefinedVlan(tag);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get VLAN tag failed";
      return act_status;
    }

    if (tag.GetVlanId() < project.GetVlanRange().GetMin() || tag.GetVlanId() > project.GetVlanRange().GetMax()) {
      QString error_msg = QString("Stream (%1) - The valid VLAN id should be %2 ~ %3, instead of %4")
                              .arg(stream.GetStreamName())
                              .arg(project.GetVlanRange().GetMin())
                              .arg(project.GetVlanRange().GetMax())
                              .arg(tag.GetVlanId());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // If user defined VLAN, check the pcp is valid in the pcp setting of project setting
    // Assign a pcp for this time slot if all streams in this time slot has no defined VLAN tag
    QSet<quint8> available_pcp_set;
    act_status = project.GetAvailablePriorityCodePointsForTrafficType(available_pcp_set, stream.GetStreamTrafficType());
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get Cyclic PCP failed";
      return act_status;
    }

    if (!available_pcp_set.contains(tag.GetPriorityCodePoint())) {
      QString message = QString("Stream (%1) - The priority code point %2 of the User-Defined VLAN is not supported")
                            .arg(stream.GetStreamName())
                            .arg(QString::number(tag.GetPriorityCodePoint()));
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }
  }

  QSet<ActDevice> device_set = project.GetDevices();
  // Check traffic spec in talker
  // Get talker device
  ActTalker talker = stream.GetTalker();
  ActDevice talker_dev;
  act_status = ActGetItemById<ActDevice>(device_set, talker.GetEndStationInterface().GetDeviceId(), talker_dev);
  if (!IsActStatusSuccess(act_status)) {
    QString message = QString("Stream (%1) - Talker %2 not found")
                          .arg(stream.GetStreamName())
                          .arg(talker.GetEndStationInterface().GetDeviceId());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  // Check device type
  if (talker_dev.GetDeviceType() != ActDeviceTypeEnum::kEndStation &&
      talker_dev.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
    QString message = QString("Stream (%1) - Talker %2 is not a valid device type")
                          .arg(stream.GetStreamName())
                          .arg(talker_dev.GetIpv4().GetIpAddress());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  // Check the talker device interface is not used
  QList<ActInterface> talker_intf_list = talker_dev.GetInterfaces();
  qint64 talker_intf_id = talker.GetEndStationInterface().GetInterfaceId();
  int talker_intf_idx = talker_intf_list.indexOf(ActInterface(talker_intf_id));
  if (talker_intf_idx < 0) {
    QString message = QString("Stream (%1) - Device id %2 Interface id %3 not found")
                          .arg(stream.GetStreamName())
                          .arg(QString::number(talker.GetEndStationInterface().GetDeviceId()))
                          .arg(QString::number(talker.GetEndStationInterface().GetInterfaceId()));
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  ActInterface talker_intf = talker_intf_list[talker_intf_idx];
  if (!talker_intf.GetUsed()) {
    QString message = QString("Stream (%1) - Talker interface %2 is not used")
                          .arg(stream.GetStreamName())
                          .arg(talker_intf.GetInterfaceName());
    qCritical() << message;
    return std::make_shared<ActBadRequest>(message);
  }

  ActTrafficSpecification talker_dev_prop_ts = talker_dev.GetDeviceProperty().GetTrafficSpecification();
  ActTrafficSpecification stream_ts = talker.GetTrafficSpecification();

  ActLink talker_link;
  QSet<ActLink> link_set = project.GetLinks();
  for (ActLink link : link_set) {
    // Use device id to filter the link
    if ((link.GetSourceDeviceId() == talker_dev.GetId() &&
         link.GetSourceInterfaceId() == talker_intf.GetInterfaceId()) ||
        (link.GetDestinationDeviceId() == talker_dev.GetId() &&
         link.GetDestinationInterfaceId() == talker_intf.GetInterfaceId())) {
      talker_link = link;
    }
  }

  // Check the basic transmission requirement is feasibility

  // Class based doesn't use those variable
  const quint32 &media_specific_overhead_bytes = project.GetAlgorithmConfiguration().GetMediaSpecificOverheadBytes();
  quint32 payload;
  stream.GetPayload(media_specific_overhead_bytes, payload);
  quint64 bandwidth_value;
  act_status = talker_link.GetBandwidthValue(bandwidth_value);
  const quint32 &stream_duration =
      qCeil(static_cast<qreal>(payload) / static_cast<qreal>(bandwidth_value) * qPow(10, 9));
  quint32 stream_interval;
  stream.GetStreamInterval(stream_interval);

  if (stream_interval < stream_duration) {
    QString message = QString("Stream (%1) - The interval %2 ns is smaller than duration %3 ns")
                          .arg(stream.GetStreamName())
                          .arg(QString::number(stream_interval))
                          .arg(QString::number(stream_duration));
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActStatusFeasibilityCheckFailed>(message);
  }

  if (stream_ts.GetTimeAware().GetLatestTransmitOffset() > stream_interval) {
    QString message = QString(
                          "Stream (%1) - The latest transmit offset %2 ns is too big to "
                          "transmit within interval %3 ns")
                          .arg(stream.GetStreamName())
                          .arg(QString::number(stream_ts.GetTimeAware().GetLatestTransmitOffset()))
                          .arg(QString::number(stream_interval));
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActStatusFeasibilityCheckFailed>(message);
  }

  quint32 talker_dev_prop_interval;
  stream.GetStreamInterval(talker_dev_prop_interval);

  if (stream_interval > talker_dev_prop_interval) {
    QString message = QString("Stream (%1) - The interval %2 ns exceeds talker capability %3 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_interval)
                          .arg(talker_dev_prop_interval);
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_interval < ACT_INTERVAL_MIN || stream_interval > ACT_INTERVAL_MAX) {
    QString message = QString("Stream (%1) - The interval %2 should be %3 ~ %4 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_interval)
                          .arg(ACT_INTERVAL_MIN)
                          .arg(ACT_INTERVAL_MAX);
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetMaxFrameSize() > talker_dev_prop_ts.GetMaxFrameSize()) {
    QString message = QString("Stream (%1) - The maximum frame size %2 bytes exceeds talker capability %3")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetMaxFrameSize())
                          .arg(talker_dev_prop_ts.GetMaxFrameSize());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetMaxFrameSize() < ACT_FRAME_SIZE_MIN || stream_ts.GetMaxFrameSize() > ACT_FRAME_SIZE_MAX) {
    QString message = QString("Stream (%1) - The maximum frame size %2 should be %3 ~ %4 bytes")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetMaxFrameSize())
                          .arg(ACT_FRAME_SIZE_MIN)
                          .arg(ACT_FRAME_SIZE_MAX);
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetMaxFramesPerInterval() > talker_dev_prop_ts.GetMaxFramesPerInterval()) {
    QString message = QString("Stream (%1) - The maximum frames per interval %2 exceeds talker capability %3")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetMaxFramesPerInterval())
                          .arg(talker_dev_prop_ts.GetMaxFramesPerInterval());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetMaxFramesPerInterval() < ACT_FRAME_PER_INTERVAL_MIN ||
      stream_ts.GetMaxFramesPerInterval() > ACT_FRAME_PER_INTERVAL_MAX) {
    QString message = QString("Stream (%1) - The maximum frames per interval %2 should be %3 ~ %4")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetMaxFramesPerInterval())
                          .arg(ACT_FRAME_PER_INTERVAL_MIN)
                          .arg(ACT_FRAME_PER_INTERVAL_MAX);
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetTimeAware().GetEarliestTransmitOffset() >
      talker_dev_prop_ts.GetTimeAware().GetEarliestTransmitOffset()) {
    QString message = QString("Stream (%1) - The earliest transmit offset %2 ns exceeds talker capability %3 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetTimeAware().GetEarliestTransmitOffset())
                          .arg(talker_dev_prop_ts.GetTimeAware().GetEarliestTransmitOffset());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetTimeAware().GetEarliestTransmitOffset() > ACT_PERIOD_MAX) {
    QString message = QString("Stream (%1) - The earliest transmit offset %2 should be %3 ~ %4 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetTimeAware().GetEarliestTransmitOffset())
                          .arg(ACT_TIME_AWARE_MIN)
                          .arg(ACT_TIME_AWARE_MAX);
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetTimeAware().GetLatestTransmitOffset() >
      talker_dev_prop_ts.GetTimeAware().GetLatestTransmitOffset()) {
    QString message = QString("Stream (%1) - The latest transmit offset %2 ns exceeds talker capability %3 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetTimeAware().GetLatestTransmitOffset())
                          .arg(talker_dev_prop_ts.GetTimeAware().GetLatestTransmitOffset());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetTimeAware().GetLatestTransmitOffset() > ACT_TIME_AWARE_MAX) {
    QString message = QString("Stream (%1) - The latest transmit offset %2 should be %3 ~ %4 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetTimeAware().GetLatestTransmitOffset())
                          .arg(ACT_TIME_AWARE_MIN)
                          .arg(ACT_TIME_AWARE_MAX);
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetTimeAware().GetLatestTransmitOffset() < stream_ts.GetTimeAware().GetEarliestTransmitOffset()) {
    QString message =
        QString("Stream (%1) - The latest transmit offset %2 ns is smaller than the earliest receive offset %3 ns")
            .arg(stream.GetStreamName())
            .arg(stream_ts.GetTimeAware().GetLatestTransmitOffset())
            .arg(stream_ts.GetTimeAware().GetEarliestTransmitOffset());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  if (stream_ts.GetTimeAware().GetJitter() > talker_dev_prop_ts.GetTimeAware().GetJitter()) {
    QString message = QString("Stream (%1) - The jitter %2 ns exceeds talker capability %3 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetTimeAware().GetJitter())
                          .arg(talker_dev_prop_ts.GetTimeAware().GetJitter());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  quint32 jitter_max =
      (stream_ts.GetTimeAware().GetLatestTransmitOffset() - stream_ts.GetTimeAware().GetEarliestTransmitOffset()) / 2;
  if (stream_ts.GetTimeAware().GetJitter() > jitter_max) {
    QString message = QString("Stream (%1) - The jitter %2 should be %3 ~ %4 ns")
                          .arg(stream.GetStreamName())
                          .arg(stream_ts.GetTimeAware().GetJitter())
                          .arg(ACT_JITTER_MIN)
                          .arg(jitter_max);
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  // Check user to network requirement in listener
  QList<ActListener> listeners = stream.GetListeners();
  if (listeners.size() == 0) {
    QString message = QString("Stream (%1) - There should be at least one listener").arg(stream.GetStreamName());
    qCritical() << message;
    return std::make_shared<ActBadRequest>(message);
  }

  for (ActListener listener : listeners) {
    ActUserToNetworkRequirement req = listener.GetUserToNetworkRequirement();
    if (stream.GetQosType() == ActQosTypeEnum::kDeadline &&
        (req.GetMaxLatency() < ACT_LATENCY_MIN || req.GetMaxLatency() > ACT_LATENCY_MAX)) {
      QString message = QString("Stream (%1) - The maximum latency %2 should be %3 ~ %4 ns")
                            .arg(stream.GetStreamName())
                            .arg(req.GetMaxLatency())
                            .arg(ACT_LATENCY_MIN)
                            .arg(ACT_LATENCY_MAX);
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    if (stream.GetQosType() == ActQosTypeEnum::kBoundedLatency &&
        (req.GetMinReceiveOffset() < ACT_RECEIVE_OFFSET_MIN || req.GetMinReceiveOffset() > ACT_RECEIVE_OFFSET_MAX)) {
      QString message = QString("Stream (%1) - The minimum receive offset %2 should be %3 ~ %4 ns")
                            .arg(stream.GetStreamName())
                            .arg(req.GetMinReceiveOffset())
                            .arg(ACT_RECEIVE_OFFSET_MIN)
                            .arg(ACT_RECEIVE_OFFSET_MAX);
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    if (stream.GetQosType() == ActQosTypeEnum::kBoundedLatency &&
        (req.GetMaxReceiveOffset() < ACT_RECEIVE_OFFSET_MIN || req.GetMaxReceiveOffset() > ACT_RECEIVE_OFFSET_MAX)) {
      QString message = QString("Stream (%1) - The maximum receive offset %2 should be %3 ~ %4 ns")
                            .arg(stream.GetStreamName())
                            .arg(req.GetMaxReceiveOffset())
                            .arg(ACT_RECEIVE_OFFSET_MIN)
                            .arg(ACT_RECEIVE_OFFSET_MAX);
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    if (stream.GetQosType() == ActQosTypeEnum::kBoundedLatency &&
        req.GetMaxReceiveOffset() < req.GetMinReceiveOffset()) {
      QString message = QString("Stream (%1) - The maximum receive offset %2 is smaller than minimum receive offset %3")
                            .arg(stream.GetStreamName())
                            .arg(req.GetMaxReceiveOffset())
                            .arg(req.GetMinReceiveOffset());
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    // Get listener device
    ActDevice listener_dev;
    act_status = ActGetItemById<ActDevice>(device_set, listener.GetEndStationInterface().GetDeviceId(), listener_dev);
    if (!IsActStatusSuccess(act_status)) {
      QString message = QString("Stream (%1) - Listener %2 not found")
                            .arg(stream.GetStreamName())
                            .arg(listener.GetEndStationInterface().GetDeviceId());
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    // Check device type
    if (listener_dev.GetDeviceType() != ActDeviceTypeEnum::kEndStation &&
        listener_dev.GetDeviceType() != ActDeviceTypeEnum::kBridgedEndStation) {
      QString message = QString("Stream (%1) - Listener %2 is not a valid device type")
                            .arg(stream.GetStreamName())
                            .arg(listener_dev.GetIpv4().GetIpAddress());
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    // Check the listener device interface is not used
    QList<ActInterface> listener_intf_list = listener_dev.GetInterfaces();
    qint64 listener_intf_id = listener.GetEndStationInterface().GetInterfaceId();
    int listener_intf_idx = listener_intf_list.indexOf(ActInterface(listener_intf_id));
    if (listener_intf_idx < 0) {
      QString message = QString("Stream (%1) - Device id %2 Interface id %3 not found")
                            .arg(stream.GetStreamName())
                            .arg(QString::number(listener.GetEndStationInterface().GetDeviceId()))
                            .arg(QString::number(listener.GetEndStationInterface().GetInterfaceId()));
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    ActInterface listener_intf = listener_intf_list[listener_intf_idx];
    if (!listener_intf.GetUsed()) {
      QString message = QString("Stream (%1) - Listener interface id %2 is not used")
                            .arg(stream.GetStreamName())
                            .arg(QString::number(listener.GetEndStationInterface().GetInterfaceId()));
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    if (talker_intf.GetInterfaceId() == listener_intf.GetInterfaceId() &&
        talker_intf.GetDeviceId() == listener_intf.GetDeviceId()) {
      QString message = QString("Stream (%1) - talker & listener are the same").arg(stream.GetStreamName());
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }
  }

  // Get DA from stream
  QString stream_da;
  act_status = stream.GetDAFromStream(stream_da);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get unicast/multicast destination MAC address failed";
    return act_status;
  }

  // If multicast is active, check multicast address is valid
  if (stream.GetMulticast()) {
    act_status = CheckMacAddress(stream_da);
    if (!IsActStatusSuccess(act_status)) {
      QString error_msg =
          QString("Stream (%1) - Check DA failed - %2").arg(stream.GetStreamName()).arg(act_status->GetErrorMessage());
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }

    // Transfer the MAC address to byte array
    quint64 da;
    qint8 mac_idx = 0;
    quint8 da_array[6] = {0};
    ParseMac(stream_da, da);
    for (mac_idx = 5; mac_idx >= 0; mac_idx--) {
      da_array[mac_idx] = 0xFF & da;
      da = da >> 8;
    }

    bool multicast = static_cast<quint8>(da_array[0]) % 2 == 1;
    if (!multicast) {
      QString error_msg = QString("Project (%1) - The MAC address %2 is not a valid multicast format")
                              .arg(stream.GetStreamName())
                              .arg(stream_da);
      qCritical() << error_msg.toStdString().c_str();
      return std::make_shared<ActBadRequest>(error_msg);
    }
  }

  bool check_transmission_offset_overlap = false;
  if (stream_ts.GetTimeAware().GetEarliestTransmitOffset() == stream_ts.GetTimeAware().GetLatestTransmitOffset()) {
    check_transmission_offset_overlap = true;
  }

  // Check stream conflict
  QSet<ActStream> streams = project.GetStreams();
  for (ActStream other_stream : streams) {
    // Skip itself for update method
    if (other_stream.GetId() == stream.GetId()) {
      continue;
    }

    // Check the basic transmission requirement is feasibility
    if (check_feasibility && check_transmission_offset_overlap) {
      ActTalker other_stream_talker_ = other_stream.GetTalker();
      if (other_stream_talker_ == talker) {
        ActTrafficSpecification other_stream_ts = other_stream_talker_.GetTrafficSpecification();

        // [bugfix:1659] Check this stream's duration is overlap with other streams allowed transmission time
        if (other_stream_ts.GetTimeAware().GetEarliestTransmitOffset() >=
                stream_ts.GetTimeAware().GetEarliestTransmitOffset() &&
            other_stream_ts.GetTimeAware().GetLatestTransmitOffset() <=
                (stream_ts.GetTimeAware().GetEarliestTransmitOffset() + stream_duration)) {
          QString error_msg = QString(
                                  "Stream (%1) - The transmission offset will occupy %2 ~ %3, which will be "
                                  "overlapped with Stream (%4)")
                                  .arg(stream.GetStreamName())
                                  .arg(stream_ts.GetTimeAware().GetEarliestTransmitOffset())
                                  .arg(stream_ts.GetTimeAware().GetEarliestTransmitOffset() + stream_duration)
                                  .arg(other_stream.GetStreamName());
          qCritical() << error_msg.toStdString().c_str();
          return std::make_shared<ActStatusFeasibilityCheckFailed>(error_msg);
        }
      }
    }

    // Check VID + DA duplicate
    // [bugfix:2859] Stream - VID + DA need to check system assigned
    qint64 talker_id = stream.GetTalker().GetEndStationInterface().GetDeviceId();
    qint64 talker_intf_id = stream.GetTalker().GetEndStationInterface().GetInterfaceId();
    quint16 vlan_id = (!stream.GetUserDefinedVlan() && !stream.GetTagged()) ? 0 : stream.GetVlanId();
    qint64 other_talker_id = other_stream.GetTalker().GetEndStationInterface().GetDeviceId();
    qint64 other_talker_intf_id = other_stream.GetTalker().GetEndStationInterface().GetInterfaceId();
    quint16 other_vlan_id =
        (!other_stream.GetUserDefinedVlan() && !other_stream.GetTagged()) ? 0 : other_stream.GetVlanId();
    QString other_stream_da;
    act_status = other_stream.GetDAFromStream(other_stream_da);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << QString("Get stream %1 unicast/multicast destination MAC address failed")
                         .arg(other_stream.GetStreamName());
      return act_status;
    }

    if (stream_da == ACT_FAKE_MAC_ADDRESS) {
      continue;
    } else if (stream_da != other_stream_da) {
      continue;
    } else if (vlan_id != other_vlan_id) {
      continue;
    } else if ((!stream.GetTagged() && stream.GetUntaggedMode() == ActStreamUntaggedModeEnum::kPerStreamPriority &&
                !stream.GetUserDefinedVlan()) ||
               (!other_stream.GetTagged() &&
                other_stream.GetUntaggedMode() == ActStreamUntaggedModeEnum::kPerStreamPriority &&
                !other_stream.GetUserDefinedVlan())) {
      // [bugfix:3200] Algorithm - Pre-stream Priority can support multiple system assigned VLAN on the same port
      continue;
    } else if (!stream.GetUserDefinedVlan() && !other_stream.GetUserDefinedVlan() &&
               (talker_id != other_talker_id || talker_intf_id != other_talker_intf_id)) {  // [bugfix:2856, 2859]
      continue;
    }

    QString message = QString("Stream (%1) - VID(%2) + DA(%3) is duplicated with Stream (%4)")
                          .arg(stream.GetStreamName())
                          .arg(vlan_id)
                          .arg(stream_da)
                          .arg(other_stream.GetStreamName());
    qCritical() << message.toStdString().c_str();
    return std::make_shared<ActBadRequest>(message);
  }

  return act_status;
}

ACT_STATUS ActCore::CreateStream(qint64 &project_id, ActStream &stream) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->CreateStream(project, stream);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Create stream failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::CreateStream(ActProject &project, ActStream &stream) {
  ACT_STATUS_INIT();

  QSet<ActStream> &stream_set = project.GetStreams();

  // check the stream is unique
  for (ActStream s : stream_set) {
    if (s.GetStreamName() == stream.GetStreamName()) {
      qCritical() << "The stream name" << stream.GetStreamName() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(stream.GetStreamName());
    }
  }

  if (stream.GetUserDefinedVlan() || stream.GetTagged()) {
    for (ActDataFrameSpecification dfs : stream.GetTalker().GetDataFrameSpecifications()) {
      if (dfs.GetType() == ActFieldTypeEnum::kIeee802VlanTag) {
        stream.SetVlanId(dfs.GetIeee802VlanTag().GetVlanId());
        stream.SetPriorityCodePoint(dfs.GetIeee802VlanTag().GetPriorityCodePoint());
      }
    }
  }

  // Check vlan id is not duplicated with vlan group
  if (project.GetTopologySetting().GetIntelligentVlanGroup().contains(ActIntelligentVlan(stream.GetVlanId()))) {
    return std::make_shared<ActDuplicatedError>(QString("User defined vlan id %1").arg(stream.GetVlanId()));
  }

  // Generate a new unique id
  if (stream.GetId() == -1) {
    qint64 id;
    act_status = this->GenerateUniqueId<ActStream>(stream_set, project.last_assigned_stream_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    stream.SetId(id);
  }

  // Get MAC address from talker
  QString talker_mac_address;
  act_status = stream.GetTalkerAddressFromStream(talker_mac_address);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get destination address failed";
    return act_status;
  }

  // assign stream id
  ActStreamId stream_id(talker_mac_address, stream.GetId());
  stream.SetStreamId(stream_id);

  // [feat:793] Set the stream status to planned (802.1Qdj)
  stream.SetStreamStatus(ActStreamStatusEnum::kPlanned);

  // [bugfix:2845] Unicast Stream - The destination MAC is wrong
  if (!stream.GetMulticast()) {
    ActTalker talker = stream.GetTalker();
    qint64 talker_id = talker.GetEndStationInterface().GetDeviceId();
    ActListener listener = stream.GetListeners().first();
    qint64 listener_id = listener.GetEndStationInterface().GetDeviceId();
    ActDevice listener_device, talker_device;
    project.GetDeviceById(listener_device, listener_id);
    project.GetDeviceById(talker_device, talker_id);

    ActDataFrameSpecification data_frame_specification(
        ActIeee802MacAddresses(talker_device.GetMacAddress(), listener_device.GetMacAddress()));
    stream.GetTalker().GetDataFrameSpecifications().append(data_frame_specification);
  }

  // interface capability in the listener???
  // stream rank????

  // Insert the stream to project
  stream_set.insert(stream);

  // Send update msg to temp
  InsertStreamMsgToNotificationTmp(
      ActStreamPatchUpdateMsg(ActPatchUpdateActionEnum::kCreate, project.GetId(), stream, true));

  return act_status;
}

ACT_STATUS ActCore::CreateStreams(qint64 &project_id, QList<ActStream> &stream_list, QSet<qint64> &created_stream_ids) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  // Create Streams
  act_status = this->CreateStreams(project, stream_list, created_stream_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Batch create streams failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg
  ActProjectPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, project, true);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project.GetId());

  return act_status;
}

ACT_STATUS ActCore::CreateStreams(ActProject &project, QList<ActStream> &stream_list,
                                  QSet<qint64> &created_stream_ids) {
  ACT_STATUS_INIT();

  // Check stream list is empty
  if (stream_list.isEmpty()) {
    QString error_msg = QString("Stream list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  // Take streams from stream_list to generate ActStream
  for (auto stream : stream_list) {
    act_status = this->CreateStream(project, stream);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Create stream failed";
      return act_status;
    }

    created_stream_ids.insert(stream.GetId());
  }
  return act_status;
}

ACT_STATUS ActCore::GetStream(qint64 &project_id, qint64 &stream_id, ActStream &stream) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  QSet<ActStream> stream_set = project.GetStreams();
  act_status = ActGetItemById<ActStream>(stream_set, stream_id, stream);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Stream id:" << stream_id << "not found";
    return act_status;
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateStream(qint64 &project_id, ActStream &stream) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateStream(project, stream);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update stream failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateStream(ActProject &project, ActStream &stream) {
  ACT_STATUS_INIT();

  QSet<ActStream> &stream_set = project.GetStreams();

  if (stream.GetUserDefinedVlan()) {
    // User might change the User Defined VLAN setting
    // get VLAN or assign unique VLAN id
    ActIeee802VlanTag tag;
    act_status = stream.GetUserDefinedVlan(tag);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Get VLAN tag failed";
      return act_status;
    }

    // If user defined VLAN is set, update the stream VLAN
    if (tag.GetVlanId() == 0) {
      QString message = QString("Stream (%1) - Get User-Defined VLAN tag failed").arg(stream.GetStreamName());
      qCritical() << message.toStdString().c_str();
      return std::make_shared<ActBadRequest>(message);
    }

    // Check vlan id is not duplicated with vlan group
    if (project.GetTopologySetting().GetIntelligentVlanGroup().contains(ActIntelligentVlan(tag.GetVlanId()))) {
      return std::make_shared<ActDuplicatedError>(QString("User defined vlan id %1").arg(tag.GetVlanId()));
    }

    stream.SetVlanId(tag.GetVlanId());
    stream.SetPriorityCodePoint(tag.GetPriorityCodePoint());
  }

  // Check the item does exist by id
  typename QSet<ActStream>::const_iterator iterator;
  iterator = stream_set.find(stream);
  if (iterator != stream_set.end()) {
    // If yes, delete it
    stream_set.erase(iterator);
  }

  // check the stream is unique
  for (ActStream s : stream_set) {
    if (s.GetStreamName() == stream.GetStreamName()) {
      qCritical() << "The stream name" << stream.GetStreamName() << "is duplicated";
      return std::make_shared<ActDuplicatedError>(stream.GetStreamName());
    }
  }

  // [feat:793] Update the stream status of configured stream to modified (802.1Qdj)
  if (stream.GetStreamStatus() == ActStreamStatusEnum::kScheduled) {
    stream.SetStreamStatus(ActStreamStatusEnum::kModified);
  }

  if (stream.GetId() == -1) {
    // Generate a new unique id
    qint64 id;
    act_status = this->GenerateUniqueId<ActStream>(stream_set, project.last_assigned_stream_id_, id);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Cannot get an available unique id";
      return act_status;
    }
    stream.SetId(id);
  }

  // User might change the Talker or Listener
  // Update Stream ID (SA + Unique ID)
  // Get MAC address from talker
  QString talker_mac_address;
  act_status = stream.GetTalkerAddressFromStream(talker_mac_address);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get destination address failed";
    return act_status;
  }

  // assign stream id
  ActStreamId stream_id(talker_mac_address, stream.GetId());
  stream.SetStreamId(stream_id);

  // [bugfix:2845] Unicast Stream - The destination MAC is wrong
  if (!stream.GetMulticast()) {
    ActTalker talker = stream.GetTalker();
    qint64 talker_id = talker.GetEndStationInterface().GetDeviceId();
    ActListener listener = stream.GetListeners().first();
    qint64 listener_id = listener.GetEndStationInterface().GetDeviceId();
    ActDevice listener_device, talker_device;
    project.GetDeviceById(listener_device, listener_id);
    project.GetDeviceById(talker_device, talker_id);

    for (auto &data_frame_specification : stream.GetTalker().GetDataFrameSpecifications()) {
      if (data_frame_specification.GetType() == ActFieldTypeEnum::kIeee802MacAddresses) {
        data_frame_specification.GetIeee802MacAddresses().SetSourceMacAddress(talker_device.GetMacAddress());
        data_frame_specification.GetIeee802MacAddresses().SetDestinationMacAddress(listener_device.GetMacAddress());
      }
    }
  }

  // Insert the stream to project
  stream_set.insert(stream);

  // Send update msg to temp
  InsertStreamMsgToNotificationTmp(
      ActStreamPatchUpdateMsg(ActPatchUpdateActionEnum::kUpdate, project.GetId(), stream, true));

  return act_status;
}

ACT_STATUS ActCore::UpdateStreams(qint64 &project_id, QList<ActStream> &stream_list) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->UpdateStreams(project, stream_list);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Update streams failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::UpdateStreams(ActProject &project, QList<ActStream> &stream_list) {
  ACT_STATUS_INIT();

  // Check stream list is empty
  if (stream_list.isEmpty()) {
    QString error_msg = QString("Stream list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (auto stream : stream_list) {
    act_status = this->UpdateStream(project, stream);
    if (!IsActStatusSuccess(act_status)) {
      qCritical() << "Update stream failed with stream id:" << stream.GetId();
      return act_status;
    }
  }
  return act_status;
}

ACT_STATUS ActCore::DeleteStream(qint64 &project_id, qint64 &stream_id) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteStream(project, stream_id);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete stream failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteStream(ActProject &project, qint64 &stream_id) {
  ACT_STATUS_INIT();

  QSet<ActStream> &stream_set = project.GetStreams();

  ActStream stream;
  // Check the item does exist by id
  typename QSet<ActStream>::const_iterator iterator;
  iterator = stream_set.find(ActStream(stream_id));
  if (iterator == stream_set.end()) {
    QString error_msg = QString("Delete stream failed, cannot found stream id %1").arg(stream_id);
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  stream = *iterator;
  // If yes, delete it
  // qDebug() << "Delete item id:" << QString::number(stream_id);
  stream_set.erase(iterator);

  // Send update msg to temp
  InsertStreamMsgToNotificationTmp(
      ActStreamPatchUpdateMsg(ActPatchUpdateActionEnum::kDelete, project.GetId(), stream, true));

  return act_status;
}

ACT_STATUS ActCore::DeleteStreams(qint64 &project_id, QList<qint64> &stream_ids) {
  ACT_STATUS_INIT();

  this->InitNotificationTmp();

  // Get project by id
  ActProject project;
  act_status = this->GetProject(project_id, project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Get project failed with project id:" << project_id;
    return act_status;
  }

  act_status = this->DeleteStreams(project, stream_ids);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Delete streams failed";
    return act_status;
  }

  // compute topology setting
  act_status = this->ComputeTopologySetting(project);
  if (!IsActStatusSuccess(act_status)) {
    qCritical() << "Compute topology setting failed";
    return act_status;
  }

  // Update the project in core memory
  act_status = this->UpdateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    qDebug() << "Cannot update the project id:" << project.GetId();
    return act_status;
  }

  // Send update msg from NotificationTmp
  this->SendMessageToListener(ActWSTypeEnum::kProject, true, ActBasePatchUpdateMsg(), project_id);

  return act_status;
}

ACT_STATUS ActCore::DeleteStreams(ActProject &project, QList<qint64> &stream_ids) {
  ACT_STATUS_INIT();

  // Check stream list is empty
  if (stream_ids.isEmpty()) {
    QString error_msg = QString("Stream id list is empty");
    qCritical() << error_msg.toStdString().c_str();
    return std::make_shared<ActBadRequest>(error_msg);
  }

  for (auto stream_id : stream_ids) {
    act_status = this->DeleteStream(project, stream_id);
    if (!IsActStatusSuccess(act_status)) {  // fail to delete stream
      qCritical() << "Delete stream failed with stream id:" << stream_id;
      return act_status;
    }
  }

  return act_status;
}

ACT_STATUS ActCore::UpdateAllStreamStatus(ActProject &project, ActStreamStatusEnum status) {
  ACT_STATUS_INIT();

  QSet<ActStream> stream_set = project.GetStreams();
  QSet<ActStream> new_stream_set;

  for (auto stream : stream_set) {
    stream.SetStreamStatus(status);

    // Insert the stream to project
    new_stream_set.insert(stream);
  }

  qint64 project_id = project.GetId();

  ActStreamStatusPatchUpdateMsg msg(ActPatchUpdateActionEnum::kUpdate, status, false);
  this->SendMessageToListener(ActWSTypeEnum::kProject, false, msg, project_id);

  project.SetStreams(new_stream_set);

  return act_status;
}

}  // namespace core
}  // namespace act
