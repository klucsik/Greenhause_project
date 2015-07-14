/* Greenhouse project 0_9 display
made by: Klucsik Krisztián Pál, 2014-2015, University of Pannon, Hungary
This sketch is a part of a greenhouse automatisation.
This arduino is in the living part of the house, it is displaying climatic information (temp, humidity, air presure, light intensity), alarms when a problem occours, and basically, watch over the greenhouse automatism
The connection to the greenhouse is a 4 wire cable: 
Greenhouse arduino <<<>>>> Display arudino
TX3  <----------------------> RX
RESET  <--------------------> TX
Vin    <--------------------> Vin
GND    <--------------------> GND
 */

//chinesee LCD display 20x4 characters
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27, 20, 4);
#include <Wire.h>
//easy trensfer lib from Bill Porter to transfer the collected data
#include <EasyTransfer.h>
EasyTransfer ET;
struct RECEIVE_DATA_STRUCTURE {
  double luxatlag;
  float DHTTEMPatlag;
  float DHTHUMatlag;
  float Tmin;
  float Tout;
  float Hout;
  int hour;
  int minute;
  double P;
};
RECEIVE_DATA_STRUCTURE mydata;


//The display arduino is also control a seed germination device, a simple refigerator wich has a traditionally 60 Watt bulb to heat the boxes with soil and seeds.
//A DS18B20 temp sensor atached to digital pin 10, a relay module with the bulb to the digital pin 11
float celfutes = 28; //the default target temp
boolean felfutes;  // for marking the heatup process
#define heatpin 11
#define dspin 10
#include <OneWire.h>
OneWire  ds(dspin);
float celsius;


bool disabled; //to mute the alarm beeping
byte resets; //this will store the number of the measuring system setups

struct statauto { //structs for data statistics
  float Toutmin = 100;
  float Toutav;
  float Toutmax;
  float Tuhazmin = 100;
  float Tuhazav;
  float Tuhazmax;
  float Tinmin = 100;
  float Tinav;
  float Tinmax;
  int c;
  int resH = 18;
};

statauto au;

struct statkezi {
  float Toutmin = 100;
  float Toutav;
  float Toutmax;
  float Tuhazmin = 100;
  float Tuhazav;
  float Tuhazmax;
  float Tinmin = 100;
  float Tinav;
  float Tinmax;
  int c;
};

statkezi ke;
//distance sensor, HC-SR04, the lcd will powered up only if something, or somebody in front of it.
#define trigPin 7
#define echoPin 6
long duration, distance;

//inner temperature and humidity DHT22
#include "DHT.h"
#define DHTPIN 8
#define DHTTYPE DHT22
DHT dht(DHTPIN, DHTTYPE);

int merorend = 0;
int z = 0;
int u = 0;
//buttons
#define but1 2
#define but2 4
#define but3 3
//menu
int page = 0;
int prevpage = 1;

void alarm(int pause) {  //the alarm function, it is a buzzer attached to dpin 9
  if (!disabled) {
    for (byte g = 0; g < 2; g++) {
      digitalWrite(9, HIGH);
      delay(750);
      digitalWrite(9, LOW);
      delay(pause);
    }
  }
}

void setup() {
  Serial.begin(9600);
  ET.begin(details(mydata), &Serial); 
  pinMode(5, OUTPUT);    //dpin 5 <------> Vcc lcd display
  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(9, OUTPUT);  ///dpin 9 <------> buzzzer +
  pinMode(but1, INPUT_PULLUP);
  pinMode(but2, INPUT_PULLUP);
  pinMode(but3, INPUT_PULLUP);
  pinMode(heatpin, OUTPUT);
  felfutes = 1;
}



void loop() {
  delay(500); 

  float h; //inner humidity and temperature from DHT22
  float t;
     

//the seed germination section        
  if (z < 100) z++; 
  else {
    dsfunc();
    z = 0;
  
  if (celsius < celfutes - 1.0) {   //heat on
    digitalWrite(heatpin, LOW);
  }
  if (celsius > celfutes + 1.0) {  //heat down, the heat up is done, so the alarm is on
    digitalWrite(heatpin, HIGH);
    felfutes = 0;
  }
  if (celsius < celfutes - 3.0 && felfutes == 0) { //if the bulb are dead, or somehow the "fridge" is cooled down too much, alarm and display the alarm page
    alarm(1000);
    page = 32;
  }
 }

if (ET.receiveData()) { //if there are incoming data, blink, the do the staistics
   disabled=0; //if the data are coming once the setup time alarm disabling will shut off, basically the program not
  // selecting the disables, so if you muted it for some point, if the data stream arrives it  will turn back the alarm.
    digitalWrite(13, HIGH);
    delay(1000);
    digitalWrite(13, LOW); 

                   h = dht.readHumidity();     //read the DHT22
                   t = dht.readTemperature();
  
    if ( mydata.hour == au.resH && mydata.minute < 10) { //reset the daily statistics
      au.c = 0;
      au.Tinav = 0;
      au.Tinmax = 0;
      au.Tinmin = 100;
      au.Toutav = 0;
      au.Toutmax = 0;
      au.Toutmin = 100;
      au.Tuhazav = 0;
      au.Tuhazmax = 0;
      au.Tuhazmin = 100;
    }
    if (mydata.DHTTEMPatlag < au.Tuhazmin) au.Tuhazmin = mydata.DHTTEMPatlag;
    if (mydata.DHTTEMPatlag > au.Tuhazmax) au.Tuhazmax = mydata.DHTTEMPatlag;
    au.Tuhazav = au.Tuhazav + mydata.DHTTEMPatlag;
    if (mydata.Tout < au.Toutmin) au.Toutmin = mydata.Tout ;
    if (mydata.Tout > au.Toutmax) au.Toutmax = mydata.Tout ;
    au.Toutav = au.Toutav + mydata.Tout ;
    if (t < au.Tinmin) au.Tinmin = t;
    if (t > au.Tinmax ) au.Tinmax = t;
    au.Tinav = au.Tinav + t;
    au.c = au.c + 1;
    if (mydata.DHTTEMPatlag < ke.Tuhazmin) ke.Tuhazmin = mydata.DHTTEMPatlag;
    if (mydata.DHTTEMPatlag > ke.Tuhazmax) ke.Tuhazmax = mydata.DHTTEMPatlag;
    ke.Tuhazav = ke.Tuhazav + mydata.DHTTEMPatlag;
    if (mydata.Tout < ke.Toutmin) ke.Toutmin = mydata.Tout ;
    if (mydata.Tout > ke.Toutmax) ke.Toutmax = mydata.Tout ;
    ke.Toutav = ke.Toutav + mydata.Tout ;
    if (t < ke.Tinmin) ke.Tinmin = t;
    if (t > ke.Tinmax ) ke.Tinmax = t;
    ke.Tinav = ke.Tinav + t;
    ke.c = ke.c + 1;
}
//read the incoming signals
       byte reading= Serial.read(); 
    if(reading==221){ //cold in the greenhouse
        alarm(250);
        page==34;
    }
    if(reading==211){  //the loop has ended, this section watch over the other arduino
      merorend=0;
        digitalWrite(13, HIGH);
    delay(500);
    digitalWrite(13, LOW);
      
        } 
    else merorend++;
    
if (merorend > 2000) {
    digitalWrite(1, LOW); // if the other arduino doesn't signal, try to reset it (tx is atached to reset)
    delay(5); //there is a stabilizing capacitor in the Tx-||-GND line because if we reset the this controller the voltage is drop for a moment and resets the other arduino and we dont need that, this delay is to wait for discharge to capacitor         
  }    
digitalWrite(1,HIGH);  
  if (merorend > 4000) {  //give a fair amount of time to the setup, it has a pump process in it, that is usually takes 1-2 minute
        
    alarm(3000); //if the other arduino doesn't signal for a long time, and didn't reseted corectly, alarm
    page = 31;
        
  }    
    if(reading==222){   //pump error
        alarm(1350);
        //it will be a menu page   
        }
int waitforit;
    if (reading==212) {
resets++;  
   waitforit =5000;            
  }    //if the measuring system is reseted, it send cold signal for the first 2 minute, eliminate this.
  waitforit--;
    if (waitforit>0) disabled=1; 
    //the distance reading, it tell if anyone/anything is standing in front of the display

    digitalWrite(trigPin, LOW);
    delayMicroseconds(2);
    digitalWrite(trigPin, HIGH);
    delayMicroseconds(10);
    digitalWrite(trigPin, LOW);
    duration = pulseIn(echoPin, HIGH);
    distance = (duration / 2) / 29.1;
   
  
  if (distance < 60) { 
    digitalWrite(5, HIGH); //turn on the lcd, the lcd Vcc pin is attached to dpin (you may need a transistor if the lcd is consuming too much)
    lcd.init();
    lcd.backlight();  
    int ii = 0;
    while ( ii < 40) { //the lcd will powered up until the distance sensor dont see the person for 8 seconds
      delay(200);
      digitalWrite(trigPin, LOW);
      delayMicroseconds(2);
      digitalWrite(trigPin, HIGH);
      delayMicroseconds(10);
      digitalWrite(trigPin, LOW);
      duration = pulseIn(echoPin, HIGH);
      distance = (duration / 2) / 29.1;
      if (distance > 100) ii = ii + 1;
//the whole menu with buttons and stuff
      //page0
        if (page == 0) {
        if (digitalRead(but1) == LOW) {
          page = 11;
          prevpage = 0;
          delay(200);
        }
        if ( digitalRead(but3) == LOW) {
          page = 21;
          prevpage = 0;
          delay(200);
        }
      }
        
      if (page == 0 && prevpage != 0) { 
        prevpage = 0; //the prevpage stuff is only needed for to know when we need to refresh te LCD, without this it will referesh all the time and will be blurry, and frustrating to see it        
        h = dht.readHumidity();
        t = dht.readTemperature();
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("kint:")); //the F() function will save a lot of RAM becouse the text are readed from the flash only
        lcd.print(mydata.Tout);
        lcd.print("C ");
        lcd.print(mydata.Hout);
        lcd.print("%");
        lcd.setCursor(0, 1);
        lcd.print(F("uhaz:"));
        lcd.print(mydata.DHTTEMPatlag);
        lcd.print("C ");
        lcd.print(mydata.DHTHUMatlag);
        lcd.print("%");
        lcd.setCursor(0, 2);
        lcd.print(F("bent:"));
        lcd.print(t);
        lcd.print("C ");
        lcd.print(h);
        lcd.print("%");
        lcd.setCursor(0, 3);
        lcd.print(F("stat  "));
        lcd.print(mydata.P);
        lcd.print("mB");
        lcd.setCursor(16, 3);
        lcd.print(F("beav")); //the displayed text in Hungarian
        
      }
      //page11 (autostat)
            if (page == 11) {
        if (digitalRead(but1) == LOW) {
          page = 13;
          prevpage = 11;
          delay(200);
        }
        if (digitalRead(but3) == LOW) {
          page = 12;
          prevpage = 11;
          delay(200);
        }
      }
                     
      if (page == 11 && prevpage != 11) {
        prevpage = 11;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(au.Toutmin);
        lcd.print("C ");
        lcd.print((au.Toutav / au.c));
        lcd.print("C ");
        lcd.print(au.Toutmax);
        lcd.print("C ");
        lcd.setCursor(0, 1);
        lcd.print(au.Tuhazmin);
        lcd.print("C ");
        lcd.print((au.Tuhazav / au.c));
        lcd.print("C ");
        lcd.print(au.Tuhazmax);
        lcd.print("C ");
        lcd.setCursor(0, 2);
        lcd.print(au.Tinmin);
        lcd.print("C ");
        lcd.print((au.Tinav / au.c));
        lcd.print("C ");
        lcd.print(au.Tinmax);
        lcd.print("C ");
        lcd.setCursor(0, 3);
        lcd.print(F("tov"));
        lcd.setCursor(6, 3);
        lcd.print(F("'auto'"));
        lcd.setCursor(15, 3);
        lcd.print(F("beall"));
      }
      //page12(autostat setup)
      if (page == 12 && prevpage != 12) {
        prevpage = 12;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("atlagolasi ciklus"));
        lcd.setCursor(0, 1);
        lcd.print(F("kezdopontja: "));
        lcd.print(au.resH);
        lcd.print(F(" ora"));
        lcd.setCursor(9, 3);
        lcd.print("ok");
        lcd.setCursor(15, 3);
        lcd.print(F("+ora"));
        int cc = 0;
        while (cc < 1) {
          delay(200);  //it is important to tweak these delays, becouse if it is too short, one normal human pushing will be countad as twoo or more
          if (digitalRead(but3) == LOW) {
            delay(200);
            if (au.resH < 24) {
              au.resH = au.resH + 1;
            }
            else {
              au.resH = 0;
            }
            lcd.setCursor(0, 1);   //if the button is pushed the reset hour of the daily stats are increased
            lcd.print(F("kezdopontja: "));
            lcd.print(au.resH);
            lcd.print(F(" ora"));
          }
          if ( digitalRead(but2) == LOW) {
            page = 11;
            prevpage = 12;
            cc = 1;
          }
        }
      }
      //page13 (manual reseted stat)
                         if (page == 13) {
        if (digitalRead(but1) == LOW) {
          page = 0;
          prevpage = 13;
          delay(200);
        }
        if (digitalRead(but3) == LOW) {
          ke.c = 0;
          ke.Tinav = 0;
          ke.Tinmax = 0;
          ke.Tinmin = 100;
          ke.Toutav = 0;
          ke.Toutmax = 0;
          ke.Toutmin = 100;
          ke.Tuhazav = 0;
          ke.Tuhazmax = 0;
          ke.Tuhazmin = 100;
          page = 13;
          prevpage = 1;
          delay(200);
        }
      }
            
      if (page == 13 && prevpage != 13) {
        prevpage = 13;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(ke.Toutmin);
        lcd.print("C ");
        lcd.print((ke.Toutav / ke.c));
        lcd.print("C ");
        lcd.print(ke.Toutmax);
        lcd.print("C ");
        lcd.setCursor(0, 1);
        lcd.print(ke.Tuhazmin);
        lcd.print("C ");
        lcd.print((ke.Tuhazav / ke.c));
        lcd.print("C ");
        lcd.print(ke.Tuhazmax);
        lcd.print("C ");
        lcd.setCursor(0, 2);
        lcd.print(ke.Tinmin);
        lcd.print("C ");
        lcd.print((ke.Tinav / ke.c));
        lcd.print("C ");
        lcd.print(ke.Tinmax);
        lcd.print("C ");
        lcd.setCursor(0, 3);
        lcd.print(F("tov"));
        lcd.setCursor(6, 3);
        lcd.print(F("'kezi'"));
        lcd.setCursor(14, 3);
        lcd.print(F("nulazz"));
      }
      //page21 seed germinator
            
           if (page == 21) {
        if (digitalRead(but1) == LOW) {
          page = 0;
          prevpage = 21;
          delay(200);
        }
        if (digitalRead(but3) == LOW) {
          page = 0;
          prevpage = 21;
          delay(200);
        }
      }
      
      if (page == 21 && prevpage != 21) {
        prevpage = 21;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("csiraztato cel:"));
        lcd.setCursor(0, 1);
        lcd.print(F("akt:"));
        lcd.print(celsius);
        lcd.setCursor(13, 1);
        lcd.print(celfutes);
        lcd.print(" C");
        lcd.setCursor(9, 3);
        lcd.print(F("tov"));
        lcd.setCursor(14, 3);
        lcd.print("+0.5 C");
        lcd.setCursor(2, 2);
        lcd.print(felfutes);
        lcd.setCursor(0, 3);
        lcd.print(F("felfutes"));
        int cc = 0;
        while (cc < 1) {
          delay(200);
          if (digitalRead(but3) == LOW) {

            if (celfutes < 30.0) {
              celfutes = celfutes + 0.5;
            }
            else {
              celfutes  = 15.0;
            }
            lcd.setCursor(13, 1);
            lcd.print(celfutes);
            lcd.print(" C");
          }
          if ( digitalRead(but2) == LOW) {
            page = 22;
            prevpage = 21;
            cc = 1;
          }
          if ( digitalRead(but1) == LOW) {
            lcd.setCursor(2, 2);
            if (felfutes == 0) felfutes = 1;
            else felfutes = 0;
            lcd.print(felfutes);
          }
        }
      }
      //page22 debug mode
      if (page == 22 && prevpage != 22) {
        prevpage = 22;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("hibakereso mod:"));
        lcd.setCursor(0, 1);
        lcd.print(F("erkezo:"));
        lcd.setCursor(5, 1);
        lcd.print(reading);
        lcd.setCursor(0, 2);
        lcd.print(F("resets:"));
        lcd.print(resets);
        lcd.setCursor(14, 3);
        lcd.print(F("kilep"));

        int cc = 0;
        while (cc < 1) {
          delay(20);
          if (Serial.available()) {
            reading= Serial.read();
            lcd.setCursor(5, 1);
            lcd.print(reading);
            lcd.print("  ");
          }
          if (digitalRead(but3) == LOW) {
            page = 0;
            prevpage = 22;
            cc = 1;
            delay(200);
          }
          if ( digitalRead(but2) == LOW) {
           
          }
          if ( digitalRead(but1) == LOW) {
          
        }
        }
      }
      //page 31 communication error
          if (page == 31) {
        if (digitalRead(but1) == LOW) {
          page = 0;
          prevpage = 31;
          delay(200);
        }
        if (digitalRead(but3) == LOW) {
          disabled = 1;
          page = 0;
                    prevpage = 31;
          delay(200);
        }
      }
                        
                
      if (page == 31 && prevpage != 31) {
        prevpage = 31;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("uhaz komm hiba"));
        lcd.setCursor(0, 1);
        lcd.print(F("utolso komm:"));
        lcd.print(merorend);
        lcd.setCursor(0, 2);
        lcd.print(F("read:"));
        lcd.print(reading);
        lcd.setCursor(0, 3);
        lcd.print(F("vissza"));
        lcd.setCursor(13, 3);
        lcd.print(F("nemitas"));
      }
        //page 32 germinator alarm
     if (page == 32) {
        if (digitalRead(but1) == LOW) {
          page = 0;
          prevpage = 32;
          delay(200);
        }
        if (digitalRead(but3) == LOW) {
          disabled = 1;
          page = 0;
                    prevpage = 32;
          delay(200);
        }
      }
                      
    if (page == 32 && prevpage != 32) {
      prevpage = 32;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print(F("csiraztato hiba"));
      lcd.setCursor(0, 1);
      lcd.print(F("homerseklet:"));
      lcd.print(celsius);
      lcd.setCursor(0, 3);
      lcd.print(F("vissza"));
      lcd.setCursor(13, 3);
      lcd.print(F("nemitas"));
    }
    //page 34 cold alarm
           if (page == 34) {
        if (digitalRead(but1) == LOW) {
          page = 0;
          prevpage = 34;
          delay(200);
        }
        if (digitalRead(but3) == LOW) {
         page = 0;
        disabled = 1;
          prevpage = 34;
          delay(200);
        }
      }
                    
                
  if (page == 34 && prevpage != 34) {
        prevpage = 34;
        lcd.clear();
        lcd.setCursor(0, 0);
        lcd.print(F("HIDEG"));
        lcd.setCursor(0, 3);
        lcd.print(F("vissza"));
        lcd.setCursor(13, 3);
        lcd.print(F("nemitas"));
      }
    
        
   }
             
prevpage=1; //in some case, when the alarm page displayed, if no buttons are pushed, the next displaying get blank, this always reset the display
    digitalWrite(5, LOW); //power down the LCD
  }
}






void dsfunc() {
  byte i;
  byte present = 0;
  byte type_s;
  byte data[12];
  byte addr[8];


  if ( !ds.search(addr)) {
    ds.reset_search();
    delay(250);
    return;
  }


  if (OneWire::crc8(addr, 7) != addr[7]) {
    Serial.println(F("CRC is not valid!"));
    return;
  }

  ds.reset();
  ds.select(addr);
  ds.write(0x44, 1);        // start conversion, with parasite power on at the end

  delay(1000);     // maybe 750ms is enough, maybe not
  // we might do a ds.depower() here, but the reset will take care of it.

  present = ds.reset();
  ds.select(addr);
  ds.write(0xBE);         // Read Scratchpad

  for ( i = 0; i < 9; i++) {           // we need 9 bytes
    data[i] = ds.read();
  }

  // Convert the data to actual temperature
  // because the result is a 16 bit signed integer, it should
  // be stored to an "int16_t" type, which is always 16 bits
  // even when compiled on a 32 bit processor.
  int16_t raw = (data[1] << 8) | data[0];
  if (type_s) {
    raw = raw << 3; // 9 bit resolution default
    if (data[7] == 0x10) {
      // "count remain" gives full 12 bit resolution
      raw = (raw & 0xFFF0) + 12 - data[6];
    }
  } else {
    byte cfg = (data[4] & 0x60);
    // at lower res, the low bits are undefined, so let's zero them
    if (cfg == 0x00) raw = raw & ~7;  // 9 bit resolution, 93.75 ms
    else if (cfg == 0x20) raw = raw & ~3; // 10 bit res, 187.5 ms
    else if (cfg == 0x40) raw = raw & ~1; // 11 bit res, 375 ms
    //// default is 12 bit resolution, 750 ms conversion time
  }
  celsius = (float)raw / 16.0;
}

