/** Kingswood Monitoring System Gas sensor firmware 
 *  kingswood-monitor-sensor-energy-gas
 *  V1.0
 */

#include "mgos.h"
#include "mgos_wifi.h"
#include "mgos_provision.h"
#include "mgos_mqtt.h"
#include "mgos_gpio.h"

#define LIGHT_SENSOR_PIN 5
#define LED_PIN 16

bool oldValue, newValue;

static void process_boiler_state_cb(void *arg);

enum mgos_app_init_result mgos_app_init(void)
{

  mgos_gpio_setup_output(LED_PIN, 0);
  mgos_gpio_set_mode(LIGHT_SENSOR_PIN, MGOS_GPIO_MODE_INPUT);

  long update_interval_ms = mgos_sys_config_get_sensor_interval();

  newValue = mgos_gpio_read(LIGHT_SENSOR_PIN);
  oldValue = !newValue;

  mgos_set_timer(update_interval_ms /* ms */, MGOS_TIMER_REPEAT, process_boiler_state_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}

static void process_boiler_state_cb(void *arg)
{
  newValue = mgos_gpio_read(LIGHT_SENSOR_PIN);

  /** Send the state if configuration is true */
  bool sendState = (mgos_sys_config_get_sensor_sendstate());
  if (sendState)
  {
    if (newValue == true)
    {
      mgos_mqtt_pub("/boiler/state", "ON", 2, 1, 0);
    }
    else
    {
      mgos_mqtt_pub("/boiler/state", "OFF", 3, 1, 0);
    }
  }

  /** Send the change if it has changed */
  if (newValue != oldValue)
  {
    oldValue = newValue;
    if (newValue == true)
    {
      LOG(LL_INFO, ("ON"));
      mgos_mqtt_pub("/boiler/change", "ON", 2, 1, 0);
      mgos_gpio_write(LED_PIN, false); // inverted; = "ON"
    }
    else
    {
      LOG(LL_INFO, ("OFF"));
      mgos_mqtt_pub("/boiler/change", "OFF", 3, 1, 0);
      mgos_gpio_write(LED_PIN, true); // inverted; = "OFF"
    }
  }

  // mgos_gpio_toggle(LED_PIN);
  (void)arg;
}
