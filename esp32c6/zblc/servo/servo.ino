#include "zb_device.h"
#include "zbesp_runtime.h"
#include "zb_button.h"
#include "esp_servo.h"
#include <esp32-hal-periman.h>

#define TAG "zed_runtime"

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED);
ZbEndpoint ep1(zd, 1, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);

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

ServoAdapter servo(18, 1200, 2500); //Servo available on: 2,4,5,12-19,21-23,25-27,32-33

static void currentLevelChanged(ZbAttribute *za, uint8_t *data)
{
  uint16_t position = *data;
  ESP_LOGI(TAG, "currentLevelChanged position=%d", position);
  servo.move(map(position, 1, 254, servo.getMin(), servo.getMax()));
}

void servoMove(ServoAdapter *sa, uint16_t position)
{
  ESP_LOGI(TAG, "servoMove=%d", position);

  if(!perimanGetPinBus(sa->getPin(), ESP32_BUS_TYPE_LEDC)) 
    ledcAttach(sa->getPin(), 50, 16);
  ledcWrite(sa->getPin(), position);  
}

void servoComplete(ServoAdapter *sa)
{
  uint16_t position = ledcRead(sa->getPin()); //servo1.read();
  ESP_LOGI(TAG, "servoComplete position=%d", position);
  
  if(perimanGetPinBus(sa->getPin(), ESP32_BUS_TYPE_LEDC)) 
    ledcDetach(sa->getPin());
  zaCurrentLevel.setValue(map(position, sa->getMin(), sa->getMax(), 1, 254));
}

void buttonClick(void)
{
  ESP_LOGI(TAG, "buttonClick()");
  zaCurrentLevel.setValue((uint8_t) 34);
}

void setup()
{
  servo.onMove(servoMove);
  servo.onComplete(servoComplete);

  button.attachClick(buttonClick);

  rt.init(zd, true);
  zaCurrentLevel.onValueChanged(currentLevelChanged);
  rt.start();
}

void loop() {
  button.tick();
  servo.loop();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
