/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_CLASS_BASED_DTO_HPP
#define ACT_CLASS_BASED_DTO_HPP

#include "./act_stream_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)
ENUM(TimeSlotTrafficTypeDtoEnum, v_int32, VALUE(kNA, 1, "NA"), VALUE(kBestEffort, 2, "BestEffort"),
     VALUE(kCyclic, 3, "Cyclic"), VALUE(kTimeSync, 4, "TimeSync"))

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActAdminBaseTimeDto : public oatpp::DTO {
  DTO_INIT(ActAdminBaseTimeDto, DTO)

  DTO_FIELD(UInt64, seconds, "Second");
  DTO_FIELD(UInt64, fractional_seconds, "FractionalSeconds");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActAdminCycleTimeDto : public oatpp::DTO {
  DTO_INIT(ActAdminCycleTimeDto, DTO)

  DTO_FIELD(UInt32, numerator, "Numerator");
  DTO_FIELD(UInt32, denominator, "Denominator");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActTimeSlotSettingDto : public oatpp::DTO {
  DTO_INIT(ActTimeSlotSettingDto, DTO)

  DTO_FIELD(UInt8, index, "Index");
  DTO_FIELD(UInt16, period, "Period");
  DTO_FIELD(Enum<TimeSlotTrafficTypeDtoEnum>, traffic_type, "TrafficType");
};

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActCycleSettingDto : public oatpp::DTO {
  DTO_INIT(ActCycleSettingDto, DTO)

  DTO_FIELD(Object<ActAdminBaseTimeDto>, admin_base_time, "AdminBaseTime");
  DTO_FIELD(Object<ActAdminCycleTimeDto>, admin_cycle_time, "AdminCycleTime");
  DTO_FIELD(List<Object<ActTimeSlotSettingDto>>, time_slots, "TimeSlots");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_CLASS_BASED_DTO_HPP */
