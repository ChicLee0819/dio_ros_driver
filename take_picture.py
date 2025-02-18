# Copyright 2023 Intel Corporation. All Rights Reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import rclpy
from rclpy.node import Node
from sensor_msgs.msg import Image as msg_Image
from sensor_msgs.msg import CameraInfo
from std_msgs.msg import String
from dio_ros_driver.msg import DIOPort
from cv_bridge import CvBridge, CvBridgeError
import sys
import os
import numpy as np
import pyrealsense2 as rs2
if (not hasattr(rs2, 'intrinsics')):
    import pyrealsense2.pyrealsense2 as rs2

class ImageListener(Node):
    def __init__(self, image_topic):
        node_name = os.path.basename(sys.argv[0]).split('.')[0]
        super().__init__(node_name)
        self.bridge = CvBridge()
        self.sub = self.create_subscription(msg_Image, image_topic, self.imageCallback, 1)
        #self.sub_take = self.create_subscription(String, 'take_picture', self.takePictureCallback, 1)
        self.sub_take = self.create_subscription(DIOPort, '/dio/gpiochip2/dout3', self.takePictureCallback, 1)
        self.pub = self.create_publisher(msg_Image, '/image', 1)
        self.take_pic = 0

    def imageCallback(self, data):
        try:
            cv_image = self.bridge.imgmsg_to_cv2(data, data.encoding)
            if self.take_pic != 0:
                self.pub.publish(data)
                self.take_pic = self.take_pic - 1
                #sys.stdout.write("bbb")

            #sys.stdout.write("aaa")
            #sys.stdout.flush()

        except CvBridgeError as e:
            print(e)
            return
        except ValueError as e:
            return

    def takePictureCallback(self, msg):
        #self.get_logger().info('Publishing: "%d"' % msg.value)
        if msg.value == 0:
            self.take_pic = 3

def main():
    image_topic = '/camera/camera/color/image_raw'

    print ()
    print ('take_picture.py')
    print ('--------------------')
    print ()
    
    listener = ImageListener(image_topic)
    rclpy.spin(listener)
    listener.destroy_node()
    rclpy.shutdown()    

if __name__ == '__main__':
    rclpy.init(args=sys.argv)
    main()
