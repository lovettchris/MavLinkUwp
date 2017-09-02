#pragma once
#include <vector>
#include <string>

namespace MavLinkUwp
{
    class Pixhawk 
    {
        static std::wstring toWideString(std::string s) {
            return std::wstring(s.begin(), s.end());
        }
    public:
        std::wstring Id;
        std::wstring Name;
        int baudRate;

        static std::vector<Pixhawk> findSerialDevices();
    };
}