README
========
**Device Summary:**   
This lock-device was created as a senior design project for the University of California, Riverside. The lock-device was developed using C language in the Arduino IDE and uses two Arduino Uno units. The two Arduino Uno units are used as the transmitter (Tx) and Receiver (Rx) for the project. Attached to the Tx Arduino Uno is the IR LED and two buttons. Attached to the Rx Arduino Uno is a TFT display, stepper motor, keypad, and IR Receiver. The lock-device uses the Tx to send an unlock or lock signal to the Rx. Once the Rx receives the correct signals it will perform the lock or unlock motion as needed.

**Notes:**  
Included in this repository is all the necessary header files and main-file code needed to run the device for both the Tx and Rx components. The header files that are not included in this repository is already built into the Arduino IDE.

**Parts Summary:**    
This section will provide a brief overview of the parts used and their function.
- Arduino Uno R3 (x2)   
  Two Arduino uno R3 units were used for its simplicity and powerful shield library.
- Adafruit Motorshield & NEMA-17 200-steps 12V/350mA Stepper Motor   
  The Motorshield being used was needed to power the stepper motor because the Arduino R3 unit is not able to power the    motor on its own. Used the stepper motor because of its torque holding capabilities and smooth motion.
- Adafruit Datashield   
  The primary roll for the datashield was using the Real-time Clock (RTC) that was already built into it so that we can    constantly have the correct time.
- Adafruit 1.8" TFT Display   
  The Display was used to show the Real-time of when unlocking or locking occurred. 
- IR Transmitter & IR Receiver    
  These components were used to send and receive the unlock or lock signal.
- Keypad & Buttons    
  The Keypad is used on the Rx Arduino unit to lock or unlock if the Tx Arduino unit is not present. Buttons were used on   the Tx Arduino to send the unlock or lock signal to the Rx Arduino.
