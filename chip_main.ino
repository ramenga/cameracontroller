/******************************
 *Code for the main chip which controls the Camera aperture manually
 *Connects to a H-Bridge driver for driving bipolar stepper motor of the aperture drive
 *Takes input for shutter through shutter button, and redirects it to main camera shutter controls through a MUX
 *So it performs somewhat of a passthrough for the Shutter button signals
 *Takes Hotshoe signal for confirmation
 *Make default mode of camera as Widest Aperture, so that focusing can be performed
 *As shutter is pressed, detach lens signal and set aperture, then fires shutter button
 *Aperture steps have been determined by crude method, so may have some error
 **/
 
#include<Stepper.h>
const int stepsPerRev = 180; //Match motor config
const int en_pin_ap = 6; //Enable pin for aperture driver
const int half_po = 4; //Half press out
const int full_po = 5; // Full press out
const int lens_enbar = 3; //Lens enable^T 
const int p_threshold = 5; //Button press threshold
int half_p,full_p;
int aperture_prev_val=0;
const int ttl_xsync_pin = 2; //set pin for x sync TTL pin
long int time1=0,time2=0;
int aperture_step=1; //Step size for aperture, initialize to 1
char a='K';



Stepper aperture(stepsPerRev,7,8,9,10); //Pin config for aperture motor

void setup() {
  Serial.begin(19200);
  pinMode(en_pin_ap,OUTPUT); //Enable pin for Motor Driver
  pinMode(half_po,OUTPUT);
  pinMode(full_po,OUTPUT);
  pinMode(lens_enbar,OUTPUT);
  //pinMode(ttl_xsync_pin,INPUT); //Sensing TTL sync
  digitalWrite(en_pin_ap,LOW); //EN_pin is LOW until needed to save power
  digitalWrite(lens_enbar,LOW); //LOW to enable lens at start
  digitalWrite(half_po,LOW); //LOW to disable half_po at start
  digitalWrite(full_po,LOW); //LOW to disable full_po at start
  
  
  half_p=0;
  full_p=0; 
  
  aperture.setSpeed(90); //Aperture motor speed
  aperture.step(50);                                                                                                                                                                                         

}

void loop() {
  time1=0;
  time2=0;
  half_p = analogRead(A1);
  full_p = analogRead(A3);
  char x=Serial.read();
  if((x>='K')&&(x<='Z')){  //K to Z used for aperture symbols
    a=x;
  }
  if(analogRead(A4)<5){
    aperture_set(aperture_step);
    AS:
    if(analogRead(A4)<5)
      goto AS;
    aperture_reset();
    
  }
  
  if(half_p > p_threshold){
    digitalWrite(half_po,LOW);
  }
  if(half_p > p_threshold){
    digitalWrite(full_po,LOW);
  }
  if(half_p < p_threshold){ //Half pressed
    digitalWrite(half_po,HIGH);
    

    if((analogRead(A3) < p_threshold)&&(analogRead(A1) < p_threshold)){ //Full pressed after Half pressed
      aperture_req();
      digitalWrite(half_po,LOW); //Unpress button first
      digitalWrite(lens_enbar,HIGH); //Disable lens
      delay(50);
      aperture_set(aperture_step);
      delay(50);

      digitalWrite(half_po,HIGH);
      delay(90);
      digitalWrite(full_po,HIGH);//Shutter release
      delay(40);
      digitalWrite(half_po,LOW);//Reset both buttons
      digitalWrite(full_po,LOW);
      delay(1000);
      time1 = millis();          
      while(analogRead(A5)<10){
        time2=millis();
        if((time2-time1)>5000){
          break;
        }
      }
      
      aperture_reset();
    }
    
  }
  digitalWrite(lens_enbar,LOW);//Enable lens again
  

}

void aperture_req(){  //Requests aperture value through Serial Bus
  Serial.println('A');
  //delay(30);
  {
    //a = Serial.read();
    Serial.println(a);
    switch(a){
      case 'K': 
        aperture_step = 1; //f2.0
        break;
      case 'L':
        aperture_step = 11; //f2.5
        break;
      case 'M':
        aperture_step = 13; //f2.8
        break;
      case 'N':
        aperture_step = 16; //f3.5
        break;
      case 'O':
        aperture_step = 18; //f4.0
        break;
      case 'P':
        aperture_step = 20; //f4.5
        break;
      case 'Q':
        aperture_step = 25; //f5.6
        break;
      case 'R':
        aperture_step = 26; //f6.7
        break;
      case 'S':
        aperture_step = 27; //f8.0
        break;
      case 'T':
        aperture_step = 30; //f9.5
        break;
      case 'U':
        aperture_step = 34; //f11
        break;
      case 'V':
        aperture_step = 35; //f13
        break;
      case 'W':
        aperture_step = 38; //f16
        break;
      case 'X':
        aperture_step = 39; //f19
        break;
      case 'Y':
        aperture_step = 40; //f22
        break;
      case 'Z':
        aperture_step = 47; //Min Iris Opening
        break;
    }
  }
}

void aperture_set(int steps){ //Set aperture to the required step
  digitalWrite(en_pin_ap,HIGH);
  aperture.step(-steps);
  aperture_prev_val = steps;
  digitalWrite(en_pin_ap,LOW);
  
}

void aperture_reset(){ // Reset aperture to widest(full open iris)
  digitalWrite(en_pin_ap,HIGH);
  aperture.step(aperture_prev_val+30); //offset value
  aperture_prev_val = 0;
  digitalWrite(en_pin_ap,LOW);
  
}

