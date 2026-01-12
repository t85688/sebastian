/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/
#pragma once

#include "act_json.hpp"
#include "act_project.hpp"

/**
 * @brief The request body of intelligent request
 *
 */
class ActIntelligentRecognizeRequest : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, intelligent_endpoint, IntelligentEndpoint);  // Config from Chamberlain system.json
  ACT_JSON_FIELD(QString, session_id, SessionId);                      // Device serial number
  ACT_JSON_FIELD(QString, project_id, ProjectId);                      // Which project you are working
  ACT_JSON_FIELD(QString, text, Text);                                 // User input text, use help to show commands
  ACT_JSON_FIELD(QString, sys_version, SysVersion);                    // Chamberlain version
  ACT_JSON_FIELD(bool, auto_send, AutoSend);  // If request by front-end automatically (ex. Button events)
  ACT_JSON_ENUM(ActProjectModeEnum, project_mode, ProjectMode);
  ACT_JSON_ENUM(ActQuestionnaireModeEnum, questionnaire_mode, QuestionnaireMode);
  ACT_JSON_ENUM(ActServiceProfileForLicenseEnum, profile, Profile);

 public:
  ActIntelligentRecognizeRequest() {
    intelligent_endpoint_ = "";
    session_id_ = "";
    project_id_ = "";
    text_ = "";
    sys_version_ = "";
    auto_send_ = false;
    project_mode_ = ActProjectModeEnum::kDesign;
    questionnaire_mode_ = ActQuestionnaireModeEnum::kNone;
    profile_ = ActServiceProfileForLicenseEnum::kSelfPlanning;
  }
};

/**
 * @brief The request body of intelligent upload
 *
 */
class ActIntelligentUploadFile : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, file_name, FileName);
  ACT_JSON_FIELD(QString, file, File);

 public:
  ActIntelligentUploadFile() {
    file_name_ = "";
    file_ = "";
  }
};

/**
 * @brief The request body of intelligent upload
 *
 */
class ActIntelligentQuestionnaireUpload : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, intelligent_endpoint, IntelligentEndpoint);  // Config from Chamberlain system.json
  ACT_JSON_FIELD(QString, session_id, SessionId);                      // Device serial number
  ACT_JSON_FIELD(QString, project_id, ProjectId);                      // Which project you are working
  ACT_JSON_FIELD(QString, sys_version, SysVersion);                    // Chamberlain version
  ACT_JSON_ENUM(ActProjectModeEnum, project_mode, ProjectMode);
  ACT_JSON_ENUM(ActQuestionnaireModeEnum, questionnaire_mode, QuestionnaireMode);
  ACT_JSON_ENUM(ActServiceProfileForLicenseEnum, profile, Profile);
  ACT_JSON_FIELD(QString, file_name, FileName);
  ACT_JSON_FIELD(QString, file, File);
  ACT_JSON_FIELD(bool, verify, Verify);

 public:
  ActIntelligentQuestionnaireUpload() {
    intelligent_endpoint_ = "";
    session_id_ = "";
    project_id_ = "";
    sys_version_ = "";
    project_mode_ = ActProjectModeEnum::kDesign;
    questionnaire_mode_ = ActQuestionnaireModeEnum::kUnVerified;
    profile_ = ActServiceProfileForLicenseEnum::kGeneral;
    file_name_ = "";
    file_ = "";
    verify_ = true;
  }
};

/**
 * @brief The request body of intelligent questionnaire download API
 *
 */
class ActIntelligentQuestionnaireDownload : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, intelligent_endpoint, IntelligentEndpoint);  // Config from Chamberlain system.json
  ACT_JSON_FIELD(QString, session_id, SessionId);                      // Device serial number
  ACT_JSON_FIELD(QString, project_id, ProjectId);                      // Which project you are working
  ACT_JSON_FIELD(QString, sys_version, SysVersion);                    // Chamberlain version
  ACT_JSON_ENUM(ActProjectModeEnum, project_mode, ProjectMode);
  ACT_JSON_ENUM(ActQuestionnaireModeEnum, questionnaire_mode, QuestionnaireMode);
  ACT_JSON_ENUM(ActServiceProfileForLicenseEnum, profile, Profile);

 public:
  ActIntelligentQuestionnaireDownload() {
    intelligent_endpoint_ = "";
    session_id_ = "";
    project_id_ = "";
    sys_version_ = "";
    project_mode_ = ActProjectModeEnum::kDesign;
    questionnaire_mode_ = ActQuestionnaireModeEnum::kUnVerified;
    profile_ = ActServiceProfileForLicenseEnum::kGeneral;
  }
};

/**
 * @brief The recognize action response in the intelligent response body
 *
 */
class ActIntelligentRecognizeActionResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, type, type);
  ACT_JSON_FIELD(QString, target, target);
};

/**
 * @brief The recognize button response in the intelligent response body
 *
 */
class ActIntelligentRecognizeButtonResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, text, text);
  ACT_JSON_FIELD(QString, display, display);

 public:
  ActIntelligentRecognizeButtonResponse() {
    this->display_ = "";
    this->text_ = "";
  }

  ActIntelligentRecognizeButtonResponse(const QString &display, const QString &text)
      : ActIntelligentRecognizeButtonResponse() {
    this->display_ = display;
    this->text_ = text;
  }
};

/**
 * @brief The response reply in the intelligent response body
 *
 */
class ActIntelligentResponseReply : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, reply, reply);   // reply: Reply to the user in the chat room
  ACT_JSON_FIELD(bool, toolbar, toolbar);  // toolbar
  ACT_JSON_COLLECTION_OBJECTS(QList, ActIntelligentRecognizeActionResponse, actions,
                              actions);  // actions: Front-end do something
  ACT_JSON_COLLECTION_OBJECTS(QList, ActIntelligentRecognizeButtonResponse, buttons,
                              buttons);  // buttons: Display buttons and click to send text (autoSend=true)
};

/**
 * @brief The status code in the intelligent response body
 *
 */
class ActIntelligentStatusResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(qint32, code, code);
  ACT_JSON_FIELD(QString, message, message);
};

class ActIntelligentRecognizeResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, id, id);
  ACT_JSON_FIELD(QString, role, role);
  ACT_JSON_FIELD(QString, input, input);
  ACT_JSON_OBJECT(ActIntelligentResponseReply, reply, reply);
};

/**
 * @brief The response body of the intelligent request
 *
 */
class ActIntelligentResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActIntelligentStatusResponse, status, status);
  ACT_JSON_OBJECT(ActIntelligentRecognizeResponse, response, response);
};

class ActIntelligentReport : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, intelligent_endpoint, IntelligentEndpoint);  // Config from Chamberlain system.json
  ACT_JSON_FIELD(QString, id, Id);  // Chat ID return from /api/v1/send body response.id
  ACT_JSON_FIELD(bool, positive,
                 Positive);  // Determine quality of reply by clicking positive (True) or negative (False)
};

/**
 * @brief The request body of intelligent history
 *
 */
class ActIntelligentHistory : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, intelligent_endpoint, IntelligentEndpoint);  // Config from Chamberlain system.json
  ACT_JSON_FIELD(QString, session_id, SessionId);    // User unique ID to distinguish different chat box
  ACT_JSON_FIELD(QString, project_id, ProjectId);    // Which project you are working
  ACT_JSON_FIELD(QString, sys_version, SysVersion);  // The version of ACT
  ACT_JSON_ENUM(ActProjectModeEnum, project_mode, ProjectMode);
  ACT_JSON_ENUM(ActQuestionnaireModeEnum, questionnaire_mode, QuestionnaireMode);
  ACT_JSON_ENUM(ActServiceProfileForLicenseEnum, profile, Profile);
  ACT_JSON_FIELD(QString, offset, Offset);  // Next offset id, not passed when the first loading

 public:
  /**
   * @brief Construct a new intelligent history request object
   *
   */
  ActIntelligentHistory() {
    intelligent_endpoint_ = "";
    session_id_ = "";
    project_id_ = "";
    project_mode_ = ActProjectModeEnum::kDesign;
    questionnaire_mode_ = ActQuestionnaireModeEnum::kNone;
    profile_ = ActServiceProfileForLicenseEnum::kSelfPlanning;
    offset_ = "";
  }
};

/**
 * @brief The request body of intelligent history
 *
 */
class ActIntelligentHistoryMessage : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, id, id);
  ACT_JSON_FIELD(QString, time, time);
  ACT_JSON_FIELD(QString, role, role);
  ACT_JSON_FIELD(QString, input, input);
  ACT_JSON_OBJECT(ActIntelligentResponseReply, reply, reply);
};

/**
 * @brief The request body of intelligent history
 *
 */
class ActIntelligentHistoryResponseBody : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_FIELD(QString, next_offset, nextOffset);
  ACT_JSON_COLLECTION_OBJECTS(QList, ActIntelligentHistoryMessage, messages, messages);
};

/**
 * @brief The response of the intelligent history
 *
 */
class ActIntelligentHistoryResponse : public QSerializer {
  Q_GADGET
  QS_SERIALIZABLE

  ACT_JSON_OBJECT(ActIntelligentStatusResponse, status, status);
  ACT_JSON_OBJECT(ActIntelligentHistoryResponseBody, response, response);
};
