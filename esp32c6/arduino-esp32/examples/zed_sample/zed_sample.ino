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

static const char *TAG = "zigbee";

#define MANUFACTURER_NAME   "MEA"
#define MODEL_NAME          "thp.sensor"
#define ZCL_VERSION         0x03
#define APP_VERSION         0x01
#define POWER_SOURCE        0x04

#define DEVICE_ENDPOINT     0x01
#define DEVICE_PROFILE_ID   ESP_ZB_AF_HA_PROFILE_ID
#define DEVICE_ID           ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID

static char manufacturer[32], model[32];
static uint8_t zcl_version = ZCL_VERSION, app_version = APP_VERSION, power_source = POWER_SOURCE;
uint16_t relay_status = 0, temperature = 2500;
bool connected = false;

void relay_init(void)
{
    relay_status = 0;
    relay_update();
}

void relay_update(void)
{
    if (relay_status) {
      ESP_LOGI(TAG, "ON");
      rgbLedWrite(RGB_BUILTIN,20,20,20);
    } else {
      ESP_LOGI(TAG, "OFF");
      rgbLedWrite(RGB_BUILTIN,0,0,0);
    }
}

/********************* Zigbee functions **************************/
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
                    relay_status = *(uint8_t*) data;
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
    //cmd.cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    esp_zb_zcl_attr_t *value_r = esp_zb_zcl_get_attribute(endpoint, clusterID, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, attributeID);
    memcpy(value_r->data_p, value, value_length);
    esp_zb_zcl_report_attr_cmd_req(&cmd);
}

static void update_relay(void *pvParameters)
{
  while(1) {
    if (connected) {
      temperature = temperature + (esp_random() % 200 - 100);
      esp_zb_lock_acquire(portMAX_DELAY);
      esp_zb_zcl_status_t state_tmp = esp_zb_zcl_set_attribute_val(DEVICE_ENDPOINT, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, ESP_ZB_ZCL_ATTR_TEMP_MEASUREMENT_VALUE_ID, &temperature, false);
      esp_zb_lock_release();
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

esp_err_t esp_zb_ep_list_add_ep_(esp_zb_ep_list_t *ep_list, esp_zb_cluster_list_t *cluster_list, uint8_t endpoint, uint16_t app_profile_id, uint16_t app_device_id)
{
    esp_zb_endpoint_config_t endpoint_config;

    endpoint_config.endpoint = endpoint;
    endpoint_config.app_profile_id = app_profile_id;
    endpoint_config.app_device_id = app_device_id;
    endpoint_config.app_device_version = 1;
    return esp_zb_ep_list_add_ep(ep_list, cluster_list, endpoint_config);
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
    esp_zb_identify_cluster_add_attr(attr_list_identify, ESP_ZB_ZCL_ATTR_IDENTIFY_IDENTIFY_TIME_ID, &identyfi_id);

    // OnOff cluster
    esp_zb_attribute_list_t *attr_list_on_off = esp_zb_zcl_attr_list_create(ESP_ZB_ZCL_CLUSTER_ID_ON_OFF);
    esp_zb_on_off_cluster_add_attr(attr_list_on_off, ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, &relay_status);

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
    esp_zb_ep_list_add_ep_(ep_list, cluster_list, DEVICE_ENDPOINT, DEVICE_PROFILE_ID, DEVICE_ID);

    esp_zb_init(&zigbee_config);
    esp_zb_device_register(ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK);

  //  esp_zb_nvram_erase_at_start(true); // Comment out this line to erase NVRAM data if you are connecting to new Coordinator
    
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_main_loop_iteration();
}

void setup() {
    Serial.begin(115200);
    Serial.println("-------------------------------------------------------------------------");

    esp_zb_platform_config_t platform_config;
    memset(&platform_config, 0, sizeof(platform_config));
    platform_config.radio_config.radio_mode = ZB_RADIO_MODE_NATIVE;
    platform_config.host_config.host_connection_mode = ZB_HOST_CONNECTION_MODE_NONE;
  
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_zb_platform_config(&platform_config));
    relay_init();

    xTaskCreate(update_relay, "update_relay", 4096, NULL, 5, NULL);
    xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}

void loop() {
}
