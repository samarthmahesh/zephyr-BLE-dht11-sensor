/*
 * my_sensor_service.c
 *
 * Custom MySensor Bluetooth Service implementation with explanatory comments.
 * Defines a GATT service containing a notify-only characteristic for sending
 * sensor data to a connected central device.
 *
 * SPDX-License-Identifier: LicenseRef-Nordic-5-Clause
 */

#include <zephyr/types.h>           // Standard types: u8_t, u32_t
#include <stddef.h>                 // NULL, size_t
#include <string.h>                 // memcpy, memset
#include <errno.h>                  // Error codes like -EACCES
#include <zephyr/sys/printk.h>      // printk for debug output
#include <zephyr/kernel.h>          // Kernel utilities
#include <zephyr/logging/log.h>     // Logging macros
#include <zephyr/bluetooth/bluetooth.h>  // Bluetooth API
#include <zephyr/bluetooth/conn.h>       // Connection management
#include <zephyr/bluetooth/gatt.h>       // GATT service API
#include "my_sensor.h"              // Header with UUIDs and notify API

LOG_MODULE_DECLARE(Lesson4_Exercise2);

// Flag to track if notifications are enabled
static bool notify_mysensor_enabled;

/*
 * Called when central enables/disables notifications for MySensor.
 */
static void ccc_cfg_changed_handler(const struct bt_gatt_attr *attr, uint16_t value)
{
    notify_mysensor_enabled = (value == BT_GATT_CCC_NOTIFY);
}

/*
 * GATT service declaration for MySensor.
 * Contains:
 * - Primary Service UUID
 * - One Notify-only Characteristic with CCC descriptor
 */
BT_GATT_SERVICE_DEFINE(my_sensor_service, // Define a GATT service instance named 'my_sensor_service'

    // Primary service declaration with custom 128-bit UUID
    BT_GATT_PRIMARY_SERVICE(BT_UUID_MYSENSOR_SERVICE),

    // Characteristic declaration:
    // - UUID: BT_UUID_MYSENSOR_CHARACTERISTIC
    // - Properties: Notify only
    // - Permissions: No read/write from client
    // - NULL read/write callbacks (since it's notify-only)
    BT_GATT_CHARACTERISTIC(
        BT_UUID_MYSENSOR_CHARACTERISTIC,
        BT_GATT_CHRC_NOTIFY,         // Allows notifications to be sent
        BT_GATT_PERM_NONE,           // No client-side read/write access
        NULL, NULL, NULL),           // No read/write callbacks needed

    // Client Characteristic Configuration Descriptor (CCCD):
    // - Lets central device enable/disable notifications
    // - Triggers 'ccc_cfg_changed_handler' on write
    BT_GATT_CCC(ccc_cfg_changed_handler,
                BT_GATT_PERM_READ | BT_GATT_PERM_WRITE),
);


/*
 * Sends a string sensor value to the central via notification.
 * Central must have subscribed by writing to CCC.
 */

int my_sensor_notify_string(const char *sensor_str)
{
    if (!notify_mysensor_enabled) {
        return -EACCES;
    }

    return bt_gatt_notify(NULL,
                          &my_sensor_service.attrs[2],
                          sensor_str,
                          strlen(sensor_str));
}