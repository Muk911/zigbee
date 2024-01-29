#ifndef __ZB_DEVICE_H__
#define __ZB_DEVICE_H__

//#include <Arduino.h>
#include "esp_log.h"
#include "esp_check.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "zb_defs.h"
#include "zcl/esp_zigbee_zcl_common.h" // временно

static const char *TAG = "zb_device";

#define DEVICE_TYPE_COORDINATOR  1
#define DEVICE_TYPE_ROUTER       2
#define DEVICE_TYPE_ENDPOINT     3

enum zb_zcl_on_off_attr_e {
  ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID = 0, 
  ZB_ZCL_ATTR_ON_OFF_GLOBAL_SCENE_CONTROL = 0x4000, 
  ZB_ZCL_ATTR_ON_OFF_ON_TIME = 0x4001, 
  ZB_ZCL_ATTR_ON_OFF_OFF_WAIT_TIME = 0x4002,
  ZB_ZCL_ATTR_ON_OFF_START_UP_ON_OFF = 0x4003
};

class ZbEndpoint;
class ZbCluster;
class ZbAttribute;

typedef void (*AttributeU8ValueChangedCB)(ZbAttribute *za, uint8_t data);
typedef void (*AttributeU16ValueChangedCB)(ZbAttribute *za, uint16_t data);
typedef void (*AttributeStrValueChangedCB)(ZbAttribute *za, char *data);
typedef uint8_t (*SetAttributeValueFunc)(uint8_t endpoint, uint16_t cluster_id, uint8_t cluster_role, uint16_t attr_id, void *value_p, bool check);

class ZbDevice {
public:
	ZbDevice(uint8_t deviceType) {
    m_deviceType = deviceType; // END_DEVICE;
    m_panId = 0x1a62;  //!!!!!!!
    m_channel = 11; //!!!!!! 
    m_connected = false;
    m_endpointCount = 0;
    m_endpoints = (ZbEndpoint**) malloc(MAX_DEVICE_ENDPOINTS * sizeof(ZbEndpoint*)); 
	}
	~ZbDevice() {}

  int getEndpointCount(void) {
    return m_endpointCount;
  }

  ZbEndpoint *getEndpoint(int index) {
    return m_endpoints[index];
  }

  void addEndpoint(ZbEndpoint *endpoint) {
    m_endpoints[m_endpointCount++] = endpoint;
  }

  ZbEndpoint* findEndpoint(uint8_t id);

  bool connected(void) {
    return m_connected;
  }

  void setConnected(bool value) {
    m_connected = value;
  }

  uint8_t getDeviceType(void) {
    return m_deviceType;
  }

  void setNetworkChannel(uint8_t channel);
  void setInstallCodePolicy(bool installCodePolicy) {}
  void setMaxChildren(uint8_t smaxChildren) {}
 // void setPanId(uint16_t panId);
 // void setExtpanPanId(uint16_t *extpanId);
 // void setNetworkKey(uint16_t *networkKey);
  
  SetAttributeValueFunc setAttributeValueFunc;

private:
  uint8_t m_deviceType;
  uint8_t m_channel;
  uint16_t m_panId;
  uint8_t m_extpanId[8] = {0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD};
  uint8_t m_networkKey[16] = {1, 3, 5, 7, 9, 11, 13, 15, 0, 2, 4, 6, 8, 10, 12, 13};
  bool m_connected;
  int m_endpointCount;
  ZbEndpoint **m_endpoints; //MAX_DEVICE_ENDPOINTS
};

class ZbEndpoint {
public:
	ZbEndpoint(ZbDevice &device, uint8_t id, uint16_t profileId, uint16_t deviceId) {
    m_device = &device;                              
    m_id = id;
    m_profileId = profileId;
    m_deviceId = deviceId;
    m_clusterCount = 0;
    m_clusters = (ZbCluster**) malloc(MAX_ENDPOINT_CLUSTERS * sizeof(ZbCluster*)); 
    m_device->addEndpoint(this);
	}
	~ZbEndpoint() {}

  ZbDevice* getDevice(void) {
    return m_device;
  }

  uint8_t getId(void) {
    return m_id;
  }

  int getClusterCount(void) {
    return m_clusterCount;
  }

  ZbCluster *getCluster(int index) {
    return m_clusters[index];
  }

 void addCluster(ZbCluster *cluster) {
    m_clusters[m_clusterCount++] = cluster;
  }

  ZbCluster* findCluster(uint16_t id);

  uint16_t getProfileId(void) {
    return m_profileId;
  }

  uint16_t getDeviceId(void) {
    return m_deviceId;
  }

protected:
  ZbDevice *m_device;
private:
  uint8_t m_id; 
  uint16_t m_profileId; 
  uint16_t m_deviceId;
  int m_clusterCount;
  ZbCluster **m_clusters; //MAX_ENDPOINT_CLUSTERS
};

class ZbCluster {
public:
  ZbCluster(ZbEndpoint &endpoint, uint16_t id, uint8_t role) {
    m_endpoint = &endpoint;
    m_id = id;
    m_role = role;
    m_attrCount = 0;
    m_attributes = (ZbAttribute**) malloc(MAX_CLUSTER_ATTRIBUTES * sizeof(ZbAttribute*)); 
    m_endpoint->addCluster(this);
  }
  ~ZbCluster() {}
  
  ZbEndpoint* getEndpoint(void) {
    return m_endpoint;
  }

  ZbDevice* getDevice(void) {
    return m_endpoint->getDevice();
  }

  uint16_t getId(void) {
    return m_id;
  }

  uint8_t getRole(void) {
    return m_role;
  }

  int getAttributeCount(void) {
    return m_attrCount;
  }

  ZbAttribute *getAttribute(int index) {
    return m_attributes[index];
  }

  void addAttribute(ZbAttribute *attribute) {
    m_attributes[m_attrCount++] = attribute;
  }

  ZbAttribute* findAttribute(uint16_t id);

protected:
  ZbEndpoint *m_endpoint;
private:
  uint16_t m_id;
  uint8_t m_role;
  int m_attrCount;
  ZbAttribute **m_attributes; //MAX_CLUSTER_ATTRIBUTES
};

class ZbAttribute {
public:
  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t attrType, uint8_t attrAccess) {
    m_cluster = &cluster;
    m_id = id;
    m_attrType = attrType;
    m_attrAccess = attrAccess;
    m_dataOwned = false;
    m_cluster->addAttribute(this);
  }
  ~ZbAttribute() {
    //if (m_dataOwned) free(m_data);
  }

  ZbCluster* getCluster(void) {
    return m_cluster;
  }

  ZbEndpoint* getEndpoint(void) {
    return m_cluster->getEndpoint();
  }

  ZbDevice* getDevice(void) {
    return m_cluster->getEndpoint()->getDevice();
  }

  uint16_t getId(void) {
    return m_id;
  }
/*
  ZbAttribute* setVar(void *varData) {
    m_data = (uint8_t *) varData;
    m_dataOwned = false;
    return this;
  }
*/

  virtual void valueChanged(uint8_t *data) {
  }

  uint8_t *getData(void) {
    return m_data;
  }

protected:
  ZbCluster *m_cluster;
  uint16_t m_id;
  uint8_t m_attrType;
  uint8_t m_attrAccess;
  uint8_t *m_data;
  uint32_t m_intData;
  bool m_dataOwned;
};

class ZbAttributeU8 : public ZbAttribute {
public:
  ZbAttributeU8(ZbCluster &cluster, uint16_t attrId, uint8_t attrData) : ZbAttribute(cluster, attrId, ESP_ZB_ZCL_ATTR_TYPE_U8, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE) {
    setValue(attrData); 
  }
 
  ZbAttributeU8* setValue(uint8_t value) {
    if (m_data && m_dataOwned) free(m_data);
    m_intData = value;
    m_data = (uint8_t *) &m_intData;
    m_dataOwned = false;

    //ESP_LOGI(TAG, "ZbAttributeU8::setValue(%d)", value);
    if(getDevice()->connected()) {
      esp_zb_zcl_status_t state_tmp = (esp_zb_zcl_status_t) getDevice()->setAttributeValueFunc(getEndpoint()->getId(), getCluster()->getId(), getCluster()->getRole(), m_id, &value, false);
      if(state_tmp != ESP_ZB_ZCL_STATUS_SUCCESS)
        ESP_LOGE(TAG, "Setting attribute failed!");
      else 
        ESP_LOGI(TAG, "Setting attribute success (value=%d).", value);
    }
    return this;
  }
  
  ZbAttributeU8* setValueChangedCB(AttributeU8ValueChangedCB valueChangedCB) {
    m_valueChangedCB = valueChangedCB;
    return this;
  }

  void valueChanged(uint8_t *data) override
  {
    if(m_valueChangedCB)
      m_valueChangedCB(this, *data);
  }

private:
  AttributeU8ValueChangedCB m_valueChangedCB;
};

class ZbAttributeU16 : public ZbAttribute {
public:
  ZbAttributeU16(ZbCluster &cluster, uint16_t attrId, uint16_t attrData) : ZbAttribute(cluster, attrId, ESP_ZB_ZCL_ATTR_TYPE_U16, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE) {
    setValue(attrData); 
  }

  ZbAttributeU16* setValue(uint16_t value) {
    //ESP_LOGI(TAG, "ZbAttributeU16::setValue()");
    if (m_data && m_dataOwned) free(m_data);
    m_intData = value;
    m_data = (uint8_t *) &m_intData;
    m_dataOwned = false;
 
    if(getDevice()->connected()) {
      //ESP_LOGI(TAG, "setAttributeValueFunc");
      esp_zb_zcl_status_t state_tmp = (esp_zb_zcl_status_t) getDevice()->setAttributeValueFunc(getEndpoint()->getId(), getCluster()->getId(), getCluster()->getRole(), m_id, &value, false);
      if(state_tmp != ESP_ZB_ZCL_STATUS_SUCCESS)
        ESP_LOGE(TAG, "Setting attribute failed!");
      else 
        ESP_LOGI(TAG, "Setting attribute success (value=%d).", value);
    }
    return this;
  }

  ZbAttributeU16* setValueChangedCB(AttributeU16ValueChangedCB valueChangedCB) {
    m_valueChangedCB = valueChangedCB;
    return this;
  }
  
  void valueChanged(uint8_t *data) override
  {
    if(m_valueChangedCB)
      m_valueChangedCB(this, *(uint16_t *)data);
  }
 
private:
  AttributeU16ValueChangedCB m_valueChangedCB;
};

class ZbAttributeStr : public ZbAttribute {
public:
  ZbAttributeStr(ZbCluster &cluster, uint16_t attrId, char *attrData) : ZbAttribute(cluster, attrId, ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING, ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE) {
    setValue(attrData);
  }
 
  ZbAttributeStr* setValue(char* value) 
  {
    ESP_LOGI(TAG, "ZbAttributeStr::setValue('%s')", value);
    if (m_data && m_dataOwned) free(m_data);
    int len = strlen(value);
    m_data = (uint8_t *) malloc(len + 1);
    m_data[0] = len;
    memcpy(m_data + 1, value, len);
    m_dataOwned = true;

    if(getDevice()->connected()) {
      esp_zb_zcl_status_t state_tmp = (esp_zb_zcl_status_t) getDevice()->setAttributeValueFunc(getEndpoint()->getId(), getCluster()->getId(), getCluster()->getRole(), m_id, m_data, false);
      if(state_tmp != ESP_ZB_ZCL_STATUS_SUCCESS)
        ESP_LOGE(TAG, "Setting attribute failed!");
      else 
        ESP_LOGI(TAG, "Setting attribute success (value='%s').", value);
    }
    return this;
  }
  
  ZbAttribute* setValueChangedCB(AttributeStrValueChangedCB valueChangedCB) {
    m_valueChangedCB = valueChangedCB;
    return this;
  }

  void valueChanged(uint8_t *data) override
  {
    if(m_valueChangedCB)
      m_valueChangedCB(this, (char*)data); // переделать
  }

private:
  AttributeStrValueChangedCB m_valueChangedCB;
};

//*****************************************************
class ZbBasicCluster : public ZbCluster {
public:
  ZbBasicCluster(ZbEndpoint &endPoint, char *manufacturerName, char *modelIdentifier, uint8_t zclVersion, uint8_t applicationVersion, uint8_t powerSource) : ZbCluster(endPoint, 0x0000U, 0x01U) {
// https://csa-iot.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf
    m_zclVersion = zclVersion;                                           
    m_applicationVersion = applicationVersion;                                   
    m_stackVersion = 2;                                        
    m_hardwareVersion = 1;                                         
    m_manufacturerName = manufacturerName;
    m_modelIdentifier = modelIdentifier;
    m_dateCode = "20230101";                                   
    m_powerSource = powerSource;                                   
  // LocationDescription
  // PhysicalEnvironment
  // DeviceEnabled
  // AlarmMask
  // DisableLocalConfig
    m_softwareBuildID = "1000-0001";

    ESP_LOGI(TAG, "ZbBasicCluster()");
    m_attrManufacturerName = new ZbAttributeStr(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, manufacturerName);
    m_attrModelIdentifier = new ZbAttributeStr(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, modelIdentifier);
    m_attrZclVersion = new ZbAttributeU8(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, zclVersion);
    m_attrPowerSource = new ZbAttributeU8(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, powerSource);
    m_attrApplicationVersion = new ZbAttributeU8(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID, applicationVersion);
  }

  ~ZbBasicCluster() {
    delete m_attrManufacturerName; 
    delete m_attrModelIdentifier;
    delete m_attrZclVersion;
    delete m_attrPowerSource;
    delete m_attrApplicationVersion;
  }

private:
  uint8_t m_zclVersion;                   // ZCL version
  uint8_t m_applicationVersion;           // Application Version
  uint8_t m_stackVersion;                 // Stack version
  uint8_t m_hardwareVersion;              // Hardware version
  char *m_manufacturerName;               // Manufacturer Name: Vertorix
  char *m_modelIdentifier;                // Model Identifier: VT1100DHT11
  uint8_t m_powerSource;                  // Power Source: 0 = Unknown, 1 = mains(single phase), 2 = mains(3 phase), 3 = battery, 4 = DC source.
  char *m_dateCode;                       // Date Code
  char *m_softwareBuildID;                // Software Build ID 
  ZbAttributeStr *m_attrManufacturerName;
  ZbAttributeStr *m_attrModelIdentifier;
  ZbAttributeU8 *m_attrZclVersion;
  ZbAttributeU8 *m_attrPowerSource;
  ZbAttributeU8 *m_attrApplicationVersion;
};

class ZbIdentifyCluster : public ZbCluster {
public:
  ZbIdentifyCluster(ZbEndpoint &endPoint) : ZbCluster(endPoint, 0x0003U, 0x01U) {}
private:
};

class ZbOnOffCluster : public ZbCluster {
public:
  ZbOnOffCluster(ZbEndpoint &endPoint) : ZbCluster(endPoint, 0x0006U, 0x01U) {}
private:
};
  
class ZbTemperatureMeasCluster : public ZbCluster {
public:
  ZbTemperatureMeasCluster(ZbEndpoint &endPoint) : ZbCluster(endPoint, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, 0x01U) {}
private:
};

#endif //__ZB_DEVICE_H__
