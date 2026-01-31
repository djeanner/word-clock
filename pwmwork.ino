//#include <Arduino.h>
#include "hardware/pwm.h"
#include "hardware/irq.h"

#define MAX_LEDS_PER_PWM 6
#define PWM_MASTER_GPIO 22   // not connected to LEDs


#define H1_ 0
#define H2_ 1
#define H3_ 2
#define H4_ 3
#define H5_ 9
#define H6_ 5
#define H7_ 6
#define H8_ 7
#define H9_ 8 
#define H10 4
#define H11 10
#define H12 11
//#define PM 12
//#define AM 13
#define HTO 14
#define HPAST 15
#define HALF 16
#define M10 17
#define M5 18
#define MITIS 12 // NEW alwas on
#define MOCLOCK  19
#define MQUARTER 20 
#define MTWENTY 13 // was 22
#define EXTRA 21 // new
#define RADIOINPUT 28 // 28: GP28 A2
#define LEDPIN 25

volatile uint32_t ledMask = 0;
volatile bool pwmState = false;

uint pwmSlice;
uint16_t pwmTop = 1000;
uint16_t pwmLevel = 500;

const size_t totOutputPins = 22;
size_t values[totOutputPins];

void ocReleaseAll() {
  for (size_t lo = 0; lo < totOutputPins; lo++) { 
    pinMode(lo, INPUT);   // no pull
  }
}

void ocDriveLowAll(size_t cycles = 100) {
  for (size_t t = 0; t < cycles; t++) {

    for (size_t lo = 0; lo < totOutputPins; lo++) { 
      pinMode(lo, INPUT);   // no pull
    }
    delay(10);
    for (size_t lo = 0; lo < totOutputPins; lo++) { const size_t pin = lo;
        
      if (values[pin] > 0) {
        pinMode(pin, OUTPUT);
        digitalWrite(pin, LOW);
      } else {
        pinMode(pin, INPUT);
        //digitalWrite(pin, LOW); // dummy
      }
       
    }
  }
}

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

  if (count > MAX_LEDS_PER_PWM) count = MAX_LEDS_PER_PWM;

  for (uint8_t i = 0; i < count; i++) {
    uint8_t g = gpios[i];
    if (g <= 21) {
      pinMode(g, OUTPUT);
      digitalWrite(g, HIGH); // OFF (open collector)
      ledMask |= (1u << g);
    }
  }
}

void displayUnit(int unitDigit = 0, int numverCycles = 5) {
  for (size_t lo = 0; lo < totOutputPins; lo++) {
    const size_t pin = lo;
    values[pin] = 0;
  }
  if (unitDigit == 1) {values[H1_] = 1;}
  if (unitDigit == 2) {values[H2_] = 1;}
  if (unitDigit == 3) {values[H3_] = 1;}
  if (unitDigit == 4) {values[H4_] = 1;}
  if (unitDigit == 5) {values[H5_] = 1;}
  if (unitDigit == 6) {values[H6_] = 1;}
  if (unitDigit == 7) {values[H7_] = 1;}
  if (unitDigit == 8) {values[H8_] = 1;}
  if (unitDigit == 9) {values[H9_] = 1;}
  if (unitDigit == 10) {values[H10] = 1;}
  if (unitDigit == 11) {values[H11] = 1;}
  if (unitDigit == 12) {values[H12] = 1;}
  if (unitDigit == 0) {ocDriveLowAll(1);}
  else { ocDriveLowAll(numverCycles);}
  return;
}

/* ---------- Arduino setup ---------- */
void setup() {
  ocReleaseAll();
  for (size_t lo = 0; lo < totOutputPins; lo++) {
    const size_t pin = lo;
    values[pin] = 0;
  }
  uint8_t leds1[] = {2, 4, 6};
  setActiveGPIOs(leds1, 3);

  setupPWM(1000.0f, 0.1f); // 1 kHz, 50%
  displayUnit(8,100);
displayUnit(6,100);
displayUnit(1,100);
}
void disablePWM() { // disablePWMAndReset

  // Disable PWM slice
  pwm_set_enabled(pwmSlice, false);

  // Disable and clear PWM IRQ
  pwm_set_irq_enabled(pwmSlice, false);
  pwm_clear_irq(pwmSlice);
  irq_set_enabled(PWM_IRQ_WRAP, false);

  // Remove PWM function from GPIO
  gpio_set_function(PWM_MASTER_GPIO, GPIO_FUNC_SIO);
  gpio_put(PWM_MASTER_GPIO, 0);

  // Optional: reset slice config to defaults
  pwm_config cfg = pwm_get_default_config();
  pwm_init(pwmSlice, pwmSlice, &cfg, false);

  // Invalidate globals (safety)
  pwmTop = 0;
  pwmLevel = 0;
  pwmSlice = 0;
}

void setPWMDutyCycle(float dutyCycle) {
  if (dutyCycle < 0.0f) dutyCycle = 0.0f;
  if (dutyCycle > 1.0f) dutyCycle = 1.0f;

  pwmLevel = (uint16_t)(pwmTop * dutyCycle);
  pwm_set_gpio_level(PWM_MASTER_GPIO, pwmLevel);
}

/* ---------- Arduino loop ---------- */
void loop() {
  static uint32_t lastSwitch = 0;
  static int state = 0;


  if (millis() - lastSwitch > 5000) {
      ocReleaseAll();

    lastSwitch = millis();
    state = (++state % 3);

    if (state == 0) {
      uint8_t leds2[] = {1,2,3,4};
      setActiveGPIOs(leds2, 4);

    } 
    if (state == 1) {
      // setPWMDutyCycle(0.2);
      disablePWM();
        setupPWM(1000.0f, 0.2f);

      uint8_t leds1[] = {5,6,7};
      setActiveGPIOs(leds1, 3);

    }
if (state == 2) {
        disablePWM();

    setupPWM(1000.0f, 0.1f); 

      uint8_t leds1[] = {5,6,8};
      setActiveGPIOs(leds1, 3);

    }
  }
}
