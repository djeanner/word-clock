# word-clock
Construction and software of a word clock controled by a DCF77 signal

We used the GPIO as mosfet transitors to directly drive 12V connected LEDs. We swithed the GPIO as input to turn the LEDs OFF. Indeed, a high states is still to low for current not passing and being ON. It is a risky ide because of the leaks through the protection diodes.

Therefore we could not use PWM
- [use of PWM](pwm.md) and (pwmwork.ino)[pwmwork.ino]

We used a interrupts to control light intensity. 


...

20:49:53.421 -> DCF77 pin : 28
20:49:54.019 -> End setup
20:49:54.019 -> Starts listening to dcf77 signal ...

20:50:59.967 -> 
20:50:59.967 -> Time submitted for validation Earlyer data reliable : N
20:50:59.967 -> tested time      : 11 1 2026 20:51:00
20:50:59.967 -> now :            : 1 1 1970  0:01:08
20:50:59.967 -> Crit : |1768164592| < 100
20:50:59.967 -> Crit failed: Time is not consistent (normal if first call). It is not considered as reliable
20:50:59.967 -> Just stored a time : 
20:50:59.967 -> 11 1 2026 20:51:00
20:50:59.967 -> Stops listening to dcf77 for 10 min.

21:15:00.011 -> 
21:15:00.011 -> Time submitted for validation Earlyer data reliable : N
21:15:00.011 -> tested time      : 11 1 2026 21:15:00
21:15:00.011 -> now :            : 11 1 2026 21:15:03
21:15:00.011 -> Crit : |-3| < 100
21:15:00.011 -> Crit OK: Switch to reliable
21:15:02.133 ->  Crit consistent :  first time quality, save time but not calculating error...
? blocked until 1:13

----
01:15:39.450 -> DCF77 pin : 28
01:15:40.062 -> End setup
01:15:40.062 -> Starts listening to dcf77 signal ...