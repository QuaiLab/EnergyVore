#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Servo.h>
#include <EEPROM.h>
#include <EnergyVore.h>

uint16_t _left = 1500 , _right = 1500;

Motors motors(
    13, /* Motor left pin */
    12 /* Motor right pin */
);

void setup() 
{
    delay(100);
    Serial.begin(57600);
    motors.setup();
}

void loop() 
{
    if(Serial.available())
    {
        char cmd = Serial.read();    
        switch(cmd)
        {
            case 'S':
                motors.Save();
                break;
            case 'D':
                _left++;
                break;
            case 'A':
                _right++;
                break;
            case 'R':
                _right--;
                break;
            case 'G':
                _left--;
                break;
            default:
                /* nothing to do */
                break;
        }   
        motors.WriteMs(_left, _right);
        motors.Stop();
        Serial.print(_left);
        Serial.print(" ");
        Serial.print(_right);
        Serial.println();
    }
}
