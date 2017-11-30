/*******************************
 *Code for the main LCD interface
 *Creates a simple interface on a 16x2 LCD for setting Aperture and other settings
 *Takes input from four buttons, debounce code included
 *Sends settings to main chip through Serial COM upon request
 *Stores last aperture value in EEPROM using EEPROM library, this is not yet working properly
 **********/
 
#include<LiquidCrystal.h>
#include<EEPROM.h>
#include <avr/sleep.h>

LiquidCrystal lcd(8, 7, 5, 4, 3, 2); //Pins for LCD signals
const int led_bklt = 6; //Pin for LCD backlight enable
const int butn_threshold = 6; //Button press threshold
char aperture_sym = 'Q'; //Initialized to Q for f5.6
int butn_up, butn_down, butn_sel; //Buttons for control UP,DN,sel

//Implement Power Save Mode (Sleep)
bool sleepEnable = false;
unsigned long int timeSleep = 0;
const  unsigned int sleepThresholdMin = 20;
unsigned long int sleepThresholdms = sleepThresholdMin*60*1000;

//Standard aperture values for the lens
const int ap[16] = {20, 25, 28, 35, 40, 45, 56, 67, 80, 95, 110, 130, 160, 190, 220, 250};
int index_ap = 6; //index holder for aperture array

unsigned long times = 0;
int timeout = 5000; //Activity timeout

byte film_count = 0;

int mode = 0; //Mode variable
int mode_max = 4;

void setup() {
  
  Serial.begin(19200); //Baud Rate
  pinMode(led_bklt, OUTPUT);
  lcd.begin(16, 2);
  lcd.print("Camera Controller for 35mm");
  lcd.setCursor(0, 1);
  lcd.print("Film Camera : Ramenga");
  
  digitalWrite(led_bklt, HIGH);
  delay(1000);
  
  delay(1500);

  film_count = EEPROM.read(0); //Initialize film count
  if ((film_count > 36) || (film_count == 0)) {
    film_count = 36;
    EEPROM.write(0, film_count);
  }



}

void loop() {
  //Sleep timer
  timeSleep += 1;
  if (timeSleep >= sleepThresholdms && sleepEnable){
    //Sleep code
    set_sleep_mode(SLEEP_MODE_PWR_DOWN); 
    sleep_enable();  
    sleep_mode();  
  }
  butn_sel = analogRead(A2); //Select button
  butn_up = analogRead(A0); //Up button
  butn_down = analogRead(A5); //Down button
  digitalWrite(led_bklt, LOW); // Turn off backlight

  //Any button is pressed
  if ((butn_sel < butn_threshold) || (butn_up < butn_threshold) || (butn_down < butn_threshold)) {
    times = millis();
    lcd.clear();
    display_ap(ap[index_ap]);
    mode = 0;
    while ((millis() - times) < timeout) { //Enter timeout period
      digitalWrite(led_bklt, HIGH);


      if (analogRead(A2) < butn_threshold) { //Mode button pressed
        times = millis();
        delay(10);
        mode += 1;
        if ((mode <= 0) || (mode >= 4)) {
          mode = 0;
        }
        if ((mode == 0) || (mode == 1)) {
          display_ap(ap[index_ap]);
        }

        if (mode == 2) {
          lcd.clear();
          display_flcount();
        }
        else if (mode == 3) {
          lcd.setCursor(0, 1);
          lcd.print("Film Count Reset");
          display_flcount();
        }
        while (analogRead(A2) < butn_threshold) { //dont go until button is released
          delay(10);
        }

      }

      if (analogRead(A0) < butn_threshold) { // Up button pressed
        times = millis();
        if ((mode == 0) || (mode == 1)) {
          inc_ap();
          display_ap(ap[index_ap]);
          update_sym();
        }
        else if (mode == 2) {
          inc_flcount();
          display_flcount();
        }
        else if (mode == 3) {
          reset_flcount();
          display_flcount();
        }
        while (analogRead(A0) < butn_threshold) {
          delay(10);
        }
      }

      if (analogRead(A5) < butn_threshold) { //Down button pressed
        if ((mode == 0) || (mode == 1)) {
          dec_ap();
          display_ap(ap[index_ap]);
          update_sym();
        }
        else if (mode == 2) {
          dec_flcount();
          display_flcount();
        }
        else if (mode == 3) {
          reset_flcount();
          display_flcount();
        }
        while (analogRead(A5) < butn_threshold) {
          delay(10);
        }
      }


    }
    digitalWrite(led_bklt, LOW);
  }
  update_sym();
  Serial.println(aperture_sym);

  if (Serial.available() > 0) { //If aperture value is requested
    char a = Serial.read();
    if (a == 'A') { //Request key is 'A'
      update_sym();
      Serial.println(aperture_sym); //Send aperture symbol to serial out
      display_ap(ap[index_ap]);
      lcd.setCursor(0, 1);
      digitalWrite(led_bklt, HIGH);
      //lcd.print("Shutter Fired");
      delay(1600);
      lcd.clear();
      digitalWrite(led_bklt, LOW);
      film_count -= 1;
      //put film count decrease fn
    }
  }//End of aperture request routine

}

void display_ap(int aes) { //Display aperture
  lcd.setCursor(0, 0);
  lcd.print("Aperture: f/");
  lcd.print(aes / 10);
  lcd.print('.');
  lcd.print(aes % 10);
}

void display_flcount() { //Display film count
  lcd.setCursor(0, 0);
  lcd.print("Film Count: ");
  lcd.print(film_count);
}

void inc_flcount() { //Increase film count
  film_count += 1;
  if (film_count > 36)
    film_count = 36;
  EEPROM.write(0, film_count);
}

void dec_flcount() { //Decrease film count
  film_count -= 1;
  if (film_count <= 0)
    film_count = 0;
  EEPROM.write(0, film_count);
}

void reset_flcount() { //Reset film count
  film_count = 36;
  EEPROM.write(0, film_count);
}

void inc_ap() { //Increase aperture
  index_ap += 1;
  if (index_ap > 15) {
    index_ap = 15;
  }

}

void dec_ap() { //Decrease aperture
  index_ap -= 1;
  if (index_ap < 0) {
    index_ap = 0;
  }

}

void update_sym() { // Update aperture symbol to value currently referenced
  switch (ap[index_ap]) {
    case 20:
      aperture_sym = 'K';
      break;
    case 25:
      aperture_sym = 'L';
      break;
    case 28:
      aperture_sym = 'M';
      break;
    case 35:
      aperture_sym = 'N';
      break;
    case 40:
      aperture_sym = 'O';
      break;
    case 45:
      aperture_sym = 'P';
      break;
    case 56:
      aperture_sym = 'Q';
      break;
    case 67:
      aperture_sym = 'R';
      break;
    case 80:
      aperture_sym = 'S';
      break;
    case 95:
      aperture_sym = 'T';
      break;
    case 110:
      aperture_sym = 'U';
      break;
    case 130:
      aperture_sym = 'V';
      break;
    case 160:
      aperture_sym = 'W';
      break;
    case 190:
      aperture_sym = 'X';
      break;
    case 220:
      aperture_sym = 'Y';
      break;
    case 250:
      aperture_sym = 'Z';
      break;
  }

}
