#include "classbased_project.h"

#include <QThread>

#include "classbased_device.h"
#include "classbased_link.h"
#include "classbased_projectsetting.h"
#include "classbased_trafficdesign.h"

namespace ClassBased {

UaStatus updateProjectNode(const ActProject &project, OpcUa_UInt32 &errorCode, UaString &errorMessage) {
  UaStatus ret;

  if (project.GetProjectMode() != ActProjectModeEnum::kDesign) {
    return ret;
  }

  UaString projectName(project.GetProjectName().toStdString().c_str());

  // NodeId for the node to create
  UaNodeId projectNodeId(pMoxaNodeManager->getProjectNodeId(project.GetId()));
  MoxaClassBased::ProjectType *pProjectType = (MoxaClassBased::ProjectType *)pMoxaNodeManager->getNode(projectNodeId);

  if (pProjectType == NULL) {
    // create the new node to the OPC UA nodemanager
    pProjectType = new MoxaClassBased::ProjectType(projectNodeId, projectName, pMoxaNodeManager->getNameSpaceIndex(),
                                                   pMoxaNodeManager->getNodeManagerConfig());
    if (!pProjectType) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Allocate memory failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }

    // Add node to the specified folder
    UaNodeId projectFolderNodeId(MoxaClassBasedId_Server_Resources_Communication_Projects,
                                 pClassBased->getNameSpaceIndex());
    ret = pMoxaNodeManager->addNodeAndReference(projectFolderNodeId, pProjectType->getUaReferenceLists(),
                                                OpcUaId_HasComponent);
    if (ret.isNotGood()) {
      errorCode = M_UA_INTERNAL_ERROR;
      errorMessage = UaString("Invalid: Add node reference failed");
      qDebug() << errorMessage.toUtf8();
      return ret;
    }
  } else {
    pProjectType->setBrowseName(UaQualifiedName(projectName, pMoxaNodeManager->getNameSpaceIndex()));
    pProjectType->setDisplayName(UaLocalizedText(UaString(), projectName));
  }

  ret = updateProjectSettingNode(project, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateDeviceNodes(project, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateLinkNodes(project, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  ret = updateTrafficDesignNodes(project, errorCode, errorMessage);
  if (ret.isNotGood() || errorCode != M_UA_NO_ERROR) {
    return ret;
  }

  return ret;
}

UaStatus removeProjectNode(const ActProject& project, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;

  UaNodeId projectNodeId(pMoxaNodeManager->getProjectNodeId(project.GetId()));

  UaNode* pNode = pMoxaNodeManager->getNode(projectNodeId);
  if (!pNode) {
    // Node already deleted, treat as success
    return ret;
  }

  if (pNode->typeDefinitionId().identifierNumeric() != MoxaClassBasedId_ProjectType) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Project %1 is not found").arg(projectNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  ret = pMoxaNodeManager->deleteUaNode(pNode, OpcUa_True, OpcUa_True, OpcUa_True);
  if (ret.isNotGood()) {
    errorCode = M_UA_INTERNAL_ERROR;
    errorMessage = UaString("Invalid: Remove node %1 failed").arg(projectNodeId.toFullString());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  return ret;
}

UaStatus addProjectMethod(const UaString &projectName, UaNodeId &projectNodeId, OpcUa_UInt32 &errorCode,
                          UaString &errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  // Create ACT Project
  ActProject project;
  project.SetProjectName(QString(projectName.toUtf8()));

  QMutexLocker lock(&act::core::g_core.mutex_);

  act_status = act::core::g_core.CreateProject(project);
  if (!IsActStatusSuccess(act_status)) {
    errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
    errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
    qDebug() << errorMessage.toUtf8();
    return ret;
  }

  projectNodeId = pMoxaNodeManager->getProjectNodeId(project.GetId());

  return ret;
}

UaStatus removeProjectMethod(const UaString& projectName, OpcUa_UInt32& errorCode, UaString& errorMessage) {
  UaStatus ret;
  ACT_STATUS_INIT();

  UaNodeId projectFolderNodeId(MoxaClassBasedId_Server_Resources_Communication_Projects,
                               pClassBased->getNameSpaceIndex());

  UaReferenceDescriptions references;
  MoxaClassBased::ProjectType* pProjectType = NULL;
  pMoxaNodeManager->getNodeReference(projectFolderNodeId, OpcUa_False, references);
  for (OpcUa_UInt32 idx = 0; idx < references.length(); idx++) {
    if (references[idx].TypeDefinition.NodeId.Identifier.Numeric != MoxaClassBasedId_ProjectType) {
      continue;
    }

    UaNode* pNode = pMoxaNodeManager->getNode(references[idx].NodeId.NodeId);
    if (pNode->browseName().toString() == projectName) {
      qint64 project_id = pMoxaNodeManager->getProjectId(pNode->nodeId());

      QMutexLocker lock(&act::core::g_core.mutex_);

      // Delete project
      act_status = act::core::g_core.DeleteProject(project_id);
      if (!IsActStatusSuccess(act_status)) {
        errorCode = static_cast<OpcUa_UInt32>(act_status->GetStatus());
        errorMessage = UaString(act_status->GetErrorMessage().toStdString().c_str());
        qDebug() << errorMessage.toUtf8();
        return ret;
      }
    }
  }

  errorCode = M_UA_NO_ERROR;
  return ret;
}

}  // namespace ClassBased