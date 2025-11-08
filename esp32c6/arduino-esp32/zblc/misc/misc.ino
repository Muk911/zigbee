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
#include "zcl/esp_zigbee_zcl_analog_output.h"
#include "esp_random.h"
#include "esp_check.h"
#include "zb_device.h"
#include "zbesp_runtime.h"
#include "zb_crc16.h"
#include "zb_button.h"

#define TAG "zed_runtime"
extern bool connected;

#define MANUFACTURER_NAME   "MEA"
#define MODEL_NAME          "Test10"
#define ZCL_VERSION         0x03
#define APP_VERSION         0x01
#define POWER_SOURCE        0x01

#define OTA_MANUFACTURER        0x1234
#define OTA_IMAGE_TYPE          0x0010
#define OTA_FILE_VERSION        0x00000001

uint8_t relay_state = 0;
uint16_t temperature = 2000, humidity = 7000, pressure = 1000;
uint16_t servo_position = 0;

#define THERMOSTAT_HYSTERESIS   50 // 0.5°C

int16_t thermostatSetpoint = 25;
uint8_t thermostatSystemMode = 0;
int8_t thermostatTemperatureCalibration = 0;

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED); //DEVICE_TYPE_ENDDEVICE);
ZbEndpoint ep1(zd, 1, DEVICE_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);

ZbBasicCluster cb(ep1, MANUFACTURER_NAME, MODEL_NAME, ZCL_VERSION, APP_VERSION, POWER_SOURCE);

ZbIdentifyCluster ci(ep1);
//ZbAttribute ci1(ci, ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID);

ZbOnOffCluster coo(ep1);
ZbAttribute coo1(coo, ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, relay_state);
//ZbAttribute coo101(coo, ESP_ZB_ZCL_ATTR_ON_OFF_GLOBAL_SCENE_CONTROL, ESP_ZB_ZCL_ATTR_TYPE_BOOL,      ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY, NULL); //&relay_state);
//ZbAttribute *pcoo1;
/*
ZbTemperatureMeasCluster ct(ep1);
ZbAttribute ct1(ct, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, temperature);
ZbAttribute ct2(ct, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID, (int16_t) 0x8000);
ZbAttribute ct3(ct, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID, (int16_t) 0x8000);

ZbHumidityMeasCluster ch(ep1);
ZbAttribute ch1(ch, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_VALUE_ID, humidity);
ZbAttribute ch2(ch, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MIN_VALUE_ID, (int16_t) 0xffff);
ZbAttribute ch3(ch, ESP_ZB_ZCL_ATTR_REL_HUMIDITY_MEASUREMENT_MAX_VALUE_ID, (int16_t) 0xffff);

ZbPressureMeasCluster cp(ep1);
ZbAttribute cp1(cp, ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_VALUE_ID, pressure);
ZbAttribute cp2(cp, ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MIN_VALUE_ID, (int16_t) 0x8000);
ZbAttribute cp3(cp, ESP_ZB_ZCL_ATTR_PRESSURE_MEASUREMENT_MAX_VALUE_ID, (int16_t) 0x8000);

//ZbMultiValueCluster cmv(ep1);
//ZbAttributeU16 cmv1(cai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_OUT_OF_SERVICE_ID, 0);

ZbAnalogInputCluster cai(ep1);
ZbAttribute cai1(cai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_PRESENT_VALUE_ID, (float) 0);
ZbAttribute cai2(cai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_MIN_PRESENT_VALUE_ID, (float) 0);
ZbAttribute cai3(cai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_MAX_PRESENT_VALUE_ID, (float) 2500);
ZbAttribute cai4(cai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_OUT_OF_SERVICE_ID, (uint8_t) 0);
ZbAttribute cai5(cai, ESP_ZB_ZCL_ATTR_ANALOG_INPUT_STATUS_FLAGS_ID, (uint8_t) 0);

ZbAnalogOutputCluster cao(ep1);
ZbAttribute cao1(cao, ESP_ZB_ZCL_ATTR_ANALOG_OUTPUT_PRESENT_VALUE_ID, (float) 0);
ZbAttribute cao2(cao, ESP_ZB_ZCL_ATTR_ANALOG_OUTPUT_MIN_PRESENT_VALUE_ID, (float) 0);
ZbAttribute cao3(cao, ESP_ZB_ZCL_ATTR_ANALOG_OUTPUT_MAX_PRESENT_VALUE_ID, (float) 2500);
ZbAttribute cao4(cao, ESP_ZB_ZCL_ATTR_ANALOG_OUTPUT_OUT_OF_SERVICE_ID, (uint8_t) 0);
ZbAttribute cao5(cao, ESP_ZB_ZCL_ATTR_ANALOG_OUTPUT_STATUS_FLAGS_ID, (uint8_t) 0);
*/
ZbThermostatCluster cth(ep1);
ZbAttribute zaLocalTemperature(cth, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_ID, temperature);
ZbAttribute zaHeatingSetpoint(cth, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID, thermostatSetpoint);
ZbAttribute zaSystemMode(cth, ESP_ZB_ZCL_ATTR_THERMOSTAT_SYSTEM_MODE_ID, thermostatSystemMode);
ZbAttribute zaRunningState(cth, ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID, (uint8_t) 0);
ZbAttribute zaLocalTemperatureCalibration(cth, ESP_ZB_ZCL_ATTR_THERMOSTAT_LOCAL_TEMPERATURE_CALIBRATION_ID, thermostatTemperatureCalibration);
//ZbAttribute cth11(cth, ESP_ZB_ZCL_ATTR_THERMOSTAT_MIN_HEAT_SETPOINT_LIMIT_ID, (int16_t) 700);
//ZbAttribute cth12(cth, ESP_ZB_ZCL_ATTR_THERMOSTAT_MAX_HEAT_SETPOINT_LIMIT_ID, (int16_t) 3000);
/*
ZbOTACluster cota(ep1, OTA_MANUFACTURER, OTA_IMAGE_TYPE, OTA_FILE_VERSION);
*/
EspZbRuntime rt;

ZbDeviceButton button(zd, GPIO_NUM_9);

void relay_update(void)
{
    if (relay_state) {
//      ESP_LOGI(TAG, "ON");
      neopixelWrite(RGB_BUILTIN,20,20,20);
    } else {
//      ESP_LOGI(TAG, "OFF");
      neopixelWrite(RGB_BUILTIN,0,0,0);
    }
    //coo1.setValue(relay_state);
    //zaRunningState.setValue(relay_state); 
    //zaRunningState.report();
}

void servo_update(void)
{
    ESP_LOGI(TAG, "Servo position updated: %d", servo_position);
}

static void OnOff_StateChanged(ZbAttribute *za, uint8_t *data)
{
  relay_state = *data;
  relay_update(); 
}

static void AnalogOutput_StateChanged(ZbAttribute *za, uint8_t *data)
{
  servo_position = *(float *) data;
  servo_update(); 
}

static void update_measurements(void *pvParameters)
{
  while(1) {
    if (zd.connected()) {
      ESP_LOGI(TAG, "update_measurements");
      temperature = temperature + (esp_random() % 400 - 200);
      humidity = humidity + (esp_random() % 200 - 100);
      pressure = pressure + (esp_random() % 400 - 200);
      //ct1.setValue(temperature);
      //ch1.setValue(humidity);
      //cp1.setValue(pressure);
      //cth6.setValue((uint16_t)3000);
      //cth1.setValue(temperature);
      //cth2.setValue((uint16_t)2590);
 //     cth11.setValue((uint16_t)700);
 //     cth12.setValue((uint16_t)3000);
/*      
      esp_zb_zcl_status_t err = esp_zb_zcl_set_attribute_val(1, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_THERMOSTAT_OCCUPIED_HEATING_SETPOINT_ID, &cth2_v, false);
      if(err != ESP_ZB_ZCL_STATUS_SUCCESS)
        ESP_LOGE(TAG, "Setting attribute failed (%04X)!", err);
      else 
        ESP_LOGI(TAG, "Setting attribute success");
*/
      //cth3.setValue((uint8_t)4);
      //cth4.setValue((uint8_t)0);
      //cth5.setValue((uint16_t)25);
    }
    vTaskDelay(10000 / portTICK_PERIOD_MS);
  }
}

void setRelayState(uint8_t value)
{
  relay_state = value;
  relay_update();
  coo1.setValue(relay_state);
}

void button_click(void)
{
  setRelayState(relay_state ? 0 : 1);
}

void Thermostat_HeatingSetpointChanged(ZbAttribute *za, uint8_t *data)
{
    thermostatSetpoint = *(int16_t*) data;
    updateThermostatState();
}

void Thermostat_SystemModeChanged(ZbAttribute *za, uint8_t *data)
{
    thermostatSystemMode = *data;
    updateThermostatState();
}

void Thermostat_LocalTemperatureCalibrationChanged(ZbAttribute *za, uint8_t *data)
{
    thermostatTemperatureCalibration = *(int8_t*) data;
    //zaLocalTemperature.setValue((int16_t) (temperature + 10 * thermostatTemperatureCalibration));
    updateThermostatState();
}

void updateThermostatState()
{
    if (thermostatSystemMode) {
        ESP_LOGI(TAG, "temperature=%d, thermostatTemperatureCalibration=%d, thermostatSetpoint=%d, relay_state=%d", temperature, thermostatTemperatureCalibration, thermostatSetpoint, relay_state);
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
        //zaLocalTemperature.setValue((int16_t) (temperature + 10 * thermostatTemperatureCalibration));
        updateThermostatState();
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void thermostat_init(void)
{
    thermostatSetpoint = 0x07D0;
    thermostatSystemMode = 0x00;
    xTaskCreate(thermostat_task,  "thermostat",  4096, NULL, 5, NULL);
}

void setup()
{
  Serial.begin(115200);
  Serial.println("==========================================================================");

  button.attachClick(button_click);

  rt.init(zd, true);

  coo1.onValueChanged(OnOff_StateChanged);
  //cao1.onValueChanged(AnalogOutput_StateChanged);

  //zaHeatingSetpoint.setPersistent(true, "heating_setpoint");
  //zaHeatingSetpoint.onValueChanged(Thermostat_HeatingSetpointChanged);
  //zaSystemMode.setPersistent(true, "system_mode");
  //zaSystemMode.onValueChanged(Thermostat_SystemModeChanged);
  //zaLocalTemperatureCalibration.setPersistent(true, "temperature_calibration");
  //zaLocalTemperatureCalibration.onValueChanged(Thermostat_LocalTemperatureCalibrationChanged);
    
  relay_update();
 // servo_update();

//  thermostat_init();

//  xTaskCreate(update_measurements, "update_measurements", 4096, NULL, 5, NULL);
  rt.start(); //, false);
}

void loop() {
  button.tick();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}

/*
static char manufacturer[32], model[32];
static uint8_t zcl_version = ZCL_VERSION, app_version = APP_VERSION, power_source = POWER_SOURCE;

bool connected = false;

//********************* Zigbee functions **************************
static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p       = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = (esp_zb_app_signal_type_t)*p_sg_p;

    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Zigbee stack initialized");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
 
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, "Start network steering");
            esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
        } else {
            // commissioning failed
            log_w("Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;

    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            connected = true;
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        } else {
            connected = false;
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));
        break;
    }
}

// Handle the attributes
static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
    bool light_state = 0;

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)", message->info.status);
//    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.size);

    set_attr_value_cb(message->info.status, message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.value);
    return ret;
}

static esp_err_t zb_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);
    ESP_LOGI(TAG, "Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)", message->info.status,
             message->info.cluster, message->variables->attribute.id, message->variables->attribute.data.type,
             message->variables->attribute.data.value ? *(uint8_t *)message->variables->attribute.data.value : 0);
             
    if (message->info.dst_endpoint == DEVICE_ENDPOINT) {
        switch (message->info.cluster) {
        case ESP_ZB_ZCL_CLUSTER_ID_TIME:
            ESP_LOGI(TAG, "Server time recieved %lu", *(uint32_t*) message->variables->attribute.data.value);
            struct timeval tv;
            tv.tv_sec = *(uint32_t*) message->variables->attribute.data.value + 946684800 - 1080; //after adding OTA cluster time shifted to 1080 sec... strange issue ... 
            //settimeofday(&tv, NULL);
            //time_updated = true;
            break;
        default:
            ESP_LOGI(TAG, "Message data: cluster(0x%x), attribute(0x%x)  ", message->info.cluster, message->variables->attribute.id);
        }
    }
    return ESP_OK;
}

static esp_err_t zb_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;

    switch (callback_id) {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
        ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
        ret = zb_read_attr_resp_handler((esp_zb_zcl_cmd_read_attr_resp_message_t *)message);
        break;
    default:
        //log_w("Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

static void set_attr_value_cb(uint8_t status, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id, void *data)
{
    switch (cluster_id) {
        case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
            switch (attr_id) {
                case ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID:
                    relay_state = *(uint8_t*) data;
                    relay_update();
                    break;
            }
            break;
    }
    //ESP_LOGI(TAG, "cluster 0x%04X attribute 0x%04X value updated", cluster_id, attr_id);
}

static void reportAttribute(uint8_t endpoint, uint16_t clusterID, uint16_t attributeID, void *value, uint8_t value_length)
{
    esp_zb_zcl_report_attr_cmd_t cmd;
    cmd.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;
    cmd.zcl_basic_cmd.dst_endpoint = endpoint;
    cmd.zcl_basic_cmd.src_endpoint = endpoint;
    cmd.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    cmd.clusterID = clusterID;
    cmd.attributeID = attributeID;
    cmd.cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    esp_zb_zcl_attr_t *value_r = esp_zb_zcl_get_attribute(endpoint, clusterID, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, attributeID);
    memcpy(value_r->data_p, value, value_length);
    esp_zb_zcl_report_attr_cmd_req(&cmd);
}

static void update_relay(void *pvParameters)
{
  while(1) {
    if (connected) {
      temperature = temperature + (esp_random() % 200 - 100);
      esp_zb_zcl_status_t state_tmp = esp_zb_zcl_set_attribute_val(DEVICE_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &temperature, false);
      if(state_tmp != ESP_ZB_ZCL_STATUS_SUCCESS) {
        ESP_LOGE(TAG, "Setting temperature attribute failed!");
      }
      else ESP_LOGI(TAG, "Setting temperature attribute success (%d).", temperature);
    }
    vTaskDelay(5000 / portTICK_PERIOD_MS);
  }
}

static void set_zcl_string(char *buffer, char *value)
{
    buffer[0] = (char) strlen(value);
    memcpy(buffer + 1, value, buffer[0]);
}

static void esp_zb_task(void *pvParameters)
{
    esp_zb_cfg_t zigbee_config;
    memset(&zigbee_config, 0, sizeof(zigbee_config));
    zigbee_config.esp_zb_role = ESP_ZB_DEVICE_TYPE_ED; 
    zigbee_config.install_code_policy = false;
    zigbee_config.nwk_cfg.zczr_cfg.max_children = 16;

    uint16_t undefined_value;
    undefined_value = 0x8000;   
    set_zcl_string(manufacturer, MANUFACTURER_NAME);
    set_zcl_string(model, MODEL_NAME);

    // Basic cluster
    esp_zb_attribute_list_t *attr_list_basic = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_BASIC);
    esp_zb_basic_cluster_add_attr(attr_list_basic, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, &zcl_version);
    esp_zb_basic_cluster_add_attr(attr_list_basic, ESP_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID, &app_version);
    esp_zb_basic_cluster_add_attr(attr_list_basic, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, manufacturer);
    esp_zb_basic_cluster_add_attr(attr_list_basic, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, model);
    esp_zb_basic_cluster_add_attr(attr_list_basic, ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, &power_source);
    
    // Identify cluster
    uint8_t identyfi_id;
    identyfi_id = 0;
    esp_zb_attribute_list_t *attr_list_identify = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY);
    esp_zb_identify_cluster_add_attr(attr_list_identify, ESP_ZB_ZCL_CMD_IDENTIFY_IDENTIFY_ID, &identyfi_id);

    // OnOff cluster
    esp_zb_attribute_list_t *attr_list_on_off = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
    esp_zb_on_off_cluster_add_attr(attr_list_on_off, ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, &relay_state);

    // Temperature cluster
    esp_zb_attribute_list_t *attr_list_temperature_meas = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT);
    esp_zb_temperature_meas_cluster_add_attr(attr_list_temperature_meas, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &undefined_value);
    esp_zb_temperature_meas_cluster_add_attr(attr_list_temperature_meas, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MIN_VALUE_ID, &undefined_value);
    esp_zb_temperature_meas_cluster_add_attr(attr_list_temperature_meas, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_MAX_VALUE_ID, &undefined_value);

    esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();

    esp_zb_cluster_list_add_basic_cluster(cluster_list, attr_list_basic, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_identify_cluster(cluster_list, attr_list_identify, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_on_off_cluster(cluster_list, attr_list_on_off, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_cluster_list_add_temperature_meas_cluster(cluster_list, attr_list_temperature_meas, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE);
    esp_zb_ep_list_add_ep(ep_list, cluster_list, DEVICE_ENDPOINT, DEVICE_PROFILE_ID, DEVICE_ID);

    esp_zb_init(&zigbee_config);
    esp_zb_device_register(ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK);

    //esp_zb_nvram_erase_at_start(true); // Comment out this line to erase NVRAM data if you are connecting to new Coordinator
    
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_main_loop_iteration();
}

void setup() {
    esp_zb_platform_config_t platform_config;
    memset(&platform_config, 0, sizeof(platform_config));
    platform_config.radio_config.radio_mode = RADIO_MODE_NATIVE;
    platform_config.host_config.host_connection_mode = HOST_CONNECTION_MODE_NONE;
  
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&platform_config));
    relay_init();

    xTaskCreate(update_relay, "update_relay", 4096, NULL, 5, NULL);
    xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}

void loop() {
}
*/


