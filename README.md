![C](https://img.shields.io/badge/C-A8B9CC?style=for-the-badge&logo=c&logoColor=white)
![Zephyr RTOS](https://img.shields.io/badge/Zephyr_RTOS-2D3252?style=for-the-badge&logo=zephyr&logoColor=white)
![Bluetooth LE](https://img.shields.io/badge/Bluetooth_LE-0082FC?style=for-the-badge&logo=bluetooth&logoColor=white)
![nRF Connect SDK](https://img.shields.io/badge/nRF_Connect_SDK-00A9E0?style=for-the-badge&logo=nordicsemiconductor&logoColor=white)

-Samarth Mahesh

# Zephyr BLE Sensor Broadcaster for DHT11

This project demonstrates a Bluetooth Low Energy (BLE) peripheral built with the Zephyr RTOS, running on a Nordic Semiconductor development kit (nRF52840 DK). The device reads temperature and humidity data from a DHT11 sensor and transmits data wirelessly via a custom BLE service.

The main focus is to create a custom GATT service that first advertises, then sends sensor readings through notifications to a phone with nRF mobile connect app.

---

## 1. What is Bluetooth Low Energy (BLE)?

BLE is a wireless protocol optimized for devices requiring low energy consumption and short-range communication. Everyday gadgets such as fitness trackers and IoT sensors use BLE to exchange small pieces of data efficiently while preserving battery life.

###Recently Bit-chat by Jack Dorsey ( Co-founder twitter ) used BLE technology for messaging. This is a revolution since it decentralizes communication.

BLE organizes information using the **Generic Attribute Profile (GATT)**, which breaks down into two main parts:
-   **Services**: Groups of related data. For our project, we define a custom **"MySensor Service"**.
-   **Characteristics**: These are specific data items inside a service, like the current temperature reading. Our characteristic holds the temperature and humidity as a formatted string.

**Notifications** let the sensor send new data automatically to any connected central device. This saves power compared to the polling method
---

<img width="547" height="380" alt="image" src="https://github.com/user-attachments/assets/f09a1aa2-5bfc-4a00-bb33-5cb7b990a971" />

The setup of nRF 52840dk and sensor DHT11.

<img width="919" height="426" alt="image" src="https://github.com/user-attachments/assets/93002dbb-acfe-4c7c-b744-574438e3511c" />

Output seen in the serial terminal

<img width="397" height="868" alt="image" src="https://github.com/user-attachments/assets/d5b1c8ac-5716-4953-9ca9-34749ff09a0d" />

c)	Readings from the client (smartphone)


## 2. Prerequisites

### Hardware
-   A Nordic development board (like the **nRF52840 DK** or **nRF5340 DK**).
-   A **DHT11** temperature and humidity sensor module.
-   Jumper wires for easy connections.
-   A Micro-USB cable for programming and power.
-   Pull-up resistor 

### Software
-   The **nRF Connect SDK**, which bundles Zephyr and all essentials for Nordic chips.
-   **nRF Connect for Mobile** (iOS/Android) or **nRF Connect for Desktop** for scanning, connecting, and testing BLE devices.

---

## 3. How the Code Works

The code is organized into three files for clarity and modular design:

### `main.c` — The Heart of the Application
-   Initializes Bluetooth using `bt_enable()`.
-   Sets up on-board LEDs for connection status feedback with `dk_leds_init()`.
-   Starts advertising so other devices can discover the sensor via `bt_le_adv_start()`.
-   Manages BLE connection events—an LED lights up when a central connects and turns off when it disconnects.
-   Launches a dedicated thread that reads the sensor periodically and sends notifications with the latest data.

### `my_sensor.h` — The Service's Public Interface
-   Declares the unique 128-bit UUIDs for our custom service and characteristic.
-   Provides the function prototype for sending notifications: `my_sensor_notify_string()`.

### `my_sensor.c` — The Service's Implementation
-   Defines the GATT service with a single, notify-only characteristic.
-   Configures the Client Characteristic Configuration Descriptor (CCCD) to allow clients to subscribe to notifications.
-   Implements the function that sends notifications only if a client has enabled them.

---

## 4. Connecting Your Hardware

You need to tell your Zephyr project which GPIO pin the DHT11 sensor's data line is connected to. This is done via a device tree overlay file.

1.  Create a file named `app.overlay` in your project's root directory.
2.  Add the following snippet. This example maps the DHT11 data pin to **P0.11** on the nRF52840 DK.

    ```
    / {
        dht11_0: dht11@0 {
            compatible = "worldsemi,dht11";
            label = "DHT11";
            status = "okay";
            gpios = <&gpio0 11 GPIO_ACTIVE_HIGH>; /* DATA pin is connected to P0.11 */
        };

        aliases {
            dht11 = &dht11_0;
        };
    };
    ```

3.  Connect your sensor to the development board:

| DHT11 Sensor Pin | nRF52840 DK Pin |
| :--------------- | :-------------- |
| **VCC** / **+**  | `3.3V`          |
| **DATA** / **OUT**| `P0.11`         |
| **GND** / **-**  | `GND`           |

*Note: If your DHT11 module doesn’t have a built-in pull-up resistor, you may need to add one (e.g., 4.7kΩ–10kΩ) between the DATA and VCC lines.*

---

## 5. How to Build & Test

1.  **Build** the project for your board using the `west` tool:
    ```
    west build -b nrf52840dk_nrf52840 --pristine
    ```

2.  **Flash** the firmware to your board:
    ```
    west flash
    ```

3.  Open **nRF Connect for Mobile** or **Desktop** and start scanning for BLE devices. You should see a device named "Zephyr MySensor App".

4.  Connect to it, and you should observe:
    -   The connection status LED on your Nordic board lights up.
    -   In the list of services, find your **"MySensor Service"** (identified by its custom UUID).
    -   Enable notifications on the characteristic within that service.
    -   Sensor readings like `"25C | 45%"` should start streaming to the app every few seconds.

---

## Final Thoughts

This project provides a solid foundation for creating BLE-enabled IoT sensors using Zephyr. We get the basics of BLE advertising, connection management, custom services, and notifications, plus how to integrate a common sensor.


*Happy coding!*
