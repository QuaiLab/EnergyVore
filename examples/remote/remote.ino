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

Joysticks joysticks(
    24, /* angle count */
    0, /* polar min */
    9, /* polar max */
    1, /* left right min */
    9, /* left right max */
    13, /* multiplexer selector pin A */
    12, /* multiplexer selector pin B */
    14  /* multiplexer selector pin C */
);

uint32_t _lastSendTime;
uint32_t _sendPeriod = 200;

void setup() 
{
    Serial.begin(57600);
    EV.setup();
    delay(50);
    EV.connect("energyvore", "energyvore", IPAddress(192, 168, 12, 1), 4242);
    joysticks.setup();
    _lastSendTime = millis();
}

void loop() 
{
    EV.loop();
    uint32_t now = millis();
    if(now - _lastSendTime > _sendPeriod)
    {
        if(_associated)
        {
            if(joysticks.IsPressed())
            {
                uint8_t strenght = joysticks.GetLeftRight();
                EV.SendToServer('Y', strenght);
            }
            else
            {
                uint8_t angle, strenght;
                joysticks.GetPolar(angle, strenght);
                EV.SendToServer(angle + 'A', strenght + '0');
            }
        }
        else
        {
            uint8_t id = EV.ReadId();
            EV.SendToServer('Z', id + '4');
        }
        _lastSendTime = now;
    }
}

void OnServerCommand(uint8_t code)
{
    static enum
    {
        command,
        parameter
    }state = command;
    switch(state)
    {
        case command:
            if(code == 'Z')
            {
                state = parameter;
            }
            break;
        case parameter :
            switch(code)
            {
                case 'V':
                    _associated = true;
                    break;
                default:
                    break;
            }
            state = command;
            break;
    }
}
