#include "zb_device.h"
#include "zbesp_runtime.h"
#include "zb_button.h"
#include <esp32-hal-periman.h>
#include "zcl/esp_zigbee_zcl_door_lock.h"

#define TAG "zed_runtime"

uint8_t lockState = 0x02; // Lock Unlocked
uint8_t doorState = 0xFF;

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED);
ZbEndpoint ep1(zd, 1, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);

ZbBasicCluster zcBasic(ep1, "MEA", "door-lock", 0x01, 0x01);
ZbIdentifyCluster zcIdentify(ep1);

ZbDoorLockCluster zcDoorLock(ep1);
ZbAttribute zaLockState(zcDoorLock, ESP_ZB_ZCL_ATTR_DOOR_LOCK_LOCK_STATE_ID, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_RP, (uint8_t) lockState);
ZbAttribute zaLockType(zcDoorLock, ESP_ZB_ZCL_ATTR_DOOR_LOCK_LOCK_TYPE_ID, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, (uint8_t) 0x06);
ZbAttribute zaActuatorEnabled(zcDoorLock, ESP_ZB_ZCL_ATTR_DOOR_LOCK_ACTUATOR_ENABLED_ID, ESP_ZB_ZCL_ATTR_TYPE_BOOL, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, (uint8_t) 0);
ZbAttribute zaDoorState(zcDoorLock, ESP_ZB_ZCL_ATTR_DOOR_LOCK_DOOR_STATE_ID, ESP_ZB_ZCL_ATTR_TYPE_8BIT_ENUM, ESP_ZB_ZCL_ATTR_ACCESS_RP, (uint8_t) doorState);

EspZbRuntime rt;

ZbDeviceButton button(zd, GPIO_NUM_9);

void lockStateChanged(void)
{
  switch(lockState) {
    case 0x00:
      ESP_LOGI(TAG, "Lock Not fully locked");
      rgbLedWrite(RGB_BUILTIN,20,10,0);
      break;
    case 0x01:
      ESP_LOGI(TAG, "Lock Locked");
      rgbLedWrite(RGB_BUILTIN,20,0,0);
      break;
    case 0x02:
      ESP_LOGI(TAG, "Lock Unlocked");
      rgbLedWrite(RGB_BUILTIN,0,20,0);
      break;
    default:
      ESP_LOGI(TAG, "Lock Undefined");
      rgbLedWrite(RGB_BUILTIN,0,0,0);
  }
  zaLockState.setValue(lockState);
}

void doorStateChanged(void)
{
  switch(doorState) {
    case 0x00:
      ESP_LOGI(TAG, "Door Open");
      break;
    case 0x01:
      ESP_LOGI(TAG, "Door Closed");
      break;
    case 0x02:
      ESP_LOGI(TAG, "Door Error (jammed)");
      break;
    case 0x03:
      ESP_LOGI(TAG, "Door Error (forced open)");
      break;
    case 0x04:
      ESP_LOGI(TAG, "Door Error (unspecified)");
      break;
    default:
      ESP_LOGI(TAG, "Door Undefined");
  }
  zaDoorState.setValue(doorState);
}

static void zcDoorLockCommandReceived(ZbCluster *zc, uint8_t command)
{
  //ESP_LOGI(TAG, "zcDoorLockCommand command=%d", command);
  lockState = (command == ESP_ZB_ZCL_CMD_DOOR_LOCK_LOCK_DOOR ? 1 : 2);
  lockStateChanged();
}

void setLockState(uint8_t value)
{
  lockState = value;
  lockStateChanged();
}

void setDoorState(uint8_t value)
{
  doorState = value;
  doorStateChanged();
}

void buttonClick(void)
{
  setLockState(lockState == 1 ? 2 : 1);
}

void doorStateUpdate()
{
  static uint32_t lastDoorStateToggleTime = millis();
  if (lockState == 2 && millis() - lastDoorStateToggleTime > 5000) { // если замок разблокирован, дверь можно открыть и закрыть
    setDoorState(doorState ? 0 : 1); // меняем состояние двери
    lastDoorStateToggleTime = millis();
  }
}

void setup()
{
  button.attachClick(buttonClick);

  lockStateChanged();
  doorStateChanged();
  
  rt.init(zd, true);
  zcDoorLock.onCommandReceived(zcDoorLockCommandReceived);
  rt.start();
}

void loop() {
  button.tick();
  doorStateUpdate();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
