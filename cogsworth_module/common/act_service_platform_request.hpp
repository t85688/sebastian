/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once
#include "act_json.hpp"

class ActServicePlatformLoginCheck : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(bool, status, Status);
};

/**
 * @brief The service platform login request
 *
 */
class ActServicePlatformLoginRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, username, Username);
  ACT_JSON_FIELD(QString, password, Password);
};

class ActServicePlatformLoginResponseUser : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint64, id, Id);
  ACT_JSON_FIELD(QString, username, Username);
  ACT_JSON_FIELD(QString, role, Role);
  ACT_JSON_FIELD(QList<QString>, profiles, Profiles);
  ACT_JSON_FIELD(quint64, data_version, DataVersion);
};

/**
 * @brief The service platform login response
 */
class ActServicePlatformLoginResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, token, token);
  ACT_JSON_OBJECT(ActServicePlatformLoginResponseUser, user, user);
};

class ActServicePlatformRegisterRequestBaselineModel : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(quint64, qty, Qty);
  ACT_JSON_FIELD(QString, price, Price);
  ACT_JSON_FIELD(QString, total_price, TotalPrice);
};

class ActServicePlatformRegisterRequestBaseline : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, id, Id);
  ACT_JSON_FIELD(QString, name, Name);
  ACT_JSON_FIELD(quint64, created_time, CreatedTime);
  ACT_JSON_FIELD(QString, total_price, TotalPrice);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActServicePlatformRegisterRequestBaselineModel, model_list, ModelList);
};

class ActServicePlatformRegisterRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, project_name, ProjectName);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActServicePlatformRegisterRequestBaseline, baseline_list, BaselineList);
};

class ActServicePlatformRegisterResponseBaselineModel : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, model_name, ModelName);
  ACT_JSON_FIELD(quint64, qty, Qty);
  ACT_JSON_FIELD(QString, price, Price);
  ACT_JSON_FIELD(QString, total_price, TotalPrice);
};

class ActServicePlatformRegisterResponseBaseline : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, id, Id);
  ACT_JSON_FIELD(QString, name, Name);
  ACT_JSON_FIELD(quint64, created_time, CreatedTime);
  ACT_JSON_FIELD(QString, total_price, TotalPrice);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActServicePlatformRegisterResponseBaselineModel, model_list, ModelList);
};

class ActServicePlatformRegisterResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(quint64, id, Id);
  ACT_JSON_FIELD(QString, project_name, ProjectName);
  ACT_JSON_FIELD(quint64, created_time, CreatedTime);
  ACT_JSON_FIELD(quint64, last_modified_time, LastModifiedTime);
  ACT_JSON_FIELD(QString, status, Status);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActServicePlatformRegisterResponseBaseline, baseline_list, BaselineList);
};

class ActServicePlatformGetPriceRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_COLLECTION(QList, QString, model_list, ModelList);
};

class ActServicePlatformGetPriceResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_QT_DICT(QMap, QString, QString, data, data);
};

class ActServicePlatformUpdateProjectRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, project_name, ProjectName);
  ACT_JSON_FIELD(QString, status, Status);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActServicePlatformRegisterRequestBaseline, baseline_list, BaselineList);
};