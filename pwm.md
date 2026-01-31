
# PWM on the Raspberry pi Pico (RP2040)

Work on PWM to control GPIO.

***Sections of interests of an AI chatbot exchange:***

On the Pico (RP2040) you can drive PWM on many GPIOs at once; you just create one PWM object per pin, then manage their duty cycles and (shared) frequencies.

Hardware / constraints in two sentences

-The RP2040 has 8 PWM slices, each with 2 channels (A/B), giving up to 16 hardware PWM outputs that can be mapped to the GPIOs.

-Each slice has one counter and frequency shared by its two channels; A and B on the same slice can have different duty cycles but not different frequencies.

Minimal MicroPython example: several PWM GPIOs


```python
# Create PWM objects on several GPIOs
pwm0 = PWM(Pin(0))   # GP0
pwm1 = PWM(Pin(1))   # GP1
pwm2 = PWM(Pin(2))   # GP2
pwm3 = PWM(Pin(3))   # GP3

# Optionally, choose a common frequency for all of them
FREQ = 1000          # 1 kHz
for p in (pwm0, pwm1, pwm2, pwm3):
    p.freq(FREQ)

# Set independent duty cycles (0..65535)
pwm0.duty_u16(10000)
pwm1.duty_u16(30000)
pwm2.duty_u16(50000)
pwm3.duty_u16(65535)
```

All these pins will output PWM at 1 kHz; only the duty cycle differs per pin.

# All pins with same frequency and duty cycle

 If you want the same PWM frequency and duty cycle on (many or all) Pico pins in C/C++, the simplest way is to configure one PWM slice/channel, then map that signal to all desired GPIOs by setting them to PWM and using the helper that ties them to the same slice/channel.
Concept in one sentence

    Configure one PWM slice (e.g. slice 0, channel A) with your target frequency and duty; then for every GPIO you want, set its function to PWM and set that GPIO’s level via pwm_set_gpio_level with the same level value so all outputs match.

Minimal Pico SDK C example (one slice, cloned to many pins)

This example puts identical PWM (1 kHz, 50%) on GPIOs 0, 1, 2, 3; extend the pin list as you like.

```c
#include "pico/stdlib.h"
#include "hardware/pwm.h"

int main() {
    stdio_init_all();

    // Pins you want identical PWM on
    const uint pins[] = {0, 1, 2, 3};
    const int pin_count = sizeof(pins) / sizeof(pins[0]);

    // Choose PWM parameters
    const uint32_t freq = 1000;      // 1 kHz
    const float duty   = 0.5f;       // 50 %

    // Use the first pin to configure the slice (all others will follow)
    uint first_pin = pins[0];
    uint slice = pwm_gpio_to_slice_num(first_pin);

    // Compute wrap for desired frequency: f = clock / (wrap + 1) / div
    // Here we keep div = 1.0 for simplicity
    uint32_t clock = clock_get_hz(clk_sys);
    uint32_t wrap  = (clock / freq) - 1;

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_clkdiv(&cfg, 1.0f);    // divider = 1
    pwm_config_set_wrap(&cfg, wrap);

    pwm_init(slice, &cfg, false);         // init slice, but don't start yet

    // Set all pins to PWM function
    for (int i = 0; i < pin_count; ++i) {
        gpio_set_function(pins[i], GPIO_FUNC_PWM);
    }

    // Compute same level (duty) for the entire slice
    uint16_t level = (uint16_t)(duty * (wrap + 1));

    // Apply same level to each GPIO's associated channel
    for (int i = 0; i < pin_count; ++i) {
        pwm_set_gpio_level(pins[i], level);
    }

    // Enable the slice (starts PWM on all mapped pins)
    pwm_set_enabled(slice, true);

    while (true) {
        tight_loop_contents();
    }
}
```

-pwm_gpio_to_slice_num and pwm_set_gpio_level come from hardware/pwm.h and automatically pick the correct slice/channel for a given GPIO.

-All listed pins now output the same PWM waveform (same period, same duty), because they share the same slice configuration and use the same level value.

If you tell me your exact pin list (e.g. 0–15, or a sparse subset) and desired frequency and duty, I can adapt this snippet exactly to your case (including clock‑divider math if you want a non‑integer divider).

# more information

ref. [pwm.h](doc/pwm.h)

