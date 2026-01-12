/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "act_device_profile.hpp"
#include "act_json.hpp"
#include "act_network_baseline.hpp"
#include "act_project.hpp"

class ActImportProject : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(QString, project_name, ProjectName);  ///< The name of the project
  ACT_JSON_OBJECT(ActProject, project, Project);
  ACT_JSON_FIELD(bool, overwrite, Overwrite);
  ACT_JSON_QT_SET_OBJECTS(ActDeviceProfile, device_profiles, DeviceProfiles);
  ACT_JSON_QT_SET_OBJECTS(ActExportDeviceProfileInfo, device_profile_infos, DeviceProfileInfos);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActNetworkBaseline, design_baselines, DesignBaselines);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActNetworkBaseline, operation_baselines, OperationBaselines);
};

class ActExportProject : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  // Create data members to be serialized - you can use this members in code
  ACT_JSON_FIELD(QString, project_name, ProjectName);  ///< The name of the project
  ACT_JSON_OBJECT(ActProject, project, Project);
  ACT_JSON_QT_SET_OBJECTS(ActDeviceProfile, device_profiles, DeviceProfiles);
  ACT_JSON_QT_SET_OBJECTS(ActExportDeviceProfileInfo, device_profile_infos, DeviceProfileInfos);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActNetworkBaseline, design_baselines, DesignBaselines);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActNetworkBaseline, operation_baselines, OperationBaselines);
};