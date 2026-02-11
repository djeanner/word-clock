# word-clock
Construction and software of a word clock controled by a DCF77 signal

We used the GPIO as mosfet transitors to directly drive 12V connected LEDs. We swithed the GPIO as input to turn the LEDs OFF. Indeed, a high states is still to low for current not passing and being ON. It is a risky ide because of the leaks through the protection diodes.

Therefore we could not use PWM
- [use of PWM](pwm.md) and (pwmwork.ino)[pwmwork.ino]

We used a fast loop to control light intensity. 


Note time... the DCF annonces the time of the next minute.
Note when valid...
19:55:59.993 -> Time submitted for validation Earlyer data reliable : Y
19:55:59.993 -> tested time      : 10 01 2026 19:56:00
19:55:59.993 -> now :            : 10 01 2026 19:56:00
19:55:59.993 ->  Crit consistent : consider calculate error for fine tuning in quality mode
19:55:59.993 -> last stored time : 10 01 2026 18:54:00
19:55:59.993 ->  Crit consistent :  calculate error for fine tuning in quality mode
19:55:59.993 ->  Calculate time correction:
19:55:59.993 ->  errorSecondsPerDay = 0 * 86400 / (1768074960 - 1768071240) = 0 (including previous correction:0) period:-0
19:55:59.993 -> Stops listening to dcf77 for 60 min.





22:48:45.967 -> DCF77 pin : 28
22:48:46.513 -> End setup
22:48:46.513 -> Starts listening to dcf77 signal ...
22:49:59.904 -> 
22:49:59.904 -> Time submitted for validation Earlyer data reliable : N
22:49:59.904 -> tested time      : 10 1 2026 22:50:00
22:49:59.904 -> now :            : 1 1 1970  0:01:15
22:49:59.904 -> Crit : |1768085325| < 100
22:49:59.904 -> Crit failed: Time is not consistent (normal if first call). It is not considered as reliable
22:49:59.904 -> Stops listening to dcf77 for 10 min.

22:59:59.871 -> Starts listening to dcf77 signal ...
23:01:59.862 -> 
23:01:59.862 -> Time submitted for validation Earlyer data reliable : N
23:01:59.899 -> tested time      : 10 1 2026 23:02:00
23:01:59.899 -> now :            : 10 1 2026 23:01:59
23:01:59.899 -> Crit : |1| < 100
23:01:59.899 -> Crit OK: Switch to reliable
23:01:59.899 ->  Crit consistent :  first time quality, save time but not calculating error...
23:01:59.899 -> Stops listening to dcf77 for 60 min.



07:17:00.015 -> tested time      : 11 1 2026  7:17:00
07:17:00.015 -> now :            : 11 1 2026  7:17:01
07:17:00.015 ->  Crit consistent : consider calculate error for fine tuning in quality mode
07:17:00.015 -> last stored time : 11 1 2026  6:15:59Crit consistent : calculate error for fine tuning in quality mode
07:17:00.015 ->  Calculate time correction:
07:17:00.015 -> errorSecondsPerDay = -1 * 86400 / (1768115821 - 1768112159) = -23
07:17:00.015 -> errorSecondsPerDay = -1 * 86400 / (3662) = -23
07:17:00.015 -> Including previous correction : 0
07:17:00.015 ->  Period subtract a second : every 0 s
07:17:00.015 -> Stops listening to dcf77 for 60 min.
