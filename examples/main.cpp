#include "../src/ButtplugClient.hpp"
#include "../src/Util.hpp"
#include <boost/asio.hpp>
#include <iostream>

int main() {
  boost::asio::io_context ioc;
  auto bc = std::make_shared<ButtplugClient>(ioc, "localhost", "12345");
  std::cout << "Hello" << std::endl;
  bc->connect([bc]() {
    bc->start_scanning([bc]() {
      bc->send_linear_cmd(0, Util::random_position(), 500);
      bc->request_device_list([bc](std::vector<Device> devices) {
        std::for_each(devices.begin(), devices.end(), [](const Device &device) {
          std::cout << device.device_name << std::endl;
          std::for_each(device.device_messages.begin(),
                        device.device_messages.end(),
                        [](const std::pair<std::string, DeviceMessage> &pair) {
                          auto msg = pair.second;
                          std::cout << msg.key << std::endl;
                          std::for_each(msg.message_attributes.begin(),
                                        msg.message_attributes.end(),
                                        [](const MessageAttribute &attr) {
                                          std::cout << attr.name << ": "
                                                    << attr.attribute_value
                                                    << std::endl;
                                        });
                        });
        });
      });
    });
  });

  ioc.run();
  return EXIT_SUCCESS;
}
