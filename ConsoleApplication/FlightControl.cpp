#include "pch.h"
#include "FlightControl.h"
#include "UwpSerialPort.h"
#include "ppltasks.h"
#include "collection.h"
#include "StringUtils.h"
#include <string>
#include "Pixhawk.h"

using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::SerialCommunication;
using namespace concurrency;
using namespace MavLinkUwp;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace mavlinkcom;


FlightControl::FlightControl()
{
}


FlightControl::~FlightControl()
{
}

void FlightControl::Run() 
{

    try {
        auto deviceIdList = MavLinkUwp::Pixhawk::findSerialDevices();
        for (auto iter = deviceIdList.begin(), end = deviceIdList.end(); iter != end; iter++)
        {
            MavLinkUwp::Pixhawk pixhawk = *iter;
            auto port = std::make_shared<UwpSerialPort>();
            if (port->connect(ref new Platform::String(pixhawk.Id.c_str()), pixhawk.baudRate)) {
                std::shared_ptr<MavLinkConnection> con = MavLinkConnection::connectPort("pixhawk", port);
                _vehicle = std::make_shared<MavLinkVehicle>(1, 166);
                _vehicle->connect(con);
                _vehicle->startHeartbeat();
                _vehicle->waitForHeartbeat().then([this](MavLinkHeartbeat result) {
                    std::string msg = stringf("received heart beat from (%d,%d):\n", (int)result.sysid, (int)result.compid);
                    msg += stringf("    autopilot=0x%X:\n", (int)result.autopilot);
                    msg += stringf("    base_mode=0x%X:\n", (int)result.base_mode);
                    msg += stringf("    custom_mode=0x%X:\n", (int)result.custom_mode);
                    msg += stringf("    mavlink_version=%d:\n", (int)result.mavlink_version);
                    msg += stringf("    system_status=0x%X:\n", (int)result.system_status);
                        
                    printf("%s\n", msg.c_str());
                });
            }
        }
    }
    catch (Platform::Exception^ e)
    {
        printf("%S\n", e->Message->Data());
    }
    catch (std::exception& e)
    {
        printf("%s\n", e.what());

    }
}
