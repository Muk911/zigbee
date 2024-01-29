#include <Arduino.h>
#include "esp_log.h"
#include "esp_check.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_zigbee_type.h"
#include "esp_zigbee_core.h"
#include "nvs_flash.h"
#include "ha/esp_zigbee_ha_standard.h"
#include "zcl/esp_zigbee_zcl_command.h"
#include "zbesp_runtime.h"

#define MANUFACTURER_NAME   "MEA"
#define MODEL_NAME          "Test1"
#define ZCL_VERSION         0x03
#define APP_VERSION         0x01
#define POWER_SOURCE        0x01

#define DEVICE_ENDPOINT     0x01
#define DEVICE_PROFILE_ID   ESP_ZB_AF_HA_PROFILE_ID
#define DEVICE_ID           ESP_ZB_HA_CUSTOM_ATTR_DEVICE_ID

#define TAG "zbesp_runtime"

ZbDevice *g_device;

ZbEndpoint* ZbDevice::findEndpoint(uint8_t id) {
  for(int iEndpoint=0; iEndpoint < m_endpointCount; iEndpoint++) {
    if(m_endpoints[iEndpoint]->getId() == id) 
      return m_endpoints[iEndpoint];
  }
  return NULL;
}

ZbCluster* ZbEndpoint::findCluster(uint16_t id) {
  for(int iCluster=0; iCluster < m_clusterCount; iCluster++) {
    if(m_clusters[iCluster]->getId() == id) 
      return m_clusters[iCluster];
  }
  return NULL;
}

ZbAttribute* ZbCluster::findAttribute(uint16_t id) {
  for(int iAttribute=0; iAttribute < m_attrCount; iAttribute++) {
    if(m_attributes[iAttribute]->getId() == id) 
      return m_attributes[iAttribute];
  }
  return NULL;
}

static void set_zcl_string(char *buffer, char *value)
{
    buffer[0] = (char) strlen(value);
    memcpy(buffer + 1, value, buffer[0]);
}

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
            ESP_LOGW(TAG, "Failed to initialize Zigbee stack (status: %s)", esp_err_to_name(err_status));
        }
        break;

    case ESP_ZB_BDB_SIGNAL_STEERING:
        if (err_status == ESP_OK) {
            g_device->setConnected(true);
            esp_zb_ieee_addr_t extended_pan_id;
            esp_zb_get_extended_pan_id(extended_pan_id);
            ESP_LOGI(TAG, "Joined network successfully (Extended PAN ID: %02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x, PAN ID: 0x%04hx, Channel:%d, Short Address: 0x%04hx)",
                     extended_pan_id[7], extended_pan_id[6], extended_pan_id[5], extended_pan_id[4],
                     extended_pan_id[3], extended_pan_id[2], extended_pan_id[1], extended_pan_id[0],
                     esp_zb_get_pan_id(), esp_zb_get_current_channel(), esp_zb_get_short_address());
        } else {
            g_device->setConnected(false);
            ESP_LOGI(TAG, "Network steering was not successful (status: %s)", esp_err_to_name(err_status));
            esp_zb_scheduler_alarm((esp_zb_callback_t)bdb_start_top_level_commissioning_cb, ESP_ZB_BDB_MODE_NETWORK_STEERING, 1000);
        }
        break;
    default:
        ESP_LOGI(TAG, "ZDO signal: %s (0x%x), status: %s", esp_zb_zdo_signal_to_string(sig_type), sig_type, esp_err_to_name(err_status));
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
    ESP_LOGI(TAG, "endpoint %d cluster 0x%04X attribute 0x%04X value updated", endpoint, cluster_id, attr_id);
}

// Handle the attributes
static esp_err_t zb_attribute_handler(const esp_zb_zcl_set_attr_value_message_t *message)
{
    esp_err_t ret = ESP_OK;
 
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
            ESP_LOGI(TAG, "Server time received %lu", *(uint32_t*) message->variables->attribute.data.value);
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
        //ESP_LOGI(TAG, "ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID");
        ret = zb_attribute_handler((esp_zb_zcl_set_attr_value_message_t *)message);
        break;
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
        ESP_LOGI(TAG, "ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID");
        ret = zb_read_attr_resp_handler((esp_zb_zcl_cmd_read_attr_resp_message_t *)message);
        break;
    default:
        ESP_LOGW(TAG, "Receive Zigbee action(0x%x) callback", callback_id);
        break;
    }
    return ret;
}

esp_err_t zbesp_cluster_list_add_cluster(esp_zb_cluster_list_t *cluster_list, uint16_t cluster_id, esp_zb_attribute_list_t *attr_list, uint8_t role_mask)
{
  switch(cluster_id) {
    case ESP_ZB_ZCL_CLUSTER_ID_BASIC:
      ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster ESP_ZB_ZCL_CLUSTER_ID_BASIC");
      return esp_zb_cluster_list_add_basic_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG:
      return esp_zb_cluster_list_add_power_config_cluster(cluster_list, attr_list, role_mask);
//    ESP_ZB_ZCL_CLUSTER_ID_DEVICE_TEMP_CONFIG:
    case ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY:
      ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY");
      return esp_zb_cluster_list_add_identify_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_GROUPS:
      return esp_zb_cluster_list_add_groups_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_SCENES:
      return esp_zb_cluster_list_add_scenes_cluster(cluster_list, attr_list, role_mask);
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
      return esp_zb_cluster_list_add_on_off_cluster(cluster_list, attr_list, role_mask);
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
      ESP_LOGI(TAG, "zbesp_cluster_list_add_cluster ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT");
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

// esp_err_t zbesp_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t cluster_id, uint16_t attr_id, uint8_t attr_type, uint8_t attr_access, void *value_p)
esp_err_t zbesp_cluster_add_attr(esp_zb_attribute_list_t *attr_list, uint16_t cluster_id, uint16_t attr_id, void *value_p) 
{ 
  switch(cluster_id) {
    case ESP_ZB_ZCL_CLUSTER_ID_BASIC:
      //ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_BASIC");
      return esp_zb_basic_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_POWER_CONFIG:
      return esp_zb_power_config_cluster_add_attr(attr_list, attr_id, value_p);
//    ESP_ZB_ZCL_CLUSTER_ID_DEVICE_TEMP_CONFIG:
    case ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY:
      ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY");
      return esp_zb_identify_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_GROUPS:
      return esp_zb_groups_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_SCENES:
      return esp_zb_scenes_cluster_add_attr(attr_list, attr_id, value_p);
    case ESP_ZB_ZCL_CLUSTER_ID_ON_OFF:
      ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_ON_OFF");
      return esp_zb_on_off_cluster_add_attr(attr_list, attr_id, value_p);
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
      ESP_LOGI(TAG, "zbesp_cluster_add_attr ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT");
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

static void esp_zb_task(void *pvParameters)
{
    uint16_t undefined_value = 0x8000;
    ESP_LOGI(TAG, "esp_zb_task()");

    esp_zb_cfg_t zigbee_config;
    memset(&zigbee_config, 0, sizeof(zigbee_config));
    zigbee_config.esp_zb_role = (esp_zb_nwk_device_type_t) g_device->getDeviceType();
    zigbee_config.install_code_policy = false;
    zigbee_config.nwk_cfg.zczr_cfg.max_children = 16;

    esp_zb_ep_list_t *ep_list = esp_zb_ep_list_create();

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

    esp_zb_init(&zigbee_config);
    esp_zb_device_register(ep_list);
    esp_zb_core_action_handler_register(zb_action_handler);
    esp_zb_set_primary_network_channel_set(ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK);

    esp_zb_nvram_erase_at_start(true); // Comment out this line to erase NVRAM data if you are connecting to new Coordinator
    
    ESP_ERROR_CHECK(esp_zb_start(false));
    esp_zb_main_loop_iteration();
}

uint8_t esp_zb_zcl_set_attribute_val_func(uint8_t endpoint, uint16_t cluster_id, uint8_t cluster_role, uint16_t attr_id, void *value_p, bool check) 
{
  //ESP_LOGI(TAG, "esp_zb_zcl_set_attribute_val_func");
  return esp_zb_zcl_set_attribute_val(endpoint, cluster_id, cluster_role, attr_id, value_p, check);
}

void ZbRuntimeESP::start(ZbDevice &device)
{
  ESP_LOGI(TAG, "ZbRuntimeESP_start()");
  m_device = &device;
  g_device = &device;
  device.setAttributeValueFunc = esp_zb_zcl_set_attribute_val_func;

  esp_zb_platform_config_t platform_config;
  memset(&platform_config, 0, sizeof(platform_config));
  platform_config.radio_config.radio_mode = RADIO_MODE_NATIVE;
  platform_config.host_config.host_connection_mode = HOST_CONNECTION_MODE_NONE;

  ESP_ERROR_CHECK(nvs_flash_init());
  ESP_ERROR_CHECK(esp_zb_platform_config(&platform_config));

  xTaskCreate(esp_zb_task, "Zigbee_main", 5096, NULL, 5, NULL);
}
