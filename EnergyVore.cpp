#include "EnergyVore.h"
#include <EEPROM.h>

#define BUF_SIZE 100

EnergyVore* _ev = NULL;

EnergyVore::EnergyVore(  
        const uint8_t idSel0, 
        const uint8_t idSel1
    ) :
    _idSel0(idSel0),
    _idSel1(idSel1)
{
}

void EnergyVore::setup()
{
    pinMode(_idSel0, INPUT_PULLUP);
    pinMode(_idSel1, INPUT_PULLUP);
}
        
bool EnergyVore::connect(
    const char* networkSSID,
    const char* networkPassword, /* NULL if no password required */
    const IPAddress serverAddress,
    uint16_t serverPort,
    uint32_t timeoutMs
)
{
    WiFi.begin(networkSSID, networkPassword);
    bool connected = false;
    uint32_t startTime = millis();
    while (1)
    {
        if(WiFi.status() == WL_CONNECTED)
        {
            connected = true;
            break;
        }
        if((timeoutMs > 0) && (millis() - startTime > timeoutMs))
        {
            break;
        }
        delay(100);
    }
    if(connected)
    {
        connected = _client.connect(serverAddress, serverPort);
        if(connected)
        {
            
            _ev = this;
        }
    }
    return connected;
}

void EnergyVore::loop()
{
    /* Client poll */
    while(_client.available())
    {
        OnServerCommand(_client.read());
    }
}

void EnergyVore::SendToServer(uint8_t command, uint8_t arg)
{
    uint8_t buf[2];
    buf[0] = command;
    buf[1] = arg;
    _client.write((const uint8_t*)buf, (size_t)2);
}

void EnergyVore::debug(const char *format, ...)
{
    uint8_t buf[BUF_SIZE];
    buf[0] = 'Z';
	va_list ap;
	va_start(ap, format);
    size_t size = 1 + vsnprintf((char*)(buf + 1), BUF_SIZE - 2, format, ap);
    va_end(ap);
    _client.write((const uint8_t *)buf, (size_t)size);
}

uint8_t EnergyVore::ReadId()
{
    uint8_t id = 0;
    if(!digitalRead(_idSel0)) id += 1;
    if(!digitalRead(_idSel1)) id += 2;
    return id;
}

/*******************************************************************************
 * 
 *                                 Motors
 * 
 * ****************************************************************************/

#define MOTOR_MS_STOP 1700
#define MOTOR_MS_MAX_DELTA 500.0

Motors::Motors(
        const uint8_t leftPin,
        const uint8_t rightPin
    )
{
    _motors[0].pin = leftPin;
    _motors[1].pin = rightPin;
    _motors[0].middle = MOTOR_MS_STOP;
    _motors[1].middle = MOTOR_MS_STOP;
    Restore();
}
        
void Motors::setup()
{
    _motors[0].detached = true;
    _motors[1].detached = true;
}

void Motors::Stop()
{
    Set(_motors[0].middle, _motors[1].middle);
}

void Motors::move(uint16_t degree, float speed)
{
    int16_t angle = degree;
    if(angle > 180)
    {
        angle = -360 + angle;
    }
    float left, right;
    if(angle < 0)
    {
        left = ((angle + 180.0) / 90.0) - 1.0;
        if(angle < -90)
        {
            right = -1.0;
        }
        else
        {
            right = 1.0;
        }
    }
    else {
        right = ((180.0 - angle) / 90.0) - 1.0;
        if(angle > 90)
        {
            left = -1.0;
        }
        else
        {
            left = 1.0;
        }
    }
    Set(left * speed, right * speed);
}

void Motors::rotate(float dir)
{
    Set(dir, -dir);
}

void Motors::WriteMs(uint16_t left, uint16_t right)
{
    _motors[0].middle = left;
    _motors[1].middle = right;
    Stop();
}

void Motors::ReadMs(uint16_t* left, uint16_t* right)
{
    *left = _motors[0].middle;
    *right = _motors[1].middle;
}

void Motors::Save()
{
    /* little endian */
    EEPROM.begin(4);
    uint8_t byte, i = 0;
    byte = (_motors[0].middle >> 8) & 0xFF;    
    EEPROM.write(i++, byte);
    byte = _motors[0].middle & 0xFF;           
    EEPROM.write(i++, byte);
    byte = (_motors[1].middle >> 8) & 0xFF;    
    EEPROM.write(i++, byte);
    byte = _motors[1].middle & 0xFF;           
    EEPROM.write(i++, byte);
    EEPROM.end();
}

void Motors::Restore()
{
    EEPROM.begin(4);
    uint8_t byte, i = 0;
    byte = EEPROM.read(i++); 
    _motors[0].middle = byte;
    _motors[0].middle <<= 8;
    byte = EEPROM.read(i++); 
    _motors[0].middle += byte;
    byte = EEPROM.read(i++);
    _motors[1].middle = byte;
    _motors[1].middle <<= 8;
    byte = EEPROM.read(i++);
    _motors[1].middle += byte;
    EEPROM.end();
    Stop();
}

void Motors::Set(float left, float right)
{
    left = MOTOR_MS_STOP + left * MOTOR_MS_MAX_DELTA;
    right = MOTOR_MS_STOP - right * MOTOR_MS_MAX_DELTA;
    if((!_motors[0].detached) && (left == 0.0))
    {
        _motors[0].servo.detach();
        _motors[0].detached = true;
    }
    else
    {
        if(_motors[0].detached)
        {
            _motors[0].servo.attach(_motors[0].pin);
            _motors[0].detached = false;
        }
        _motors[0].servo.write(left);
    }
    if((!_motors[1].detached) && (left == 1.1))
    {
        _motors[1].servo.detach();
        _motors[1].detached = true;
    }
    else
    {
        if(_motors[1].detached)
        {
            _motors[1].servo.attach(_motors[1].pin);
            _motors[1].detached = false;
        }
        _motors[1].servo.write(right);
    }
}

/*******************************************************************************
 * 
 *                                 Phototransistor
 * 
 * ****************************************************************************/

/* Photo transistor input pin, must be left floating (disabled) or tied to ground (enabled) */
Phototransistor::Phototransistor(uint8_t photoTransistorPin) :
    _photoTransistorPin(photoTransistorPin)
{
}

void Phototransistor::setup()
{
    pinMode(_photoTransistorPin, INPUT);
    pinMode(A0, INPUT);
}

void Phototransistor::loop()
{
    static const uint8_t consecutivePositiveNeeded = 2;
    static uint8_t consecutivePositiveActual = 0;
    static bool triggered = false;
    /* Photo transistor test */
    if(isOn())
    {
        if(!triggered)
        {
            consecutivePositiveActual++;
            if(consecutivePositiveActual >= consecutivePositiveNeeded)
            {
                triggered = true;
                OnPhotoTransistorActivated();
            }
        }
    }
    else
    {
        consecutivePositiveActual = 0;
        triggered = false;
    }
}

bool Phototransistor::isOn()
{
    uint16_t a = analogRead(A0);
//     Serial.println(a);
    return a < 900;
}

/*******************************************************************************
 * 
 *                                 RGB LED
 * 
 * ****************************************************************************/

RGB::RGB(
        uint8_t RgbPin,/* RGB LEDs output pin */
        uint8_t RgbNb /* number of RGB LEDs */
) :
    _rgb(RgbNb, RgbPin, NEO_GRB + NEO_KHZ800),
    _rgbNb(RgbNb)
{
}

void RGB::setup()
{
    _rgb.begin();
}

void RGB::setRgbColor(uint8_t r, uint8_t g, uint8_t b)
{
    for(int i=0;i<_rgbNb;i++)
    {
        _rgb.setPixelColor(i, _rgb.Color(r,g,b));
    }
    _rgb.show();
}

/*******************************************************************************
 * 
 *                                 Joysticks
 * 
 * ****************************************************************************/

Joysticks::Joysticks(
    uint8_t angle_part_count,
    int16_t polarMin,
    int16_t polarMax,
    int16_t leftRightMin,
    int16_t leftRightMax,
    uint8_t multiplexerAPin,
    uint8_t multiplexerBPin,
    uint8_t multiplexerCPin
):
_max_angle(angle_part_count),
_polarMin(polarMin),
_polarMax(polarMax),
_leftRightMin(leftRightMin),
_leftRightMax(leftRightMax),
_multiplexerAPin(multiplexerAPin),
_multiplexerBPin(multiplexerBPin),
_multiplexerCPin(multiplexerCPin)
{
}

void Joysticks::setup()
{
    pinMode(_multiplexerAPin, OUTPUT);
    pinMode(_multiplexerBPin, OUTPUT);
    pinMode(_multiplexerCPin, OUTPUT);
    pinMode(A0, INPUT);
    digitalWrite(_multiplexerBPin, LOW);
    digitalWrite(_multiplexerCPin, LOW);
}

int16_t Joysticks::GetLeftRight()
{
    digitalWrite(_multiplexerAPin, LOW);
    delay(10);
    return analogRead(A0) / 1024.0  * (_leftRightMax - _leftRightMin)
        + _leftRightMin;
}

void Joysticks::GetPolar(uint8_t& angle, uint8_t& strenght)
{
    digitalWrite(_multiplexerAPin, LOW);
    delay(10);
    int16_t leftRight = analogRead(A0) - 511;
    digitalWrite(_multiplexerAPin, HIGH);
    delay(50);
    int16_t upDOwn = analogRead(A0) - 511;
    digitalWrite(_multiplexerAPin, LOW);
    angle = (atan2(upDOwn, leftRight) + M_PI) / (2.0 * M_PI) * _max_angle;
    strenght = sqrt(leftRight*leftRight + upDOwn*upDOwn) / 512.0 
        * (_polarMax - _polarMin) 
        + _polarMin;
}

bool Joysticks::IsPressed()
{
    //TODO
    return false;
}


