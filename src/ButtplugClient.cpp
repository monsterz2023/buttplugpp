
#include <iostream>
#include <nlohmann/json.hpp>
#include "ButtplugClient.hpp"
#include "Util.hpp"

ButtplugClient::ButtplugClient(asio::io_context& ioc, std::string host, std::string port):resolver_(ioc),ws_(ioc),host_(std::move(host)),port_(std::move(port)){}
ButtplugClient::~ButtplugClient(){
    std::cout << "closing" << std::endl;
    close();
}

void ButtplugClient::close() {
    ws_.async_close(websocket::close_code::none, [](boost::system::error_code ec){
        std::cout << "socket closed" << std::endl;
    });
}

void ButtplugClient::connect(Callback connected_cb){
    resolver_.async_resolve(
        host_, port_,
        [this,connected_cb](boost::system::error_code ec, tcp::resolver::results_type results){
            if(!ec){
                on_connected_ = connected_cb;
                do_connect(results);
            } else {
                std::cerr << "resolve failed:" << ec.message() << std::endl;
            }
        }
    );
}

void ButtplugClient::start_scanning(Callback cb){
    auto id = Util::random_message_id();
    json req = {
        {"StartScanning", {{"Id", id}}}
    };

    callbacks_[id]=[this, id, cb](const json& json){
        cb();
        callbacks_.erase(id);
    };

    send_request(req);
}

void ButtplugClient::request_device_list(std::function<void(std::vector<Device>)> cb){
    auto id = Util::random_message_id();
    json req = {
        {"RequestDeviceList", {{"Id", id}}}
    };

    callbacks_[id]=[this, id, cb](const json& json){
        callbacks_.erase(id);

        std::vector<Device> devices;
        for (auto &ele: json) {
            for(auto &d: ele["DeviceList"]["Devices"]) {
                devices.push_back(Device(d));
            }
        }
        cb(devices);
    };

    send_request(req);
}

void ButtplugClient::send_linear_cmd(std::uint32_t device_idx, std::uint32_t position, std::uint32_t duration){
    auto id = Util::random_message_id();
    auto p = Util::clamp<std::uint32_t>(position, 0, 100);
    json vectors=json::array({});
    json v = {
        {"Index", 0},
        {"Duration", duration},
        {"Position", p / 100.0}
    };
    vectors.push_back(v);

    json msg_body = {
        {"Id", id},
        {"DeviceIndex", device_idx},
        {"Vectors", vectors}
    };
    json req = {
        {"LinearCmd", msg_body}
    };

    send_request(req);
}

void ButtplugClient::do_connect(tcp::resolver::results_type& results){
    asio::async_connect(
        ws_.next_layer(), results.begin(), results.end(), 
        [this](boost::system::error_code ec, tcp::resolver::iterator){
        if(!ec){
            do_handshake();
        } else {
            std::cerr << "connect failed:" << ec.message() << std::endl;
        }
    });
}

void ButtplugClient::register_client(){
    auto id = Util::random_message_id();
    json message_body = {
        {"Id", id},
        {"ClientName", "Buttplug C++ Client"},
        {"MessageVersion", 3}
    };
    json req = {
        {"RequestServerInfo", message_body}
    };

    callbacks_[id]=[this, id](const json& json){
        callbacks_.erase(id);

        if(on_connected_) {
            on_connected_();
        }
    };
    send_request(req);
}

void ButtplugClient::do_handshake(){
    ws_.async_handshake(
        host_, "/",
        [this](boost::system::error_code ec) {
            if (!ec) {
                register_client();
                do_read();
            } else {
                std::cerr << "Handshake failed: " << ec.message() << std::endl;
            }
        });
}

void ButtplugClient::do_read(){
    ws_.async_read(
        buffer_,
        [this](boost::system::error_code ec, std::size_t) {
            if (!ec) {
                auto data=boost::beast::buffers_to_string(buffer_.data());
                try {
                    auto json_resp = json::parse(data);
                    std::cout << "<<" << json_resp.dump() << std::endl;
                    distribute_callback(json_resp);
                } catch(json::parse_error &e) {
                    std::cerr << "Invalid json: " << e.what() << std::endl;
                }

                buffer_.consume(buffer_.size());
                do_read();
            } else {
                std::cerr << "read error: " << ec.message() << std::endl;
            }
        });
}


void ButtplugClient::send_request(const json &j) {
    // Create a JSON array and add the object to it
    json request = json::array();
    request.push_back(j);

    bool is_writing = !request_queue_.empty();

    std::cout << ">>" << request.dump() << std::endl;
    request_queue_.push(request.dump());
    if(!is_writing) {
        do_write();
    }
}

void ButtplugClient::do_write(){
    auto request = request_queue_.front();
    ws_.async_write(
        asio::buffer(request),
        [this](boost::system::error_code ec, std::size_t){
            if(ec){
               std::cerr << "write error: " << ec.message() << std::endl;
            } else{
                request_queue_.pop();
                if(!request_queue_.empty()) {
                    do_write();
                }
            }
        });
}

void ButtplugClient::distribute_callback(const json&j){
    for(auto &ele : j) {
        for (auto &kv: ele.items()) {
            if(kv.value().contains("Id")) {
                int id = kv.value()["Id"];
                auto cb=callbacks_.find(id);
                if(cb != callbacks_.end()) {
                    (cb -> second)(j);
                }
            }
        }
    }
}
