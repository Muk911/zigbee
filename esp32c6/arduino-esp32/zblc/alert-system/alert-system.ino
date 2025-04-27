#include "zb_device.h"
#include "zbesp_runtime.h"
#include "zb_button.h"
#include <esp32-hal-periman.h>
#include "DYPlayerArduino.h"

#define TAG "zed_runtime"
#define ESP_ZB_ZCL_ATTR_MULTI_VALUE_NUMBER_OF_STATES_ID 0x004a

uint8_t alertMode = 0; 

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED);
ZbEndpoint ep1(zd, 1, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);

ZbBasicCluster zcBasic(ep1, "MEA", "alert-system", 0x01, 0x01);
ZbIdentifyCluster zcIdentify(ep1);

ZbMultiStateValueCluster zcMultiStateValue(ep1);
ZbAttribute zaNumberOfStates(zcMultiStateValue, ESP_ZB_ZCL_ATTR_MULTI_VALUE_NUMBER_OF_STATES_ID, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, (int16_t) 0);
ZbAttribute zaOutOfService(zcMultiStateValue, ESP_ZB_ZCL_ATTR_MULTI_VALUE_OUT_OF_SERVICE_ID, ESP_ZB_ZCL_ATTR_TYPE_BOOL, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE, (uint8_t) 0);
ZbAttribute zaPresentValue(zcMultiStateValue, ESP_ZB_ZCL_ATTR_MULTI_VALUE_PRESENT_VALUE_ID, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, (int16_t) alertMode);
ZbAttribute zaStatusFlags(zcMultiStateValue, ESP_ZB_ZCL_ATTR_MULTI_VALUE_STATUS_FLAGS_ID, ESP_ZB_ZCL_ATTR_TYPE_8BITMAP, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, (uint8_t) 0);

EspZbRuntime rt;

ZbDeviceButton button(zd, GPIO_NUM_9);

DY::Player player(&Serial2);

static void presentValueChanged(ZbAttribute *za, uint8_t *data)
{
  int16_t value = *(int16_t*) data;
  setAlertMode(value);
  ESP_LOGI(TAG, "presentValueChanged value=%d", value);
}

void alertModeChanged(void)
{
  switch(alertMode) {
    case 1:
      rgbLedWrite(RGB_BUILTIN,0,0,20);
      break;
    case 2:
      rgbLedWrite(RGB_BUILTIN,0,20,0);
      break;
    case 3:
      rgbLedWrite(RGB_BUILTIN,20,20,0);
      break;
    default:
      rgbLedWrite(RGB_BUILTIN,0,0,0);
  }
  zaPresentValue.setValue((int16_t) alertMode);
}

void setAlertMode(uint8_t value)
{
  if (value == 0) 
    player.stop();
  else
    player.playSpecified(value);
  alertMode = value;
  alertModeChanged();
}

void buttonClick(void)
{
  setAlertMode(alertMode < 3 ? alertMode + 1 : 0);
}

static void player_task(void *arg)
{
  while (1) {
    if (alertMode && player.checkPlayState() == DY::PlayState::Stopped) {
      alertMode = 0;
      alertModeChanged();
    }
    vTaskDelay(300 / portTICK_PERIOD_MS);
  }
}

void setup()
{
  button.attachClick(buttonClick);

  player.begin();
  player.setVolume(15); // 50% Volume 

  xTaskCreate(player_task, "player", 4096, NULL, 5, NULL);

  rt.init(zd, true);
  zaPresentValue.onValueChanged(presentValueChanged);
  rt.start();
}

void loop() {
  button.tick();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
