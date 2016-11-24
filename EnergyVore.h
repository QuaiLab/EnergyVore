#ifndef ENERGYVORE_H
#define ENERGYVORE_H

#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
  #include <avr/power.h>
#endif
#include <Servo.h>

extern void OnServerCommand(uint8_t command);
extern void OnPhotoTransistorActivated();

class EnergyVore
{
    public:
        EnergyVore(  
            const uint8_t idSel0, 
            const uint8_t idSel1
        );
        
        void setup();
        
        /* returns true on success, false, otherwise */
        bool connect(
            const char* networkSSID,
            const char* networkPassword, /* NULL if no password required */
            const IPAddress serverAddress,
            uint16_t serverPort,
            uint32_t timeoutMs = 0/* 0 for no timeout */
        );
        
        void loop();
        
        /* Send a single byte command to server */
        void SendToServer(uint8_t command, uint8_t arg);
        void debug(const char *format, ...);
        /* Read th identifier from the Jumpers */
        uint8_t ReadId(); 
    
    private:
        
        WiFiClient _client;
        const uint8_t _idSel0;
        const uint8_t _idSel1;
        
};

class Motors
{
    public:
        Motors(
            const uint8_t leftPin,
            const uint8_t rightPin
        );
        
        void setup();
        void move(uint16_t degree, float speed);
        void rotate(float dir);
        
    private:
        struct Motor
        {
            Servo servo;
            uint16_t middle;
            uint8_t pin;
            bool detached;
        };
        Motor _motors[2];
        
        void Set(float left, float right);
};


#endif /* ENERGYVORE_H */
