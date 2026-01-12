/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include "./act_interface_dto.hpp"
#include "./act_system_dto.hpp"
#include "./act_temperature_dto.hpp"
#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

ENUM(ActCurrencyDtoEnum, v_int32, VALUE(kTWD, 1, "TWD"), VALUE(kUSD, 2, "USD"), VALUE(kEUR, 3, "EUR"))

class ActSkuDto : public oatpp::DTO {
  DTO_INIT(ActSkuDto, DTO)

  DTO_FIELD(Int64, id, "Id");
  DTO_FIELD(String, l1_category, "L1Category");
  DTO_FIELD(UInt64, data_version, "DataVersion");
  DTO_FIELD(String, module_name, "ModuleName");
  DTO_FIELD(String, icon_name, "IconName");
  DTO_FIELD(String, description, "Description");
  DTO_FIELD(Object<OperatingTemperatureCDto>, operating_temperature_c, "OperatingTemperatureC");
  DTO_FIELD(Enum<ActServiceProfileForDeviceProfileDtoEnum>, profiles,
            "Profiles") = ActServiceProfileForDeviceProfileDtoEnum::kSelfPlanning;
};

class ActSkuWithPriceDto : public ActSkuDto {
  DTO_INIT(ActSkuWithPriceDto, DTO)

  DTO_FIELD(String, price, "Price");
  DTO_FIELD(Enum<ActCurrencyDtoEnum>, currency, "Currency");
};

class ActGeneralProfilesDto : public oatpp::DTO {
  DTO_INIT(ActGeneralProfilesDto, DTO)

  DTO_FIELD(Fields<Object<ActSkuWithPriceDto>>, general_profile_map, "GeneralProfileMap");
};

class ActUpdateSkuQuantityRequestDto : public oatpp::DTO {
  DTO_INIT(ActUpdateSkuQuantityRequestDto, DTO)

  DTO_FIELD(Fields<UInt64>, sku_quantity, "SkuQuantity");
};

class ActSkuQuantityDto : public oatpp::DTO {
  DTO_INIT(ActSkuQuantityDto, DTO)

  DTO_FIELD(Int64, quantity, "Quantity");
  DTO_FIELD(Int64, inTopologyCount, "InTopologyCount");
  DTO_FIELD(String, price, "Price");
  DTO_FIELD(Enum<ActCurrencyDtoEnum>, currency, "Currency");
  DTO_FIELD(String, total_price, "TotalPrice");
};

class ActSkuQuantitiesDto : public oatpp::DTO {
  DTO_INIT(ActSkuQuantitiesDto, DTO)

  DTO_FIELD(Fields<Object<ActSkuQuantityDto>>, sku_quantities_map, "SkuQuantitiesMap");
};

class ActSkuListDto : public oatpp::DTO {
  DTO_INIT(ActSkuListDto, DTO)

  DTO_FIELD(List<String>, sku_list, "SkuList");
};

#include OATPP_CODEGEN_END(DTO)
