#ifndef BUTTPLUG_CLIENT_HPP
#define BUTTPLUG_CLIENT_HPP
#include <boost/beast.hpp>
#include <boost/asio.hpp>
#include <nlohmann/json.hpp>
#include <iostream>

using tcp=boost::asio::ip::tcp;
namespace asio=boost::asio;
namespace beast=boost::beast;
namespace websocket=boost::beast::websocket;
using json=nlohmann::json;
using Callback=std::function<void()>;
using JsonCallback=std::function<void(const json& body)>;

struct MessageAttribute {
    std::string name;
    std::string attribute_value;
};

struct DeviceMessage {
    std::string key;
    std::vector<MessageAttribute> message_attributes;
};

struct Device {
    std::string device_name;
    std::uint32_t device_index;
    std::string device_display_name;
    std::map<std::string, DeviceMessage> device_messages;

    Device(const json& j) {
        device_name = j["DeviceName"];
        device_index = j["DeviceIndex"];
        device_display_name = j.value("DeviceDisplayName","");

        for(auto &dm: j["DeviceMessages"].items()){
            for(auto &attr: dm.value().items()) {
                DeviceMessage device_msg;
                device_msg.key = dm.key();
                for (auto&[k,v]: attr.value().items()) {
                    MessageAttribute msg_attr;
                    msg_attr.name = k;
                    msg_attr.attribute_value = v.dump();
                    device_msg.message_attributes.push_back(msg_attr);
                }
                device_messages[dm.key()] = device_msg;
            }
        }
    }
};

class ButtplugClient {
    public:
    ButtplugClient(asio::io_context& ioc, std::string host, std::string port);
    ~ButtplugClient();
    void connect(Callback cb);
    void start_scanning(Callback cb);
    void request_device_list(std::function<void(std::vector<Device>)> cb);
    void close();

    private:
    tcp::resolver resolver_;
    websocket::stream<tcp::socket> ws_;
    beast::multi_buffer buffer_;
    std::string host_;
    std::string port_;
    Callback on_connected_;
    std::map<uint32_t, JsonCallback> callbacks_;

    void register_client();
    void do_connect(tcp::resolver::results_type& results);
    void do_handshake();
    void do_read();
    void do_write(const std::string& data);
    void send_request(const json &j);
    void distribute_callback(const json&j);
};

#endif