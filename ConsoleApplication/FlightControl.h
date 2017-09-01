#pragma once
#include "MavLinkVehicle.hpp"
#include <memory>

class FlightControl
{
public:
    FlightControl();
    ~FlightControl();
    void Run();
private:
    std::shared_ptr<mavlinkcom::MavLinkVehicle> _vehicle;
};

