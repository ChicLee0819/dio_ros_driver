# Copyright 2011 Brown University Robotics.
# Copyright 2017 Open Source Robotics Foundation, Inc.
# All rights reserved.
#
# Software License Agreement (BSD License 2.0)
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
#  * Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
#  * Redistributions in binary form must reproduce the above
#    copyright notice, this list of conditions and the following
#    disclaimer in the documentation and/or other materials provided
#    with the distribution.
#  * Neither the name of the Willow Garage nor the names of its
#    contributors may be used to endorse or promote products derived
#    from this software without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
# "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
# LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
# FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
# COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
# INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
# BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
# CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
# LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
# ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.

import sys
import threading
import signal

import geometry_msgs.msg
import rclpy
from rclpy.node import Node
from dio_ros_driver.msg import DIOPort

if sys.platform == 'win32':
    import msvcrt
else:
    import termios
    import tty


msg = """
This node takes keypresses from the keyboard and publishes them
as Twist/TwistStamped messages. It works best with a US keyboard layout.
---------------------------
Moving around:
   u    i    o
   j    k    l
   m    ,    .

For Holonomic mode (strafing), hold down the shift key:
---------------------------
   U    I    O
   J    K    L
   M    <    >

t : up (+z)
b : down (-z)

anything else : stop

q/z : increase/decrease max speeds by 10%
w/x : increase/decrease only linear speed by 10%
e/c : increase/decrease only angular speed by 10%

CTRL-C to quit
"""


class MyTwist(Node):
    def __init__(self):
        super().__init__('MyTwist')
        self.pub = self.create_publisher(geometry_msgs.msg.Twist, 'cmd_vel', 10)
        self.go2stop_sub = self.create_subscription(DIOPort, '/dio/gpiochip1/din1', self.go2stop_callback, 10)
        self.back2stop_sub = self.create_subscription(DIOPort, '/dio/gpiochip1/din0', self.back2stop_callback, 10)
        twist = geometry_msgs.msg.Twist()
        twist.linear.x = 0.0
        twist.linear.y = 0.0
        twist.linear.z = 0.0
        twist.angular.x = 0.0
        twist.angular.y = 0.0
        twist.angular.z = 0.0
        self.pub.publish(twist)
    
    def go2stop_callback(self, msg):
        if msg.value == 0:
            twist_ = geometry_msgs.msg.Twist()
            twist_.linear.x = -0.5
            twist_.linear.y = 0.0
            twist_.linear.z = 0.0
            twist_.angular.x = 0.0
            twist_.angular.y = 0.0
            twist_.angular.z = 0.0
            self.pub.publish(twist_)

    def back2stop_callback(self, msg):
        if msg.value == 0:
            twist_ = geometry_msgs.msg.Twist()
            twist_.linear.x = 0.5
            twist_.linear.y = 0.0
            twist_.linear.z = 0.0
            twist_.angular.x = 0.0
            twist_.angular.y = 0.0
            twist_.angular.z = 0.0
            self.pub.publish(twist_)

    def stop(self):
        twist_ = geometry_msgs.msg.Twist()
        twist_.linear.x = 0.0
        twist_.linear.y = 0.0
        twist_.linear.z = 0.0
        twist_.angular.x = 0.0
        twist_.angular.y = 0.0
        twist_.angular.z = 0.0
        self.pub.publish(twist_)

def signal_handler(signum, frame):
    print('signal_handler: caught signal ' + str(signum))
    global my_Twist
    if signum == signal.SIGINT.value:
        print('twist STOP')
        my_Twist.stop()
        my_Twist.destroy_node()
        rclpy.shutdown()
        sys.exit(1)

rclpy.init(args=sys.argv)
my_Twist = MyTwist()

def main():
    signal.signal(signal.SIGINT, signal_handler)
    global my_Twist
    #my_Twist = MyTwist()
    rclpy.spin(my_Twist)
    my_Twist.destroy_node()
    rclpy.shutdown()

if __name__ == '__main__':
    main()
