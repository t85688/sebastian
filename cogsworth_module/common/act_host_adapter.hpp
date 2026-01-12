/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

class ActHostAdapter : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, adapter_name, AdapterName);
  ACT_JSON_FIELD(QString, ip_address, IpAddress);
  ACT_JSON_FIELD(QString, net_mask, NetMask);

 public:
  QList<QString> key_order_;
  /**
   * @brief Construct a new Act Host Adapter object
   *
   */
  ActHostAdapter() {
    this->adapter_name_ = "";
    this->ip_address_ = "";
    this->net_mask_ = "";

    this->key_order_.append(QList<QString>({QString("AdapterName"), QString("IpAddress"), QString("NetMask")}));
  }
};

class ActHostAdapterList : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION_OBJECTS(QList, ActHostAdapter, adapters, Adapters);
};
