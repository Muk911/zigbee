#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "esp_log.h"
#include "esp_check.h"
#include "esp_random.h"
#include "zb_device.h"
#include "zbesp_runtime.h"
#include "zb_button.h"

#define TAG "zed_runtime"

uint16_t temperature = 2000, humidity = 7000, pressure = 1000;

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED); // ESP_ZB_DEVICE_TYPE_ROUTER
ZbEndpoint ep1(zd, 1, DEVICE_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);

ZbBasicCluster zcBasic(ep1, "MEA", "thp.sensor", 0x01, 0x04);
ZbIdentifyCluster zcIdentify(ep1);

ZbTemperatureMeasCluster zcTemperatureMeas(ep1);
ZbAttribute zaTemperatureValue(zcTemperatureMeas, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, temperature);
ZbAttribute zaTemperatureMinValue(zcTemperatureMeas, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID, (int16_t) 0x8000);
ZbAttribute zaTemperatureMaxValue(zcTemperatureMeas, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID, (int16_t) 0x8000);

ZbHumidityMeasCluster zcHumidityMeas(ep1);
ZbAttribute zaHumidityValue(zcHumidityMeas, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, humidity);
ZbAttribute zaHumidityMinValue(zcHumidityMeas, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_ID, (int16_t) 0xffff);
ZbAttribute zaHumidityMaxValue(zcHumidityMeas, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_ID, (int16_t) 0xffff);

ZbPressureMeasCluster zcPressureMeas(ep1);
ZbAttribute zaPressureValue(zcPressureMeas, ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID, pressure);
ZbAttribute zaPressureMinValue(zcPressureMeas, ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MIN_VALUE_ID, (int16_t) 0x8000);
ZbAttribute zaPressureMaxValue(zcPressureMeas, ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MAX_VALUE_ID, (int16_t) 0x8000);

EspZbRuntime rt;

ZbDeviceButton button(zd, GPIO_NUM_9);

static void updateMeasurements(void *pvParameters)
{
  while(1) {
    if (zd.joined()) {
      ESP_LOGI(TAG, "updateMeasurements");
      temperature = temperature + (esp_random() % 400 - 200);
      humidity = humidity + (esp_random() % 200 - 100);
      pressure = pressure + (esp_random() % 400 - 200);
      zaTemperatureValue.setValue(temperature);
      zaHumidityValue.setValue(humidity);
      zaPressureValue.setValue(pressure);
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void buttonClick(void)
{
  zd.reportNow();
}

uint8_t initDrivers(void)
{
    return 0;
}

void setup()
{
  Serial.begin(115200);
  Serial.println("==========================================================================");

  button.attachClick(buttonClick);
  zd.onDeferrerInit(initDrivers);

  rt.init(zd, true);
  xTaskCreate(updateMeasurements, "updateMeasurements", 4096, NULL, 5, NULL);
  rt.start();
}

void loop() {
  button.tick();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
