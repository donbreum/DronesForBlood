#!/usr/bin/env python3
"""
GCS top level node.

It communicates with the other nodes (mavlink, UI, and path planner)
through ROS topics. Hence asynchronous communication.

Implements a Finite-State-Machine (FSM) for determining and evolving the
state of the drone controlling system.

In order to ensure the correct working of the FSM, there must be mutexes
placed in order to avoid variable changes while evolving the states.
"""
# Standard libraries
# Third-party libraries
import rospy
import geometry_msgs.msg
import std_msgs.msg
import pymap3d as pm
# Local libraries
from gcs_master import drone_fsm
try:
    import mavlink_lora.msg
except ModuleNotFoundError:
    print("Mavlink module not found")


class GcsMasterNode():

    # Node variables
    HEARBEAT_PERIOD = 0.5       # Seconds
    HEARTBEAT_TIMEOUT = 1.5     # Seconds

    def __init__(self):
        # Subscribers configuration
        rospy.Subscriber("mavlink_drone_heartbeat",
                         mavlink_lora.msg.mavlink_lora_heartbeat,
                         self.heartbeat_callback, queue_size=1)
        rospy.Subscriber('dronelink/start', std_msgs.msg.Bool,
                         self.ui_start_callback, queue_size=1)

        # Publishers configuration
        self.heartbeat_pub = rospy.Publisher(
                "mavlink_heartbeat",
                mavlink_lora.msg.mavlink_lora_heartbeat,
                queue_size=1)
        self.drone_arm_pub = rospy.Publisher(
                "mavlink_interface/command/arm_disarm",
                std_msgs.msg.Bool,
                queue_size=1)
        self.drone_takeoff_pub = rospy.Publisher(
                "mavlink_interface/command/takeoff",
                mavlink_lora.msg.mavlink_lora_command_takeoff,
                queue_size=1)
        self.calc_path_pub = rospy.Publisher(
                "gcs_master/calculate_path",
                std_msgs.msg.Bool,
                queue_size=1)

        # Create an instance of the drone finite-state-machine class.
        self.state_machine = drone_fsm.DroneFSM()

        # Timestamps variables. Sending time set to zero for forcing the sending
        # of a heartbeat in the first iteration.
        self.heartbeat_send_time = 0.0
        self.heartbeat_receive_time = rospy.get_time()
        # hearbeat flag. Starts true so the heartbeat is sent for the first time
        self.heartbeat_ok = False

    def update_flags(self):

        if self.state_machine.ARM:
            self.drone_arm_pub.publish(True)
            self.state_machine.ARM = False

        if self.state_machine.TAKE_OFF:
            #TODO: Set the parameters to adequate values
            msg = mavlink_lora.msg.mavlink_lora_command_takeoff()
            msg.lattitude = None
            msg.longtitude = None
            msg.altitude = 50
            msg.yaw_angle = float('NaN') # unchanged angle
            msg.pitch = 0
            self.drone_takeoff_pub.publish(msg)
            self.state_machine.TAKE_OFF = False

        if self.state_machine.CALCULATE_PATH:
            self.calc_path_pub.publish(True)
            self.state_machine.CALCULATE_PATH = False

        if self.state_machine.FLY:
            pass

        if self.state_machine.WAYPOINT_REACHED:
            pass

        if self.state_machine.LAND:
            pass

        if self.state_machine.EMERGENCY_LANDING:
            print("shits fucked")

    def heartbeat_callback(self, data):
        self.heartbeat_ok = True
        self.heartbeat_receive_time = rospy.get_time()

    def ui_start_callback(self, data):
        self.state_machine.new_operation = True
        return

    def ui_callback(self, data):
        self.state_machine.destination = [data.x, data.y]
        return

    def dronelink_callback(self, data):
        self.state_machine.batt_ok = True
        self.state_machine.comm_ok = True
        if data._connection_header["topic"] == "/mavlink/drone/error":
            # Differentiate between batt and comms error here.
            self.state_machine.batt_ok = False
            self.state_machine.comm_ok = False

        elif data._connection_header["topic"] == "/mavlink/drone/ack":
            if data[2] == 1:
                self.state_machine.ack = True
            else:
                self.state_machine.ack = False

        elif data._connection_header["topic"] == "/mavlink/drone/position":
            # data[0] = header
            # data[1] = time_usec
            self.state_machine.position = [data[2], data[3]]
            self.state_machine.altitude = data[4]
            # data[5] = relative_altitude
            # data[6] = heading
        return

    def planner_callback(self, data):
        self.state_machine.waypoint = [data[0][0], data[0][1]]
        self.state_machine.new_waypoint = True
        return

    def send_heartbeat(self):
        """
        Broadcast a heartbeat containing the GCS basic information.
        """
        msg = mavlink_lora.msg.mavlink_lora_heartbeat()
        msg.type = 6
        msg.autopilot = 8
        msg.base_mode = 192
        msg.custom_mode = 0
        msg.system_status = 4
        msg.system_id = 255
        self.heartbeat_pub.publish(msg)
        self.heartbeat_send_time = rospy.get_time()
        print("BIP")
        return

    def run(self):
        """ 
        Main loop. Update the FSM and publish variables.
        """
        rate = rospy.Rate(100)
        while not rospy.is_shutdown():
            now = rospy.get_time()
            # Update the state of the FSM.
            self.state_machine.update_state()
            self.state_machine.update_outputs()
            # Publish the flags of the FSM.
            self.update_flags()
            # Publish the heartbeat with the adequate rate
            if now > self.heartbeat_send_time + self.HEARBEAT_PERIOD:
                self.send_heartbeat()
            # Reset the received heartbeat. If false, dronelink is lost.
            if now > self.heartbeat_receive_time + self.HEARTBEAT_TIMEOUT:
                if self.heartbeat_ok:
                    self.heartbeat_ok = False
                else:
                    print("Dronelink lost")
                    self.heartbeat_receive_time = rospy.get_time()
            # Finish the loop cycle.
            rate.sleep()
        return


def main():
    # Instantiate the gcs_master node class and run it
    gcs_master = GcsMasterNode()
    gcs_master.run()
    return
