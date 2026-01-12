
/* Copyright (C) MOXA Inc. All rights reserved.
This software is distributed under the terms of the MOXA SOFTWARE NOTICE.
See the file MOXA-SOFTWARE-NOTICE for details.
*/

#pragma once

#include <string>
#include <unordered_map>

#include "oatpp/core/Types.hpp"
#include "oatpp/core/concurrency/SpinLock.hpp"

class StaticFilesManager {
 private:
  oatpp::String m_base_path;
  oatpp::concurrency::SpinLock m_lock;
  std::unordered_map<oatpp::String, oatpp::String> m_cache;

 public:
  StaticFilesManager(const oatpp::String &base_path) : m_base_path(base_path) {}

  oatpp::String GetFile(const oatpp::String &path);
};

class StaticDeviceIconFilesManager {
 private:
  oatpp::String m_base_path;
  oatpp::concurrency::SpinLock m_lock;
  std::unordered_map<oatpp::String, oatpp::String> m_cache;

 public:
  StaticDeviceIconFilesManager(const oatpp::String &base_path) : m_base_path(base_path) {}

  oatpp::String GetFile(const oatpp::String &path);
};

class StaticTopologyIconFilesManager {
 private:
  oatpp::String m_base_path;
  oatpp::concurrency::SpinLock m_lock;
  std::unordered_map<oatpp::String, oatpp::String> m_cache;

 public:
  StaticTopologyIconFilesManager(const oatpp::String &base_path) : m_base_path(base_path) {}

  oatpp::String GetFile(const oatpp::String &path);
};

class StaticQuestionnaireTemplateFilesManager {
 private:
  oatpp::String m_base_path;
  oatpp::concurrency::SpinLock m_lock;
  std::unordered_map<oatpp::String, oatpp::String> m_cache;

 public:
  StaticQuestionnaireTemplateFilesManager(const oatpp::String &base_path) : m_base_path(base_path) {}

  oatpp::String GetFile(const oatpp::String &path);
};

// kene+
oatpp::String GetCogsworthConfPath();
int GetCogsworthServerPort();
oatpp::String GetMainHttpsUrlForSwagger();
// kene-
