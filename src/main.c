/*
 * lesson4_exercise2_commented.c
 *
 * Zephyr application: Custom MySensor Service example.
 * Sends simulated 32-bit sensor data via BLE notifications using a custom GATT characteristic.
 * Includes detailed inline comments explaining every macro, function, and flow.
 */

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/gap.h>
#include <zephyr/bluetooth/uuid.h>
#include <zephyr/bluetooth/conn.h>
#include <dk_buttons_and_leds.h>
#include <zephyr/device.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <zephyr/devicetree.h>
#include <stdio.h>

/* Include header for our custom MySensor GATT service */
#include "my_sensor.h"  // You may want to rename this to my_sensor.h to match the new structure

LOG_MODULE_REGISTER(Lesson4_Exercise2, LOG_LEVEL_INF);

/*
 * Advertising parameters:
 * - Connectable advertising
 * - Uses identity address
 * - Advertising interval: 500 ms
 */
static const struct bt_le_adv_param *adv_param = BT_LE_ADV_PARAM(
    (BT_LE_ADV_OPT_CONNECTABLE | BT_LE_ADV_OPT_USE_IDENTITY),
    800, 801, NULL);
/*Explain the above line 800 and 801 are the minimum and maximum advertising intervals in 0.625 ms units.
 * 800 * 0.625 ms = 500 ms, so this sets the advertising interval to 500 ms.
 */

/* GAP device name from Kconfig */
#define DEVICE_NAME CONFIG_BT_DEVICE_NAME
#define DEVICE_NAME_LEN (sizeof(DEVICE_NAME) - 1)

/* LEDs */
#define RUN_STATUS_LED DK_LED1     // Blinks to indicate system is running
#define CON_STATUS_LED DK_LED2     // Indicates active BLE connection

/* Thread stack size and priority */
#define STACKSIZE 1024
#define PRIORITY 7

/* Blink interval and sensor notification interval */
#define RUN_LED_BLINK_INTERVAL 1000

/* Alias we defined in the overlay */
#define DHT11_NODE DT_ALIAS(dht11)

#if !DT_NODE_HAS_STATUS(DHT11_NODE, okay)
#error "DHT11 devicetree alias is not defined or status is not 'okay'"
#endif

/* Work queue item for advertising */
static struct k_work adv_work;

/* Advertising payload: flags and complete name */
static const struct bt_data ad[] = {
    BT_DATA_BYTES(BT_DATA_FLAGS, (BT_LE_AD_GENERAL | BT_LE_AD_NO_BREDR)),
    BT_DATA(BT_DATA_NAME_COMPLETE, DEVICE_NAME, DEVICE_NAME_LEN),
};

/* Scan response payload: include our custom MySensor service UUID */
static const struct bt_data sd[] = {
    BT_DATA_BYTES(BT_DATA_UUID128_ALL,
        BT_UUID_128_ENCODE(0x12345678, 0x9abc, 0xdef0, 0x1234, 0x56789abcdef0)),
    // This UUID matches BT_UUID_MYSENSOR_SERVICE used in GATT declaration
};

/* Advertising work handler */
static void adv_work_handler(struct k_work *work)
{
    int err = bt_le_adv_start(adv_param, ad, ARRAY_SIZE(ad), sd, ARRAY_SIZE(sd));
    if (err) {
        printk("Advertising failed to start (err %d)\n", err);
        return;
    }
    printk("Advertising successfully started\n");
}

/* Helper to start advertising */
static void advertising_start(void)
{
    k_work_submit(&adv_work);
}

/* Called when old connection object is recycled */
static void recycled_cb(void)
{
    printk("Recycled connection object: restarting advertising\n");
    advertising_start();
}

/* Thread function to send sensor data via notification periodically */
void dht11_notify_thread(void)
{
    const struct device *dht = DEVICE_DT_GET(DHT11_NODE);

    if (!device_is_ready(dht)) {
        printk("DHT11 device not ready\n");
        return;
    }

    struct sensor_value temp, hum;
    char data_str[20];  // Enough for "100C | 100%\0"

    while (1) {
        if (sensor_sample_fetch(dht) == 0) 
        {
            sensor_channel_get(dht, SENSOR_CHAN_AMBIENT_TEMP, &temp);
            sensor_channel_get(dht, SENSOR_CHAN_HUMIDITY, &hum);

            snprintf(data_str, sizeof(data_str), "%dC | %d%%", temp.val1, hum.val1);
            printk("Sending: %s\n", data_str);

            int err = my_sensor_notify_string(data_str);
            if (err == -EACCES) {
                printk("Notification not enabled by central.\n");
            } else if (err) {
                printk("Notification sent!\n");
            }
        } 
        else 
        {
            printk("Sample fetch failed\n");
        }

        k_sleep(K_MSEC(2000));
    }
}

/* Connection callback: connected */
static void on_connected(struct bt_conn *conn, uint8_t err)
{
    if (err) {
        printk("Connection failed (err %u)\n", err);
        return;
    }
    printk("Connected\n");
    dk_set_led_on(CON_STATUS_LED);
}

/* Connection callback: disconnected */
static void on_disconnected(struct bt_conn *conn, uint8_t reason)
{
    printk("Disconnected (reason %u)\n", reason);
    dk_set_led_off(CON_STATUS_LED);
}

/* Register BLE connection callbacks */
static struct bt_conn_cb connection_callbacks = {
    .connected    = on_connected,
    .disconnected = on_disconnected,
    .recycled     = recycled_cb,
};

/* Entry point */
int main(void)
{
    int err;
    int blink_state = 0;

    /* Initialize LED control */
    err = dk_leds_init();
    if (err) {
        LOG_ERR("LEDs init failed (err %d)\n", err);
        return -1;
    }

    /* Enable Bluetooth */
    err = bt_enable(NULL);
    if (err) {
        LOG_ERR("Bluetooth init failed (err %d)\n", err);
        return -1;
    }

    /* Register connection callbacks */
    bt_conn_cb_register(&connection_callbacks);

    LOG_INF("Bluetooth initialized\n");

    /* Initialize advertising work and start it */
    k_work_init(&adv_work, adv_work_handler);
    advertising_start();

    /* Blink LED to show device is alive */
    while (1) {
        dk_set_led(RUN_STATUS_LED, (++blink_state) % 2);
        k_sleep(K_MSEC(RUN_LED_BLINK_INTERVAL));
    }
}

   /* Launch a thread to periodically send sensor data */
    K_THREAD_DEFINE(dht11_thread_id,
                STACKSIZE,
                dht11_notify_thread,
                NULL, NULL, NULL,
                PRIORITY,
                0,
                0);