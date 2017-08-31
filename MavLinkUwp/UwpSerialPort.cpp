#include "pch.h"
#include "UwpSerialPort.h"
#include "ppltasks.h"

using namespace concurrency;
using namespace MavLinkUwp;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Devices::Enumeration;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;


static const int pixhawkVendorId = 9900;   ///< Vendor ID for Pixhawk board (V2 and V1) and PX4 Flow
static const int pixhawkFMUV4ProductId = 18;     ///< Product ID for Pixhawk V2 board
static const int pixhawkFMUV2ProductId = 17;     ///< Product ID for Pixhawk V2 board
static const int pixhawkFMUV2OldBootloaderProductId = 22;     ///< Product ID for Bootloader on older Pixhawk V2 boards
static const int pixhawkFMUV1ProductId = 16;     ///< Product ID for PX4 FMU V1 board

std::string stringf(const char* format, ...)
{
    va_list args;
    va_start(args, format);

    int size = _vscprintf(format, args) + 1;
    std::unique_ptr<char[]> buf(new char[size]);

#ifndef _MSC_VER
    IGNORE_FORMAT_STRING_ON
        vsnprintf(buf.get(), size, format, args);
    IGNORE_FORMAT_STRING_OFF
#else
    vsnprintf_s(buf.get(), size, _TRUNCATE, format, args);
#endif

    va_end(args);

    return std::string(buf.get());
}


Windows::Foundation::IAsyncOperation<IIterable<Platform::String^>^>^ Pixhawk::findSerialDevices()
{
    return create_async([=]() -> IIterable<Platform::String^>^
    {
        Platform::String^ selector = SerialDevice::GetDeviceSelector();
        auto findOperation = DeviceInformation::FindAllAsync(selector);
        auto deviceEnumTask = create_task(findOperation);
        deviceEnumTask.then([=](DeviceInformationCollection^ devices)
        {
            std::string pattern = stringf("VID_%4X", pixhawkVendorId);
            int len = devices->Size;
            for (int i = 0; i < len; ++i)
            {
                DeviceInformation^ info = devices->GetAt(i);
                std::wstring name = info->Name->Data();
                std::wstring id = info->Id->Data();
                std::wstring toFind(pattern.begin(), pattern.end());
                if (name.find(toFind) != std::string::npos || id.find(toFind) != std::string::npos)
                {
                    SerialDevice^ _device = SerialDevice::FromIdAsync(info->Id)->GetResults();
                    if (_device != nullptr)
                    {
                        _device->BaudRate = 460800;
                        _device->Parity = SerialParity::None;
                        _device->DataBits = 8;
                        _device->StopBits = SerialStopBitCount::One;
                        _device->Handshake = SerialHandshake::None;
                        long long seconds = 10000000; // 100 nanoseconds
                        TimeSpan duration;
                        duration.Duration = 5 * seconds;
                        _device->ReadTimeout = duration;
                        _device->WriteTimeout = duration;
                        //_device.IsRequestToSendEnabled = false;
                        //_device.IsDataTerminalReadyEnabled = false;

                        auto writer = ref new DataWriter(_device->OutputStream);
                        auto reader = ref new DataReader(_device->InputStream);
                        //reader.InputStreamOptions = InputStreamOptions.Partial;

                        connect(writer, reader);
                        return true;
                    }
                }
            }
        }
    }

}

void UwpSerialPort::connect(DataWriter^ w, DataReader^ r)
{
    _writer = w;
    _reader = r;
}

// write to the port, return number of bytes written or -1 if error.
int UwpSerialPort::write(const uint8_t* ptr, int count)
{
    if (count < 0)
    {
        return 0;
    }

    Platform::Array<uint8_t>^ arr = ref new Platform::Array<uint8_t>(const_cast<uint8_t*>(ptr), (unsigned int)count);
    _writer->WriteBytes(arr);

    create_task(_writer->StoreAsync()).wait();

    return count;
}

// read a given number of bytes from the port (blocking until the requested bytes are available).
// return the number of bytes read or -1 if error.
int UwpSerialPort::read(uint8_t* buffer, int bytesToRead)
{
    ZeroMemory(buffer, bytesToRead);

    auto ret = _reader->LoadAsync(bytesToRead);
    create_task(ret).then([&](unsigned int bytes)
    {
        auto bytesFromSerial = Platform::ArrayReference<BYTE>(buffer, bytes);
        _reader->ReadBytes(bytesFromSerial);
        bytesToRead = bytes;

        auto bytesToLog = ref new Platform::Array<BYTE>(bytesFromSerial);

        auto writer = ref new DataWriter();
        writer->WriteBytes(bytesToLog);

    }).wait();

    return bytesToRead;
}

// close the port.
void UwpSerialPort::close()
{
    // ack!
}

bool UwpSerialPort::isClosed()
{
    return false;
}