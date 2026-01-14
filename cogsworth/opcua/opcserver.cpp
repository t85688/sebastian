/******************************************************************************
** opcserver.cpp
**
** Copyright (c) 2006-2020 Unified Automation GmbH. All rights reserved.
**
** Software License Agreement ("SLA") Version 2.7
**
** Unless explicitly acquired and licensed from Licensor under another
** license, the contents of this file are subject to the Software License
** Agreement ("SLA") Version 2.7, or subsequent versions
** as allowed by the SLA, and You may not copy or use this file in either
** source code or executable form, except in compliance with the terms and
** conditions of the SLA.
**
** All software distributed under the SLA is provided strictly on an
** "AS IS" basis, WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
** AND LICENSOR HEREBY DISCLAIMS ALL SUCH WARRANTIES, INCLUDING WITHOUT
** LIMITATION, ANY WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR
** PURPOSE, QUIET ENJOYMENT, OR NON-INFRINGEMENT. See the SLA for specific
** language governing rights and limitations under the SLA.
**
** The complete license agreement can be found here:
** http://unifiedautomation.com/License/SLA/2.7/
**
** Project: C++ OPC Server SDK sample code
**
** Description: Main OPC Server object class.
**
******************************************************************************/
#include "opcserver.h"

#include <qDebug>

#include "uamodule.h"
#include "uasession.h"

#ifndef UA_BUILD_DATE_ZONE
#define UA_BUILD_DATE_ZONE 1  // Must match UTC offset and daylight saving time at build date
#endif                        /* UA_BUILD_DATE_ZONE */

/** Construction. */
OpcServer::OpcServer() {}

/** Construction. */
OpcServer::OpcServer(int argc, char* argv[], bool bRunAsService, const UaString& sApplicationName)
    : UaServerApplication(argc, argv, bRunAsService, sApplicationName) {
  qDebug() << "OpcServer construector";
}

/** Destruction. */
OpcServer::~OpcServer() {
  qDebug() << "OpcServer deconstruector";
  if (isStarted() != OpcUa_False) {
    UaLocalizedText reason("en", "Application shut down");
    stop(0, reason);
  }
  qDebug() << "OpcServer deconstruector done";
}

UaStatus OpcServer::afterStartUp() {
  UaStatus ret = UaServerApplication::afterStartUp();
  // qDebug() << "ret:" << ret.toString().toOpcUaString();
  if (ret.isGood()) {
    UaString sRejectedCertificateDirectory;
    OpcUa_UInt32 nRejectedCertificatesCount;
    UaEndpointArray uaEndpointArray;
    getServerConfig()->getEndpointConfiguration(sRejectedCertificateDirectory, nRejectedCertificatesCount,
                                                uaEndpointArray);
    if (uaEndpointArray.length() > 0) {
      qInfo() << "***************************************************";
      qInfo() << " OPCUA Server opened endpoints for following URLs:";
      OpcUa_UInt32 idx;
      bool bError = false;
      for (idx = 0; idx < uaEndpointArray.length(); idx++) {
        if (uaEndpointArray[idx]->isOpened()) {
          qInfo() << "     " << uaEndpointArray[idx]->sEndpointUrl().toUtf8();
        } else {
          bError = true;
        }
      }
      if (bError) {
        qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
        qDebug() << "!!!! The following endpoints URLs failed:";
        for (idx = 0; idx < uaEndpointArray.length(); idx++) {
          if (uaEndpointArray[idx]->isOpened() == false) {
            qDebug() << "!!!! " << uaEndpointArray[idx]->sEndpointUrl().toUtf8();
          }
        }
        qDebug() << "!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!";
      }
      qInfo() << "***************************************************";
    }
  }

  return ret;
}

/** Get the build date from the static compiled in string.
 *  @return the build date from the static compiled in string.
 */
OpcUa_DateTime OpcServer::getBuildDate() const {
  static OpcUa_DateTime date;
  static const char szDate[] = __DATE__; /* "Mon DD YYYY" */
  static char szISO[] = "YYYY-MM-DDT" __TIME__ "Z";
  static const char* Months[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
  char mon = 0;

  /* set year */
  szISO[0] = szDate[7];
  szISO[1] = szDate[8];
  szISO[2] = szDate[9];
  szISO[3] = szDate[10];

  /* set month */
  while ((strncmp(Months[(int)mon], szDate, 3) != 0) && (mon < 11)) {
    mon++;
  }
  mon++;
  szISO[5] = '0' + mon / 10;
  szISO[6] = '0' + mon % 10;

  /* set day */
  szISO[8] = szDate[4];
  szISO[9] = szDate[5];

  /* convert to UA time */
  OpcUa_DateTime_GetDateTimeFromString(szISO, &date);

  /* correct time */
  UaDateTime buildDate(date);
  buildDate.addSecs(UA_BUILD_DATE_ZONE * 3600 * -1);

  return buildDate;
}
