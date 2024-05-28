//
// (c) 2024 Copyright, Real-Time Innovations, Inc.  All rights reserved.
//
//  RTI grants Licensee a license to use, modify, compile, and create derivative
//  works of the Software.  Licensee has the right to distribute object form
//  only for use with RTI products.  The Software is provided "as is", with no
//  warranty of any type, including any warranty for fitness for any purpose.
//  RTI is under no obligation to maintain or support the Software.  RTI shall
//  not be liable for any incidental or consequential damages arising out of the
//  use or inability to use the software.
//

#include <thread>
#include "rti/rti.hpp"
#include "home_automation.hpp"

class WindowSensor {
  public:
    WindowSensor(const std::string& sensor_name, const std::string& room_name) :
            status_(sensor_name, room_name, false),
            instance_(dds::core::InstanceHandle::nil()),
            writer_(nullptr)
    {
        dds::domain::DomainParticipant participant(0);
        dds::topic::Topic<DeviceStatus> topic(participant, "WindowStatus");
        writer_ = std::make_shared<dds::pub::DataWriter<DeviceStatus>>(
                dds::pub::DataWriter<DeviceStatus>(topic));

        instance_ = writer_->register_instance(status_);
        writer_->write(status_, instance_);
    }

    void open_window()
    {
        std::cout << "Opening the window..." << std::endl;
        status_.is_open(true);
        writer_->write(status_, instance_);
    }

    void close_window()
    {
        std::cout << "Closing the window..." << std::endl;
        status_.is_open(false);
        writer_->write(status_, instance_);
    }

    void turn_off()
    {
        std::cout << "Turning off the sensor..." << std::endl;
        writer_->dispose_instance(instance_);
    }

    void turn_on()
    {
        std::cout << "Turning on the sensor..." << std::endl;
        writer_->write(status_, instance_);
    }

  private:
    DeviceStatus status_;
    dds::core::InstanceHandle instance_;
    std::shared_ptr<dds::pub::DataWriter<DeviceStatus>> writer_;

};

void publish_sensor(
        const std::string& sensor_name,
        const std::string& room_name)
{

    WindowSensor window_sendor(sensor_name, room_name);
    std::string option;

    while (true) {
        std::cout << "\nEnter one of the following options:" << std::endl
            << "\t- Open" << std::endl
            << "\t- Close" << std::endl
            << "\t- Off" << std::endl
            << "\t- On" << std::endl
            << "\t- Exit" << std::endl;

        std::cin >> option;

        if (option == "Open") {
            window_sendor.open_window();
        } else if (option == "Close") {
            window_sendor.close_window();
        } else if (option == "Off") {
            window_sendor.turn_off();
        } else if (option == "On") {
            window_sendor.turn_on();
        } else if (option == "Exit") {
            break;
        } else {
            std::cout << "Invalid option, please enter a valid option [Open|Close|Off|On|Exit]." << std::endl;
        }
    }
}

int main(int argc, char **argv)
{
    std::string sensor_name = (argc > 1) ? argv[1] : "Sensor1";
    std::string room_name = (argc > 2) ? argv[2] : "LivingRoom";
    publish_sensor(sensor_name, room_name);
}
