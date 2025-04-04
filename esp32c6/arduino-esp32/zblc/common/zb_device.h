#ifndef __ZB_DEVICE_H__
#define __ZB_DEVICE_H__

#include "esp_log.h"
#include "esp_check.h"
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "nvs_flash.h"
#include "zb_defs.h"
#include "zb_zcl.h"
#include "esp_zigbee_core.h" // временно
#include "zcl/esp_zigbee_zcl_common.h" // временно

#define TAG "zb_device"

#define DEVICE_TYPE_COORDINATOR  1
#define DEVICE_TYPE_ROUTER       2
#define DEVICE_TYPE_ENDPOINT     3
/*
enum zb_zcl_on_off_attr_e {
  ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID = 0, 
  ZB_ZCL_ATTR_ON_OFF_GLOBAL_SCENE_CONTROL = 0x4000, 
  ZB_ZCL_ATTR_ON_OFF_ON_TIME = 0x4001, 
  ZB_ZCL_ATTR_ON_OFF_OFF_WAIT_TIME = 0x4002,
  ZB_ZCL_ATTR_ON_OFF_START_UP_ON_OFF = 0x4003
};
*/
class ZbEndpoint;
class ZbCluster;
class ZbAttribute;
class ZbRuntime;

extern ZbRuntime *g_runtime;

typedef void (*CommandReceivedCB)(ZbCluster *zc, uint8_t command);
typedef void (*AttributeValueChangedCB)(ZbAttribute *za, uint8_t *data);
typedef uint8_t (*DeferredInitCB)(void);
typedef void (*DeviceStartedCB)(void);

//typedef uint8_t (*SetAttributeValueFunc)(uint8_t endpoint, uint16_t clusterId, uint8_t clusterRole, uint16_t attrId, void *value, bool check);
//typedef uint8_t (*ReportAttributeFunc)(uint8_t endpoint, uint16_t clusterId, uint16_t attributeId);
//typedef void (*ReportNowFunc)(void);

class ZbDevice {
public:
	ZbDevice(uint8_t deviceType) {
    m_deviceType = deviceType; // END_DEVICE;
    m_installCodePolicy = false;
    m_maxChildren = 16;
    m_panId = 0x1a62;  //!!!!!!!
    m_channelMask = ESP_ZB_TRANSCEIVER_ALL_CHANNELS_MASK;
    m_registered = false;
    m_started = false;
    m_joined = false;
    m_leaveAfterStart = false;
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

  bool registered(void) {
    return m_registered;
  }

  void setRegistered(bool value) {
    m_registered = value;
  }

  bool started(void) {
    return m_started;
  }

  void setStarted(bool value) {
    m_started = value;
    if (m_deviceStartedCB)
      m_deviceStartedCB();
    if (m_leaveAfterStart)
      leave();      
  }

  bool joined(void) {
    return m_joined;
  }

  void setJoined(bool value) {
    m_joined = value;
    //ESP_LOGI(TAG, "joined=%d", value);
  }

  uint8_t getDeviceType(void) {
    return m_deviceType;
  }

  void setChannelMask(uint32_t channelMask)
  {
    m_channelMask = channelMask;
  }

  uint32_t getChannelMask(void)
  {
    return m_channelMask;
  }

  bool getInstallCodePolicy(void) {
    return m_installCodePolicy;
  }

  void setInstallCodePolicy(bool installCodePolicy) {
    m_installCodePolicy = installCodePolicy;
  }

  uint8_t getMaxChildren(void) {
    return m_maxChildren;
  }

  void setMaxChildren(uint8_t maxChildren) {
    m_maxChildren = maxChildren;
  }

  void leave(void); 
  void reportNow(void);
  void appendMandatories(void);

  void leaveAfterStart() {
    if (m_started)
      leave();
    else
      m_leaveAfterStart = true;
  }
 
  void onDeferrerInit(DeferredInitCB deferredInitCB) {
    p_deferredInitCB = deferredInitCB;
  }

  void onDeviceStarted(DeviceStartedCB deviceStartedCB) {
    m_deviceStartedCB = deviceStartedCB;
  }

 // void setPanId(uint16_t panId);
 // void setExtpanPanId(uint16_t *extpanId);
 // void setNetworkKey(uint16_t *networkKey);
  
  //SetAttributeValueFunc setAttributeValueFunc;
  //ReportAttributeFunc reportAttributeFunc;
  //FactoryResetFunc factoryResetFunc;

  DeferredInitCB p_deferredInitCB;

private:
  //ZbRuntime *m_runtime;
  uint8_t m_deviceType;
  uint32_t m_channelMask;
  uint8_t m_maxChildren;
  bool m_installCodePolicy;
  uint16_t m_panId;
  uint8_t m_extpanId[8] = {0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD, 0xDD};
  uint8_t m_networkKey[16] = {1, 3, 5, 7, 9, 11, 13, 15, 0, 2, 4, 6, 8, 10, 12, 13};
  bool m_registered;
  bool m_started;
  bool m_joined;
  bool m_leaveAfterStart;
  int m_endpointCount;
  ZbEndpoint **m_endpoints; //MAX_DEVICE_ENDPOINTS
  DeviceStartedCB m_deviceStartedCB;
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

  void commandReceived(uint8_t command)
  {
    if(m_commandReceivedCB)
      m_commandReceivedCB(this, command);
  }

  void onCommandReceived(CommandReceivedCB commandReceivedCB) {
    m_commandReceivedCB = commandReceivedCB;
  }

protected:
  ZbEndpoint *m_endpoint;
private:
  uint16_t m_id;
  uint8_t m_role;
  int m_attrCount;
  ZbAttribute **m_attributes; //MAX_CLUSTER_ATTRIBUTES
  CommandReceivedCB m_commandReceivedCB;
};

class ZbAttribute {
public:
  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, void *data, uint16_t dataLen) {
    m_cluster = &cluster;
    m_id = id;
    m_type = type;
    m_access = access;
    if (dataLen > 0) {
      m_data = (uint8_t *) malloc(dataLen);
      memcpy(m_data, data, dataLen);
      m_dataOwned = true;
    }
    else {
      m_data = (uint8_t *) data;
      m_dataOwned = false;
    }
    m_isCustom = false;
    m_cluster->addAttribute(this);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, int8_t value) : ZbAttribute(cluster, id, type, access, &value, 1) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, int16_t value) : ZbAttribute(cluster, id, type, access, &value, 2) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, int32_t value) : ZbAttribute(cluster, id, type, access, &value, 4) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, uint8_t value) : ZbAttribute(cluster, id, type, access, &value, 1) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, uint16_t value) : ZbAttribute(cluster, id, type, access, &value, 2) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, uint32_t value) : ZbAttribute(cluster, id, type, access, &value, 4) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, float value) : ZbAttribute(cluster, id, type, access, &value, sizeof(float)) {
    //setValue(value);
  }

  //ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, char *value) : ZbAttribute(cluster, id, type, access, value, strlen(value) + 1) {
    //setValue(value);
  //}

  ZbAttribute(ZbCluster &cluster, uint16_t id, void *data, uint16_t dataLen) : ZbAttribute(cluster, id, 0, 0, data, dataLen) {
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, int8_t value) : ZbAttribute(cluster, id, 0, 0, &value, 1) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, int16_t value) : ZbAttribute(cluster, id, 0, 0, &value, 2) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, int32_t value) : ZbAttribute(cluster, id, 0, 0, &value, 4) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint8_t value) : ZbAttribute(cluster, id, 0, 0, &value, 1) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint16_t value) : ZbAttribute(cluster, id, 0, 0, &value, 2) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, uint32_t value) : ZbAttribute(cluster, id, 0, 0, &value, 4) {
    //setValue(value);
  }

  ZbAttribute(ZbCluster &cluster, uint16_t id, float value) : ZbAttribute(cluster, id, 0, 0, &value, sizeof(float)) {
    //setValue(value);
  }

  //ZbAttribute(ZbCluster &cluster, uint16_t id, char *value) : ZbAttribute(cluster, id, 0, 0, value, strlen(value) + 1) {
    //setValue(value);
  //}

  ~ZbAttribute() {
    if (m_data && m_dataOwned) free(m_data);
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

  uint8_t getType(void) {
    return m_type;
  }

  void setType(uint8_t value) {
    m_type = value;
  }

  uint8_t getAccess(void) {
    return m_access;
  }

  void setAccess(uint8_t value) {
    m_access = value;
  }

  bool isCustom(void) {
    return m_isCustom;
  }
/*
  ZbAttribute* setVar(void *varData) {
    m_data = (uint8_t *) varData;
    m_dataOwned = false;
    return this;
  }
*/
  void test(void);

  void setData(void *data, uint16_t dataLen);

/*
  void setData(void *data) {
    if(!data) {
      m_data = NULL;
      m_dataOwned = false;
    }
    else if(isStringAttrType(m_type)) 
      setStringData(data);
    else {
      uint8_t len = getAttrTypeLength();
      m_data = (uint8_t *) malloc(len);
      memcpy(m_data, data, len);
      m_dataOwned = true;    
    }
  }
*/
  void setValue(int8_t value) {
    int32_t value32 = value;
    setData(&value32, 4);
  }

  void setValue(int16_t value) {
    int32_t value32 = value;
    setData(&value32, 4);
  }

  void setValue(int32_t value) {
    setData(&value, 4);
  }

  void setValue(uint8_t value) {
    uint32_t value32 = value;
    setData(&value32, 4);
  }

  void setValue(uint16_t value) {
    uint32_t value32 = value;
    setData(&value32, 4);
  }

  void setValue(uint32_t value) {
    setData(&value, 4);
  }
  
  void setValue(float value) {
    setData(&value, sizeof(float));
  }
 
  void setStrValue(char* value)
  {
    //ESP_LOGI(TAG, "ZbAttribute::setValue(char* value)");
    if (m_data && m_dataOwned) free(m_data);
  //  if(m_type == ESP_ZB_ZCL_ATTR_TYPE_CHAR_STRING || m_type == ESP_ZB_ZCL_ATTR_TYPE_OCTET_STRING) { // дополнить!!!
    int len = strlen(value);
    m_data = (uint8_t *) malloc(len + 1);
    m_data[0] = len;
    memcpy(m_data + 1, value, len);
    m_dataOwned = true;
    updateValue();
  }

  void updateValue(void);

  void onValueChanged(AttributeValueChangedCB valueChangedCB) {
    m_valueChangedCB = valueChangedCB;
  }

  void valueChanged(uint8_t *data) 
  {
    if(m_valueChangedCB)
      m_valueChangedCB(this, data);
  }

  uint32_t getUInt(void) {
    return *(uint32_t*) m_data;
  }

  int32_t getInt(void) {
    return *(int32_t*) m_data;
  }

  uint8_t *getData(void) {
    return m_data;
  }

  void report(void);

protected:
  ZbCluster *m_cluster;
  uint16_t m_id;
  uint8_t m_type;
  uint8_t m_access;
  uint8_t *m_data;
  uint32_t m_intData;
  short m_floatData;
  bool m_dataOwned;
  bool m_isCustom;
  AttributeValueChangedCB m_valueChangedCB;
};

class ZbCustomAttribute : public ZbAttribute {
public:
  ZbCustomAttribute(ZbCluster &cluster, uint16_t id, uint8_t type, uint8_t access, void *data, uint16_t dataLen) : ZbAttribute(cluster, id, type, access, data, dataLen) {
    m_isCustom = true;
  }
};

//*****************************************************
class ZbBasicCluster : public ZbCluster {
public:
  ZbBasicCluster(ZbEndpoint &endpoint, char *manufacturerName, char *modelIdentifier, uint8_t applicationVersion, uint8_t powerSource) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_BASIC, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {
// https://csa-iot.org/wp-content/uploads/2019/12/07-5123-06-zigbee-cluster-library-specification.pdf
    m_zclVersion = 0x03;                                           
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

    //ESP_LOGI(TAG, "ZbBasicCluster()");
    m_attrManufacturerName = new ZbAttribute(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_MANUFACTURER_NAME_ID, NULL, 0);
    m_attrManufacturerName->setStrValue(m_manufacturerName);
    m_attrModelIdentifier = new ZbAttribute(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_MODEL_IDENTIFIER_ID, NULL, 0);
    m_attrModelIdentifier->setStrValue(m_modelIdentifier);
    m_attrZclVersion = new ZbAttribute(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_ZCL_VERSION_ID, m_zclVersion);
    m_attrPowerSource = new ZbAttribute(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_POWER_SOURCE_ID, m_powerSource);
    m_attrApplicationVersion = new ZbAttribute(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_BASIC_APPLICATION_VERSION_ID, m_applicationVersion);
  }

  ~ZbBasicCluster() {
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
  ZbAttribute *m_attrManufacturerName;
  ZbAttribute *m_attrModelIdentifier;
  ZbAttribute *m_attrZclVersion;
  ZbAttribute *m_attrPowerSource;
  ZbAttribute *m_attrApplicationVersion;
};

class ZbIdentifyCluster : public ZbCluster {
public:
  ZbIdentifyCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_IDENTIFY, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbOnOffCluster : public ZbCluster {
public:
  ZbOnOffCluster(ZbEndpoint &endpoint, uint8_t role = ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_ON_OFF, role) {}
private:
};
  
class ZbLevelControlCluster : public ZbCluster {
public:
  ZbLevelControlCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_LEVEL_CONTROL, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbBinaryInputCluster : public ZbCluster {
public:
  ZbBinaryInputCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_BINARY_INPUT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbAnalogInputCluster : public ZbCluster {
public:
  ZbAnalogInputCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_ANALOG_INPUT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbAnalogOutputCluster : public ZbCluster {
public:
  ZbAnalogOutputCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_ANALOG_OUTPUT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbMultiStateValueCluster : public ZbCluster {
public:
  ZbMultiStateValueCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_MULTI_VALUE, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbTemperatureMeasCluster : public ZbCluster {
public:
  ZbTemperatureMeasCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_TEMP_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbPressureMeasCluster : public ZbCluster {
public:
  ZbPressureMeasCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_PRESSURE_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbHumidityMeasCluster : public ZbCluster {
public:
  ZbHumidityMeasCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_REL_HUMIDITY_MEASUREMENT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbMultiValueCluster : public ZbCluster {
public:
  ZbMultiValueCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_MULTI_VALUE, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbThermostatCluster : public ZbCluster {
public:
  ZbThermostatCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_THERMOSTAT, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbDoorLockCluster : public ZbCluster {
public:
  ZbDoorLockCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_DOOR_LOCK, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbWindowCoveringCluster : public ZbCluster {
public:
  ZbWindowCoveringCluster(ZbEndpoint &endpoint) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_WINDOW_COVERING, ESP_ZB_ZCL_CLUSTER_SERVER_ROLE) {}
private:
};

class ZbOTACluster : public ZbCluster {
public:
  ZbOTACluster(ZbEndpoint &endpoint, uint16_t otaManufacturer, uint16_t otaImageType, uint32_t otaFileVersion) : ZbCluster(endpoint, ESP_ZB_ZCL_CLUSTER_ID_OTA_UPGRADE, ESP_ZB_ZCL_CLUSTER_CLIENT_ROLE) 
  {
    m_otaManufacturer = otaManufacturer;
    m_otaImageType = otaImageType;
    m_otaFileVersion = otaFileVersion;
    m_otaData.timer_query = ESP_ZB_ZCL_OTA_UPGRADE_QUERY_TIMER_COUNT_DEF;
    m_otaData.hw_version = 0x0001;
    m_otaData.max_data_size = 0x40; 

    m_attrOtaUpgradeClientData = new ZbAttribute(*(ZbCluster*)this, ESP_ZB_ZCL_ATTR_OTA_UPGRADE_CLIENT_DATA_ID, &m_otaData, sizeof(esp_zb_zcl_ota_upgrade_client_variable_t));
  }

  uint16_t getOtaManufacturer(void) {
    return m_otaManufacturer;
  }

  uint16_t getOtaImageType(void) {
    return m_otaImageType;
  }

  uint32_t getOtaFileVersion(void) {
    return m_otaFileVersion;
  }

private:
  uint16_t m_otaManufacturer;
  uint16_t m_otaImageType;
  uint32_t m_otaFileVersion;
  esp_zb_zcl_ota_upgrade_client_variable_t m_otaData;
  ZbAttribute *m_attrOtaUpgradeClientData;
};


// ***********************
class ZbRuntime {
public:
  ZbRuntime() {}
  ~ZbRuntime() {}

  void init(ZbDevice &device, bool appendMandatories) {
      g_runtime = this;
      m_device = &device;
      if(appendMandatories) 
        m_device->appendMandatories();
  }

  void start(void) {}

  virtual void leave(void) {}
  virtual uint8_t setAttributeValue(uint8_t endpoint, uint16_t clusterId, uint8_t clusterRole, uint16_t attrId, void *value, bool check) {}
  virtual uint8_t reportAttribute(uint8_t endpoint, uint16_t clusterId, uint16_t attributeId) {}

protected:
  ZbDevice *m_device;
};

uint8_t getClusterAttributeInfo(uint16_t clusterId, uint16_t attributeId, uint8_t *deviceType, bool *isMandatory);

#endif //__ZB_DEVICE_H__
