#pragma once

#include "act_core.hpp"
#include "act_status.hpp"
#include "dto/act_group_dto.hpp"
#include "oatpp/core/macro/codegen.hpp"
#include "oatpp/core/macro/component.hpp"
#include "oatpp/web/server/api/ApiController.hpp"

#include OATPP_CODEGEN_BEGIN(ApiController)

class ActGroupController : public oatpp::web::server::api::ApiController {
 private:
  oatpp::concurrency::SpinLock m_lock;

 private:
  std::shared_ptr<MyBearerAuthorizationHandler> m_authHandler = std::make_shared<MyBearerAuthorizationHandler>();

 public:
  typedef ActGroupController __ControllerType;

 public:
  /**
   * @brief Construct a new My Controller object
   *
   * objectMapper - default object mapper used to serialize/deserialize DTOs.
   */
  ActGroupController(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>, objectMapper))
      : oatpp::web::server::api::ApiController(objectMapper) {
    setDefaultAuthorizationHandler(
        std::make_shared<oatpp::web::server::handler::BearerAuthorizationHandler>("my-realm"));  // JWT
  }

 public:
  /**
   *  Inject @objectMapper component here as default parameter
   */
  static std::shared_ptr<ActGroupController> createShared(OATPP_COMPONENT(std::shared_ptr<ObjectMapper>,
                                                                          objectMapper)) {
    return std::make_shared<ActGroupController>(objectMapper);
  }

 public:
  // TODO Insert Your endpoints here !!!

  ENDPOINT_INFO(GetGroups) {
    info->summary = "Get all group configurations";
    info->addSecurityRequirement("my-realm");
    info->addTag("Group");
    info->description = "This RESTful API only permit for [Admin, Supervisor, User]";
    info->addResponse<Object<ActGroupsDto>>(Status::CODE_200, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_404, "application/json")
        .addExample("Not Found", ActStatusDto::createShared(StatusDtoEnum::kNotFound, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;
    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("GET", QString("%1/project/{projectId}/groups").arg(ACT_API_PATH_PREFIX).toStdString(), GetGroups,
           PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "GET URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    // Handle request
    ACT_STATUS_INIT();
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    ActProject project;
    qint64 id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act_status = act::core::g_core.GetProject(id, project, is_operation);
    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Success(200)";
    return createResponse(Status::CODE_200, project.GetTopologySetting().ToString("Groups").toStdString());
  }

  ENDPOINT_INFO(CreateGroup) {
    info->summary = "Create a new group";
    info->addSecurityRequirement("my-realm");
    info->addTag("Group");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActCreateGroupDto>>("application/json");
    info->addResponse<Object<ActGroupDto>>(Status::CODE_201, "application/json");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActDuplicatedDto>>(Status::CODE_409, "application/json", "The group already exist");

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("POST", QString("%1/project/{projectId}/group").arg(ACT_API_PATH_PREFIX).toStdString(), CreateGroup,
           PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "POST URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActGroup group;
    try {
      group.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.CreateGroup(project_id, group, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    // qDebug() << "Response: Created(201)";
    return createResponse(Status::CODE_201, group.ToString().toStdString());
  }

  ENDPOINT_INFO(UpdateGroup) {
    info->summary = "Update a group configuration";
    info->addSecurityRequirement("my-realm");
    info->addTag("Group");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addConsumes<Object<ActGroupDto>>("application/json");
    info->addResponse(Status::CODE_204, "The project was successfully deleted (or did not exist)");

    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
  }
  ENDPOINT("PUT", QString("%1/project/{projectId}/group").arg(ACT_API_PATH_PREFIX).toStdString(), UpdateGroup,
           PATH(UInt64, projectId), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "PUT URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Dto -> ACT Class
    QString str = request->readBodyToString().getPtr()->c_str();
    ActGroup group;
    try {
      group.FromString(str);
    } catch (std::exception &e) {
      ActBadRequest bad_request(e.what());
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(bad_request.GetStatus(), kActStatusTypeMap)
               << bad_request.ToString(bad_request.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(bad_request.GetStatus()),
                            bad_request.ToString(bad_request.key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UpdateGroup(project_id, group, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status->SetStatus(ActStatusType::kNoContent);
    // qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(DeleteGroup) {
    info->summary = "Delete a group";
    info->addSecurityRequirement("my-realm");
    info->addTag("Group");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "If you DELETE something that doesn't exist, you should just return a 204 (even if the resource never "
        "existed). The client wanted the resource gone and it is gone.");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["groupId"].description = "The identifier of the group";
  }
  ENDPOINT("DELETE", QString("%1/project/{projectId}/group/{groupId}/delete").arg(ACT_API_PATH_PREFIX).toStdString(),
           DeleteGroup, PATH(UInt64, projectId), PATH(UInt64, groupId),
           REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "DELETE URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;
    qint64 group_id = *groupId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.DeleteGroup(project_id, group_id, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status->SetStatus(ActStatusType::kNoContent);
    // qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }

  ENDPOINT_INFO(UnGroup) {
    info->summary = "Ungroup";
    info->addSecurityRequirement("my-realm");
    info->addTag("Group");
    info->description = "This RESTful API only permit for [Admin, Supervisor]";
    info->addResponse<Object<ActStatusDto>>(
        Status::CODE_204, "application/json",
        "If you DELETE something that doesn't exist, you should just return a 204 (even if the resource never "
        "existed). The client wanted the resource gone and it is gone.");
    info->addResponse<Object<ActBadRequestDto>>(Status::CODE_400, "application/json")
        .addExample("Bad Request", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_401, "application/json")
        .addExample("Unauthorized", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));
    info->addResponse<Object<ActStatusDto>>(Status::CODE_403, "application/json")
        .addExample("Forbidden", ActStatusDto::createShared(StatusDtoEnum::kFailed, SeverityDtoEnum::kCritical));

    info->queryParams.add<String>("mode").description = "Project mode: 'design' or 'operation'. Default: 'design'.";
    info->queryParams["mode"].required = false;

    info->pathParams["projectId"].description = "The identifier of the project";
    info->pathParams["groupId"].description = "The identifier of the group";
  }
  ENDPOINT("DELETE", QString("%1/project/{projectId}/group/{groupId}/ungroup").arg(ACT_API_PATH_PREFIX).toStdString(),
           UnGroup, PATH(UInt64, projectId), PATH(UInt64, groupId), REQUEST(std::shared_ptr<IncomingRequest>, request),
           AUTHORIZATION(std::shared_ptr<BearerAuthorizationObject>, authorizationBearer, m_authHandler)) {
    if (authorizationBearer->role == ActRoleEnum::kUnauthorized) {
      ActUnauthorized unauthorized;
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(unauthorized.GetStatus(), kActStatusTypeMap)
               << unauthorized.ToString(unauthorized.key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(unauthorized.GetStatus()),
                            unauthorized.ToString(unauthorized.key_order_).toStdString());
    }

    auto routes = request->getStartingLine().path.std_str();
    qDebug() << "DELETE URL:" << routes.c_str();

    // 解析 mode（僅接受 design / operation，其餘視為 design）
    oatpp::String modeQ = request->getQueryParameter("mode", "design");  // 若沒帶，回傳預設
    std::string mode_str = modeQ ? modeQ->c_str() : std::string("design");
    std::transform(mode_str.begin(), mode_str.end(), mode_str.begin(), ::tolower);
    bool is_operation = (mode_str == "operation");
    if (mode_str != "design" && mode_str != "operation") {
      qDebug() << "Warning: Invalid 'mode' value:" << mode_str.c_str() << " -> fallback to 'design'";
    }

    ACT_STATUS_INIT();

    if (authorizationBearer->role == ActRoleEnum::kUser) {
      qDebug() << "Response:" << GetStringFromEnum<ActStatusType>(ActStatusType::kForbidden, kActStatusTypeMap);
      act_status->SetActStatus(ActStatusType::kForbidden, ActSeverity::kCritical);
      return createResponse(Status::CODE_403, act_status->ToString(act_status->key_order_).toStdString());
    }

    // Handle request
    std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);

    qint64 project_id = *projectId;
    qint64 group_id = *groupId;

    QMutexLocker core_lock(&act::core::g_core.mutex_);

    act::core::g_core.StartTransaction(project_id);
    act_status = act::core::g_core.UnGroup(project_id, group_id, is_operation);
    if (IsActStatusSuccess(act_status)) {
      act::core::g_core.CommitTransaction(project_id);
    } else {
      act::core::g_core.StopTransaction(project_id);
    }

    if (!IsActStatusSuccess(act_status)) {
      qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
      return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                            act_status->ToString(act_status->key_order_).toStdString());
    }

    act_status->SetStatus(ActStatusType::kNoContent);
    // qDebug() << "Response:" << act_status->ToString(act_status->key_order_).toStdString().c_str();
    return createResponse(TransferActStatusToOatppStatus(act_status->GetStatus()),
                          act_status->ToString(act_status->key_order_).toStdString());
  }
};

#include OATPP_CODEGEN_END(ApiController)