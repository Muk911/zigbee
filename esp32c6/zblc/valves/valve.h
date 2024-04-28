#ifndef __VALVE_H__
#define __VALVE_H__

#include <stdlib.h>
#include <stdint.h>
#include <Arduino.h>

#define TAG "zbesp_debug"
 
class Valve;

typedef void (*RelaySetStateCB)(uint8_t relayNo, uint8_t state);
typedef void (*RelaySetPolarityCB)(uint8_t relayNo, uint8_t polarity);
typedef void (*ValveStateChangedCB)(Valve *valve);

/*
class PolarityRelays {
public:
	PolarityRelays(uint8_t relay1No, uint8_t relay2No) {
    m_polarity = 0; // 0 - нормальная, 1 - обратная 
	}
	~PolarityRelays() {}

  uint8_t getPolarity(void)
  {
    return m_polarity;
  }

  void setPolarity(uint8_t value)
  {
    assert(m_setStateCB);
    m_setStateCB(m_relay1No, value);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    m_setStateCB(m_relay2No, value);
    vTaskDelay(20 / portTICK_PERIOD_MS);
    m_polarity = value;
  }

  void onSetState(RelaySetStateCB setStateCB)
  {
    m_setStateCB = setStateCB;
  }

private:
  uint8_t m_polarity;
  uint8_t m_relay1No;
  uint8_t m_relay2No;
  RelaySetStateCB m_setStateCB;
};
*/

class Valve {
public:
	Valve(int relayNo, bool bistable = false) {
    m_relayNo = relayNo;
    m_state = 0;
    m_duration = 3600;
    m_start = 0;
    m_bistable = bistable;
	}

	~Valve() {}

  uint8_t getRelayNo()
  {
    return m_relayNo;
  }

  uint8_t getState()
  {
    return m_state;
  }

  void setState(uint8_t value)
  {
    ESP_LOGI(TAG, "setState %d", value);
  //  if (m_state == value) return;
    assert(m_relaySetPolarityCB);

    if (m_bistable) {
      m_relaySetPolarityCB(m_relayNo, value);
      vTaskDelay(20 / portTICK_PERIOD_MS);
      m_relaySetStateCB(m_relayNo, 1);
      vTaskDelay(200 / portTICK_PERIOD_MS); // задержку в свойства
      m_relaySetStateCB(m_relayNo, 0);
      vTaskDelay(20 / portTICK_PERIOD_MS);
      m_relaySetPolarityCB(m_relayNo, 0);
    }
    else {
      m_relaySetStateCB(m_relayNo, value);
    }
    m_state = value;
    if (value) 
      m_start = millis();
    if (m_stateChangedCB) 
      m_stateChangedCB(this);
  }

  void toggle()
  {
    setState(m_state ? 0 : 1);
  }

  uint8_t getDuration()
  {
    return m_duration;
  }

  void setDuration(uint8_t value)
  {
    m_duration = value;
  }

  bool getBistable()
  {
    return m_bistable;
  }

  void setBistable(bool value)
  {
    m_bistable = value;
  }

  uint16_t getUserData()
  {
    return m_userData;
  }

  void setUserData(uint16_t value)
  {
    m_userData = value;
  }

  void onRelaySetState(RelaySetStateCB relaySetStateCB)
  {
    m_relaySetStateCB = relaySetStateCB;
  }

  void onRelaySetPolarity(RelaySetPolarityCB relaySetPolarityCB)
  {
    m_relaySetPolarityCB = relaySetPolarityCB;
  }

  void onStateChanged(ValveStateChangedCB stateChangedCB)
  {
    m_stateChangedCB = stateChangedCB;
  }

  void loop(void);

private:
  uint8_t m_relayNo;
  uint8_t m_state;
  uint16_t m_duration;
  uint32_t m_start; // время запуска
  uint8_t m_bistable;
  uint16_t m_userData;
  ValveStateChangedCB m_stateChangedCB;
  RelaySetStateCB m_relaySetStateCB;
  //PolarityRelays* m_polarityRelays; 
  RelaySetPolarityCB m_relaySetPolarityCB;
 
};

#endif