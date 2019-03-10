#pragma once

#include "driver/mcpwm.h"

class Motor {
public:
  struct PWMSettings {
    mcpwm_unit_t unit;
    mcpwm_io_signals_t outputSignal;
    mcpwm_timer_t timer;
    mcpwm_operator_t opr;
  };

  struct PPMSettings {
    int frequency;
    int minPulseUs;
    int maxPulseUs;
  };
 
  Motor(const PWMSettings& pwmSettings, const PPMSettings& ppmSettings, int gpioPin);
  ~Motor();

  void setSpeed(float speed);
  float getSpeed() const;

private:
  PWMSettings m_pwmSettings;
  PPMSettings m_ppmSettings;

  float m_setSpeed;
};
