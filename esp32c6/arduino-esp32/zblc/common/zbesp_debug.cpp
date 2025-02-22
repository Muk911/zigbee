#include "esp_log.h"
#include "esp_zigbee_core.h"
#include "zbesp_debug.h"

#define TAG "zbesp_debug"

char *get_core_action_callback_name(esp_zb_core_action_callback_id_t callback_id)
{
  switch(callback_id) 
  {
    case ESP_ZB_CORE_SET_ATTR_VALUE_CB_ID:
      return "Set attribute value";
    case ESP_ZB_CORE_SCENES_STORE_SCENE_CB_ID:
      return "Store scene";
    case ESP_ZB_CORE_SCENES_RECALL_SCENE_CB_ID:
      return "Recall scene";
    case ESP_ZB_CORE_IAS_ZONE_ENROLL_RESPONSE_VALUE_CB_ID:
      return "IAS Zone enroll response";
    case ESP_ZB_CORE_OTA_UPGRADE_VALUE_CB_ID:
      return "Upgrade OTA";
    case ESP_ZB_CORE_OTA_UPGRADE_SRV_STATUS_CB_ID:
      return "OTA Server status";
    case ESP_ZB_CORE_OTA_UPGRADE_SRV_QUERY_IMAGE_CB_ID:
      return "OTA Server query image";
    case ESP_ZB_CORE_THERMOSTAT_VALUE_CB_ID:
      return "Thermostat value";
    case ESP_ZB_CORE_METERING_GET_PROFILE_CB_ID:
      return "Metering get profile";
    case ESP_ZB_CORE_METERING_GET_PROFILE_RESP_CB_ID:
      return "Metering get profile response";
    case ESP_ZB_CORE_METERING_REQ_FAST_POLL_MODE_CB_ID:
      return "Metering request fast poll mode";
    case ESP_ZB_CORE_METERING_REQ_FAST_POLL_MODE_RESP_CB_ID:
      return "Metering request fast poll mode response";
    case ESP_ZB_CORE_METERING_GET_SNAPSHOT_CB_ID:
      return "Metering get snapshot";
    case ESP_ZB_CORE_METERING_PUBLISH_SNAPSHOT_CB_ID:
      return "Metering publish snapshot";
    case ESP_ZB_CORE_METERING_GET_SAMPLED_DATA_CB_ID:
      return "Metering get sampled data";
    case ESP_ZB_CORE_METERING_GET_SAMPLED_DATA_RESP_CB_ID:
      return "Metering get sampled data response";
    case ESP_ZB_CORE_DOOR_LOCK_LOCK_DOOR_CB_ID:
      return "Lock/unlock door request";
    case ESP_ZB_CORE_DOOR_LOCK_LOCK_DOOR_RESP_CB_ID:
      return "Lock/unlock door response";
    case ESP_ZB_CORE_IDENTIFY_EFFECT_CB_ID:
      return "Identify triggers effect request";
//    case ESP_ZB_CORE_BASIC_RESET_TO_FACTORY_RESET_CB_ID:
//      return "Reset all clusters of endpoint to factory default";
    case ESP_ZB_CORE_CMD_READ_ATTR_RESP_CB_ID:
      return "Read attribute response";
    case ESP_ZB_CORE_CMD_WRITE_ATTR_RESP_CB_ID:
      return "Write attribute response";
    case ESP_ZB_CORE_CMD_REPORT_CONFIG_RESP_CB_ID:
      return "Configure report response";
    case ESP_ZB_CORE_CMD_READ_REPORT_CFG_RESP_CB_ID:
      return "Read report configuration response";
    case ESP_ZB_CORE_CMD_DISC_ATTR_RESP_CB_ID:
      return "Discover attributes response";
    case ESP_ZB_CORE_CMD_DEFAULT_RESP_CB_ID:
      return "Default response";
    case ESP_ZB_CORE_CMD_OPERATE_GROUP_RESP_CB_ID:
      return "Group add group response";
    case ESP_ZB_CORE_CMD_VIEW_GROUP_RESP_CB_ID:
      return "Group view response";
    case ESP_ZB_CORE_CMD_GET_GROUP_MEMBERSHIP_RESP_CB_ID:
      return "Group get membership response";
    case ESP_ZB_CORE_CMD_OPERATE_SCENE_RESP_CB_ID:
      return "Scenes operate response";
    case ESP_ZB_CORE_CMD_VIEW_SCENE_RESP_CB_ID:
      return "Scenes view response";
    case ESP_ZB_CORE_CMD_GET_SCENE_MEMBERSHIP_RESP_CB_ID:
      return "Scenes get membership response";
    case ESP_ZB_CORE_CMD_IAS_ZONE_ZONE_ENROLL_REQUEST_ID:
      return "IAS Zone enroll request";
    case ESP_ZB_CORE_CMD_IAS_ZONE_ZONE_STATUS_CHANGE_NOT_ID:
      return "IAS Zone status change notification";
    case ESP_ZB_CORE_CMD_CUSTOM_CLUSTER_REQ_CB_ID:
      return "Custom Cluster request";
    case ESP_ZB_CORE_CMD_CUSTOM_CLUSTER_RESP_CB_ID:
      return "Custom Cluster response";
    case ESP_ZB_CORE_CMD_PRIVILEGE_COMMAND_REQ_CB_ID:
      return "Custom Cluster request";
    case ESP_ZB_CORE_CMD_PRIVILEGE_COMMAND_RESP_CB_ID:
      return "Custom Cluster response";
//    case ESP_ZB_CORE_CMD_TOUCHLINK_GET_GROUP_ID_RESP_CB_ID:
//      return "Touchlink commissioning cluster get group id response";
//    case ESP_ZB_CORE_CMD_TOUCHLINK_GET_ENDPOINT_LIST_RESP_CB_ID:
//      return "Touchlink commissioning cluster get endpoint list response";
//    case ESP_ZB_CORE_CMD_GREEN_POWER_RECV_CB_ID:
//      return "Green power cluster command receiving";
//    case ESP_ZB_CORE_REPORT_ATTR_CB_ID:
//      return "Attribute Report";
  }
}

void dumpZclAttrList(esp_zb_attribute_list_t *attr_list)
{
  //ESP_LOGI(TAG, " dumpZclAttrList:");
  struct esp_zb_attribute_list_s *cur = attr_list;

  while(cur) {
    ESP_LOGI(TAG, "  cluster_id=0x%04X, attribute 0x%04X type=%d", cur->cluster_id, cur->attribute.id, cur->attribute.type);
    cur = cur->next;
  }
}

void dumpZclAttribute(esp_zb_zcl_attr_t *attr)
{
  ESP_LOGI(TAG, "  attribute_id=0x%04X type=%02X", attr->id, attr->type); 
}

void dumpZclCluster(esp_zb_zcl_cluster_t *cluster)
{
  ESP_LOGI(TAG, "  cluster_id=%04X, attr_count=%d", cluster->cluster_id, cluster->attr_count);
  for(int iAttribute=0; iAttribute < cluster->attr_count; iAttribute++) {
    dumpZclAttribute(&cluster->attr_desc_list[iAttribute]);
  }
}

void dumpZclEndpoint(esp_zb_endpoint_t *endpoint)
{
  ESP_LOGI(TAG, "endpoint_id %d, cluster_count %d", endpoint->ep_id, endpoint->cluster_count);
  for(int iCluster=0; iCluster < endpoint->cluster_count; iCluster++) {
    dumpZclCluster(&endpoint->cluster_desc_list[iCluster]);
  }
}

void dumpSimpleDesc(esp_zb_af_simple_desc_1_1_t *simple_desc)
{ 
  ESP_LOGI(TAG, "  SimpleDesc input_cluster_count=%d, output_cluster_count=%d  %04X  %04X", simple_desc->app_input_cluster_count, simple_desc->app_output_cluster_count, simple_desc->app_cluster_list[0], simple_desc->app_cluster_list[1]); 
}

void dumpEndpointList(esp_zb_ep_list_t *ep_list)
{
  struct esp_zb_ep_list_s *cur = ep_list->next;

  while(cur) {
    dumpZclEndpoint(&cur->endpoint);
    dumpSimpleDesc(cur->endpoint.simple_desc);
    cur = cur->next;
  }
}
