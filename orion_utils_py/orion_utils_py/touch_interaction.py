#!/usr/bin/env python3
"""Touch-sensor interaction node for ORION robot.

Maps the four capacitive touch sensors on ESP32 #2 to robot behaviors
combining arm gestures, emotion display, and spoken responses.

Sensor → Behavior
  touch_ur (upper-right) → Tickle / surprise reaction
  touch_ul (upper-left)  → Greeting wave
  touch_lr (lower-right) → Thoughtful pose
  touch_ll (lower-left)  → Sad / tired reaction
"""

import threading
import time

import rclpy
from rclpy.node import Node
from std_msgs.msg import Bool, Float64MultiArray, Int32, String

# Arm joint position constants (radians) — joint limits are ±π/3 ≈ ±1.047
_NEUTRAL =  0.0
_UP      =  1.0
_HALF    =  0.5
_DOWN    = -0.5

# Emotion index restored after each action (neutral/default face)
_EMOTION_DEFAULT = 3


class TouchInteractionNode(Node):
    """React to rising-edge touch events with arm, emotion, and TTS outputs."""

    def __init__(self):
        super().__init__('touch_interaction_node')

        self._left_pub    = self.create_publisher(
            Float64MultiArray, '/simple_left_arm_controller/commands',  10)
        self._right_pub   = self.create_publisher(
            Float64MultiArray, '/simple_right_arm_controller/commands', 10)
        self._emotion_pub = self.create_publisher(Int32,  '/emotion/int',    10)
        self._tts_pub     = self.create_publisher(String, '/orion_response', 10)

        self._prev = {s: False for s in ('ur', 'ul', 'lr', 'll')}
        for sensor, topic in [
            ('ur', '/interaction/touch_ur'),
            ('ul', '/interaction/touch_ul'),
            ('lr', '/interaction/touch_lr'),
            ('ll', '/interaction/touch_ll'),
        ]:
            self.create_subscription(
                Bool, topic,
                lambda msg, s=sensor: self._on_touch(msg, s),
                10,
            )

        self._busy = False
        self._lock = threading.Lock()

        self.get_logger().info('Touch interaction node ready.')

    # ------------------------------------------------------------------
    # Low-level publishers
    # ------------------------------------------------------------------

    def _arms(self, left: float, right: float) -> None:
        l = Float64MultiArray(); l.data = [left]
        r = Float64MultiArray(); r.data = [right]
        self._left_pub.publish(l)
        self._right_pub.publish(r)

    def _emotion(self, idx: int) -> None:
        msg = Int32(); msg.data = idx
        self._emotion_pub.publish(msg)

    def _say(self, text: str) -> None:
        msg = String(); msg.data = text
        self._tts_pub.publish(msg)

    # ------------------------------------------------------------------
    # Rising-edge detection and dispatch
    # ------------------------------------------------------------------

    def _on_touch(self, msg: Bool, sensor: str) -> None:
        if not msg.data:
            self._prev[sensor] = False
            return
        if self._prev[sensor]:
            return  # still held — ignore until released
        self._prev[sensor] = True

        with self._lock:
            if self._busy:
                return
            self._busy = True

        threading.Thread(
            target=self._run, args=(sensor,), daemon=True
        ).start()

    def _run(self, sensor: str) -> None:
        try:
            {'ur': self._tickle,
             'ul': self._greet,
             'lr': self._think,
             'll': self._sad}[sensor]()
        finally:
            with self._lock:
                self._busy = False

    # ------------------------------------------------------------------
    # Behaviors
    # ------------------------------------------------------------------

    def _tickle(self) -> None:
        """Upper-right: tickle / surprise — both arms flail twice then reset."""
        self._emotion(4)
        self._say('¡Me haces cosquillas!')
        for _ in range(2):
            self._arms(_UP, _UP)
            time.sleep(0.35)
            self._arms(_DOWN, _DOWN)
            time.sleep(0.25)
        self._arms(_NEUTRAL, _NEUTRAL)
        time.sleep(0.5)
        self._emotion(_EMOTION_DEFAULT)

    def _greet(self) -> None:
        """Upper-left: greeting — left arm waves twice, happy emotion."""
        self._emotion(1)
        self._say('¡Hola! ¿Cómo estás?')
        for _ in range(2):
            self._arms(_UP,   _NEUTRAL)
            time.sleep(0.45)
            self._arms(_HALF, _NEUTRAL)
            time.sleep(0.35)
        self._arms(_NEUTRAL, _NEUTRAL)
        time.sleep(0.4)
        self._emotion(_EMOTION_DEFAULT)

    def _think(self) -> None:
        """Lower-right: thoughtful — right arm rises slowly, neutral emotion."""
        self._emotion(2)
        self._say('Mmm... déjame pensar en eso.')
        self._arms(_NEUTRAL, _HALF)
        time.sleep(0.8)
        self._arms(_NEUTRAL, _UP)
        time.sleep(1.5)
        self._arms(_NEUTRAL, _NEUTRAL)
        time.sleep(0.5)
        self._emotion(_EMOTION_DEFAULT)

    def _sad(self) -> None:
        """Lower-left: sad / tired — both arms droop, sad emotion."""
        self._emotion(6)
        self._say('Eso no me gustó mucho...')
        self._arms(_DOWN, _DOWN)
        time.sleep(2.0)
        self._arms(_NEUTRAL, _NEUTRAL)
        time.sleep(0.5)
        self._emotion(_EMOTION_DEFAULT)


def main(args=None):
    """Start the TouchInteractionNode and spin until shutdown."""
    rclpy.init(args=args)
    node = TouchInteractionNode()
    try:
        rclpy.spin(node)
    except KeyboardInterrupt:
        node.get_logger().info('Shutting down.')
    finally:
        node.destroy_node()
        rclpy.shutdown()


if __name__ == '__main__':
    main()
