#ifndef __ZB_ZCL_H__
#define __ZB_ZCL_H__

#define ESP_ZB_ZCL_ATTR_ACCESS_RPS (ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING | ESP_ZB_ZCL_ATTR_ACCESS_SCENE)
#define ESP_ZB_ZCL_ATTR_ACCESS_RP (ESP_ZB_ZCL_ATTR_ACCESS_READ_ONLY | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING)
#define ESP_ZB_ZCL_ATTR_ACCESS_RWS (ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_SCENE)
#define ESP_ZB_ZCL_ATTR_ACCESS_RWP (ESP_ZB_ZCL_ATTR_ACCESS_READ_WRITE | ESP_ZB_ZCL_ATTR_ACCESS_REPORTING)

typedef struct zb_attr_info_s {
  uint16_t attrId;
  uint8_t attrType;
  uint8_t attrAccess;
  bool mandatory;
  void *defaultValue;                        
} zb_attr_info_t;

typedef struct zb_cluster_info_s {
  uint16_t clusterId;
  zb_attr_info_t *attrs;
  uint16_t attrCount; 
  struct zb_cluster_info_s *next;                      
} zb_cluster_info_t;

void initZclData(void);
zb_cluster_info_t *findClusterInfo(uint16_t clusterId);
zb_attr_info_t *findClusterAttrInfo(uint16_t clusterId, uint16_t attrId);

bool isVarAttrType(uint8_t attrType);
uint8_t getAttrTypeLength(uint8_t attrType);

void dumpClusterInfos(void);

#endif //__ZB_ZCL_H__