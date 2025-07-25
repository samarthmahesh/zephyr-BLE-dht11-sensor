/*
 * my_sensor.h
 *
 * Header file for the custom MySensor Bluetooth Low Energy (BLE) service.
 * This service provides a notify-only characteristic used to send sensor data
 * (e.g., temperature or any 32-bit sensor value) to a connected central device.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#ifndef MY_SENSOR_H_
#define MY_SENSOR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <zephyr/types.h>
/*
 * Standard Zephyr type definitions (e.g., uint32_t, bool).
 */

/*
 * ===== Custom UUID Definitions =====
 * These UUIDs are 128-bit and uniquely identify the MySensor service and characteristic.
 */

// Custom 128-bit UUID for the MySensor GATT Service
#define BT_UUID_MYSENSOR_SERVICE \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345678, 0x9abc, 0xdef0, 0x1234, 0x56789abcdef0))

// Custom 128-bit UUID for the notify-only characteristic within MySensor Service
#define BT_UUID_MYSENSOR_CHARACTERISTIC \
    BT_UUID_DECLARE_128(BT_UUID_128_ENCODE(0x12345679, 0x9abc, 0xdef0, 0x1234, 0x56789abcdef0))

/**
 * @brief Notify connected central(s) with a 32-bit sensor value.
 *
 * Sends a BLE notification to all connected devices that have enabled
 * notifications on the custom MySensor characteristic.
 *
 * @param[in] sensor_value The 32-bit sensor reading to transmit.
 * @return 0 if the notification was queued successfully,
 *         -EACCES if notifications are not enabled by the client.
 */
int my_sensor_notify_string(const char *sensor_str);


#ifdef __cplusplus
}
#endif

#endif /* MY_SENSOR_H_ */
