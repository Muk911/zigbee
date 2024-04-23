#include <Arduino.h>
#include "esp_log.h"
#include "zb_zcl.h"
#include "zb_device.h"

#define TAG "zb_device"

ZbRuntime *g_runtime;

void ZbDevice::leave(void)
{
  //ESP_LOGI(TAG, "ZbDevice::leave(void)");
  assert(g_runtime);
  g_runtime->leave();
}

void ZbDevice::reportNow(void)
{
  ESP_LOGI(TAG, "ZbDevice::reportNow(void)");
  //if(reportNowFunc) reportNowFunc();
}

void ZbDevice::appendMandatories(void)
{
  initZclData();
  //dumpClusterInfos();
  
  zb_cluster_info_t *ci;
  for(int iEndpoint=0; iEndpoint < getEndpointCount(); iEndpoint++) {
    ZbEndpoint *ep = getEndpoint(iEndpoint);
    assert(ep);
    //ESP_LOGI(TAG, "Endpoint %02X", ep->getId());
    for(int iCluster=0; iCluster < ep->getClusterCount(); iCluster++) {
      ZbCluster *cluster = ep->getCluster(iCluster);
      assert(cluster);
      //ESP_LOGI(TAG, "cluster %04X", cluster->getId());
      zb_cluster_info_t *ci = findClusterInfo(cluster->getId());
      //ESP_LOGI(TAG, "ci %04X", ci);
      if(ci) {
        for(int iAttribute=0; iAttribute < ci->attrCount; iAttribute++) {
          zb_attr_info_t *ai = &ci->attrs[iAttribute];
          if(ai->mandatory && !cluster->findAttribute(ai->attrId)) {
            //ESP_LOGI(TAG, "Append clusterId=%04X, attrId=%04X, attrAccess=%02X", ci->clusterId, ai->attrId, ai->attrAccess);
            ZbAttribute *attr = new ZbAttribute(*cluster, ai->attrId, ai->attrType, ai->attrAccess, ai->defaultValue);
          }
        }
      } 
    }
  } 
  //ESP_LOGI(TAG, "exit appendMandatories()"); 
}

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

void ZbAttribute::report(void)
{
  if(getDevice()->joined()) {
    assert(g_runtime);
    esp_zb_zcl_status_t state_tmp = (esp_zb_zcl_status_t) g_runtime->reportAttribute(getEndpoint()->getId(), getCluster()->getId(), m_id);
  }
}

void ZbAttribute::updateValue(void)
{
  ESP_LOGI(TAG, "ZbAttribute::updateValue(void)");
  if(getDevice()->joined()) {
    assert(g_runtime);
    ESP_LOGI(TAG, "g_runtime->setAttributeValue endpoint=%d clusterId=0x%04X attrId=0x%04X: 0x%02X", getEndpoint()->getId(), getCluster()->getId(), m_id, *m_data);
    esp_zb_zcl_status_t err = (esp_zb_zcl_status_t) g_runtime->setAttributeValue(getEndpoint()->getId(), getCluster()->getId(), getCluster()->getRole(), m_id, m_data, false);
    if(err != ESP_ZB_ZCL_STATUS_SUCCESS)
      ESP_LOGE(TAG, "Setting attribute failed (%04X)!", err);
    else 
      ESP_LOGI(TAG, "Setting attribute success.");
  }
}

void ZbAttribute::setValue(int32_t value)
{
//  ESP_LOGI(TAG, "ZbAttribute::setValue(uint8_t value)");
  if (m_data && m_dataOwned) free(m_data);
  m_intData = (uint32_t) value;
  m_data = (uint8_t *) &m_intData;
  m_dataOwned = false;
  updateValue();
}

void ZbAttribute::setValue(uint32_t value)
{
//  ESP_LOGI(TAG, "ZbAttribute::setValue(uint8_t value)");
  if (m_data && m_dataOwned) free(m_data);
  m_intData = value;
  m_data = (uint8_t *) &m_intData;
  m_dataOwned = false;
  updateValue();
}

void ZbAttribute::setValue(float value)
{
  //ESP_LOGI(TAG, "ZbAttribute::setValue(float value)");
  if (m_data && m_dataOwned) free(m_data);
  m_floatData = value;
  m_data = (uint8_t *) &m_floatData;
  m_dataOwned = false;
  updateValue();
}

void ZbAttribute::setValue(char* value)
{
  //ESP_LOGI(TAG, "ZbAttribute::setValue(char* value)");
  if (m_data && m_dataOwned) free(m_data);
  if(m_type == ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING || m_type == ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING) { // дополнить!!!
    int len = strlen(value);
    m_data = (uint8_t *) malloc(len + 1);
    m_data[0] = len;
    memcpy(m_data + 1, value, len);
    m_dataOwned = true;
  }
  else {
    m_data = (uint8_t *) value;
    m_dataOwned = false;
  }
  updateValue();
}
