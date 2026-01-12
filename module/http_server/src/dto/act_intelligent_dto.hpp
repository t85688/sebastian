/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_INTELLIGENT_DTO_HPP
#define ACT_INTELLIGENT_DTO_HPP

// #include "./act_device_dto.hpp"
#include "./act_project_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIntelligentUploadFileDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentUploadFileDto, DTO)

  DTO_FIELD(String, file_name, "FileName");
  DTO_FIELD(String, file, "File");
};

// class ActIntelligentRecognizeResponseDto : public oatpp::DTO {
//   DTO_INIT(ActIntelligentRecognizeResponseDto, DTO)

//   DTO_FIELD(String, status, "status");
//   DTO_FIELD(String, type, "type");
//   DTO_FIELD(String, value, "value");

//   ActIntelligentRecognizeResponseDto() {}

//   ActIntelligentRecognizeResponseDto(String status_, String type_, String value_) {
//     this->status = status_;
//     this->type = type_;
//     this->value = value_;
//   }
// };

class ActIntelligentRecognizeButtonResponseDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentRecognizeButtonResponseDto, DTO)

  DTO_FIELD(String, text, "text");
  DTO_FIELD(String, display, "display");
};

class ActIntelligentRecognizeActionResponseDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentRecognizeActionResponseDto, DTO)

  DTO_FIELD(String, type, "type");
  DTO_FIELD(String, target, "target");
};

class ActIntelligentRecognizeResponseDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentRecognizeResponseDto, DTO)

  DTO_FIELD(List<Object<ActIntelligentRecognizeActionResponseDto>>, actions, "actions");
  DTO_FIELD(List<Object<ActIntelligentRecognizeButtonResponseDto>>, buttons, "buttons");
  DTO_FIELD(Boolean, feedback, "feedback");
  DTO_FIELD(String, id, "id");
  DTO_FIELD(String, reply, "reply");
};

class ActIntelligentStatusResponseDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentStatusResponseDto, DTO)

  DTO_FIELD(UInt32, code, "code");
  DTO_FIELD(String, message, "message");
};

class ActIntelligentResponseDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentResponseDto, DTO)

  DTO_FIELD(Object<ActIntelligentStatusResponseDto>, status, "status");
  DTO_FIELD(Object<ActIntelligentRecognizeResponseDto>, response, "response");
};

class ActIntelligentResponseReplyDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentResponseReplyDto, DTO)

  DTO_FIELD(String, reply, "reply");
  DTO_FIELD(Boolean, toolbar, "toolbar");
  DTO_FIELD(List<Object<ActIntelligentRecognizeActionResponseDto>>, actions, "actions");
  DTO_FIELD(List<Object<ActIntelligentRecognizeButtonResponseDto>>, buttons, "buttons");
};

class ActIntelligentHistoryMessageDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentHistoryMessageDto, DTO)

  DTO_FIELD(String, id, "id");
  DTO_FIELD(String, time, "time");
  DTO_FIELD(String, role, "role");
  DTO_FIELD(String, input, "input");
  DTO_FIELD(Object<ActIntelligentResponseReplyDto>, reply, "reply");
};

class ActIntelligentHistoryResponseBodyDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentHistoryResponseBodyDto, DTO)

  DTO_FIELD(String, nextOffset, "nextOffset");
  DTO_FIELD(List<Object<ActIntelligentHistoryMessageDto>>, messages, "messages");
};

class ActIntelligentHistoryResponseDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentHistoryResponseDto, DTO)

  DTO_FIELD(Object<ActIntelligentStatusResponseDto>, status, "status");
  DTO_FIELD(Object<ActIntelligentHistoryResponseBodyDto>, response, "response");
};

class ActIntelligentReportDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentReportDto, DTO)

  DTO_FIELD(String, id, "Id");
  DTO_FIELD(Boolean, positive, "Positive") = false;
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActIntelligentHistoryDto : public oatpp::DTO {
  DTO_INIT(ActIntelligentHistoryDto, DTO)

  // DTO_FIELD(String, session_id, "SessionId");
  DTO_FIELD(Enum<ActProjectModeDtoEnum>, project_mode, "ProjectMode");
  DTO_FIELD(Enum<ActQuestionnaireModeDtoEnum>, questionnaire_mode, "QuestionnaireMode");
  DTO_FIELD(Enum<ActServiceProfileForLicenseDtoEnum>, profile, "Profile");

  DTO_FIELD(String, offset, "Offset");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_INTELLIGENT_DTO_HPP */
