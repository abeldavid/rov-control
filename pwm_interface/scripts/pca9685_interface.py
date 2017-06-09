#!/usr/bin/env python

import Adafruit_PCA9685
import rospy
from vortex_msgs.msg import Pwm

FREQUENCY = rospy.get_param('/pwm/frequency/set')


class Pca9685InterfaceNode(object):
    def __init__(self):
        rospy.init_node('pwm_node')
        self.pca9685 = Adafruit_PCA9685.PCA9685()
        self.pca9685.set_pwm_freq(FREQUENCY)
        self.pca9685.set_all_pwm(0, 0)

        self.sub = rospy.Subscriber('pwm', Pwm, self.callback, queue_size=10)
        rospy.loginfo("Ready for PWM messages")

    def callback(self, msg):
        if len(msg.pins) == len(msg.on) == len(msg.off):
            for i in range(len(msg.pins)):
                self.pca9685.set_pwm(msg.pins[i], msg.on[i], msg.off[i])


if __name__ == '__main__':
    try:
        pwm_node = Pca9685InterfaceNode()
        rospy.spin()
    except rospy.ROSInterruptException:
        pass