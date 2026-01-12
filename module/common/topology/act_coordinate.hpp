/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include <QMetaType>

#include "act_json.hpp"

/**
 * @brief The Act Topology's ActCoordinate class
 *
 */
class ActCoordinate : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, x, X);  ///< x-axis item
  ACT_JSON_FIELD(qint64, y, Y);  ///< y-axis item

 public:
  /**
   * @brief Construct a new ActCoordinate object
   *
   */
  ActCoordinate() {
    this->x_ = 404;
    this->y_ = 404;
  }

  /**
   * @brief Construct a new ActCoordinate object
   *
   * @param x
   * @param y
   */
  ActCoordinate(qint64 x, qint64 y) : x_(x), y_(y) {}

  /**
   * @brief The comparison operator for QSet
   *
   * @param x
   * @param y
   * @return true
   * @return false
   */
  friend bool operator==(const ActCoordinate &x, const ActCoordinate &y) { return (x.x_ == y.x_) && (x.y_ == y.y_); }

  /**
   * @brief The hash function for QSet
   *
   * @param x
   * @return uint
   */
  friend uint qHash(const ActCoordinate &x_y) { return qHash(x_y.GetX(), 0) ^ qHash(x_y.GetY(), 0 << 1); }
};
