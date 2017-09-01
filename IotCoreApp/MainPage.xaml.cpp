//
// MainPage.xaml.cpp
// Implementation of the MainPage class.
//

#include "pch.h"
#include "MainPage.xaml.h"
#include "UwpSerialPort.h"
#include "ppltasks.h"
#include "collection.h"
#include "StringUtils.h"
#include <string>

using namespace concurrency;
using namespace IotCoreApp;
using namespace MavLinkUwp;
using namespace Platform;
using namespace Windows::Foundation;
using namespace Windows::Foundation::Collections;
using namespace Windows::UI::Xaml;
using namespace Windows::UI::Xaml::Controls;
using namespace Windows::UI::Xaml::Controls::Primitives;
using namespace Windows::UI::Xaml::Data;
using namespace Windows::UI::Xaml::Input;
using namespace Windows::UI::Xaml::Media;
using namespace Windows::UI::Xaml::Navigation;
using namespace mavlinkcom;

// The Blank Page item template is documented at https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x409

MainPage::MainPage()
{
    InitializeComponent();
}

void MainPage::OnNavigatedTo(NavigationEventArgs^ e) 
{
    try {
        MavLinkUwp::Pixhawk^ pixhawk = ref new MavLinkUwp::Pixhawk();
        auto findOp = pixhawk->findSerialDevices();
        auto task = create_task(findOp).then([this](IIterator<Platform::String^>^ deviceIdList)
        {
            for (auto iter = deviceIdList; iter->HasCurrent; iter->MoveNext())
            {
                Platform::String^ id = iter->Current;
                auto port = std::make_shared<UwpSerialPort>();
                if (port->connect(id)) {
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
                        std::wstring wide(msg.begin(), msg.end());

                        this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, wide]() {
                            this->StatusText->Text = ref new Platform::String(wide.c_str());
                        }));
                    });
                }
            }
        });
    }
    catch (Platform::Exception^ e) 
    {
        this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, e]() {
            this->StatusText->Text = e->Message;
        }));
    }
    catch (std::exception& e)
    {
        std::string msg(e.what());
        this->Dispatcher->RunAsync(Windows::UI::Core::CoreDispatcherPriority::Normal, ref new Windows::UI::Core::DispatchedHandler([this, msg]() {
            this->StatusText->Text = ref new Platform::String(std::wstring(msg.begin(), msg.end()).c_str());
        }));
    }
}
