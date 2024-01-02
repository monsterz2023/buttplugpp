#include "../src/ButtplugClient.hpp"

#include <iostream>
#include <boost/asio.hpp>

int main(){
    boost::asio::io_context ioc;
    auto bc = std::make_shared<ButtplugClient>(ioc, "localhost","12345");

    bc -> connect([bc](){
        bc -> start_scanning([bc](){
            bc -> request_device_list([](std::vector<Device> devices){
                std::for_each(
                    devices.begin(), devices.end(), 
                    [](const Device &device){
                        std::cout << device.device_name << std::endl;
                        std::for_each(
                            device.device_messages.begin(), device.device_messages.end(),
                            [](const std::pair<std::string, DeviceMessage>& pair) {
                                auto msg = pair.second;                                
                                std::cout << pair.first << "||" << msg.key << std::endl;
                                std::for_each(
                                    msg.message_attributes.begin(), msg.message_attributes.end(),
                                    [](const MessageAttribute& attr){
                                        std::cout << attr.name << ":" << attr.attribute_value << std::endl;
                                    });
                            });
                    });
            });
        });
    });
    
    ioc.run();
    return EXIT_SUCCESS;
}