/*
esp_zb_get_short_address() - быстрый способ идентификации сети, настроенной при запуске. Если сеть не настроена, возвращаемое значение будет 0xfffe.

https://github.com/espressif/esp-zigbee-sdk/issues/84
*/

#include <Arduino.h>
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_zigbee_type.h"
#include "esp_zigbee_core.h"
#include "esp_ota_ops.h"
#include "nvs_flash.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl/esp_zigbee_zcl_command.h"
#include "zdo/esp_zigbee_zdo_command.h"
//#include "zboss_api_zdo.h"
#include "nvs_flash.h"
#include "zb_crc16.h"
#include "zb_zcl.h"
#include "zbesp_runtime.h"
#include "zbesp_debug.h"

#define TAG "zbesp_runtime"

ZbDevice *g_device;

/*
static void set_zcl_string(char *buffer, char *value)
{
    buffer[0] = (char) strlen(value);
    memcpy(buffer + 1, value, buffer[0]);
}
*/

static void bdb_start_top_level_commissioning_cb(uint8_t mode_mask)
{
    ESP_ERROR_CHECK(esp_zb_bdb_start_top_level_commissioning(mode_mask));
}

// https://github.com/espressif/esp-zigbee-sdk/blob/main/examples/esp_zigbee_HA_sample/HA_color_dimmable_light/main/esp_zb_light.c
// https://github.com/espressif/esp-idf/blob/master/examples/zigbee/esp_zigbee_gateway/main/esp_zigbee_gateway.c
void esp_zb_app_signal_handler(esp_zb_app_signal_t *signal_struct)
{
    uint32_t *p_sg_p = signal_struct->p_app_signal;
    esp_err_t err_status = signal_struct->esp_err_status;
    esp_zb_app_signal_type_t sig_type = (esp_zb_app_signal_type_t)*p_sg_p;
    esp_zb_zdo_signal_leave_params_t *leave_params;

    ESP_LOGI(TAG, "ZDO signal: %s (sig_type=0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));

    switch (sig_type) {
    case ESP_ZB_ZDO_SIGNAL_SKIP_STARTUP:
        ESP_LOGI(TAG, "Initialize Zigbee stack");
        esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_INITIALIZATION);
        break;
    case ESP_ZB_BDB_SIGNAL_DEVICE_FIRST_START:
    case ESP_ZB_BDB_SIGNAL_DEVICE_REBOOT:
        if (err_status == ESP_OK) {
            ESP_LOGI(TAG, ">");
            if(g_device->p_deferredInitCB) 
                ESP_LOGI(TAG, "Deferred initialization %s", g_device->p_deferredInitCB() ? "failed." : "successful.");
            ESP_LOGI(TAG, "Device started up in %sfactory-reset mode", esp_zb_bdb_is_factory_new() ? "" : "NON ");
            if (esp_zb_bdb_is_factory_new()) {
                ESP_LOGI(TAG, "Start network steering");
                esp_zb_bdb_start_top_level_commissioning(ESP_ZB_BDB_MODE_NETWORK_STEERING);
            } else {
                g_device->setJoined(true); // перезагрузка, считаем, что устройство само войдет в сеть
                ESP_LOGI(TAG, "Device was rebooted");
            }
        } else {
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s), try to factory reset.", esp_err_to_name(err_status));
        }
        break;
    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            g_device->setJoined(true);
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        } else {
            g_device->setJoined(false);
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    case ESP_ZB_NWK_SIGNAL_PERMIT_JOIN_STATUS:
        if (err_status == ESP_OK) {
            if (*(uint8_t *)esp_zb_app_signal_get_params(p_sg_p)) {
                ESP_LOGI(TAG, "Network(0x%04hx) is open for %d seconds", esp_zb_get_pan_id(), *(uint8_t *)esp_zb_app_signal_get_params(p_sg_p));
            } else {
                ESP_LOGW(TAG, "Network(0x%04hx) closed, devices joining not allowed.", esp_zb_get_pan_id());
            }
        }
        break;

        //https://github.com/otaviojr/zigbee_light_sensor/blob/master/main.c
        //https://github.com/espressif/esp-zigbee-sdk/issues/66

    case ESP_ZB_ZDO_SIGNAL_LEAVE:
        leave_params = (esp_zb_zdo_signal_leave_params_t *) esp_zb_app_signal_get_params(p_sg_p);
        //ESP_LOGI(TAG, "****** ESP_ZB_ZDO_SIGNAL_LEAVE (leave_type=%d)", leave_params->leave_type);
        g_device->setJoined(false);
        if (leave_params->leave_type == ESP_ZB_NWK_LEAVE_TYPE_RESET) {
            ESP_LOGI(TAG, "Reset device");
            esp_restart(); // вынести в настройки устройства
        }
        break;

    default:
        //ESP_LOGI(TAG, "ZDO signal: %s (sig_type=0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));
        break;
    }
}

static void set_attr_value_cb(uint8_t status, uint8_t endpoint, uint16_t cluster_id, uint16_t attr_id, void *data)
{
    //ESP_LOGI(TAG, "set_attr_value_cb()");

    ZbEndpoint *ep = g_device->findEndpoint(endpoint);
    if(!ep) {
      ESP_LOGI(TAG, "Endpoint %d not found.", endpoint);
      return;
    }
    ZbCluster *cluster = ep->findCluster(cluster_id);
    if(!cluster) {
      ESP_LOGI(TAG, "Endpoint %d cluster 0x%04X not found.", cluster_id);
      return;
    }
    ZbAttribute *attr = cluster->findAttribute(attr_id);
    if(!attr) {
      ESP_LOGI(TAG, "Endpoint %d cluster 0x%04X Attribute 0x%04X not found.", attr_id);
      return;
    }
    attr->valueChanged((uint8_t *)data);

/*
    switch (cluster_id) {
        case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
            switch (attr_id) {
                case ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID:
//                    relay_status = *(uint8_t*) data;
//                    relay_update();
                    break;
            }
            break;
    }
*/
    ESP_LOGI(TAG, "endpoint %d cluster 0x%04X attribute 0x%04X value updated %d", endpoint, cluster_id, attr_id, *(uint8_t*)data);
}

// Handle the attributes
static esp_err_t zb_set_attr_value_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
 
    //ESP_LOGI(TAG, "zb_set_attr_value_handler()");

    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)", message->info.status);
//    ESP_LOGI(TAG, "Received message: endpoint(%d), cluster(0x%x), attribute(0x%x), data size(%d)", message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.size);

    set_attr_value_cb(message->info.status, message->info.dst_endpoint, message->info.cluster, message->attribute.id, message->attribute.data.value);
    return ret;
}

/*
static esp_err_t zb_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);

    ESP_LOGI(TAG, "Read attribute response: from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)",
             message->info.src_address.u.short_addr, message->info.src_endpoint,
             message->info.dst_endpoint, message->info.cluster);

    esp_zb_zcl_read_attr_resp_variable_t *variable = message->variables;
    while (variable) {
        ESP_LOGI(TAG, "Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)",
                    variable->status, message->info.cluster,
                    variable->attribute.id, variable->attribute.data.type,
                    variable->attribute.data.value ? *(uint8_t *)variable->attribute.data.value : 0);
        if (variable->status == ESP_ZB_ZCL_STATUS_SUCCESS) {
            esp_app_zb_set_attr_value_handler(message->info.cluster, &variable->attribute);
        }

        variable = variable->next;
    }

    return ESP_OK;
}
*/

static esp_err_t zb_read_attr_resp_handler(const esp_zb_zcl_cmd_read_attr_resp_message_t *message)
{
    ESP_LOGI(TAG, "zb_read_attr_resp_handler()");
    
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);
    ESP_LOGI(TAG, "Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)", message->info.status,
             message->info.cluster, message->variables->attribute.id, message->variables->attribute.data.type,
             message->variables->attribute.data.value ? *(uint8_t *)message->variables->attribute.data.value : 0);

    ZbEndpoint *ep = g_device->findEndpoint(message->info.dst_endpoint);
    assert(ep);

    esp_zb_zcl_read_attr_resp_variable_t *variable = message->variables;
    while (variable) {
        ESP_LOGI(TAG, "Read attribute response: status(%d), cluster(0x%x), attribute(0x%x), type(0x%x), value(%d)",
                    variable->status, message->info.cluster,
                    variable->attribute.id, variable->attribute.data.type,
                    variable->attribute.data.value ? *(uint8_t *)variable->attribute.data.value : 0);

//        if (variable->status == ESP_ZB_ZCL_STATUS_SUCCESS) {
//            esp_app_zb_set_attr_value_handler(message->info.cluster, &variable->attribute);
//        }

        switch (message->info.cluster) {
        case ESP_ZB_ZCL_CLUSTER_ID_TIME:
            ESP_LOGI(TAG, "Server time received %lu", *(uint32_t*) variable->attribute.data.value);
            struct timeval tv;
            tv.tv_sec = *(uint32_t*) variable->attribute.data.value + 946684800 - 1080; //after adding OTA cluster time shifted to 1080 sec... strange issue ... 
            //settimeofday(&tv, NULL);
            //time_updated = true;
            break;
        default:
            ESP_LOGI(TAG, "Message data: cluster(0x%x), attribute(0x%x)  ", message->info.cluster, variable->attribute.id);
        }
        variable = variable->next;
    }
    return ESP_OK;
}

static const esp_partition_t *ota_partition = NULL;
static esp_ota_handle_t ota_handle = 0;
static uint32_t ota_size = 0, ota_offset = 0;

static esp_err_t ota_handler(esp_zb_zcl_ota_upgrade_value_message_t messsage)
{
    esp_err_t result = ESP_OK;

    if (messsage.info.status != ESP_ZB_ZCL_STATUS_SUCCESS)
        return ESP_FAIL;

    switch (messsage.upgrade_status)
    {
        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_START:
            ESP_LOGI(TAG, "OTA upgrade started");
            ota_partition = esp_ota_get_next_update_partition(NULL);
            if ((result = esp_ota_begin(ota_partition, OTA_WITH_SEQUENTIAL_WRITES, &ota_handle)) != ESP_OK)
                ESP_LOGE(TAG, "OTA uprage begin failed, status: %s", esp_err_to_name(result));
            break;

        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_APPLY:
            ESP_LOGI(TAG, "OTA upgrade apply");
            break;

        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_RECEIVE:
            ota_size = messsage.ota_header.image_size;
            ota_offset += messsage.payload_size;
            ESP_LOGI(TAG, "OTA upgrade received %06ld/%ld bytes", ota_offset, ota_size);
            if (messsage.payload_size && messsage.payload && (result = esp_ota_write(ota_handle, (const void*) messsage.payload, messsage.payload_size)) != ESP_OK)
                ESP_LOGE(TAG, "OTA uprage write failed, status: %s", esp_err_to_name(result));
            break;

        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_FINISH:
            if ((result = esp_ota_end(ota_handle)) != ESP_OK)
                ESP_LOGE(TAG, "OTA uprage end failed, status: %s", esp_err_to_name(result));
            if ((result = esp_ota_set_boot_partition(ota_partition)) != ESP_OK)
                ESP_LOGE(TAG, "OTA uprage set boot partition failed, status: %s", esp_err_to_name(result));
            ESP_LOGI(TAG, "OTA uprage finished: version: 0x%lx, manufacturer code: 0x%x, image type: 0x%x, total size: %ld", messsage.ota_header.file_version, messsage.ota_header.manufacturer_code, messsage.ota_header.image_type, messsage.ota_header.image_size);
            esp_restart();
            break;

        case ESP_ZB_ZCL_OTA_UPGRADE_STATUS_CHECK:
            if (ota_offset != ota_size) {
                ESP_LOGE(TAG, "OTA uprage check failed");
                return ESP_FAIL;
            }
            break;

        default:
            ESP_LOGI(TAG, "OTA uprage status: 0x%04x", messsage.upgrade_status);
            break;
    }
    return result;
}

/*
static esp_err_t zb_attribute_reporting_handler(const esp_zb_zcl_report_attr_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->status);
    ESP_LOGI(TAG, "Received report from address(0x%x) src endpoint(%d) to dst endpoint(%d) cluster(0x%x)",
             message->src_address.u.short_addr, message->src_endpoint,
             message->dst_endpoint, message->cluster);
    esp_app_zb_set_attr_value_handler(message->cluster, &message->attribute);
    return ESP_OK;
}

static esp_err_t zb_configure_report_resp_handler(const esp_zb_zcl_cmd_config_report_resp_message_t *message)
{
    ESP_RETURN_ON_FALSE(message, ESP_FAIL, TAG, "Empty message");
    ESP_RETURN_ON_FALSE(message->info.status == ESP_ZB_ZCL_STATUS_SUCCESS, ESP_ERR_INVALID_ARG, TAG, "Received message: error status(%d)",
                        message->info.status);

    esp_zb_zcl_config_report_resp_variable_t *variable = message->variables;
    while (variable) {
        ESP_LOGI(TAG, "Configure report response: status(%d), cluster(0x%x), direction(0x%x), attribute(0x%x)",
                 variable->status, message->info.cluster, variable->direction, variable->attribute_id);
        variable = variable->next;
    }

    return ESP_OK;
}
*/

static esp_err_t zb_core_action_handler(esp_zb_core_action_callback_id_t callback_id, const void *message)
{
    esp_err_t ret = ESP_OK;

    ESP_LOGI(TAG, "Receive Zigbee action (0x%x - %s) callback", callback_id, get_core_action_callback_name(callback_id));
    switch (callback_id) {
    //case ESP_ZB_CORE_REPORT_ATTR_CB_ID:  // Attribute Report - для серверных кластеров?
        //ret = zb_attribute_reporting_handler((esp_zb_zcl_report_attr_message_t *)message);
        //break;
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID: // Set attribute value
        ESP_LOGI(TAG, "ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID");
        ret = zb_set_attr_value_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID: // Read attribute response
        ESP_LOGI(TAG, "ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID");
        ret = zb_read_attr_resp_handler((esp_zb_zcl_cmd_read_attr_resp_message_t *) message);
        break;
    //case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID: // Configure report response
      //ret = zb_configure_report_resp_handler((esp_zb_zcl_cmd_config_report_resp_message_t *)message);
      //  break;
    case ESP_ZB_CORE_OTA_UPGRADE_VALUE_CB_ID: // Upgrade OTA
        return ota_handler(*(esp_zb_zcl_ota_upgrade_value_message_t*) message);  
    default:
        //ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

esp_err_t zbesp_cluster_list_add_cluster(esp_zb_cluster_list_t *cluster_list, uint16_t cluster_id, esp_zb_attribute_list_t *attr_list, uint8_t role_mask)
{
  switch(cluster_id) {
    case ESP_ZB_ZCL_CLUSTER_ID_BASIC:
      //ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster ESP_ZB_ZCL_CLUSTER_ID_BASIC");
      return esp_zb_cluster_list_add_basic_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG:
      return esp_zb_cluster_list_add_power_config_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_DEVICE_TEMP_CONFIG:
    case ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY:
      //ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY");
      return esp_zb_cluster_list_add_identify_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_GROUPS:
      return esp_zb_cluster_list_add_groups_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_SCENES:
      return esp_zb_cluster_list_add_scenes_cluster(cluster_list, attr_list, role_mask);
#ifdef USE_ON_OFF_CLUSTER
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
      return esp_zb_cluster_list_add_on_off_cluster(cluster_list, attr_list, role_mask);
#endif // USE_ON_OFF_CLUSTER
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF_SWITCH_CONFIG:
      return esp_zb_cluster_list_add_on_off_switch_config_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:
      return esp_zb_cluster_list_add_level_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_ALARMS:
    case ESP_ZB_ZCL_CLUSTER_ID_TIME:
      return esp_zb_cluster_list_add_time_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_RSSI_LOCATION:
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT:
      return esp_zb_cluster_list_add_analog_input_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT:
      return esp_zb_cluster_list_add_analog_output_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE:
      return esp_zb_cluster_list_add_analog_value_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_INPUT:
      return esp_zb_cluster_list_add_binary_input_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_BINARY_OUTPUT:
//    ESP_ZB_ZCL_CLUSTER_ID_BINARY_VALUE:
//    ESP_ZB_ZCL_CLUSTER_ID_MULTI_INPUT:
//    ESP_ZB_ZCL_CLUSTER_ID_MULTI_OUTPUT:
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_VALUE:
      return esp_zb_cluster_list_add_multistate_value_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_COMMISSIONING:
    case ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE:
      return esp_zb_cluster_list_add_ota_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_POLL_CONTROL:
//    ESP_ZB_ZCL_CLUSTER_ID_GREEN_POWER:    
//    ESP_ZB_ZCL_CLUSTER_ID_KEEP_ALIVE:    
    case ESP_ZB_ZCL_CLUSTER_ID_SHADE_CONFIG:
      return esp_zb_cluster_list_add_shade_config_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_DOOR_LOCK:
      return esp_zb_cluster_list_add_door_lock_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_WINDOW_COVERING:
      return esp_zb_cluster_list_add_window_covering_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_PUMP_CONFIG_CONTROL:
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT:
      return esp_zb_cluster_list_add_thermostat_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_FAN_CONTROL:
      return esp_zb_cluster_list_add_fan_control_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_DEHUMID_CONTROL:     
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG:
      esp_zb_cluster_list_add_thermostat_ui_config_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL:
      return esp_zb_cluster_list_add_color_control_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_BALLAST_CONFIG:
    case ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT:
      return esp_zb_cluster_list_add_illuminance_meas_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT:
      //ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT");
      return esp_zb_cluster_list_add_temperature_meas_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT:
      return esp_zb_cluster_list_add_pressure_meas_cluster(cluster_list, attr_list, role_mask);
      break;      
    case ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT:
      return esp_zb_cluster_list_add_humidity_meas_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING:
      return esp_zb_cluster_list_add_occupancy_sensing_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE_MEASUREMENT:
      return esp_zb_cluster_list_add_carbon_dioxide_measurement_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_PM2_5_MEASUREMENT:
      return esp_zb_cluster_list_add_pm2_5_measurement_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE:
      return esp_zb_cluster_list_add_ias_zone_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT:
      return esp_zb_cluster_list_add_electrical_meas_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_METERING:
      return esp_zb_cluster_list_add_metering_cluster(cluster_list, attr_list, role_mask);
  }
}

uint16_t attr_crc;

// esp_err_t zbesp_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t cluster_id, uint16_t attr_id, uint8_t attr_type, uint8_t attr_access, void *value_p)
esp_err_t zbesp_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t cluster_id, uint16_t attr_id, void *value_p) 
{ 
  crc16_update(&attr_crc, (uint8_t *)&attr_id, sizeof(attr_id));
  switch(cluster_id) {
    case ESP_ZB_ZCL_CLUSTER_ID_BASIC:
      //ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_BASIC");
      return esp_zb_basic_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG:
      return esp_zb_power_config_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_DEVICE_TEMP_CONFIG:
    case ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY:
      //ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY");
      return esp_zb_identify_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_GROUPS:
      return esp_zb_groups_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_SCENES:
      return esp_zb_scenes_cluster_add_attr(attr_list, attr_id, value_p);
#ifdef USE_ON_OFF_CLUSTER
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
      //ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_ON_OFF");
      return esp_zb_on_off_cluster_add_attr(attr_list, attr_id, value_p);
#endif // USE_ON_OFF_CLUSTER
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF_SWITCH_CONFIG:
      return esp_zb_on_off_switch_config_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL:
      return esp_zb_level_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_ALARMS:
    case ESP_ZB_ZCL_CLUSTER_ID_TIME:
      return esp_zb_time_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_RSSI_LOCATION:
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT:
      return esp_zb_analog_input_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT:
      return esp_zb_analog_output_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_ANALOG_VALUE:
      return esp_zb_analog_value_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_BINARY_INPUT:
      return esp_zb_binary_input_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_BINARY_OUTPUT:
//    ESP_ZB_ZCL_CLUSTER_ID_BINARY_VALUE:
//    ESP_ZB_ZCL_CLUSTER_ID_MULTI_INPUT:
//    ESP_ZB_ZCL_CLUSTER_ID_MULTI_OUTPUT:
    case ESP_ZB_ZCL_CLUSTER_ID_MULTI_VALUE:
      return esp_zb_multistate_value_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_COMMISSIONING:
    case ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE:
      return esp_zb_ota_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_POLL_CONTROL:
//    ESP_ZB_ZCL_CLUSTER_ID_GREEN_POWER:    
//    ESP_ZB_ZCL_CLUSTER_ID_KEEP_ALIVE:    
    case ESP_ZB_ZCL_CLUSTER_ID_SHADE_CONFIG:
      return esp_zb_shade_config_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_DOOR_LOCK:
      return esp_zb_door_lock_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_WINDOW_COVERING:
      return esp_zb_window_covering_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_PUMP_CONFIG_CONTROL:
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT:
      if(attr_id == ESP_ZB_ZCL_ATTR_THERMOSTAT_THERMOSTAT_RUNNING_STATE_ID) // делаем reportable
        return esp_zb_cluster_add_attr(attr_list, cluster_id, attr_id, ESP_ZB_ZCL_ATTR_TYPE_16BITMAP, ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING, value_p);
      else
        return esp_zb_thermostat_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_FAN_CONTROL:
      return esp_zb_fan_control_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_DEHUMID_CONTROL:     
    case ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT_UI_CONFIG:
      return esp_zb_thermostat_ui_config_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_COLOR_CONTROL:
      return esp_zb_color_control_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_BALLAST_CONFIG:
    case ESP_ZB_ZCL_CLUSTER_ID_ILLUMINANCE_MEASUREMENT:
      return esp_zb_illuminance_meas_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT:
      //ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT");
      return esp_zb_temperature_meas_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT:
      return esp_zb_pressure_meas_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT:
      return esp_zb_humidity_meas_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_OCCUPANCY_SENSING:
      return esp_zb_occupancy_sensing_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_CARBON_DIOXIDE_MEASUREMENT:
      return esp_zb_carbon_dioxide_measurement_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_PM2_5_MEASUREMENT:
      return esp_zb_pm2_5_measurement_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_IAS_ZONE:
      return esp_zb_ias_zone_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_ELECTRICAL_MEASUREMENT:
      return esp_zb_electrical_meas_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_METERING:
  }
}

void eraseIfModified(void)
{
    nvs_handle_t nvs_handle;
    uint16_t prev_attr_crc = 0;
    size_t size = sizeof(prev_attr_crc);

    nvs_open("nvs", NVS_READWRITE, &nvs_handle);
    nvs_get_u16(nvs_handle, "attr_crc", &prev_attr_crc);
    if(attr_crc != prev_attr_crc) {
      nvs_set_u16(nvs_handle, "attr_crc", prev_attr_crc);
      nvs_commit(nvs_handle);
      nvs_close(nvs_handle);
      esp_zb_nvram_erase_at_start(true); // Comment out this line to erase NVRAM data if you are connecting to new Coordinator. Включите или отключите очистку NVRAM при каждом запуске приложения. По умолчанию стирание отключено.
    }
    else
      nvs_close(nvs_handle);
}

static esp_zb_ep_list_t *create_ep_list()
{
    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();

    //crc16_init(&attr_crc);

    for(int iEndpoint=0; iEndpoint < g_device->getEndpointCount(); iEndpoint++) {
      ZbEndpoint *ep = g_device->getEndpoint(iEndpoint);
      esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
//      ESP_LOGI(TAG, "ClusterCount=%d", ep->getClusterCount());
      for(int iCluster=0; iCluster < ep->getClusterCount(); iCluster++) {
        ZbCluster *cluster = ep->getCluster(iCluster);
        esp_zb_attribute_list_t *attr_list;
        if(cluster->getId() == ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE) {
          ZbOTACluster *otaCluster = (ZbOTACluster*) cluster;
          esp_zb_ota_cluster_cfg_t ota_config;
          memset(&ota_config, 0, sizeof(ota_config));
          ota_config.ota_upgrade_manufacturer = otaCluster->getOtaManufacturer();
          ota_config.ota_upgrade_image_type = otaCluster->getOtaImageType();
          ota_config.ota_upgrade_downloaded_file_ver = otaCluster->getOtaFileVersion();
          attr_list = esp_zb_ota_cluster_create(&ota_config);
        }
        else
          attr_list = esp_zb_zcl_attr_list_create(cluster->getId());
//        ESP_LOGI(TAG, "ClusterId %04X", cluster->getId()); 
        //ESP_LOGI(TAG, "getAttributeCount=%d", cluster->getAttributeCount());
        for(int iAttribute=0; iAttribute < cluster->getAttributeCount(); iAttribute++) {
          ZbAttribute *attr = cluster->getAttribute(iAttribute);
          //ESP_LOGI(TAG, "zbesp_cluster_add_attr(, %d, %d, )", cluster->getId(), attr->getId());
          if (attr->isCustom()) 
            esp_zb_custom_cluster_add_custom_attr(attr_list, attr->getId(), attr->getType(), attr->getAccess(), attr->getData());
              //esp_zb_attribute_list_t *attr_list, uint16_t attr_id, uint8_t attr_type, uint8_t attr_access, void *value_p);\
          else
            zbesp_cluster_add_attr(attr_list, cluster->getId(), attr->getId(), attr->getData());
        }
        zbesp_cluster_list_add_cluster(cluster_list, cluster->getId(), attr_list, cluster->getRole());
        //ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster()");
      }
      esp_zb_ep_list_add_ep(ep_list, cluster_list, ep->getId(), ep->getProfileId(), ep->getDeviceId());
      //ESP_LOGI(TAG, "esp_zb_ep_list_add_ep()");
    }
    return ep_list;
}

static void esp_zb_task(void *pvParameters)
{
    //ESP_LOGI(TAG, "esp_zb_task()");
    uint16_t undefined_value = 0x8000;

    assert(g_device);
    esp_zb_cfg_t zigbee_config;
    memset(&zigbee_config, 0, sizeof(zigbee_config));
    zigbee_config.esp_zb_role = (esp_zb_nwk_device_type_t) g_device->getDeviceType();
    zigbee_config.install_code_policy = g_device->getInstallCodePolicy();
    zigbee_config.nwk_cfg.zczr_cfg.max_children = g_device->getMaxChildren();
    esp_zb_init(&zigbee_config);
    
    esp_zb_ep_list_t *ep_list = create_ep_list();
    esp_zb_device_register(ep_list);
    //dumpEndpointList(ep_list);
    esp_zb_core_action_handler_register(zb_core_action_handler);
    esp_zb_set_primary_network_channel_set(g_device->getChannelMask()); // AFTER esp_zb_init, BEFORE esp_zb_start

    nvs_flash_init();
    //esp_zb_nvram_erase_at_start(true);

  //  eraseIfModified();
    ESP_ERROR_CHECK(esp_zb_start(false)); 
      // NVRAM и конфигурация продукта будут загружены после вызова esp_zb_start()
      // autostart=true - после завершения запуска вызывается esp_zb_app_signal_handler. Пользователь может использовать esp_zb_bdb_start_top_level_commissioning для изменения режима BDB.
      // esp_zb_start с режимом autostart=false используется, когда приложение хочет что-то сделать перед подключением к сети.

    esp_zb_main_loop_iteration();
}

//void EspZbRuntime::reportNow()
//{
/*
    for(int iEndpoint=0; iEndpoint < g_device->getEndpointCount(); iEndpoint++) {
      ZbEndpoint *ep = g_device->getEndpoint(iEndpoint);
      esp_zb_cluster_list_t *cluster_list = esp_zb_zcl_cluster_list_create();
      for(int iCluster=0; iCluster < ep->getClusterCount(); iCluster++) {
        ZbCluster *cluster = ep->getCluster(iCluster);
        esp_zb_attribute_list_t *attr_list = esp_zb_zcl_attr_list_create(cluster->getId());
        //ESP_LOGI(TAG, "getAttributeCount=%d", cluster->getAttributeCount());
        for(int iAttribute=0; iAttribute < cluster->getAttributeCount(); iAttribute++) {
          ZbAttribute *attr = cluster->getAttribute(iAttribute);
          //ESP_LOGI(TAG, "zbesp_cluster_add_attr(, %d, %d, )", cluster->getId(), attr->getId());
          zbesp_cluster_add_attr(attr_list, cluster->getId(), attr->getId(), attr->getData());
        }
        zbesp_cluster_list_add_cluster(cluster_list, cluster->getId(), attr_list, cluster->getRole());
        //ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster()");
      }
      esp_zb_ep_list_add_ep(ep_list, cluster_list, ep->getId(), ep->getProfileId(), ep->getDeviceId());
      //ESP_LOGI(TAG, "esp_zb_ep_list_add_ep()");
    }
*/
//}

uint8_t EspZbRuntime::setAttributeValue(uint8_t endpoint, uint16_t clusterId, uint8_t clusterRole, uint16_t attrId, void *value, bool check)
{
  //ESP_LOGI(TAG, "EspZbRuntime::setAttributeValueFunc");
//  ESP_LOGI(TAG, "esp_zb_zcl_set_attribute_val endpoint=%d clusterId=%02X attrId=%02X value=%d", endpoint, clusterId, attrId, *(uint8_t *)value);
  return esp_zb_zcl_set_attribute_val(endpoint, clusterId, clusterRole, attrId, value, check);
}

void leaveNetworkCB(esp_zb_zdp_status_t zdo_status, void *user_ctx)
{
  ESP_LOGI(TAG, "*************** leaveNetworkCB zdo_status=%02x", zdo_status);
  //esp_zb_factory_reset(); // После сброса само приложение получит сигнал ссылки на ZB_ZDO_SIGNAL_LEAVE. После сброса настроек будет выполнен сброс системы

  // esp_zb_zcl_reset_all_endpoints_to_factory_default();
}

// https://github.com/zigpy/zigpy/issues/831?ysclid=lu5vinxlfu163749576
// https://developer.nordicsemi.com/nRF_Connect_SDK/doc/zboss/3.11.2.0/using_zigbee__z_c_l.html
void EspZbRuntime::leave(void)
{
  ESP_LOGI(TAG, "EspZbRuntime::factoryReset()");
  //esp_zb_nvram_erase_at_start(true);
  esp_zb_factory_reset(); // После сброса само приложение получит сигнал ссылки на ZB_ZDO_SIGNAL_LEAVE. После сброса настроек будет выполнен сброс системы
/*
  esp_zb_zdo_mgmt_leave_req_param_t req;
  //esp_zb_ieee_address_by_short(0, req.device_address);
  esp_zb_get_long_address(req.device_address);
  req.dst_nwk_addr = 0xffff; // целевой ZC //esp_zb_get_short_address();
  req.remove_children = 0;
  req.rejoin = 0; // 0 - сброс до заводских, 1 - без сброса до заводских
  esp_zb_zdo_device_leave_req(&req, leaveNetworkCB, NULL); //leaveNetworkCB, NULL);
*/
}

esp_err_t zbesp_report_attribute(uint8_t endpoint, uint16_t clusterId, uint16_t attributeId)
{
    esp_zb_zcl_report_attr_cmd_t cmd;

    cmd.zcl_basic_cmd.dst_addr_u.addr_short = 0x0000;
    cmd.zcl_basic_cmd.dst_endpoint = endpoint;
    cmd.zcl_basic_cmd.src_endpoint = endpoint;
    cmd.address_mode = ESP_ZB_APS_ADDR_MODE_16_ENDP_PRESENT;
    cmd.clusterID = clusterId;
    cmd.attributeID = attributeId;
    cmd.cluster_role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE;
    esp_zb_zcl_attr_t *value_r = esp_zb_zcl_get_attribute(endpoint, clusterId, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE, attributeId);
//    memcpy(value_r->data_p, value, value_length);
    ESP_LOGI(TAG, "zbesp_report_attribute: %d", *(uint8_t *) value_r->data_p);
    return esp_zb_zcl_report_attr_cmd_req(&cmd);
}

uint8_t EspZbRuntime::reportAttribute(uint8_t endpoint, uint16_t clusterId, uint16_t attributeId)
{
  return (uint8_t) zbesp_report_attribute(endpoint, clusterId, attributeId);
}

void EspZbRuntime::init(ZbDevice &device, bool appendMandatories)
{
  //ESP_LOGI(TAG, "EspZbRuntime_init()");
  ZbRuntime::init(device, appendMandatories);
  g_device = &device;
}

void EspZbRuntime::start(void)
{
  //ESP_LOGI(TAG, "EspZbRuntime_start()");
  ZbRuntime::start();

  ESP_ERROR_CHECK(nvs_flash_init());  //???

  esp_zb_platform_config_t platform_config;
  memset(&platform_config, 0, sizeof(platform_config));
  platform_config.radio_config.radio_mode = RADIO_MODE_NATIVE;
  platform_config.host_config.host_connection_mode = HOST_CONNECTION_MODE_NONE;

  ESP_ERROR_CHECK(esp_zb_platform_config(&platform_config));

  xTaskCreate(esp_zb_task, "Zigbee_main", 4096, NULL, 5, NULL);
}
