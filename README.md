# MQTT-Controlled Autonomous Obstacle Avoiding Robot using ESP32

An IoT-based autonomous mobile robot powered by dual 18650 batteries, featuring dynamic obstacle avoidance, spatial scanning, and real-time telemetry via MQTT communication.

## Overview

* This project is an intelligent, self-navigating robotic car built using an ESP32 microcontroller mounted on a custom MDF board chassis. 
* The system allows users to remotely control the robot's ignition (ON/OFF) and monitor real-time spatial distances through an MQTT dashboard.
* The robot is driven by two DC motors and stabilized by a front barrel wheel, running on a dual 18650 battery power supply for robust mobility.
* The project seamlessly combines autonomous decision-making, servo-driven ultrasonic scanning, and IoT telemetry into a single platform.

## Features

* **Autonomous Navigation:** Real-time obstacle detection using an ultrasonic sensor.
* **Dynamic Spatial Scanning:** 180-degree environmental scanning using a servo motor.
* **Remote Control:** Master ON/OFF toggle functionality using MQTT commands.
* **Intelligent Pathfinding:** Evaluates left and right clearances to decide the safest turning direction, including a failsafe "escape" maneuver.
* **Real-time Telemetry:** Publishes front, left, and right distance data, alongside the robot's turning decisions, directly to the dashboard.
* **Post-Turn Verification:** Automatically re-checks clearances after executing a turn to prevent collisions.
* **Connectivity:** Reliable Wi-Fi communication using the ESP32 and `broker.emqx.io`.
* **Robust Power:** Operates completely wirelessly using two 18650 rechargeable Li-ion batteries.

## Hardware Components

* ESP32 Development Board
* HC-SR04 Ultrasonic Sensor
* SG90 Servo Motor
* L298N (or similar) DC Motor Driver
* 2x DC Geared Motors with Wheels
* 1x Barrel Wheel (Caster Wheel)
* Custom MDF Board Chassis
* 2x 18650 Rechargeable Li-ion Batteries
* 18650 Battery Holder
* Jumper Wires & Breadboard/PCB

## Working Principle

The ESP32 connects to a local Wi-Fi network and communicates with a public MQTT broker (`broker.emqx.io`). The system operates continuously to:
* Step the motors forward incrementally.
* Measure the distance to the nearest forward obstacle using the ultrasonic sensor.
* Listen for remote ON/OFF override commands over MQTT.

### Obstacle Detection and Scanning
When an obstacle is detected directly ahead (less than 30 cm):
* The robot immediately stops and reverses slightly to create a safe turning radius.
* The servo motor sweeps the ultrasonic sensor to the Left (180°) and then to the Right (0°) to take precise distance measurements.
* The servo re-centers (90°) for forward travel.

### Autonomous Decision Logic
The microcontroller compares the collected left and right distances:
* If the **Left** path has more than 30 cm of clearance and is wider than the right side, it executes a `LEFT` turn.
* If the **Right** path is wider and adequately clear, it executes a `RIGHT` turn.
* If both directions are blocked, it triggers an `ESCAPE` turn to rotate out of the trapped space.

### Post-Turn Failsafe
After deciding and turning, the robot takes one more forward reading. If the path is still blocked, it will forcefully continue rotating in the same direction until the path is completely clear. 

## MQTT Topics

### Published Topics

| Topic | Description |
| :--- | :--- |
| `robot/front` | Distance reading to the obstacle directly ahead |
| `robot/left` | Scanned distance toward the left vector |
| `robot/right` | Scanned distance toward the right vector |
| `robot/decision` | Current navigation state (`LEFT`, `RIGHT`, or `ESCAPE`) |

### Subscribed Topics

| Topic | Description |
| :--- | :--- |
| `robot/control` | Master control string handler for the robot |

## Control Commands

### Master Toggle Control

| Command | Action |
| :--- | :--- |
| `ON` | Enable driving sequence and autonomous loops |
| `OFF` | Safely halt motors and enter standby telemetry mode |

## Software and Technologies Used

* ESP32 Platform
* Arduino IDE
* MQTT Protocol (PubSubClient Library)
* ESP32Servo Library
* Wi-Fi Communication
* Embedded C/C++

## Author

**Developed by Romil Atmaramani** as an IoT-based Autonomous Obstacle Avoiding Robot using ESP32, MQTT communication, environmental scanning, and mobile platform robotics.
