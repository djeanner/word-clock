#include <Arduino.h>
#include "hardware/pwm.h"
#include "hardware/irq.h"

#define MAX_LEDS 6
#define PWM_MASTER_GPIO 22   // not connected to LEDs

volatile uint32_t ledMask = 0;
volatile bool pwmState = false;

uint pwmSlice;
uint16_t pwmTop = 1000;
uint16_t pwmLevel = 500;

/* ---------- PWM IRQ ---------- */
void pwm_wrap_isr() {
  pwm_clear_irq(pwmSlice);

  pwmState = !pwmState;  // toggle phase

  for (uint gpio = 0; gpio <= 21; gpio++) {
    if (ledMask & (1u << gpio)) {
      // Open collector:
      // LOW  -> ON
      // HIGH -> OFF
      digitalWrite(gpio, pwmState ? LOW : HIGH);
    }
  }
}

/* ---------- Configure PWM ---------- */
void setupPWM(float frequency, float dutyCycle) {
  pinMode(PWM_MASTER_GPIO, OUTPUT);

  gpio_set_function(PWM_MASTER_GPIO, GPIO_FUNC_PWM);
  pwmSlice = pwm_gpio_to_slice_num(PWM_MASTER_GPIO);

  pwm_config cfg = pwm_get_default_config();

  // 125 MHz / 125 = 1 MHz PWM clock
  pwm_config_set_clkdiv(&cfg, 125.0f);

  pwmTop = (uint16_t)(1000000.0f / frequency);
  pwmLevel = (uint16_t)(pwmTop * dutyCycle);

  pwm_config_set_wrap(&cfg, pwmTop);

  pwm_init(pwmSlice, pwmSlice, &cfg, false);
  pwm_set_gpio_level(PWM_MASTER_GPIO, pwmLevel);

  pwm_clear_irq(pwmSlice);
  pwm_set_irq_enabled(pwmSlice, true);

  irq_set_exclusive_handler(PWM_IRQ_WRAP, pwm_wrap_isr);
  irq_set_enabled(PWM_IRQ_WRAP, true);

  pwm_set_enabled(pwmSlice, true);
}

/* ---------- Select active GPIOs ---------- */
void setActiveGPIOs(const uint8_t *gpios, uint8_t count) {
  ledMask = 0;

  if (count > MAX_LEDS) count = MAX_LEDS;

  for (uint8_t i = 0; i < count; i++) {
    uint8_t g = gpios[i];
    if (g <= 21) {
      pinMode(g, OUTPUT);
      digitalWrite(g, HIGH); // OFF (open collector)
      ledMask |= (1u << g);
    }
  }
}

/* ---------- Arduino setup ---------- */
void setup() {
  uint8_t leds1[] = {2, 4, 6};
  setActiveGPIOs(leds1, 3);

  setupPWM(1000.0f, 0.5f); // 1 kHz, 50%
}

/* ---------- Arduino loop ---------- */
void loop() {
  static uint32_t lastSwitch = 0;
  static bool state = false;

  if (millis() - lastSwitch > 5000) {
    lastSwitch = millis();
    state = !state;

    if (state) {
      uint8_t leds2[] = {10, 11, 12, 13};
      setActiveGPIOs(leds2, 4);
    } else {
      uint8_t leds1[] = {2, 4, 6};
      setActiveGPIOs(leds1, 3);
    }
  }
}
