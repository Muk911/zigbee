#ifndef __ZB_BUTTON_H__
#define __ZB_BUTTON_H__

#include <Arduino.h>
#include <OneButton.h>
#include "esp_log.h"
//#include "original/button_gpio.h"
//#include "original/button_adc.h"
//#include "original/iot_button.h"
#include "zb_device.h"

#define TAG "zb_button"

class ZbDeviceButton : public OneButton {
public:
  explicit ZbDeviceButton(ZbDevice &device, uint8_t pin) : OneButton(pin) {
    m_device = &device;
    assert(m_device);
    //attachClick([](void *scope) { ((ZbDeviceButton *) scope)->Clicked();}, this);
    attachLongPressStart([](void *scope) { ((ZbDeviceButton *) scope)->LongPressStarted();}, this);
    OneButton::setPressMs(4000);
  }

  void LongPressStarted() {
    ESP_LOGI(TAG, "LongPressStarted()");
    assert(m_device);
    m_device->leave();
  }

  void Clicked() {
    ESP_LOGI(TAG, "*Clicked()");
    assert(m_device);
    m_device->reportNow();
  }

  void setPressMs(const unsigned int ms) {} // Запрещаем изменение времени длинного нажатия

private:
  ZbDevice *m_device;
};

#endif //__ZB_BUTTON_H__
