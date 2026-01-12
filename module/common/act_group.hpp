#pragma once

#include "act_json.hpp"

/**
 * @brief The ACT group class
 *
 */
class ActGroup : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id)
  ACT_JSON_FIELD(QString, name, Name)
  ACT_JSON_FIELD(qint64, parent_id, ParentId)
  ACT_JSON_FIELD(QString, background_color, BackgroundColor)
  ACT_JSON_FIELD(qint32, background_color_alpha, BackgroundColorAlpha)
  ACT_JSON_QT_SET(qint64, device_ids, DeviceIds)

 public:
  ActGroup() {
    this->id_ = -1;
    this->name_ = "";
    this->parent_id_ = -1;
    this->background_color_ = "#000000";
    this->background_color_alpha_ = 10;
  }

  ActGroup(const qint64 &id) : ActGroup() { this->id_ = id; }

  friend uint qHash(const ActGroup &obj) { return qHash(obj.name_, 0); }

  friend bool operator==(const ActGroup &x, const ActGroup &y) { return x.name_ == y.name_; }
};
