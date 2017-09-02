#include "pch.h"
#include "ppltasks.h"
#include "StringUtils.h"

using namespace concurrency;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Devices::SerialCommunication;

static const int pixhawkVendorId = 9900;   ///< Vendor ID for Pixhawk board (V2 and V1) and PX4 Flow
static const int pixhawkFMUV4ProductId = 18;     ///< Product ID for Pixhawk V2 board
static const int pixhawkFMUV2ProductId = 17;     ///< Product ID for Pixhawk V2 board
static const int pixhawkFMUV2OldBootloaderProductId = 22;     ///< Product ID for Bootloader on older Pixhawk V2 boards
static const int pixhawkFMUV1ProductId = 16;     ///< Product ID for PX4 FMU V1 board
static const int ftdiVendorId = 0x403;
static const int ftdiProductId = 0x6001;
#include "Pixhawk.h"

namespace MavLinkUwp
{

    std::vector<Pixhawk> Pixhawk::findSerialDevices()
    {
        std::vector<Pixhawk> results;
        Platform::String^ selector = SerialDevice::GetDeviceSelector();
        auto findOperation = DeviceInformation::FindAllAsync(selector);
        create_task(findOperation).wait();
        auto devices = findOperation->GetResults();
        std::wstring pattern1 = toWideString(stringf("VID_%04X&PID_%04X", pixhawkVendorId, pixhawkFMUV1ProductId));
        std::wstring pattern2 = toWideString(stringf("VID_%04X&PID_%04X", pixhawkVendorId, pixhawkFMUV2ProductId));
        std::wstring pattern4 = toWideString(stringf("VID_%04X&PID_%04X", pixhawkVendorId, pixhawkFMUV4ProductId));
        std::wstring ftdiPattern = toWideString(stringf("VID_%04X+PID_%04X", ftdiVendorId, ftdiProductId));
        int len = devices->Size;
        for (int i = 0; i < len; ++i)
        {
            DeviceInformation^ info = devices->GetAt(i);
            std::wstring name = info->Name->Data();
            std::wstring id = info->Id->Data();
            printf("Found device: %S\n", id.c_str());

            bool isFTDI = id.find(ftdiPattern) != std::string::npos;
            if (id.find(pattern1) != std::string::npos ||
                id.find(pattern2) != std::string::npos ||
                id.find(pattern4) != std::string::npos ||
                isFTDI)
            {
                Pixhawk p;
                p.Name = name;
                p.Id = id;
                p.baudRate = isFTDI ? 57600 : 460800;
                results.push_back(p);
            }
        }
        return results;
    }


}
