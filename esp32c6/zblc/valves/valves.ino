#ifndef ZIGBEE_MODE_ED
#error "Zigbee end device mode is not selected in Tools->Zigbee mode"
#endif

#include "esp_log.h"
#include "esp_check.h"
#include "esp_random.h"
#include "zb_device.h"
#include "zbesp_runtime.h"
#include "zb_button.h"
#include "valve.h"

#define TAG "zed_runtime"
//#define BISTABLE_VALVE

#define ZCL_VERSION         0x03
#define APP_VERSION         0x01
#define POWER_SOURCE        0x04

#define VALVE_COUNT 5

ZbDevice zd(ESP_ZB_DEVICE_TYPE_ED);
ZbEndpoint ep(zd, VALVE_COUNT + 1, DEVICE_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);
ZbBasicCluster zcBasic(ep, "MEA", "valves", APP_VERSION, POWER_SOURCE);
ZbIdentifyCluster zcIdentify(ep);

ZbEndpoint* eps[VALVE_COUNT];
ZbOnOffCluster* zcOnOff[VALVE_COUNT];
ZbAttribute* zaOnOff[VALVE_COUNT];;
ZbAttribute* zaOnTime[VALVE_COUNT];

EspZbRuntime rt;

ZbDeviceButton button(zd, GPIO_NUM_9);

#ifndef REG595
#define RELAY_COUNT 8
unsigned int relayPins[RELAY_COUNT] = {16, 14, 12, 13, 15, 0, 4, 5};
#else
const int dataPin = 14;
const int clockPin = 13;
const int latchPin = 12;
const int oePin = 5;
const int srCount = 2;
uint16_t data595 = 0;
#endif

#define DEFAULT_DURATION 25
unsigned int valveRelays[VALVE_COUNT] = {3, 4, 5, 7, 8}; // номера реле с единицы
Valve* valves[VALVE_COUNT];

#ifdef BISTABLE_VALVE
unsigned int valvePolarityRelays[2] = {1, 6}; // номера реле для изменения полярности питания клапанов (с единицы)
#endif

static void OnOff_StateChanged(ZbAttribute *za, uint8_t *data)
{
  int i = za->getEndpoint()->getId() - 1;
  ESP_LOGI(TAG, "OnOff_StateChanged %d", i);
  valves[i]->setState(*data);
}

// с координатора пришло изменение значения
static void OnOff_OnTimeChanged(ZbAttribute *za, uint8_t *data)
{
  int i = za->getEndpoint()->getId() - 1;
  ESP_LOGI(TAG, "OnOff_OnTimeChanged %d: %d", i, *(uint16_t *) data); 
  uint16_t duration = (*(uint16_t *) data) / 10;
  valves[i]->setDuration(duration);
}

static void Valve_StateChanged(Valve *valve)
{
  uint16_t i = valve->getUserData();
  uint16_t state = valve->getState();
  ESP_LOGI(TAG, "Valve_StateChanged %d: %d", i, state); 
  zaOnOff[i]->setValue(state);

  if (i == 0) {
    uint8_t v = state ? 20 : 0;
    neopixelWrite(RGB_BUILTIN,v,v,v);
  }
}

void buttonClick(void)
{
  ESP_LOGI(TAG, "buttonClick()");
  uint8_t newValue = zaOnOff[0]->getValue() ? 0 : 1;
  zaOnOff[0]->setValue(newValue);
  //setRelayState(0, relayStates[0] ? 0 : 1);
  //zaOnTime[0]->setValue((uint16_t) 0);
}

void Relay_SetState(uint8_t relayNo, uint8_t state)
{
  ESP_LOGI(TAG, "Relay_SetState relayNo=%d state=%d", relayNo, state);
  // установить реле relayNo в положение state
#ifdef REG595
  if (state) 
    data595 |= (1u << (relayNo - 1)); 
  else
    data595 &= ~(1u << (relayNo - 1));
  digitalWrite(latchPin, LOW);
  for (int k = srCount - 1; k >= 0; k--) 
    shiftOut(dataPin, clockPin, MSBFIRST, (data595 >> (8 * k)) & 0XFFu);
  digitalWrite(latchPin, HIGH);
#else
  ESP_LOGI(TAG, "digitalWrite %d - %d", relayPins[relayNo - 1], (state ? HIGH : LOW));
  digitalWrite(relayPins[relayNo - 1], (state ? HIGH : LOW));
#endif  
}

#ifdef BISTABLE_VALVE
void Relay_SetPolarity(uint8_t relayNo, uint8_t polarity)
{
  ESP_LOGI(TAG, "Relay_SetPolarity relayNo=%d polarity=%d", relayNo, polarity);
  for (int i = 0; i < 2; i++) {
    Relay_SetState(valvePolarityRelays[i - 1], polarity); // если надо включить тригерный клапан, меняем полярность
    vTaskDelay(20 / portTICK_PERIOD_MS);
  }
}
#endif

void relaysInit(void)
{
#ifdef REG595
  pinMode(latchPin, OUTPUT);
  pinMode(clockPin, OUTPUT);
  pinMode(dataPin, OUTPUT);
  if (oePin) {
    pinMode(oePin, OUTPUT);
    digitalWrite(oePin, LOW); // ??? https://docs.arduino.cc/tutorials/communication/guide-to-shift-out#shftout11
  }
#else
  for (int k = 2; k <= RELAY_COUNT; k++) {
    ESP_LOGI(TAG, "pinMode %d", relayPins[k - 1]);
    pinMode(relayPins[k - 1], OUTPUT);
    ESP_LOGI(TAG, "digitalWrite %d - %d", relayPins[k - 1], LOW);
    digitalWrite(relayPins[k - 1], LOW);
  }
#endif
}

void setup()
{
  Serial.begin(115200);
  Serial.println("==========================================================================");

  relaysInit();

  for(int i = 0; i < VALVE_COUNT; i++) {
#ifdef BISTABLE_VALVE
    valves[i] = new Valve(valveRelays[i], true);
    valves[i]->onRelaySetPolarity(Relay_SetPolarity);
#else
    valves[i] = new Valve(valveRelays[i]);
#endif
    valves[i]->setDuration(DEFAULT_DURATION);    
    valves[i]->setUserData(i);
    valves[i]->onRelaySetState(Relay_SetState);
    valves[i]->onStateChanged(Valve_StateChanged);
    //valves[i]->init(); // !!!!!!!!!!!!!!!!! установка режима и состояния пинов
  }

  button.attachClick(buttonClick);

  for(int i = 0; i < VALVE_COUNT; i++) {
    eps[i] = new ZbEndpoint(zd, i + 1, DEVICE_PROFILE_ID, ESP_ZB_HA_TEST_DEVICE_ID);
    zcOnOff[i] = new ZbOnOffCluster(*eps[i]);
    zaOnOff[i] = new ZbAttribute(*zcOnOff[i], ESP_ZB_ZCL_ATTR_ON_OFF_ON_OFF_ID, (uint8_t) 0);
    zaOnOff[i]->onValueChanged(OnOff_StateChanged);
    zaOnTime[i] = new ZbAttribute(*zcOnOff[i], ESP_ZB_ZCL_ATTR_ON_OFF_ON_TIME, (uint16_t) (10 * DEFAULT_DURATION));
    zaOnTime[i]->onValueChanged(OnOff_OnTimeChanged);
    //relayUpdate(i);
  }

  rt.init(zd, true);
  rt.start();
}

void loop() {
  button.tick();
  for(int i = 0; i < VALVE_COUNT; i++)
    valves[i]->loop();
  vTaskDelay(20 / portTICK_PERIOD_MS);
}
