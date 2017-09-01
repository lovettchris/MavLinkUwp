#include "pch.h"
#include "UwpSerialPort.h"
#include "ppltasks.h"

using namespace concurrency;
using namespace MavLinkUwp;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Devices::SerialCommunication;
using namespace Windows::Storage::Streams;
using namespace Windows::Storage;
using namespace Windows::Foundation::Collections;

bool UwpSerialPort::connect(Platform::String^ deviceId) 
{
    auto asyncOp = SerialDevice::FromIdAsync(deviceId);
    create_task(asyncOp).then([this](Windows::Devices::SerialCommunication::SerialDevice^ device) 
    {
        if (device != nullptr)
        {
            _device = device;
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
            reader->InputStreamOptions = InputStreamOptions::Partial;

            connect(writer, reader);
        }
    }).wait();
    return _device != nullptr;
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