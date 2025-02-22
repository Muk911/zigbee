#include "esp_log.h"
#include "esp_check.h"
#include "esp_random.h"
#include "zb_device.h"
#include "zb_zcl.h"
#include "zbesp_runtime.h"
#include "zb_button.h"
#include "esp_nvs_flash.h"

#define TAG "zed_runtime"

#define THERMOSTAT_HYSTERESIS  50 // =0.5°C

uint8_t relay_state = 0;
int16_t temperature = 2000;
int16_t outdoorTemperature = 2000;
int16_t thermostatSetpoint = 2500;
uint8_t thermostatSystemMode = 0;
uint8_t runningState = 0;
int8_t thermostatTemperatureCalibration = 0;

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ROUTER);
ZbEndpoint ep1(zd, 1, ESP_ZB_AF_HA_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);

ZbBasicCluster zcBasic(ep1, "MEA", "thermostat", 0x01, 0x01);
ZbIdentifyCluster zcIdentify(ep1);

ZbThermostatCluster zcThermostat(ep1);
ZbAttribute zaLocalTemperature(zcThermostat, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID, temperature);
ZbAttribute zaOutdoorTemperature(zcThermostat, ESP_ZB_ZCL_ATTR_THERMOSTAT_OUTDOOR_TEMPERATURE_ID, ESP_ZB_ZCL_ATTR_TYPE_S16, ESP_ZB_ZCL_ATTR_ACCESS_RP, outdoorTemperature);
ZbAttribute zaHeatingSetpoint(zcThermostat, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID, thermostatSetpoint);
ZbAttribute zaSystemMode(zcThermostat, ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID, thermostatSystemMode);
ZbAttribute zaRunningState(zcThermostat, ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID, ESP_ZB_ZCL_ATTR_TYPE_16BITMAP, ESP_ZB_ZCL_ATTR_ACCESS_RP, runningState);
ZbAttribute zaLocalTemperatureCalibration(zcThermostat, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION_ID, thermostatTemperatureCalibration);

uint8_t fanMode = 0;

ZbCluster zcFanControl(ep1, ESP_ZB_ZCL_CLUSTER_ID_FAN_CONTROL, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
ZbAttribute zaFanMode(zcFanControl, ESP_ZB_ZCL_ATTR_FAN_CONTROL_FAN_MODE_ID, fanMode);
ZbAttribute zaFanModeSequence(zcFanControl, ESP_ZB_ZCL_ATTR_FAN_CONTROL_FAN_MODE_SEQUENCE_ID, (uint8_t) 0);

EspZbRuntime rt;

ZbDeviceButton button(zd, GPIO_NUM_9);

NvsVariable nvHeatingSetpoint(2, "HeatSetpoint");
NvsVariable nvSystemMode(1, "SystemMode");
NvsVariable nvLocalTemperatureCalibration(2, "LocTempCalib");
NvsVariable nvFanMode(1, "FanMode"); 

void relay_update(void)
{
  if (relay_state) {
    //ESP_LOGI(TAG, "ON");
    neopixelWrite(RGB_BUILTIN,20,20,20);
  } else {
    //ESP_LOGI(TAG, "OFF");
    neopixelWrite(RGB_BUILTIN,0,0,0);
  }
  zaRunningState.setValue(relay_state); 
  zaRunningState.report();
}

void setRelayState(uint8_t value)
{
  if(relay_state != value) {
    relay_state = value;
    relay_update();
  }
}

void Thermostat_HeatingSetpointChanged(ZbAttribute *za, uint8_t *data)
{
  thermostatSetpoint = *(int16_t*) data;
  nvHeatingSetpoint.writeInt(thermostatSetpoint);
  updateThermostatState();
}

void Thermostat_SystemModeChanged(ZbAttribute *za, uint8_t *data)
{
  thermostatSystemMode = *data;
  nvSystemMode.writeInt(thermostatSystemMode);
  updateThermostatState();
}

void Thermostat_LocalTemperatureCalibrationChanged(ZbAttribute *za, uint8_t *data)
{
  thermostatTemperatureCalibration = *(int8_t*) data;
  nvLocalTemperatureCalibration.writeInt(thermostatTemperatureCalibration);
  zaLocalTemperature.setValue((int16_t) (temperature + 10 * thermostatTemperatureCalibration));
  updateThermostatState();
}

void FanControl_FanModeChanged(ZbAttribute *za, uint8_t *data)
{
  fanMode = *data;
  nvFanMode.writeInt(fanMode);
  ESP_LOGI(TAG, "FanMode=%d", fanMode);
}

void updateThermostatState()
{
  if (thermostatSystemMode) {
    //ESP_LOGI(TAG, "temperature=%d, thermostatTemperatureCalibration=%d, thermostatSetpoint=%d, relay_state=%d", temperature, thermostatTemperatureCalibration, thermostatSetpoint, relay_state);
    if (temperature + 10 * thermostatTemperatureCalibration <= thermostatSetpoint - THERMOSTAT_HYSTERESIS && !relay_state)
      setRelayState(1);
    else if (temperature + 10 * thermostatTemperatureCalibration >= thermostatSetpoint + THERMOSTAT_HYSTERESIS && relay_state)
      setRelayState(0);
  } 
  else 
    setRelayState(0);   
}

static void thermostat_task(void *arg)
{
  while (1) {
    // "измеряем" температуру
    temperature = temperature + (relay_state ? 30 : -30) + (esp_random() % 6 - 3);

    zaLocalTemperature.setValue((int16_t) (temperature + 10 * thermostatTemperatureCalibration));
    updateThermostatState();
    vTaskDelay(3000 / portTICK_PERIOD_MS);
  }
}

void thermostat_init(void)
{
//  thermostatSetpoint = 0x07D0;
//  thermostatSystemMode = 0x00;
  xTaskCreate(thermostat_task, "thermostat", 4096, NULL, 5, NULL);
}

void Device_Started(void)
{
  //ESP_LOGI(TAG, "zaLocalTemperature.type=0x%x", zaLocalTemperature.getType());
}

void loadSettings()
{
  thermostatSetpoint = nvHeatingSetpoint.readInt(thermostatSetpoint);
  zaHeatingSetpoint.setValue(thermostatSetpoint);
  thermostatSystemMode = nvSystemMode.readInt(thermostatSystemMode);
  ESP_LOGI(TAG, "thermostatSystemMode=%d", thermostatSystemMode);

  zaSystemMode.setValue(thermostatSystemMode);
  thermostatTemperatureCalibration = nvLocalTemperatureCalibration.readInt(thermostatTemperatureCalibration);
  zaLocalTemperatureCalibration.setValue(thermostatTemperatureCalibration);
  fanMode = nvFanMode.readInt(fanMode);
  zaFanMode.setValue(fanMode);
}

void setup()
{
  rt.init(zd, true);

  loadSettings();

  zd.onDeviceStarted(Device_Started);
  zaHeatingSetpoint.onValueChanged(Thermostat_HeatingSetpointChanged);
  zaSystemMode.onValueChanged(Thermostat_SystemModeChanged);
  zaLocalTemperatureCalibration.onValueChanged(Thermostat_LocalTemperatureCalibrationChanged);
  zaFanMode.onValueChanged(FanControl_FanModeChanged);

  thermostat_init();
  rt.start();
}

void loop() {
  button.tick();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}