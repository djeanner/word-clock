# word-clock
Construction and software of a word clock controled by a DCF77 signal

We used the GPIO as mosfet transitors to directly drive 12V connected LEDs. We swithed the GPIO as input to turn the LEDs OFF. Indeed, a high states is still to low for current not passing and being ON. It is a risky ide because of the leaks through the protection diodes.

Therefore we could not use PWM
- [use of PWM](pwm.md) and (pwmwork.ino)[pwmwork.ino]

We used a fast loop to control light intensity. 