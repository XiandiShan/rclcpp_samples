#include <chrono>
#include <cinttypes>
#include <memory>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_lifecycle/lifecycle_node.hpp"
#include "example_interfaces/srv/add_two_ints.hpp"


using namespace std::chrono_literals;

class ClientNode
{
public:
    ClientNode(const std::shared_ptr<rclcpp::Node>& node)
        : node_(node)
    {
        client_cb_group_ = node_->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        client_ = node_->create_client<example_interfaces::srv::AddTwoInts>("/add_two_ints", rmw_qos_profile_services_default, client_cb_group_);
        add_two_ints_finished_ = false;
    }

    void execute()
    {
        // client_cb_group_ = node_->create_callback_group(rclcpp::CallbackGroupType::Reentrant);
        // client_ = node_->create_client<example_interfaces::srv::AddTwoInts>("/add_two_ints", rmw_qos_profile_services_default, client_cb_group_);
        RCLCPP_INFO(node_->get_logger(), "Execute /add_two_ints");
        while(!client_->wait_for_service(std::chrono::seconds(5))) {
            if (!rclcpp::ok()) {
                RCLCPP_ERROR(node_->get_logger(), "Interrupted while waiting for the service. Exiting.");
                return;
            }
        }
        
        auto request = std::make_shared<example_interfaces::srv::AddTwoInts::Request>();
        request->a = 1;
        request->b = 2;

        // [test 1]
        // using ServiceResponseFuture = rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedFuture;
        // auto response_received_callback = [this](ServiceResponseFuture future) {
        //     RCLCPP_INFO(this->node_->get_logger(), "Execute callback.");
        //     auto result = future.get();
        //     RCLCPP_INFO(this->node_->get_logger(), "Result: %ld", result->sum);
        //     add_two_ints_finished_ = true;
        // };
        // add_two_ints_finished_ = false;
        // auto future_result = client_->async_send_request(request, response_received_callback);
        // while(!add_two_ints_finished_)
        // {
        //     rclcpp::sleep_for(1ms);
        // }
        // RCLCPP_INFO(node_->get_logger(), "Finished /add_two_ints.");

        // [test 2]
        auto future = client_->async_send_request(request);
        if (future.wait_for(std::chrono::seconds(3)) != std::future_status::timeout)  {
            const auto &response = future.get();
            RCLCPP_INFO(node_->get_logger(), "Service call succeeded, response: %ld", response->sum);
        } else {
            RCLCPP_ERROR(node_->get_logger(), "Failed to call /add_two_ints");
        }
    }

private:
    std::shared_ptr<rclcpp::Node> node_;
    rclcpp::Client<example_interfaces::srv::AddTwoInts>::SharedPtr client_;
    rclcpp::CallbackGroup::SharedPtr client_cb_group_;
    bool add_two_ints_finished_;
};
