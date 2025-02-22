#ifndef __ZB_BUTTON_H__
#define __ZB_BUTTON_H__

#include <Arduino.h>
#include <OneButton.h>
#include "esp_log.h"
#include "zb_device.h"

#define TAG "zb_button"

#define TURN_ON_OFF_LEAVE_NAMESPACE "nvs"
#define TURN_ON_OFF_LEAVE_VARNAME "_reset_counter_"
#define TURN_ON_OFF_LEAVE_COUNT 3
#define TURN_ON_OFF_LEAVE_DURATION 8000

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

class ZbTurnOnOffLeave {
public:
  ZbTurnOnOffLeave(ZbDevice &device, uint8_t count = TURN_ON_OFF_LEAVE_COUNT, uint16_t duration = TURN_ON_OFF_LEAVE_DURATION) {
    m_device = &device;
    m_count = count;
    m_duration = duration;
    m_startTime = millis();
  }

  void init() {
    nvs_handle_t handle;
    uint8_t resetCounter;
  
    // вызывается в setup()
    // проверяет счетчик NVRAM
    // если значение счетчика больше N:
    //    - счетчик удаляется
    //    - устройство сбрасывается
    // иначе счетчик увеличивается на 1
    if (nvs_flash_init() != ESP_OK) return;
    if (nvs_open(TURN_ON_OFF_LEAVE_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return;
    esp_err_t err = nvs_get_u8(handle, TURN_ON_OFF_LEAVE_VARNAME, &resetCounter);
    if (err != ESP_OK) resetCounter = 0;
    //ESP_LOGI(TAG, "*** read resetCounter=%d", resetCounter);
    if (resetCounter < m_count) {
      resetCounter++;
      ESP_ERROR_CHECK(nvs_set_u8(handle, TURN_ON_OFF_LEAVE_VARNAME, resetCounter));
      //ESP_ERROR_CHECK(nvs_set_blob(handle, TURN_ON_OFF_LEAVE_VARNAME, &resetCounter, 1));
      ESP_ERROR_CHECK(nvs_commit(handle));
      //ESP_LOGI(TAG, "*** write resetCounter=%d", resetCounter);
    }
    else {
      ESP_LOGI(TAG, "*Device leave the network after start.");
      ESP_ERROR_CHECK(nvs_erase_key(handle, TURN_ON_OFF_LEAVE_VARNAME));
      ESP_ERROR_CHECK(nvs_commit(handle));
      m_device->leaveAfterStart();
    }
    nvs_close(handle);
  }

  void loop() {
    nvs_handle_t handle;
  
    if (!m_startTime) return;
    if (millis() - m_startTime < m_duration) return;
    // если прошло более 8с с момента загрузки, удаляем счетчик из NVRAM
    if (nvs_flash_init() != ESP_OK) return;
    if (nvs_open(TURN_ON_OFF_LEAVE_NAMESPACE, NVS_READWRITE, &handle) != ESP_OK) return;
    ESP_ERROR_CHECK(nvs_erase_key(handle, TURN_ON_OFF_LEAVE_VARNAME));
    ESP_ERROR_CHECK(nvs_commit(handle));
    nvs_close(handle);
    m_startTime = 0;
  }

private:
  ZbDevice *m_device;
  uint8_t m_count;
  uint16_t m_duration;
  unsigned long m_startTime;
};

#endif //__ZB_BUTTON_H__
