/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_json.hpp"

class ActMafExportConfigFileContentProperties : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, device_id, deviceID);
  ACT_JSON_FIELD(QString, origin, origin);
};

class ActMafExportConfigFileContent : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, id, id);
  ACT_JSON_FIELD(QString, name, name);
  ACT_JSON_FIELD(quint32, size, size);
  ACT_JSON_FIELD(QString, path, path);
  ACT_JSON_OBJECT(ActMafExportConfigFileContentProperties, properties, properties);
  ACT_JSON_FIELD(QString, created_at, created_at);
  ACT_JSON_FIELD(QString, updated_at, updated_at);
};
