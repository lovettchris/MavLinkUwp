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
        std::vector<std::wstring> findSerialDevices();
    };
}