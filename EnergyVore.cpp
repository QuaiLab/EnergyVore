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
#define EPSILON 50

Motors::Motors(
        const uint8_t leftPin,
        const uint8_t rightPin
    )
{
    _motors[0].pin = leftPin;
    _motors[1].pin = rightPin;
    _motors[0].middle = MOTOR_MS_STOP;
    _motors[1].middle = MOTOR_MS_STOP;
}
        
void Motors::setup()
{
    _motors[0].detached = true;
    _motors[1].detached = true;
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

void Motors::Set(float left, float right)
{
    left = MOTOR_MS_STOP + left * MOTOR_MS_MAX_DELTA;
    right = MOTOR_MS_STOP - right * MOTOR_MS_MAX_DELTA;
    bool detach_left =  (left > MOTOR_MS_STOP - EPSILON) && 
                        (left < MOTOR_MS_STOP + EPSILON);
    bool detach_right = (right > MOTOR_MS_STOP - EPSILON) && 
                        (right < MOTOR_MS_STOP + EPSILON);
    if(detach_left && (!_motors[0].detached))
    {
        _motors[0].servo.detach();
        _motors[0].detached = true;
    }
    if((!detach_left) && _motors[0].detached)
    {
            _motors[0].servo.attach(_motors[0].pin);
            _motors[0].detached = false;
    }
    if(!_motors[0].detached)
    {
        _motors[0].servo.write(left);
    }
    if(detach_right && (!_motors[1].detached))
    {
        _motors[1].servo.detach();
        _motors[1].detached = true;
    }
    if((!detach_right) && _motors[1].detached)
    {
            _motors[1].servo.attach(_motors[1].pin);
            _motors[1].detached = false;
    }
    if(!_motors[1].detached)
    {
        _motors[1].servo.write(right);
    }
}
