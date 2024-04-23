#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "zb_device.h"
#include "zbesp_runtime.h"
#include "zb_button.h"
#include "esp_servo.h"

#define TAG "zed_runtime"

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED);
ZbEndpoint ep1(zd, 1, DEVICE_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);

ZbBasicCluster zcBasic(ep1, "MEA", "servo", 0x01, 0x01);
ZbIdentifyCluster zcIdentify(ep1);

ZbLevelControlCluster zcLevelControl(ep1);
ZbAttribute zaCurrentLevel(zcLevelControl, ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_CURRENT_LEVEL_ID, (uint8_t) 23);
ZbAttribute zaMinLevel(zcLevelControl, ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_MIN_LEVEL_ID, (uint8_t) 1);
ZbAttribute zaMaxLevel(zcLevelControl, ESP_ZB_ZCL_ATTR_LEVEL_CONTROL_MAX_LEVEL_ID, (uint8_t) 254);

// ESP32-C6 крашится с LevelControl без OnOff
ZbOnOffCluster zcOnOff(ep1);
ZbAttribute zaOnOff(zcOnOff, ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, (uint8_t) 0);

EspZbRuntime rt;

ZbDeviceButton button(zd, GPIO_NUM_9);

EspServo espServo;
ServoAdapter servo(10, 1, 254);

static void currentLevelChanged(ZbAttribute *za, uint8_t *data)
{
  uint16_t position = *data;
  ESP_LOGI(TAG, "currentLevelChanged position=%d", position);
  servo.move(map(position, 1, 254, servo.getMin(), servo.getMax()));
}

void servoMove(ServoAdapter *sa, uint16_t position)
{
  espServo.attach(sa->getPin(), sa->getMin(), sa->getMax());
  espServo.write(position);
}

void servoComplete(ServoAdapter *sa)
{
  uint16_t position = espServo.read();
  ESP_LOGI(TAG, "servoComplete position=%d", position);
  zaCurrentLevel.setValue(map(position, sa->getMin(), sa->getMax(), 1, 254));
  espServo.detach();
}

void setup()
{
  servo.onMove(servoMove);
  servo.onComplete(servoComplete);

  rt.init(zd, true);
  zaCurrentLevel.onValueChanged(currentLevelChanged);
  rt.start();
}

void loop() {
  button.tick();
  servo.loop();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
