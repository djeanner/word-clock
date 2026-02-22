# word-clock
Construction and software of a word clock controled by a DCF77 signal

We used the GPIO as mosfet transitors to directly drive 12V connected LEDs. We swithed the GPIO as input to turn the LEDs OFF. Indeed, a high states is still to low for current not passing and being ON. It is a risky ide because of the leaks through the protection diodes.

Therefore we could not use PWM
- [use of PWM](pwm.md) and (pwmwork.ino)[pwmwork.ino]

We used a interrupts to control light intensity. 

cp -rp ~/Documents/Arduino/test_rb_tft/*.cpp  ~/Documents/Arduino/test_rb_tft/*.h ~/Documents/Arduino/test_rb_tft/test_rb_tft.ino  .

cp -rp  *cpp *h  test_rb_tft.ino ~/Documents/Arduino/test_rb_tft
cp -rp WordClock.cpp WordClock.h ~/Documents/Arduino/testDelCompiling
cp -rp ClockControl.cpp ClockControl.h DCF77Decoder.cpp DCF77Decoder.h ~/Documents/Arduino/testDelCompiling
cp -p main.ino ~/Documents/Arduino/testDelCompiling/testDelCompiling.ino
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

```
DCF77 code
Index x10           [000000000011111111112222222222333333333344444444445555555555]
Index x1            [0    5    0    5    0    5    0    5    0    5    0    5   9]
Fixed   to 1        [M              R    S                                       ]                               <MRSmhM>
Code (M:1 m:2)      [                AZza                                        ]
Parity              [                            1      2                      3 ]                               <MRSmhM>
name                [ ??????????????      < min > <hour> <dayM><D><mon><year  >  ]                                      
all combined        [M??????????????RAZzaS< min >1<hour>2<dayM><D><mon><year  >3£]                               <MRSmhM>
```

Some logs...
```
10:25:18.461 -> txt:[-+---++---+++-+---+_++-+--+-+----+-+-+--+---+-+----++--+--+£],Lms:603, 10:25 Thu Feb 12/2026 FFTTTF
all combined        [M??????????????RAZzaS< min >1<hour>2<dayM><D><mon><year  >3£]                               <MRSmhM>
10:29:12.530 -> txt:[-----++--++++_----+-++--+-+-+----+-+-+--+---+-+----++--+--+£],Lms:794, 10:29 Thu Feb 12/2026 FFTTTF 
10:30:06.515 -> txt:[--+-++-_-+++++----+-+----++------+-+-+--+---+-+- --++--+--+£],Lms:795, 10:30 Thu Oct 12/2026 FFTFTT 

all combined        [M??????????????RAZzaS< min >1<hour>2<dayM><D><mon><year  >3£]                               <MRSmhM>
10:53:18.511 -> txt:[-+------+--++++---+-+++--+-+-----+-+-+--+---+-+----++--+--+£],Lms:796, 10:53 Thu Feb 12/2026 FFTFTF 
10:59:30.539 -> txt:[---++++--+-+-+----+-+--------+---+-+-+--+---+-+----++--+--+£],Lms:797, 11:00 Thu Feb 12/2026 FFTFFF 
11:10:30.503 -> txt:[-+++-++++--++++---+-++---+---+-_-+---+--+---+-+----++--+--+£],Lms:798, 11:11 Thu Feb 12/2026 FFTFFF 

all combined        [M??????????????RAZzaS< min >1<hour>2<dayM><D><mon><year  >3£]                               <MRSmhM>
12:06:07.531 -> txt:[--+-+--+_--+-+----+-+-++------+--+---+--+---+-+----++--+--+£],Lms:696, 12:06 Thu Feb 12 2026 FFTFFF 
Fixed paratiy...
12:22:55.520 -> txt:[-+++----+-++-++---+-+++---+-+-+--+---+--+---+-+----++--+_-+£],Lms:434, 12:23 Thu Feb 12 2066 FFTTFF 
12:43:55.504 -> txt:[-++-+--+++----+---+-+--+---+--+--+---+--+---+-+----++--+--+£],Lms:436, 12:44 Thu Feb 12 2026 AllT 



```









---------------
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : N
tested time      : 13  1 2026  8:19:00
now :            : 13  1 2026  8:19:05
Crit : |-5| < 100
Crit OK: Switch to reliable
Crit consistent : first time quality, save time but not calculating error...
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026  8:40:00
now :            : 13  1 2026  8:40:19
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  8:19:05
 Time since last stored time not long enough for good precision rejects time in quality mode
1255 < 3600 s.
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026  9:04:00
now :            : 13  1 2026  9:04:39
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  8:19:05
 Time since last stored time not long enough for good precision rejects time in quality mode
2695 < 3600 s.
Stop listening to dcf77 for 20 min.




=============================



Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : N
tested time      : 13  1 2026  8:19:00
now :            : 13  1 2026  8:19:05
Crit : |-5| < 100
Crit OK: Switch to reliable
Crit consistent : first time quality, save time but not calculating error...
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026  8:40:00
now :            : 13  1 2026  8:40:19
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  8:19:05
 Time since last stored time not long enough for good precision rejects time in quality mode
1255 < 3600 s.
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026  9:04:00
now :            : 13  1 2026  9:04:39
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  8:19:05
 Time since last stored time not long enough for good precision rejects time in quality mode
2695 < 3600 s.
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026  9:25:00
now :            : 13  1 2026  9:26:00
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  8:19:05
Crit consistent : calculate error for fine tuning in quality mode
Calculate time correction...
 errorSecondsPerDay = -60 * 86400 / (1768296360 - 1768292345) = -1291
 errorSecondsPerDay = -60 * 86400 / (4015) = -1291
Including previous correction : -1291
 Period subtract a second : every 66 s
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026  9:47:00
now :            : 13  1 2026  9:46:57
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  9:26:00
 Time since last stored time not long enough for good precision rejects time in quality mode
1260 < 3600 s.
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026 10:09:00
now :            : 13  1 2026 10:08:54
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  9:26:00
 Time since last stored time not long enough for good precision rejects time in quality mode
2580 < 3600 s.
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026 10:33:00
now :            : 13  1 2026 10:32:50
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026  9:26:00
Crit consistent : calculate error for fine tuning in quality mode
Calculate time correction...
 errorSecondsPerDay = 10 * 86400 / (1768300370 - 1768296360) = 215
 errorSecondsPerDay = 10 * 86400 / (4010) = 215
Including previous correction : -1076
 Period subtract a second : every 80 s
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026 10:55:00
now :            : 13  1 2026 10:54:58
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026 10:32:50
 Time since last stored time not long enough for good precision rejects time in quality mode
1330 < 3600 s.
Stop listening to dcf77 for 20 min.
Start listening to dcf77 signal ...

Time submitted for validation Earlyer data reliable : Y
tested time      : 13  1 2026 11:16:00
now :            : 13  1 2026 11:15:57
Crit consistent : consider calculate error for fine tuning in quality mode
Last stored time : 13  1 2026 10:32:50
 Time since last stored time not long enough for good precision rejects time in quality mode
2590 < 3600 s.
Stop listening to dcf77 for 20 min.



arduino-cli compile --fqbn rp2040:rp2040:rpipico
arduino-cli compile --fqbn rp2040:rp2040:rpipicow

ls -lart /dev/cu.us*
arduino-cli upload -p /dev/ttyACM0 --fqbn rp2040:rp2040:rpipico
arduino-cli upload -p "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)" --fqbn rp2040:rp2040:rpipico

// add a macro SKETCH_NAME
--build-property compiler.cpp.extra_flags="-DSKETCH_NAME=\"clockcontrol\""

  --export-binaries 
  --build-path ./build
  --warnings all 
  --verbose


 
cp ClockControl.cpp ClockControl.h DCF77Decoder.cpp DCF77Decoder.h WordClock.cpp WordClock.h milanWordClock
cp main.ino milanWordClock/milanWordClock.ino
cd milanWordClock 
	echo "************* Compile for pico pi W "
	arduino-cli compile --fqbn rp2040:rp2040:rpipicow  --build-path ./build --export-binaries  --build-property compiler.cpp.extra_flags="-DMILAN_CLOCK=1"
	arduino-cli upload -p "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)" --fqbn rp2040:rp2040:rpipicow --input-dir ./build

	echo "************* Done upoading to " "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"
	echo "************* setting rate 115200 to" "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"
	stty -f "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)" 115200 raw -echo

	echo "************* shows stream from serial port (stop with CTRL-C)" "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"
cd ..
cat "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"


 
cp ClockControl.cpp ClockControl.h DCF77Decoder.cpp DCF77Decoder.h testClock
cp DCF77Window.cpp DCF77Window.h testClock
cp TFT_Window.cpp TFT_Window.h testClock
cp TFT_Screen.cpp TFT_Screen.h testClock
cp StringWindow.cpp StringWindow.h testClock

cp main.ino testClock/testClock.ino
cd testClock 
	echo "**********************************************************************************" >> serial.txt
	date >> serial.txt
	echo "************* Compile for pico pi"
	arduino-cli compile --fqbn rp2040:rp2040:rpipico  --build-path ./build --export-binaries  --build-property compiler.cpp.extra_flags="-DTEST_CLOCK=1"
	arduino-cli upload -p "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)" --fqbn rp2040:rp2040:rpipico --input-dir ./build

	echo "************* Done upoading to " "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"
	echo "************* setting rate 115200 to" "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"
	stty -f "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)" 115200 raw -echo

	echo "************* shows stream from serial port (stop with CTRL-C)"
	 
cd ..
DEV="$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"; stty -f "$DEV" 115200 raw -echo; cat "$DEV" | tee -a testClock/serial.txt


cp ClockControl.cpp ClockControl.h DCF77Decoder.cpp DCF77Decoder.h  DCF77DispClock
cp DCF77Window.cpp DCF77Window.h DCF77DispClock
cp TFT_Window.cpp TFT_Window.h DCF77DispClock
cp TFT_Screen.cpp TFT_Screen.h DCF77DispClock
cp StringWindow.cpp StringWindow.h DCF77DispClock
cp WifiControl.cpp WifiControl.h DCF77DispClock
cp password.h DCF77DispClock
cp main.ino DCF77DispClock/DCF77DispClock.ino

cd DCF77DispClock
	echo "**********************************************************************************" >> serial.txt
	date >> serial.txt
	echo "************* Compile for pico pi W"
	arduino-cli compile --fqbn rp2040:rp2040:rpipicow  --build-path ./build --export-binaries  --build-property compiler.cpp.extra_flags="-DDCF77DispClock=1"
	arduino-cli upload -p "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)" --fqbn rp2040:rp2040:rpipicow --input-dir ./build

	echo "************* Done upoading to " "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"
	echo "************* Setting rate 115200 to" "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"
	stty -f "$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)" 115200 raw -echo

	echo "************* shows stream from serial port (stop with CTRL-C)"
	cd ..

DEV="$(ls /dev/cu.usbmodem* 2>/dev/null | head -n1)"; stty -f "$DEV" 115200 raw -echo; cat "$DEV" | tee -a DCF77DispClock/serial.txt





while true; do curl -s http://192.168.1.64 -o /Users/djeanner/git/word-clock/data/data_$(date +%Y-%m-%d_%H-%M).html; sleep 3600; done

while true; do curl -s http://192.168.1.64 -o data/data_$(date +%Y-%m-%d_%H-%M).html; sleep $((3600 - $(date +%s) % 3600)); done

012345678901234567890123456789012345678901234567890123456789-
<p>   21/ 1/2026 21:50
1  1    1 1    1 1 1    1 1    1 11 1    11  1   3  111    1-
          T       T      T                      T           21:50 Sat Feb 21 2026 AllT   </p>
 <p>   21/ 1/2026 21:51
 1      1 11   1 111    1 1    1 11 1    11  1   3  11   1  -
          T       T      T                      T           21:51 Sat Feb 21 2026 AllT   </p>
 <p>   21/ 1/2026 21:52
    1   1 1 1  1 111    1 1    1 11 1    11  1   3 1111 1  1-
          T       T      T                      T           21:52 Sat Feb 21 2026 AllT  
