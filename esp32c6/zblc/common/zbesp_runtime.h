#ifndef __ZBESP_RUNTIME_H__
#define __ZBESP_RUNTIME_H__

#include <stdlib.h>
#include <stdint.h>
#include "esp_log.h"
#include "zb_device.h"

#define TAG "zbesp_runtime"

class EspZbRuntime : public ZbRuntime {
public:
  EspZbRuntime() : ZbRuntime() {}
  ~EspZbRuntime() {}

  void init(ZbDevice &device, bool appendMandatories = false);
  void start(void);
  
  void leave(void) override ;
  uint8_t setAttributeValue(uint8_t endpoint, uint16_t clusterId, uint8_t clusterRole, uint16_t attrId, void *value, bool check) override ;
  uint8_t reportAttribute(uint8_t endpoint, uint16_t clusterId, uint16_t attributeId) override ;
//  void reportNow();
};

#endif // __ZBESP_RUNTIME_H__
