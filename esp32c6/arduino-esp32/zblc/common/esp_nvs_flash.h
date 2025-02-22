#ifndef __ESP_NVS_FLASH_H__
#define __ESP_NVS_FLASH_H__

#include <Arduino.h>
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG "esp_nvs_flash"

class NvsVariable {
public:
  NvsVariable(uint8_t size, char *varName, int16_t varIndex = -1) {
    m_size = size;
    m_varName = (char *) malloc(strlen(varName) + 10);
    strcpy(m_varName, varName);
    if (varIndex >= 0)
      itoa(varIndex, m_varName + strlen(varName), 9);
    //ESP_LOGI(TAG, "%s", m_varName);
    assert(strlen(m_varName) <= 15); // Maximal length is 15 characters.
  }
  
  ~NvsVariable() {
    free(m_varName);
  }

  bool read(void *data)
  {
    esp_err_t err;
    nvs_handle_t handle;

    if (nvs_flash_init() != ESP_OK) return false;
    err = nvs_open("storage", NVS_READONLY, &handle);
    if (err != ESP_OK) return false;
    size_t len = m_size;
    err = nvs_get_blob(handle, m_varName, data, &len);
    assert(err == ESP_OK || err == ESP_ERR_NVS_NOT_FOUND);
    if (err == ESP_ERR_NVS_NOT_FOUND) ESP_LOGI(TAG, "NvsVariable '%s' not found.", m_varName);
    nvs_close(handle);
    //ESP_LOGI(TAG, "NvsVariable->read %s", m_varName);
    return (err == ESP_OK);
  }

  int32_t readInt(int32_t defaultValue)
  {
    int32_t value = 0;

    if (!read(&value)) value = defaultValue;
    //ESP_LOGI(TAG, "NvsVariable->readInt %s -> %d", m_varName, value);
    return value;
  }

  bool write(void *data)
  {
    esp_err_t err;
    nvs_handle_t handle;

    if (nvs_flash_init() != ESP_OK) return false;
    err = nvs_open("storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) return false;
    ESP_ERROR_CHECK(err = nvs_set_blob(handle, m_varName, data, m_size));
    nvs_commit(handle);
    nvs_close(handle);
    //ESP_LOGI(TAG, "NvsVariable->write %s", m_varName);
    return (err == ESP_OK);
  }

  bool writeInt(int32_t value)
  {
    //ESP_LOGI(TAG, "NvsVariable->writeInt %s <- %d", m_varName, value);
    return write(&value);
  }

private:
  uint8_t m_size;
  char* m_varName;
};


#endif //__ESP_NVS_FLASH_H__
