#ifndef __ESP_SERVO_H__
#define __ESP_SERVO_H__

#include <Arduino.h>
#include "esp_log.h"

#define TAG "esp_servo"

class EspServo {
public:
  EspServo() {
    
  }

  void attach(uint8_t pin, uint16_t min, uint16_t max)
  {
    m_pin = pin;
    m_min = min;
    m_max = max;
    pinMode(pin, OUTPUT);            // Setting the signal output pin
    ledcAttach(pin, 50, 16);
    //delay(20); //some ESCs need a initial stop signal to connect
  }

  void detach(void)
  {
    ledcDetach(m_pin);
  }

  uint16_t read(void)
  {
    return ledcRead(m_pin);
  }

  void write(uint16_t position)
  {  
    //uint16_t dutyCycle;    // Duty Cycle of the pwm signal generated

    position = constrain(position, m_min, m_max);
    //dutyCycle = map(position, m_min, m_max, 500, 2500);  // Dutycycle to generate a pwm signal with that width in microseconds(us)
    ledcWrite(m_pin, position);                         // generating a PWM with the proper width in us
    // ledcFade(uint8_t pin, uint32_t start_duty, uint32_t target_duty, int max_fade_time_ms){
  }

private:
  uint8_t m_pin; 
  uint16_t m_min; 
  uint16_t m_max;
};

class ServoAdapter;

typedef void (*ServoMoveCB)(ServoAdapter *servo, uint16_t position);
typedef void (*ServoCompleteCB)(ServoAdapter *servo);

class ServoAdapter {
public:
  ServoAdapter(uint8_t pin, uint16_t min, uint16_t max) {
    m_pin = pin;
    m_min = min;
    m_max = max;
    m_duration = 1500;
    m_moving = false;
  }

  uint8_t getPin()
  {
    return m_pin;
  }

  void setPin(uint8_t value)
  {
    m_pin = value;
  }

  uint16_t getMin()
  {
    return m_min;
  }

  void setMin(uint16_t value)
  {
    m_min = value;
  }

  uint8_t getMax()
  {
    return m_max;
  }

  void setMax(uint8_t value)
  {
    m_max = value;
  }

  bool getDuration()
  {
    return m_duration;
  }

  void setDuration(uint16_t value)
  {
    m_duration = value;
  }

  bool getMoving(void)
  {
    return m_moving;
  }

  void startMoving(void)
  {
    startTime = millis();
    m_moving = true;
  }

  void move(uint16_t position)
  {
    if(m_moveCB) 
      m_moveCB(this, position);
    startMoving();
  }

  void loop(void)
  {
    if(!m_moving) return;
    if (millis() - startTime < m_duration) return;
    m_moving = false;  
    if (m_completeCB)
      m_completeCB(this);
  }

  void onMove(ServoMoveCB moveCB)
  {
    m_moveCB = moveCB;
  }

  void onComplete(ServoCompleteCB completeCB)
  {
    m_completeCB = completeCB;
  }

private:
  uint8_t m_pin;
  uint16_t m_min; 
  uint16_t m_max;
  uint16_t m_duration;
  bool m_moving;
  unsigned long startTime;
  ServoMoveCB m_moveCB;
  ServoCompleteCB m_completeCB;
};

#endif //__ESP_SERVO_H__
