#pragma once

#include "Port.h"
#include "Semaphore.hpp"
#include <memory>

namespace MavLinkUwp
{
    class __declspec(dllexport) UwpSerialPort : public Port
    {
        Windows::Devices::SerialCommunication::SerialDevice^ _device;
        Windows::Storage::Streams::DataWriter^ _writer;
        Windows::Storage::Streams::DataReader^ _reader;

        mavlink_utils::Semaphore dataReceived;

    public:
        void connect(Platform::String^ deviceId);
        void connect(Windows::Storage::Streams::DataWriter^ w, Windows::Storage::Streams::DataReader^ r);
        int write(const uint8_t* ptr, int count);
        int read(uint8_t* buffer, int bytesToRead);
        void close();
        bool isClosed();
        int getRssi(const char*)
        {
            return 0;
        }
    };

    ref class Pixhawk {
    public:
        Windows::Foundation::IAsyncOperation<Windows::Foundation::Collections::IIterable<Platform::String^>^>^ findSerialDevices();
    };
}