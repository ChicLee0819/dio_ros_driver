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

class DoutBuzzer : public rclcpp::Node
{
public:
DoutBuzzer()
  : Node("dout_controler_node"),
  access_frequency_(declare_parameter<int64_t>("access_frequency", 10)),
  din_topic_str_(declare_parameter<std::string>("din_topic", "")),
  chipname_str_(declare_parameter<std::string>("chipname", "")),
  line_num_(declare_parameter<int64_t>("line_num", 6))
  {
    int ret = 0;
    if (din_topic_str_.length())
    {
      subscription_ = this->create_subscription<dio_ros_driver::msg::DIOPort>(din_topic_str_, 
        access_frequency_, std::bind(&DoutBuzzer::topic_callback, this, std::placeholders::_1));
    }
    else
    {
      subscription_ = nullptr;
    }

    buzzer_chip = nullptr;
    buzzer_line = nullptr;

    buzzer_chip = gpiod_chip_open_by_name(chipname_str_.c_str());
    if (!buzzer_chip) {
      RCLCPP_ERROR(this->get_logger(), "Open chip failed");
    }
    else
    {
      buzzer_line = gpiod_chip_get_line(buzzer_chip, line_num_);
      if (!buzzer_line) {
        RCLCPP_ERROR(this->get_logger(), "Get line failed\n");
      }
      else
      {
        ret = gpiod_line_request_output(buzzer_line, "buzzer_node", 0);
        if (ret < 0) {
          RCLCPP_ERROR(this->get_logger(), "Request line as output failed\n");
        }
      }
    }
  }

private:
  void topic_callback(const dio_ros_driver::msg::DIOPort::SharedPtr msg) const
  {
    RCLCPP_DEBUG(this->get_logger(), "Got Subscription '%s': '%s'", din_topic_str_.c_str(), msg->value? "True":"False");
    if(!msg->value)
    {
      this->buzzerBee();
    }
  }

  void buzzerBee() const
  {
    int ret = 0;
    if (this->buzzer_chip && this->buzzer_line)
    {
      ret = this->pulse(this->buzzer_line, 600, 100);
      if (ret < 0) RCLCPP_ERROR(this->get_logger(), "buzzer pulse failed");
      ret = this->pulse(this->buzzer_line, 700, 100);
      if (ret < 0) RCLCPP_ERROR(this->get_logger(), "buzzer pulse failed");
      ret = this->pulse(this->buzzer_line, 500, 100);
      if (ret < 0) RCLCPP_ERROR(this->get_logger(), "buzzer pulse failed");
    }
  }

/**
  _______         _______
  |      |       |
__|      |_______|

   <---->
    frequency

len: ms

 */
  int pulse(struct gpiod_line *line, int frequency, int len) const
  {
    int ret = 0;
    int loop = 0;
    if (len == 0) loop = 1;
    else loop = 1000 * len / frequency;
    for(int i = 0; i < loop; i++)
    {
      ret = gpiod_line_set_value(line, 0);
      if (ret < 0) {
        perror("Set line output failed\n");
        return -1;
      }
      usleep(frequency);
      ret = gpiod_line_set_value(line, 1);
      if (ret < 0) {
        perror("Set line output failed\n");
        return -1;
      }
      usleep(frequency);
    }
    return 0;
  }

  rclcpp::Subscription<dio_ros_driver::msg::DIOPort>::SharedPtr subscription_;
  uint32_t access_frequency_;
  uint32_t line_num_;
  std::string din_topic_str_;
  std::string chipname_str_;

  struct gpiod_chip *buzzer_chip;
  struct gpiod_line *buzzer_line;
};

int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<DoutBuzzer>());
  rclcpp::shutdown();
  return 0;
}