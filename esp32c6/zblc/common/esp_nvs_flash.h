#ifndef __ESP_NVS_FLASH_H__
#define __ESP_NVS_FLASH_H__

#include <Arduino.h>
#include "esp_log.h"
#include "nvs_flash.h"

#define TAG "esp_nvs_flash"

class NvsVariable {
public:
  NvsVariable(uint8_t size, char *varName, uint16_t varIndex) {
    m_size = size;
    m_varName = (char *) malloc(strlen(varName) + 10);
    strcpy(m_varName, varName);
    itoa(varIndex, m_varName + strlen(varName), 9);
  }
  
  ~NvsVariable() {
    free(m_varName);
  }

  bool checkNvsFlashInitPartition() {
    static bool nvsInitialized = false;

    if(nvsInitialized) return true;
    esp_err_t err = nvs_flash_init_partition("nvs");
    if (err != ESP_OK) {
      ESP_LOGE(TAG, "Error (%s) opening NVS handle!\n", esp_err_to_name(err));
      return false;
    }
    nvsInitialized = true;
    return true;
  }

  bool read(void *data)
  {
    esp_err_t err;
    nvs_handle_t handle;

    if (!checkNvsFlashInitPartition()) return false;
    err = nvs_open_from_partition("nvs", "storage", NVS_READONLY, &handle);
    if (err != ESP_OK) return false;
    size_t len = m_size;
    err = nvs_get_blob(handle, m_varName, data, &len);
    nvs_close(handle);
//    ESP_LOGI(TAG, "NvsVariable->read %s", m_varName);
    return (err == ESP_OK);
  }

  uint32_t readInt(uint32_t defaultValue)
  {
    uint32_t value = 0;

    if (!read(&value)) value = defaultValue;
    return value;
  }

  bool write(void *data)
  {
    esp_err_t err;
    nvs_handle_t handle;

    if (!checkNvsFlashInitPartition()) return false;
    err = nvs_open_from_partition("nvs", "storage", NVS_READWRITE, &handle);
    if (err != ESP_OK) return false;
    err = nvs_set_blob(handle, m_varName, data, m_size);
    nvs_commit(handle);
    nvs_close(handle);
    // ESP_LOGI(TAG, "NvsVariable->write %s", m_varName);
    return (err == ESP_OK);
  }

  bool writeInt(uint32_t value)
  {
    return write(&value);
  }

private:
  uint8_t m_size;
  char* m_varName;
};


#endif //__ESP_NVS_FLASH_H__
