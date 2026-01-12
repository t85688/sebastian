/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#ifndef ACT_HOST_DTO_HPP
#define ACT_HOST_DTO_HPP

#include "oatpp/core/Types.hpp"
#include "oatpp/core/macro/codegen.hpp"

#include OATPP_CODEGEN_BEGIN(DTO)

/**
 *  Data Transfer Object. Object containing fields only.
 *  Used in API for serialization/deserialization and validation
 */
class ActHostAdapterDto : public oatpp::DTO {
  DTO_INIT(ActHostAdapterDto, DTO)

  DTO_FIELD(String, adapter_name, "AdapterName");
  DTO_FIELD(String, ip_address, "IpAddress");
  DTO_FIELD(String, net_mask, "NetMask");
};

class ActHostAdapterListDto : public oatpp::DTO {
  DTO_INIT(ActHostAdapterListDto, DTO)

  DTO_FIELD(List<Object<ActHostAdapterDto>>, host_adapter_list, "HostAdapterList");
};

#include OATPP_CODEGEN_END(DTO)

#endif /* ACT_HOST_DTO_HPP */
