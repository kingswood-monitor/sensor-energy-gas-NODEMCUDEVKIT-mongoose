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
#define DEBOUNCE_MS 500

bool oldValue, newValue;

static void process_light_cb(void *arg)
{
  newValue = mgos_gpio_read(LIGHT_SENSOR_PIN);
  if (newValue != oldValue)
  {
    oldValue = newValue;
    if (newValue == true)
    {
      LOG(LL_INFO, ("ON"));
      mgos_mqtt_pub("/boiler", "ON", 3, 1, 0);
      mgos_gpio_write(LED_PIN, false); // inverted; = "ON"
    }
    else
    {
      LOG(LL_INFO, ("OFF"));
      mgos_mqtt_pub("/boiler", "OFF", 4, 1, 0);
      mgos_gpio_write(LED_PIN, true); // inverted; = "OFF"
    }
  }

  // mgos_gpio_toggle(LED_PIN);
  (void)arg;
}

enum mgos_app_init_result mgos_app_init(void)
{

  mgos_gpio_setup_output(LED_PIN, 0);
  mgos_gpio_set_mode(LIGHT_SENSOR_PIN, MGOS_GPIO_MODE_INPUT);

  newValue = mgos_gpio_read(LIGHT_SENSOR_PIN);
  oldValue = !newValue;

  mgos_set_timer(DEBOUNCE_MS /* ms */, MGOS_TIMER_REPEAT, process_light_cb, NULL);
  return MGOS_APP_INIT_SUCCESS;
}
