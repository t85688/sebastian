
#include "http_utils.h"

#include <string.h>

#include <cstdarg>
#include <cstring>
#include <fstream>
#include <iostream>

// kene+
#ifdef _WIN32
#include <windows.h>
#endif
// kene-

oatpp::String StaticFilesManager::GetFile(const oatpp::String &path) {
  if (!path) {
    return nullptr;
  }
  std::string x = path;
  //   std::cout << "GetFile(): " << x.c_str() << std::endl;
  // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  // auto& file = m_cache[path];
  // if (file) {
  //   return file;
  // }
  oatpp::String filename = m_base_path + "/" + path;
  std::string y = filename;
  //   std::cout << "GetFile() filename: " << y.c_str() << std::endl;
  auto file = oatpp::String::loadFromFile(filename->c_str());
  return file;
}

oatpp::String StaticDeviceIconFilesManager::GetFile(const oatpp::String &path) {
  if (!path) {
    return nullptr;
  }
  std::string x = path;
  //   std::cout << "GetFile(): " << x.c_str() << std::endl;
  // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  // auto& file = m_cache[path];
  // if (file) {
  //   return file;
  // }
  // kene+
  /*
  oatpp::String filename = m_base_path + "/" + path;
  */
  oatpp::String filename = GetCogsworthConfPath() + "/" + m_base_path + "/" + path;
  // kene-
  std::string y = filename;
  // std::cout << "GetFile() filename: " << y.c_str() << std::endl;
  auto file = oatpp::String::loadFromFile(filename->c_str());
  return file;
}

oatpp::String StaticTopologyIconFilesManager::GetFile(const oatpp::String &path) {
  if (!path) {
    return nullptr;
  }
  std::string x = path;
  // std::cout << "GetFile(): " << x.c_str() << std::endl;
  // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  // auto& file = m_cache[path];
  // if (file) {
  //   return file;
  // }
  // kene+
  /*
  oatpp::String filename = m_base_path + "/" + path;
  */
  oatpp::String filename = GetCogsworthConfPath() + "/" + m_base_path + "/" + path;
  // kene-
  std::string y = filename;
  // std::cout << "GetFile() filename: " << y.c_str() << std::endl;
  auto file = oatpp::String::loadFromFile(filename->c_str());
  return file;
}

oatpp::String StaticQuestionnaireTemplateFilesManager::GetFile(const oatpp::String &path) {
  if (!path) {
    return nullptr;
  }
  std::string x = path;
  // std::cout << "GetFile(): " << x.c_str() << std::endl;
  // std::lock_guard<oatpp::concurrency::SpinLock> lock(m_lock);
  // auto& file = m_cache[path];
  // if (file) {
  //   return file;
  // }
  // kene+
  /*
  oatpp::String filename = m_base_path + "/" + path;
  */
  oatpp::String filename = GetCogsworthConfPath() + "/" + m_base_path + "/" + path;
  // kene-
  std::string y = filename;
  // std::cout << "GetFile() filename: " << y.c_str() << std::endl;
  auto file = oatpp::String::loadFromFile(filename->c_str());
  return file;
}

// kene+
oatpp::String GetCogsworthConfPath() {
#ifdef _WIN32
  const DWORD buffSize = 65535;
  static char buffer[buffSize];
  if (GetEnvironmentVariableA("CHAMBERLAIN_COGSWORTH_CONF_FOLDER", buffer, buffSize)) {
    return buffer;
  } else {
    return ".";
  }
#else
  return ".";
#endif
}

int GetCogsworthServerPort() {
#ifdef _WIN32
  const DWORD buffSize = 65535;
  static char buffer[buffSize];
  if (GetEnvironmentVariableA("CHAMBERLAIN_COGSWORTH_HTTP_PORT", buffer, buffSize)) {
    int port = atoi(buffer);
    return port;
  } else {
    return 8443;
  }
#else
  return 8443;
#endif
}

oatpp::String GetMainHttpsUrlForSwagger() {
#ifdef _WIN32
  const DWORD buffSize = 65535;
  static char buffer[buffSize];
  if (GetEnvironmentVariableA("CHAMBERLAIN_MAIN_HTTP_PORT", buffer, buffSize)) {
    oatpp::String baseUrl = "https://localhost:";
    return baseUrl + buffer;
  } else {
    return "https://localhost";
  }
#else
  return "https://localhost"
#endif
}
// kene-
