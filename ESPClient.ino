/*
	This sketch establishes a TCP connection to a "quote of the day" service.
	It sends a "hello" message, and then prints received data.
*/

#include <ESP8266WiFi.h>
#include <time.h>

class WaterDetector
{
private:
    uint8_t _digitalPin;
    uint8_t _analogPin;
    bool _state; // true, water detected

public:
    WaterDetector(uint8_t digitalPin, uint8_t analogPin = -1)
    {
        _digitalPin = digitalPin;
        _analogPin = analogPin;
    }

    void Begin()
    {
        _state = false;
        pinMode(_digitalPin, INPUT);
    }

    bool Detect()
    {
        // water detected t = LOW, and t = HIGH water undetected
        uint8_t t = digitalRead(_digitalPin);
        _state = (t == LOW);
        return _state;
    }

    int Read()
    {
        if (_analogPin >= 0)
            return analogRead(_analogPin);
        else
            return -1;
    }
};

class Component
{
private:
    bool _state;
    uint8_t _pin;

public:
    Component(uint8_t pin)
    {
        _pin = pin;
    }

    void Begin()
    {
        pinMode(_pin, OUTPUT);
        digitalWrite(_pin, HIGH);
        _state = false;
    }

    void Toggle()
    {
        // true, turn off | false, turn on
        (_state) ? digitalWrite(_pin, HIGH) : digitalWrite(_pin, LOW);
        _state = !_state;
    }

    void Toggle(int del)
    {
        Toggle();
        delay(del);
        Toggle();
    }

    void TurnOff()
    {
        if (_state == true)
        {
            _state = false;
            digitalWrite(_pin, HIGH);
        }
    }

    void TurnOn()
    {
        if (_state == false)
        {
            _state = true;
            digitalWrite(_pin, LOW);
        }
    }
};

const char *_ssid = "";
const char *_password = "";

const char *_host = "192.168.0.24";
const uint16_t _port = 8777;

const char *_name = "Hugh";

const int DETECTORS = 1;
WaterDetector resevoirDetector(5);
WaterDetector detectors[DETECTORS] = {WaterDetector(13, 0xA0)};
Component pumpRelay(2);
Component powerRelay(3);

int sleepTime = 0;

uint8_t _tz = -7;

void setup()
{
    Serial.begin(9600);

    ConnectToWifi();
    InitTime();

    InitComponents();
}

void loop()
{// TODO add a big if based on the time... so it doesn't check after a set time. 


    // Turn on power to sensors
    // and check readings
    unsigned long waterDuration = 5000;
    unsigned long wateringEnd = millis();

    unsigned long readDuration = 2000;
    unsigned long readEnd = millis() + readDuration;
    unsigned int readCnt = 0;
    unsigned long moisture = 0;
    bool needsWatering = false;

    powerRelay.TurnOn();
    delay(1000); // allow for the sensors to start detecting water

    // Time that is to be reading
    do
    {
        // Read the current reporting analog levels.
        for (int i = 0; i < DETECTORS; i++)
        {
            moisture += detectors[i].Read();
            //            Serial.println(detectors[i].Read());
            readCnt++;
        }

        if (needsWatering == false)
        {
            needsWatering = true;

            // Check if watering is needed
            for (int i = 0; i < DETECTORS; i++)
                needsWatering = (needsWatering && !detectors[i].Detect());

            if (resevoirDetector.Detect() && needsWatering)
            {
                pumpRelay.TurnOn();
                wateringEnd = millis() + waterDuration;
                readEnd = millis() + readDuration; // read after watering
            }
        }

        if (!resevoirDetector.Detect() && millis() < wateringEnd)
        {
            // TODO could add in a flashing LED and set a global variable such that the resevoir needs water
            pumpRelay.TurnOff();
            waterDuration = 0;
            break;
        }
    } while (millis() < readEnd);

    // get the average water level(s)
    Serial.println(moisture);

    // make sure the pump relay has been turned off after the allotted time.
    pumpRelay.TurnOff();
    // turn off the power relay
    powerRelay.TurnOff();

    // Sleep for an hour
    // delay(3600000);

    // Use WiFiClient class to create TCP connections
    WiFiClient client;
    if (!client.connect(_host, _port))
    {
        // connection failed, so just try again next loop.
        delay(5000);
        return;
    }

    // Send data to the server
    int water = 200;
    if (client.connected())
    {
        client.write(_name);
        client.write(water);
    }

    // Close the connection
    client.stop();

    delay(10000); // execute once every 5 minutes, don't flood remote service
}

void ConnectToWifi()
{
    // connect to wifi
    WiFi.mode(WIFI_STA);
    WiFi.begin(_ssid, _password);

    // wait to connect
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
}

void InitTime()
{
    configTime(_tz * 3600, 0, "pool.ntp.org", "time.nist.gov");
    Serial.println("\nWaiting for time");
    while (!time(nullptr))
    {
        Serial.println(".");
        delay(1000);
    }
}

void InitComponents()
{
    pumpRelay.Begin();
    powerRelay.Begin();
    for (int i = 0; i < DETECTORS; i++)
        detectors[i].Begin();
    resevoirDetector.Begin();
}

void PrintConnection(const char *host, uint16_t port)
{
    Serial.print("connecting to ");
    Serial.print(host);
    Serial.print(':');
    Serial.println(port);
}

void PrintTime()
{
    time_t now = time(nullptr);
    struct tm *p_tm = localtime(&now);
    Serial.print(p_tm->tm_mday);
    Serial.print("/");
    Serial.print(p_tm->tm_mon + 1);
    Serial.print("/");
    Serial.print(p_tm->tm_year + 1900);

    Serial.print(" ");

    Serial.print(p_tm->tm_hour);
    Serial.print(":");
    Serial.print(p_tm->tm_min);
    Serial.print(":");
    Serial.println(p_tm->tm_sec);
}

// static const uint8_t D0   = 16;
// static const uint8_t D1   = 5;
// static const uint8_t D2   = 4;
// static const uint8_t D3   = 0;
// static const uint8_t D4   = 2;
// static const uint8_t D5   = 14;
// static const uint8_t D6   = 12;
// static const uint8_t D7   = 13;
// static const uint8_t D8   = 15;
// static const uint8_t D9   = 3;
// static const uint8_t D10  = 1;