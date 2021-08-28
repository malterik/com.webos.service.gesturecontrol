// Copyright (c) 2020 LG Electronics, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// SPDX-License-Identifier: Apache-2.0

#include <glib.h>
#include <string>
#include <luna-service2/lunaservice.h>
#include <PmLog.h>
#include <pbnjson.hpp>
#include "rplidar.h"

using namespace rp::standalone::rplidar;
using namespace std;

static PmLogContext getPmLogContext()
{
    static PmLogContext s_context = 0;
    if (0 == s_context)
    {
        PmLogGetContext("NativeService", &s_context);
    }
    return s_context;
}

static pbnjson::JValue convertStringToJson(const char *rawData)
{
    pbnjson::JInput input(rawData);
    pbnjson::JSchema schema = pbnjson::JSchemaFragment("{}");
    pbnjson::JDomParser parser;
    if (!parser.parse(input, schema))
    {
        return pbnjson::JValue();
    }
    return parser.getDom();
}

static std::string convertJsonToString(const pbnjson::JValue json)
{
    return pbnjson::JGenerator::serialize(json, pbnjson::JSchemaFragment("{}"));
}

static bool onHello(LSHandle *sh, LSMessage* message, void* ctx)
{
    PmLogInfo(getPmLogContext(), "HANDLE_HELLO", 0, "hello method called");

    pbnjson::JValue reply = pbnjson::Object();
    if (reply.isNull())
        return false;

    reply.put("returnValue", true);
    reply.put("answer", "Hello, Native Service!!");

    LSError lserror;
    LSErrorInit(&lserror);

    if (!LSMessageReply(sh, message, reply.stringify().c_str(), &lserror))
    {
        PmLogError(getPmLogContext(), "HANDLE_HELLO", 0, "Message reply error!!");
        LSErrorPrint(&lserror, stdout);

        return false;
    }
    return true;
}

static bool cbGetTime(LSHandle *sh, LSMessage *msg, void *user_data)
{
    LSError lserror;
    LSErrorInit(&lserror);

    PmLogInfo(getPmLogContext(), "GETTIME_CALLBACK", 1, PMLOGJSON("payload", LSMessageGetPayload(msg)),  " ");

    pbnjson::JValue response = convertStringToJson(LSMessageGetPayload(msg));
    bool successCallback = response["returnValue"].asBool();
    if (successCallback)
    {
        int64_t utc= response["utc"].asNumber<int64_t>();
        PmLogInfo(getPmLogContext(), "GETTIME_CALLBACK", 1, PMLOGKFV("UTC", "%lld", utc),  " ");
    }

    return true;
}

static LSMethod serviceMethods[] = {
    { "hello", onHello }
};

bool checkRPLIDARHealth(RPlidarDriver* drv) {
  u_result op_result;
  rplidar_response_device_health_t healthinfo;

  op_result = drv->getHealth(healthinfo);
  if (IS_OK(op_result)) {  // the macro IS_OK is the preperred way to judge
                           // whether the operation is succeed.
    printf("RPLidar health status : %d\n", healthinfo.status);
    if (healthinfo.status == RPLIDAR_STATUS_ERROR) {
      fprintf(stderr,
              "Error, rplidar internal error detected. Please reboot the "
              "device to retry.\n");
      // enable the following code if you want rplidar to be reboot by software
      // drv->reset();
      return false;
    } else {
      return true;
    }

  } else {
    fprintf(stderr, "Error, cannot retrieve the lidar health code: %x\n",
            op_result);
    return false;
  }
}

int main(int argc, char* argv[])
{
    const char* opt_com_path = NULL;
    _u32 baudrateArray[2] = {115200, 256000};
    _u32 opt_com_baudrate = 0;
    u_result op_result;
    opt_com_path = "/dev/ttyUSB0";

    RPlidarDriver* drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
    if (!drv) {
        fprintf(stderr, "insufficent memory, exit\n");
        exit(-2);
    }

    rplidar_response_device_info_t devinfo;
    bool connectSuccess = false;
    size_t baudRateArraySize =
      (sizeof(baudrateArray)) / (sizeof(baudrateArray[0]));

    for (size_t i = 0; i < baudRateArraySize; ++i) {
        if (!drv) drv = RPlidarDriver::CreateDriver(DRIVER_TYPE_SERIALPORT);
        if (IS_OK(drv->connect(opt_com_path, baudrateArray[i]))) {
          op_result = drv->getDeviceInfo(devinfo);
          if (IS_OK(op_result)) {
            connectSuccess = true;
            break;
          } else {
            delete drv;
            drv = NULL;
          }
        }
    }
    
    if (!checkRPLIDARHealth(drv)) {
        fprintf(stderr, "Health failed\n");
    }

    if (!connectSuccess) {
        fprintf(stderr, "Error, cannot bind to the specified serial port %s.\n", opt_com_path);
    }
    PmLogInfo(getPmLogContext(), "SERVICE_MAIN", 0, "start com.webos.service.gesturecontrol");

    LSError lserror;
    LSErrorInit(&lserror);

    GMainLoop* mainLoop = g_main_loop_new(nullptr, false);
    LSHandle *m_handle = nullptr;

    if(!LSRegister("com.webos.service.gesturecontrol", &m_handle, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to register to luna-bus");
        LSErrorPrint(&lserror, stdout);

        return false;
    }

    if (!LSRegisterCategory(m_handle, "/", serviceMethods, NULL, NULL, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to register category and method");
        LSErrorPrint(&lserror, stdout);

        return false;
    }

    if(!LSGmainAttach(m_handle, mainLoop, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to attach service");
        LSErrorPrint(&lserror, stdout);

        return false;
    }

    if (!LSCall(m_handle,
                "luna://com.webos.service.systemservice/clock/getTime",
                "{}",
                cbGetTime,
                NULL,
                NULL,
                &lserror))
    {
        PmLogError(getPmLogContext(), "LSCALL_GETTIME", 0, "Cannot call getTime");
        LSErrorPrint(&lserror, stderr);
    }

    g_main_loop_run(mainLoop);

    if(!LSUnregister(m_handle, &lserror))
    {
        PmLogError(getPmLogContext(), "LS_REGISTER", 0, "Unable to unregister service");
        LSErrorPrint(&lserror, stdout);

        return false;
    }

    g_main_loop_unref(mainLoop);
    mainLoop = nullptr;

    return 0;
}

