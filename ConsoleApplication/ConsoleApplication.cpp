// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "pch.h"
#include "FlightControl.h"

int main(Platform::Array<Platform::String^>^ args)
{
    FlightControl  control;
    control.Run();

    printf("Press any key to continue...");
    std::string line;
    std::getline(std::cin, line);
}
