
/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_EVENT_LOG_DTO_HPP
#define ACT_EVENT_LOG_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */

class ActEventLogDataDto : public oatpp::DTO {
  DTO_INIT(ActEventLogDataDto, DTO)

  DTO_FIELD(Int32, code, "code") = 3010001;
  DTO_FIELD(String, timestamp, "timestamp") = "2020-02-25T00:08:54.97510268+08:00";
  DTO_FIELD(Int32, id, "id") = 1;
  DTO_FIELD(String, message, "message") = "Port 1 link down.";
  DTO_FIELD(String, source, "source") = "trap";
  DTO_FIELD(String, sourceIp, "sourceIp") = "127.0.0.1";
  DTO_FIELD(String, severity, "severity") = "warning";
  DTO_FIELD(List<String>, variables, "variables");
};

class ActEventLogsDto : public oatpp::DTO {
  DTO_INIT(ActEventLogsDto, DTO)

  DTO_FIELD(Int32, count, "count") = 1;
  DTO_FIELD(List<Object<ActEventLogDataDto>>, data, "data");
  DTO_FIELD(Int32, limit, "limit") = 1;
  DTO_FIELD(Int32, offset, "offset") = 0;
  DTO_FIELD(Int32, total, "total") = 100;
};

class AcSyslogDataDto : public oatpp::DTO {
  DTO_INIT(AcSyslogDataDto, DTO)

  DTO_FIELD(Int32, id, "id") = 1;
  DTO_FIELD(Int32, severity, "severity") = 5;
  DTO_FIELD(Int32, facility, "facility") = 20;
  DTO_FIELD(String, ipaddress, "ipaddress") = "192.168.127.100";
  DTO_FIELD(String, syslog_time, "syslogTime") = "Feb  25 00:08:54";
  DTO_FIELD(String, timestamp, "timestamp") = "2020-02-25T00:08:54.97510268+08:00";
  DTO_FIELD(String, message, "message") = "This is a syslog message";
};

class ActSyslogsDto : public oatpp::DTO {
  DTO_INIT(ActSyslogsDto, DTO)

  DTO_FIELD(Int32, count, "count") = 1;
  DTO_FIELD(List<Object<AcSyslogDataDto>>, data, "data");
  DTO_FIELD(Int32, limit, "limit") = 1;
  DTO_FIELD(Int32, offset, "offset") = 0;
  DTO_FIELD(Int32, total, "total") = 100;
};

class ActCsvPathDto : public oatpp::DTO {
  DTO_INIT(ActCsvPathDto, DTO)

  DTO_FIELD(String, filepath, "filepath") = "./saved_logs";
};

class ActGetSyslogConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActGetSyslogConfigurationDto, DTO)

  DTO_FIELD(Boolean, enable, "enable") = true;
  DTO_FIELD(Int32, port, "port") = 514;
};

class ActUpdateSyslogConfigurationDto : public oatpp::DTO {
  DTO_INIT(ActUpdateSyslogConfigurationDto, DTO)

  DTO_FIELD(Boolean, enable, "enable") = true;
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_INTELLIGENT_DTO_HPP */
