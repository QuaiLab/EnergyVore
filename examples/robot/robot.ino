#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Servo.h>
#include <EEPROM.h>
#include <EnergyVore.h>

bool _associated = false;

EnergyVore EV(  
    4, /* Id selector 1 */
    5  /* Id selector 2 */
);
    
Motors motors(
    13, /* Motor left pin */
    12 /* Motor right pin */
);

RGB LED(14, 1);

Phototransistor pt(16); /* unused pin left at High-Z, analog is used */

uint32_t _lastPeriodicTime;
uint32_t _periodicPeriod = 100;
const uint8_t ptPeriods = 2;
const uint8_t discoverPeriods = 2;

void setup() 
{
    Serial.begin(57600);
    EV.setup();
    delay(50);
    EV.connect("energyvore", "energyvore", IPAddress(192, 168, 12, 1), 4242);
    LED.setup();
    pt.setup();
    LED.setRgbColor(50, 50, 50);
    motors.setup();
//     /* Red LED means not connected to server */
    _lastPeriodicTime = millis();
//     motors.Restore();
}

void loop() 
{
    EV.loop();
    uint32_t now = millis();
    /* run periodically */
    if(now - _lastPeriodicTime > _periodicPeriod)
    {
        _lastPeriodicTime = now;
        Periodic();
//         loopLed();
    }
}

void Periodic()
{
    static uint8_t ptDiviser;
    static uint8_t discoverDiviser;
    if(_associated)
    {
        ptDiviser++;
        if(ptDiviser >= ptPeriods)
        {
            ptDiviser = 0;
            pt.loop();
        }
    }
    else
    {
        discoverDiviser++;
        if(discoverDiviser >= discoverPeriods)
        {
            discoverDiviser = 0;
            EV.SendToServer('Z', EV.ReadId() + '0');
        }
    }
}

void OnPhotoTransistorActivated()
{
    EV.SendToServer('D', '1');
}

void OnServerCommand(uint8_t code)
{
    static enum
    {
        command,
        parameter_state,
        parameter_dir
    }state = command;
    static uint8_t lastCode;
    switch(state)
    {
        case command:
            if((code >= 'A') && (code <= 'Y'))
            {
                lastCode = code;
                state = parameter_dir;
            }
            if(code == 'Z')
            {
                state = parameter_state;
            }
            break;
        case parameter_state :
            switch(code)
            {
                case 'V':
                    _associated = true;
                    break;
                case 'D':
//                     LED.setRgbColor(0, 100, 0);
                    break;
                case 'O':
//                     LED.setRgbColor(0, 0, 0);
                    break;
                default:
                    break;
            }
            state = command;
            break;
        case parameter_dir:
            if((code >= '0') && (code <= '9'))
            {
                if(lastCode == 'Y')
                {
                    float dir = code - '0';
                    dir = (dir - 5.0)/4.0;
                    motors.rotate(dir);
                }
                else
                {
                    uint16_t degree = (lastCode - 'A') * 15;
                    float speed = code - '0';
                    speed /= 9.0;
                    motors.move(degree, speed);
                }
            }
            state = command;
            break;
    }
}

// void loopLed()
// {
//     static uint8_t bright = 0;
//     LED.setRgbColor(bright, bright, bright);
//     bright+= 5;
// }

