#ifndef __ZBESP_DEBUG_H__
#define __ZBESP_DEBUG_H__

#include "esp_zigbee_core.h"

char *get_core_action_callback_name(esp_zb_core_action_callback_id_s callback_id);

void dumpZclAttrList(esp_zb_attribute_list_t *attr_list);
void dumpZclAttribute(esp_zb_zcl_attr_t *attr);
void dumpZclCluster(esp_zb_zcl_cluster_t *cluster);
void dumpZclEndpoint(esp_zb_endpoint_t *endpoint);
void dumpSimpleDesc(esp_zb_af_simple_desc_1_1_t *simple_desc);
void dumpEndpointList(esp_zb_ep_list_t *ep_list);


#endif