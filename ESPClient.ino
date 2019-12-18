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
    WaterDetector(uint8_t digitalPin, uint8_t analogPin)
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
        return analogRead(_analogPin);
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

uint8_t _tz = -7;

void setup()
{
	Serial.begin(9600);

	ConnectToWifi();
	InitTime();
}

void loop()
{
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
		// ESP.getChipId();
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