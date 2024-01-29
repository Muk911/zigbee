#ifndef __ZBESP_RUNTIME_H__
#define __ZBESP_RUNTIME_H__

#include <stdlib.h>
#include <stdint.h>
#include "zb_device.h"

#define TAG "zbesp_runtime"

class ZbRuntimeESP  {
public:
  ZbRuntimeESP() {
  }
  ~ZbRuntimeESP() {}

  void start(ZbDevice &device);

private:
  ZbDevice *m_device;
  
};

#endif // __ZBESP_RUNTIME_H__
