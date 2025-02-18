// Copyright 2016 Open Source Robotics Foundation, Inc.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <chrono>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "dio_ros_driver/dio_ros_driver.hpp"
#include "std_msgs/msg/string.hpp"

using namespace std::chrono_literals;

/* This example creates a subclass of Node and uses a fancy C++11 lambda
 * function to shorten the callback syntax, at the expense of making the
 * code somewhat more difficult to understand at first glance. */

class DoutControler : public rclcpp::Node
{
public:
  DoutControler()
  : Node("dout_controler_node"),
  access_frequency_(declare_parameter<int64_t>("access_frequency", 10)),
  din_topic_str_(declare_parameter<std::string>("din_topic", "")),
  do_topic_str_(declare_parameter<std::string>("do_topic", ""))
  {
    if (do_topic_str_.length())
    {
    publisher_ = this->create_publisher<dio_ros_driver::msg::DIOPort>(do_topic_str_, 
      access_frequency_);
    }
    else
    {
      publisher_ = NULL;
    }

    if (din_topic_str_.length())
    {
      subscription_ = this->create_subscription<dio_ros_driver::msg::DIOPort>(din_topic_str_, 
        access_frequency_, std::bind(&DoutControler::topic_callback, this, std::placeholders::_1));
    }
    else
    {
      subscription_ = NULL;
    }
  }

  void setValue(bool _value) const
  {
    auto message = dio_ros_driver::msg::DIOPort();
    message.value = _value;
    RCLCPP_DEBUG(this->get_logger(), "Publishing: 'set %s to %s'", do_topic_str_.c_str(), _value? "True":"False");
    this->publisher_->publish(message);
  }

  void setHigh() const
  {
    auto message = dio_ros_driver::msg::DIOPort();
    message.value = static_cast<bool>(1);
    RCLCPP_DEBUG(this->get_logger(), "Publishing: 'set /dio/gpiochip2/dout0 to True'");
    this->publisher_->publish(message);
  }

  void setLow() const
  {
    auto message = dio_ros_driver::msg::DIOPort();
    message.value = static_cast<bool>(0);
    RCLCPP_DEBUG(this->get_logger(), "Publishing: 'set /dio/gpiochip2/dout0 to Low'");
    this->publisher_->publish(message);
  }

private:
  void topic_callback(const dio_ros_driver::msg::DIOPort::SharedPtr msg) const
  {
    RCLCPP_DEBUG(this->get_logger(), "Got Subscription '%s': '%s'", din_topic_str_.c_str(), msg->value? "True":"False");
    this->setValue(msg->value);
    /*
    if (msg->value)
    {
      this->setHigh();
    }
    else
    {
      this->setLow();
    }
    */
  }
  //rclcpp::TimerBase::SharedPtr timer_;
  rclcpp::Publisher<dio_ros_driver::msg::DIOPort>::SharedPtr publisher_;
  rclcpp::Subscription<dio_ros_driver::msg::DIOPort>::SharedPtr subscription_;
  uint32_t access_frequency_;
  std::string din_topic_str_;
  std::string do_topic_str_;
  //size_t count_;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DoutControler>());
  rclcpp::shutdown();
  return 0;
}