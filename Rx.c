

#define cs   10
#define dc   9
#define rst  8


#include <Adafruit_ST7735.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_PWMServoDriver.h"
#include <Adafruit_GFX.h>
#include <Wire.h>
#include <SPI.h>
#include <RTClib.h>
#include <Keypad.h>

RTC_DS1307 RTC;

const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'}
};
byte rowPins[ROWS] = {5, 6, 7}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {2, 3, 4}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

// Create the motor shield object with the default I2C address
Adafruit_MotorShield AFMS = Adafruit_MotorShield(); 
// Or, create it with a different I2C address (say for stacking)
// Adafruit_MotorShield AFMS = Adafruit_MotorShield(0x61); 

// Connect a stepper motor with 200 steps per revolution (1.8 degree)
// to motor port #2 (M3 and M4)
Adafruit_StepperMotor *myMotor = AFMS.getStepper(200, 2);

// initializes the LCD screen
#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// Option 1: use any pins but a little slower
//Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, mosi, sclk, rst);

// Option 2: must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
Adafruit_ST7735 tft = Adafruit_ST7735(cs, dc, rst);
float p = 3.1415926;

void setup()
{
  Serial.begin(2400);
  
   Wire.begin();
  RTC.begin();
 
  if (! RTC.isrunning()) {
    Serial.println("RTC is NOT running!");
    // following line sets the RTC to the date & time this sketch was compiled
    // uncomment it & upload to set the time, date and start run the RTC!
    RTC.adjust(DateTime(__DATE__, __TIME__));
  }
  //pinMode(12, OUTPUT);
  AFMS.begin(); //create with the default frequency of 1.6KHz
  //AFMS.begin(1000); creates with the frequency of 1KHz
  
  myMotor->setSpeed(100); //sets speed to 100rpm
  
  // If your TFT's plastic wrap has a Green Tab, use the following:
  tft.initR(INITR_GREENTAB); // initialize a ST7735R chip, green tab
  tft.fillScreen(ST7735_BLACK);
  
  //Set the keys to match the keys in the encryption on the transmitter

}

//Decryption Function
void decrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0xC6EF3720, i;  /* set up */
    uint32_t delta=0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i<32; i++) {                         /* basic cycle start */
        v1 -= ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);
        v0 -= ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        sum -= delta;                                   
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

uint32_t v_out[2];              //2 space array with 32bit variables. (first parameter passed in decryption and recieved from transmitter)
uint32_t k_out[4];              //4 space array with 32bit variables. (harcoded pass keys, must match encryption)
char left_shift;                 //The number of times recieved character is left shifted to fill the main_variable)      
unsigned char z_or_o = 0;       //Variable used to determine which v_out space is filled, either v_out[0] or v_out[1] 
unsigned long receive_char = 0; //Stores the received character from transmitter       
uint32_t empty_variable = 0;    //Empty variable used to store recieved char and shifted
uint32_t main_variable = 0;     //Main variable that stores final 32bit password from the transmitter.  
uint32_t v_var[2];              //Extra variable to help flip the final v_out to check both orders
char lock_flag = 0;             //Lock flag variable
char unlock_flag = 1;           //Unlock flag variable
int cursor_row = 0;
int cursor_col = 0;
uint16_t current_color = ST7735_WHITE;
char color_flag = 0;
char key;                       //Keypad variable
char passcode[] = {'7', '1', '4', '4', '1', '5'};
char passcounter;

void loop()
{ 
  DateTime now = RTC.now(); // function to set the real time clock
  k_out[0] = 0x03002312;
k_out[1] = 0x03352313;
k_out[2] = 0x03002312;
k_out[3] = 0x12345678;
  
  
  //Initialize left shift back to 28 everytime loop restarts
  left_shift = 28;
  //Recieve a character from Tx and convert it to decimal
  if (Serial.available())
  {
  receive_char = Serial.read() - '0';
  //Output an h to signal that it is still looking for the header
  Serial.print('h');
  delay(10);
  //If header(-38) is detected, start gathering the data for password
  if (receive_char == -38)
  {
    //Print start to signal beginning of storing data for password
    Serial.println("Start: ");
    delay(10);
    
        //Initialize the main_variable that is used to store the final 32bit variable
        main_variable = 0;
        
        //Program loops until left shift is zero
        while (left_shift >= 0)
        {
         //Re-initialize the empty variable everytime
         empty_variable = 0;
         //Read in a character and convert it to decimal depending on whether it is 1-9 or A-F
         receive_char = Serial.read();
         if ((receive_char >= 64) && (receive_char <= 71))
         {
           receive_char = receive_char - 55;
         }
         else if((receive_char <= 58) && (receive_char >= 47))
         {
            receive_char = receive_char - 48;
         }
         //Mask the received char into the empty variable
         empty_variable = receive_char | empty_variable;
         //Left shift depending on which character is received(first character needs to be shifted to the very left, 28 times. Last character doesnt need to be shifted, 0 times.
         empty_variable = empty_variable << left_shift;
         //Mask the empty variable that has the character shifted to the right place into the main_variable
         main_variable = main_variable | empty_variable;
         //Decrement the left shift by four everytime to shift the received character into the correct spot
         left_shift = left_shift - 4;
        }
        
        //Ensures that the two 32bit password are stored in different spaces of the v_out[2] array
        if (z_or_o == 0)
        {
        v_var[0] = main_variable;
        z_or_o = 1;
        }
        else if (z_or_o == 1)
        {
        v_var[1] = main_variable;
        z_or_o = 0;
        }
    
        //Stores the first received password in v_out[0] and the second in v_out[1]
        v_out[0] = v_var[1];
        v_out[1] = v_var[0];
        //Decrypt the passwords in this order
        decrypt(v_out, k_out);
        Serial.println(v_out[0]);
        Serial.println(v_out[1]);
        //If the password matches the values from the Tx, SUCCESS
        if ((v_out[0] == 9482018) && (v_out[1] == 239493) && unlock_flag == 0)
        {
          Serial.println("UNLOCK! UNLOCK! UNLOCK! UNLOCK!");
          myMotor->step(100, FORWARD, SINGLE);
          tft.setTextColor(ST7735_BLACK);
          tft.setCursor(0,cursor_row);
          tft.drawRect(0, cursor_row, 120, 10, ST7735_BLACK);
          tft.fillRect(0, cursor_row, 120, 10, ST7735_BLACK); 
          tft.setCursor(0,cursor_row);
          tft.setTextColor(current_color);
          tft.print(now.year());
          tft.print('/');
          tft.print(now.month());
          tft.print('/');
          tft.print(now.day());
          tft.print(' ');
          tft.print(now.hour());
          tft.print(':');
          tft.print(now.minute());
          
          tft.setTextSize(1);
          cursor_row = cursor_row + 10;
          if(cursor_row == 160)
          {
            cursor_row = 0;
            if(current_color == ST7735_WHITE)
            {
              current_color = ST7735_YELLOW;
            }
            else if(current_color == ST7735_YELLOW)
            {
              current_color = ST7735_WHITE;
            }
          }

          unlock_flag = 1;
          lock_flag = 0;
          //digitalWrite(12, LOW);
          
        }
        else if ((v_out[0] == 5102815 ) && (v_out[1] == 696969) && lock_flag == 0)
        {
          Serial.println("LOCK! LOCK! LOCK! LOCK! LOCK!");
          myMotor->step(100, BACKWARD, SINGLE);
          lock_flag = 1;
          unlock_flag = 0;
          //digitalWrite(12, HIGH);
        }
        //A second try to see if the passwords were passed in the wrong order
        //Switch the places of the first and second password and try the decryption again
        v_out[0] = v_var[0];
        v_out[1] = v_var[1];
        decrypt(v_out, k_out);
        Serial.println(v_out[0]);
        Serial.println(v_out[1]);
        //If the password matches the values from the Tx, SUCCESS otherwise nothing
        if ((v_out[0] == 9482018) && (v_out[1] == 239493) && unlock_flag == 0)
        {
          Serial.println("UNLOCK! UNLOCK! UNLOCK! UNLOCK!");
          myMotor->step(100, FORWARD, SINGLE);
          tft.setTextColor(ST7735_BLACK);
          tft.setCursor(0,cursor_row);
          tft.drawRect(0, cursor_row, 120, 10, ST7735_BLACK);
          tft.fillRect(0, cursor_row, 120, 10, ST7735_BLACK);
          tft.setCursor(0,cursor_row);
          tft.setTextColor(current_color);
          tft.print(now.year());
          tft.print('/');
          tft.print(now.month());
          tft.print('/');
          tft.print(now.day());
          tft.print(' ');
          tft.print(now.hour());
          tft.print(':');
          tft.print(now.minute());
          
          tft.setTextSize(1);
          cursor_row = cursor_row + 10;
          if(cursor_row == 160)
          {
            cursor_row = 0;
            //ST7735_YELLOW is actually blue
            if(current_color == ST7735_WHITE)
            {
              current_color = ST7735_YELLOW;
            }
            else if(current_color == ST7735_YELLOW)
            {
              current_color = ST7735_WHITE;
            }
      
          }
          unlock_flag = 1;
          lock_flag = 0;
          //digitalWrite(12, LOW);
        }
        else if ((v_out[0] == 5102815) && (v_out[1] == 696969) && lock_flag == 0)
        {
          Serial.println("LOCK! LOCK! LOCK! LOCK! LOCK!");
          myMotor->step(100, BACKWARD, SINGLE);
          lock_flag = 1;
          unlock_flag = 0;
          //digitalWrite(12, HIGH);
        }
        delay(10);
        
      }    
  }
  else if (keypad.getKey())
  {
    Serial.print("pass: ");
    passcounter = 0;
    for ( int i = 0; i < 6; i++)
    {
      Serial.print(keypad.getKey());
      char key_pressed = keypad.waitForKey();
      if (key_pressed == passcode[i])
      {
        passcounter++;
      }
    }
    
    if (passcounter == 6)
    {
      if (unlock_flag == 0)
      {
          Serial.println("UNLOCK! UNLOCK! UNLOCK! UNLOCK!");
          myMotor->step(100, FORWARD, SINGLE);
          tft.setTextColor(ST7735_BLACK);
          tft.setCursor(0,cursor_row);
          tft.drawRect(0, cursor_row, 120, 10, ST7735_BLACK);
          tft.fillRect(0, cursor_row, 120, 10, ST7735_BLACK);
          tft.setCursor(0,cursor_row);
          tft.setTextColor(current_color);
          tft.print(now.year());
          tft.print('/');
          tft.print(now.month());
          tft.print('/');
          tft.print(now.day());
          tft.print(' ');
          tft.print(now.hour());
          tft.print(':');
          tft.print(now.minute());
          
          tft.setTextSize(1);
          cursor_row = cursor_row + 10;
          if(cursor_row == 160)
          {
            cursor_row = 0;
            //ST7735_YELLOW is actually blue
            if(current_color == ST7735_WHITE)
            {
              current_color = ST7735_YELLOW;
            }
            else if(current_color == ST7735_YELLOW)
            {
              current_color = ST7735_WHITE;
            }
      
          }
          unlock_flag = 1;
          lock_flag = 0;
          //digitalWrite(12, LOW);
      }
      else if (lock_flag == 0)
      {
          Serial.println("LOCK! LOCK! LOCK! LOCK! LOCK!");
          myMotor->step(100, BACKWARD, SINGLE);
          lock_flag = 1;
          unlock_flag = 0;
      }
          
    }
  }
  

} // end void loop

  // function used to draw text onto the LCD screen
  void testdrawtext(unsigned char text, uint16_t color, int cursor_pos0, int cursor_pos1) {
  tft.setCursor(cursor_pos1, cursor_pos0);
  tft.setTextColor(color);
  tft.setTextWrap(true);
  tft.print(text);
}
