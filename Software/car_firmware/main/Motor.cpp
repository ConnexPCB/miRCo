#include "Motor.hpp"


Motor::Motor(const PWMSettings& pwmSettings, const PPMSettings& ppmSettings, int gpioPin)
  : m_pwmSettings{pwmSettings}
  , m_ppmSettings{ppmSettings}
  , m_setSpeed{0.f} {

  mcpwm_gpio_init(m_pwmSettings.unit, m_pwmSettings.outputSignal, gpioPin);

  mcpwm_config_t pwmConfig;
  pwmConfig.frequency = m_ppmSettings.frequency;
  pwmConfig.cmpr_a = 0;
  pwmConfig.cmpr_b = 0;
  pwmConfig.counter_mode = MCPWM_UP_COUNTER;
  pwmConfig.duty_mode = MCPWM_DUTY_MODE_0;
  mcpwm_init(m_pwmSettings.unit, m_pwmSettings.timer, &pwmConfig);
}

Motor::~Motor() {
}

void Motor::setSpeed(float speed) {
  m_setSpeed = speed;

  uint32_t pulseUs = static_cast<uint32_t>(m_ppmSettings.minPulseUs +
    speed*(m_ppmSettings.maxPulseUs - m_ppmSettings.minPulseUs));
  mcpwm_set_duty_in_us(m_pwmSettings.unit, m_pwmSettings.timer, m_pwmSettings.opr,
    pulseUs);
};

float Motor::getSpeed() const {
  return m_setSpeed;
}
