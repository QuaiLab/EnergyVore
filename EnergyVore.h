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
            const uint8_t idSel0, /* idSel* input pin, must be left floating (disabled) or tied to ground (enabled) */
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
        void Stop();
        /**
         * @param degree angle from 0 up to 360°
         * @param speed speed between 0.0 and 1.0
         * */
        void move(uint16_t degree, float speed);
        /**
         * @pram dir the direction and velocity betwwen -1.0 and 1.0: -1.0 is
         * the mst left and 1.0 is the most right
         * */
        void rotate(float dir);
        void WriteMs(uint16_t left, uint16_t right);
        void ReadMs(uint16_t* left, uint16_t* right);
        void Save();
        void Restore();
        
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

class Phototransistor
{
    public:
        Phototransistor(
            uint8_t photoTransistorPin /* Photo transistor input pin, must be left floating (disabled) or tied to ground (enabled) */
        );
        
        void setup();
        void loop();
        
        bool isOn();
        
    private:
        uint8_t _photoTransistorPin;
};

class RGB
{
    public:
        RGB(
            uint8_t RgbPin,/* RGB LEDs output pin */
            uint8_t RgbNb /* number of RGB LEDs */
        );
        
        void setup();
        /* Set RGB color */
        void setRgbColor(uint8_t r, uint8_t g, uint8_t b);
        
    private :
        Adafruit_NeoPixel _rgb;
        uint8_t _rgbNb;
};

class Joysticks
{
    public:
        
        Joysticks(
            uint8_t angle_part_count,
            int16_t polarMin,
            int16_t polarMax,
            int16_t leftRightMin,
            int16_t leftRightMax,
            uint8_t multiplexerAPin,
            uint8_t multiplexerBPin,
            uint8_t multiplexerCPin
        );

        void setup();
        int16_t GetLeftRight();
        void GetPolar(uint8_t& angle, uint8_t& strenght);
        bool IsPressed();

    private:
        uint16_t _max_angle;
        int16_t _polarMin; 
        int16_t _polarMax;
        int16_t _leftRightMin; 
        int16_t _leftRightMax;
        uint8_t _multiplexerAPin;
        uint8_t _multiplexerBPin;
        uint8_t _multiplexerCPin;
};

#endif /* ENERGYVORE_H */
