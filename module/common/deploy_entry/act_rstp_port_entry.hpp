/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_json.hpp"

enum class ActRstpEdgeEnum { kAuto = 1, kYes = 2, kNo = 3 };

static const QMap<QString, ActRstpEdgeEnum> kActRstpEdgeEnumMap = {
    {"Auto", ActRstpEdgeEnum::kAuto}, {"Yes", ActRstpEdgeEnum::kYes}, {"No", ActRstpEdgeEnum::kNo}};

enum class ActRstpLinkTypeEnum { kPointToPoint = 1, kShared = 2, kAuto = 3 };

static const QMap<QString, ActRstpLinkTypeEnum> kActRstpLinkTypeEnumMap = {
    {"Point-to-point", ActRstpLinkTypeEnum::kPointToPoint},
    {"Shared", ActRstpLinkTypeEnum::kShared},
    {"Auto", ActRstpLinkTypeEnum::kAuto}};

class ActRstpPortEntry : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, port_id, PortId);
  // std1w1ap
  ACT_JSON_FIELD(bool, rstp_enable,
                 RstpEnable);  // for Enable
  // Edge:
  //  - Auto: AutoEdge: true, ForceEdge: false.
  //  - Yes:  AutoEdge: false, ForceEdge: true.
  //  - No :  AutoEdge: false, ForceEdge: false.
  ACT_JSON_ENUM(ActRstpEdgeEnum, edge, Edge);
  // ACT_JSON_FIELD(bool, auto_edge, AutoEdge);    // for Edge
  // ACT_JSON_FIELD(bool, force_edge, ForceEdge);  // for Edge

  ACT_JSON_FIELD(qint64, port_priority, PortPriority);  // for Priority
  ACT_JSON_FIELD(qint64, path_cost, PathCost);

  // std1d1ap
  ACT_JSON_ENUM(ActRstpLinkTypeEnum, link_type, LinkType);

  // mxrstp
  ACT_JSON_FIELD(bool, bpdu_guard, BpduGuard);
  ACT_JSON_FIELD(bool, root_guard, RootGuard);
  ACT_JSON_FIELD(bool, loop_guard, LoopGuard);
  ACT_JSON_FIELD(bool, bpdu_filter, BpduFilter);

 public:
  /**
   * @brief Construct a new RSTP Port Entry object
   *
   */
  ActRstpPortEntry() {
    this->port_id_ = -1;

    // std1w1ap
    this->port_priority_ = 128;
    this->rstp_enable_ = true;
    this->path_cost_ = 0;
    // this->force_edge_ = false;

    this->edge_ = ActRstpEdgeEnum::kAuto;

    // std1d1ap
    this->link_type_ = ActRstpLinkTypeEnum::kAuto;

    // mxrstp
    // this->auto_edge_ = true;
    this->bpdu_guard_ = false;
    this->root_guard_ = false;
    this->loop_guard_ = false;
    this->bpdu_filter_ = false;
  }

  /**
   * @brief Construct a new RSTP Port Entry object
   *
   * @param port_id
   */
  ActRstpPortEntry(const qint32 &port_id) : ActRstpPortEntry() { this->port_id_ = port_id; }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActRstpPortEntry &x) {
    return qHash(x.port_id_, 0);  // arbitrary value
  }

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActRstpPortEntry &x, const ActRstpPortEntry &y) { return x.port_id_ == y.port_id_; }
};
