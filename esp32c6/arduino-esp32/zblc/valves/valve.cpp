#include "valve.h"

#define TAG "zbesp_debug"

void Valve::loop(void)
{
  if (m_state && (millis() - m_start > 1000 * m_duration)) {
    ESP_LOGI(TAG, "Time Out"); 
    setState(0); // выключаем клапан по истечению интервала
  }
}
