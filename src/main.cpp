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

bool oldValue;

static void process_boiler_state_cb(void *arg);

enum mgos_app_init_result mgos_app_init(void)
{
  mgos_gpio_setup_output(LED_PIN, 0);
  mgos_gpio_set_mode(LIGHT_SENSOR_PIN, MGOS_GPIO_MODE_INPUT);

  long update_interval_ms = mgos_sys_config_get_sensor_interval();

  oldValue = !mgos_gpio_read(LIGHT_SENSOR_PIN);

  mgos_set_timer(update_interval_ms /* ms */, MGOS_TIMER_REPEAT, process_boiler_state_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}

static void process_boiler_state_cb(void *)
{
  bool newValue = mgos_gpio_read(LIGHT_SENSOR_PIN);
  const char *state = newValue ? "ON" : "OFF";

  /** always send the state if configuration is true */
  if (mgos_sys_config_get_sensor_sendstate())
  {
    mgos_mqtt_pub("/boiler/state", state, strlen(state), 1, 0);
  }

  /** only send the state if it has changed */
  if (newValue != oldValue)
  {
    LOG(LL_INFO, (state));
    mgos_mqtt_pub("/boiler/change", state, strlen(state), 1, 0);
    mgos_gpio_write(LED_PIN, !newValue); // inverted; = "ON"
    oldValue = newValue;
  }
}
