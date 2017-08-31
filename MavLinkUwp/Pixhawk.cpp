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

namespace MavLinkUwp
{
    public ref class Pixhawk sealed
    {
        static std::wstring toWideString(std::string s) {
            return std::wstring(s.begin(), s.end());
        }
    public:
        Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IIterator<Platform::String^>^>^ findSerialDevices()
        {
            return create_async([=]() -> IIterator<Platform::String^>^
            {
                auto results = ref new Platform::Collections::Vector<Platform::String^>();
                Platform::String^ selector = SerialDevice::GetDeviceSelector();
                auto findOperation = DeviceInformation::FindAllAsync(selector);
                auto deviceEnumTask = create_task(findOperation);
                deviceEnumTask.then([results](DeviceInformationCollection^ devices)
                {
                    std::wstring pattern1 = toWideString(stringf("VID_%04X&PID_%04X", pixhawkVendorId, pixhawkFMUV1ProductId));
                    std::wstring pattern2 = toWideString(stringf("VID_%04X&PID_%04X", pixhawkVendorId, pixhawkFMUV2ProductId));
                    std::wstring pattern4 = toWideString(stringf("VID_%04X&PID_%04X", pixhawkVendorId, pixhawkFMUV4ProductId));
                    int len = devices->Size;
                    for (int i = 0; i < len; ++i)
                    {
                        DeviceInformation^ info = devices->GetAt(i);
                        std::wstring name = info->Name->Data();
                        std::wstring id = info->Id->Data();
                        if (name.find(pattern1) != std::string::npos || id.find(pattern1) != std::string::npos ||
                            name.find(pattern2) != std::string::npos || id.find(pattern2) != std::string::npos ||
                            name.find(pattern4) != std::string::npos || id.find(pattern4) != std::string::npos)
                        {
                            results->Append(info->Id);
                        }
                    }
                }).wait();
                return results->First();
            });
        }
    };

}
