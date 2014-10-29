#include <stdintt.h>


const int unlockPin = 2;     // the pin to unlock
const int lockPin = 3;       // the pin to lock
int unlockState = 0; // initialize variable for unlock state
int lockState = 0;   // initlize varible for lock state

//generates 38kHz carrier wave on pin 9 and 10
//sends data via TX every 500ms
void setup()
{
  pinMode(9, INPUT);
  pinMode(10, OUTPUT);

  // Clear Timer on Compare Match (CTC) Mode
  bitWrite(TCCR1A, WGM10, 0);
  bitWrite(TCCR1A, WGM11, 0);
  bitWrite(TCCR1B, WGM12, 1);
  bitWrite(TCCR1B, WGM13, 0);

  // Toggle OC1A and OC1B on Compare Match.
  bitWrite(TCCR1A, COM1A0, 1);
  bitWrite(TCCR1A, COM1A1, 0);
  bitWrite(TCCR1A, COM1B0, 1);
  bitWrite(TCCR1A, COM1B1, 0);

  // No prescaling
  bitWrite(TCCR1B, CS10, 1);
  bitWrite(TCCR1B, CS11, 0);
  bitWrite(TCCR1B, CS12, 0);

  OCR1A = 210;
  OCR1B = 210;

  Serial.begin(2400);
}

void encrypt (uint32_t* v, uint32_t* k) {
    uint32_t v0=v[0], v1=v[1], sum=0, i;           /* set up */
    uint32_t delta=0x9e3779b9;                     /* a key schedule constant */
    uint32_t k0=k[0], k1=k[1], k2=k[2], k3=k[3];   /* cache key */
    for (i=0; i < 32; i++) {                       /* basic cycle start */
        sum += delta;
        v0 += ((v1<<4) + k0) ^ (v1 + sum) ^ ((v1>>5) + k1);
        v1 += ((v0<<4) + k2) ^ (v0 + sum) ^ ((v0>>5) + k3);  
    }                                              /* end cycle */
    v[0]=v0; v[1]=v1;
}

uint32_t v_in[2];

uint32_t k_in[4];



// In this loop we are either pressing the button to send the codes for unlock or lock.
// if nothing is pressed there will be no code transmitted.
void loop()
{
  k_in[0] = 0x03002312;
  k_in[1] = 0x03352313;
  k_in[2] = 0x03002312;
  k_in[3] = 0x12345678;
  
  unlockState = digitalRead(unlockPin); // read the state of the unlock button
  lockState = digitalRead(lockPin);    //read the state of the lock button
  
  //if button pushed
  if(unlockState == HIGH) {
    
      //unlock key code    
      v_in[0] = 9482018;
      v_in[1] = 239493;
      
    pinMode(9,OUTPUT); // start pulsing 
    encrypt(v_in, k_in); // start encryption
   
    Serial.println(v_in[0], HEX);
    Serial.println(v_in[1], HEX);
    delay(100);
  }
  
  else if(lockState == HIGH) {
 
      //lock key code    
      v_in[0] = 5102815;
      v_in[1] = 696969;
      
    pinMode(9,OUTPUT); // start pulsing 
    encrypt(v_in, k_in); // start encryption
   
    Serial.println(v_in[0], HEX);
    Serial.println(v_in[1], HEX);
    delay(100);
  }
  
  else {
    pinMode(9,INPUT); // no pulse
  }
}