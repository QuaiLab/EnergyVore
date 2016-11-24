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

uint32_t _lastPeriodicTime;
uint32_t _periodicPeriod = 100;
const uint8_t discoverPeriods = 2; /* run every 2*100ms */

void setup() 
{
    EV.setup();
    delay(50);
    EV.connect("energyvore", "energyvore", IPAddress(192, 168, 12, 1), 4242);
    motors.setup();
    _lastPeriodicTime = millis();
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
    }
}

void Periodic()
{
    static uint8_t discoverDiviser;
    if(!_associated)
    {
        discoverDiviser++;
        if(discoverDiviser >= discoverPeriods)
        {
            discoverDiviser = 0;
            EV.SendToServer('Z', EV.ReadId() + '0');
        }
    }
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
            if(code == 'V')
            {
                _associated = true;
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
