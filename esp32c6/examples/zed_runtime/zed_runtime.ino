#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_zigbee_type.h"
#include "esp_zigbee_core.h"
#include "nvs_flash.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl/esp_zigbee_zcl_command.h"
#include "esp_random.h"
#include "esp_check.h"
#include "zb_device.h"
#include "zbesp_runtime.h"

#define TAG "zed_runtime"
extern bool connected;

#define MANUFACTURER_NAME   "MEA"
#define MODEL_NAME          "Test2"
#define ZCL_VERSION         0x03
#define APP_VERSION         0x01
#define POWER_SOURCE        0x01

#define DEVICE_ENDPOINT     0x01
#define DEVICE_PROFILE_ID   ESP_ZB_AF_HA_PROFILE_ID
#define DEVICE_ID           ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID

uint8_t relay_status = 0;
uint16_t temperature = 2500;

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED); //DEVICE_TYPE_ENDDEVICE);
ZbEndpoint ep(zd, DEVICE_ENDPOINT, DEVICE_PROFILE_ID, DEVICE_ID);
ZbBasicCluster cb(ep, MANUFACTURER_NAME, MODEL_NAME, ZCL_VERSION, APP_VERSION, POWER_SOURCE);
ZbOnOffCluster coo(ep);
ZbAttributeU8 aoo(coo, ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, relay_status);
ZbTemperatureMeasCluster ct(ep);
ZbAttributeU16 at1(ct, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, temperature);
ZbAttributeU16 at2(ct, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID, 0);
ZbAttributeU16 at3(ct, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID, 10000);
ZbRuntimeESP rt;

void relay_init(void)
{
    relay_status = 0;
    relay_update();
}

void relay_update(void)
{
    if (relay_status) {
      ESP_LOGI(TAG, "ON");
      neopixelWrite(RGB_BUILTIN,20,20,20);
    } else {
      ESP_LOGI(TAG, "OFF");
      neopixelWrite(RGB_BUILTIN,0,0,0);
    }
}

static void OnOff_StateChanged(ZbAttribute *za, uint8_t data)
{
  relay_status = data;
  relay_update(); 
}

static void update_temperature(void *pvParameters)
{
  //ESP_LOGI(TAG, "update_temperature()");
  while(1) {
    if (zd.connected()) {
      temperature = temperature + (esp_random() % 400 - 200);
      at1.setValue(temperature);
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  Serial.begin(115200);

  ESP_LOGI(TAG, "setup()");
  zd.setInstallCodePolicy(false);
  zd.setMaxChildren(16);
  zd.setNetworkChannel(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK);
  
  aoo.setValueChangedCB(OnOff_StateChanged);
    
  relay_init();

  xTaskCreate(update_temperature, "update_temperature", 4096, NULL, 5, NULL);
  rt.start(zd); //, false);
}

void loop() {
}
