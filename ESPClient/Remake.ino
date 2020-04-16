#include <ESP8266WiFi.h>
#include <time.h>
#include <vector>
#include <string>

class CapacitiveSensor
{
private:
    uint8_t _analogPin;
    bool _state;
    float _min;
    float _max;

public:
    CapacitiveSensor(uint8_t analogPin)
    {
        _analogPin = analogPin;
        _min = 500.f;
        _max = 700.f;
    }

    void Begin()
    {
        _state = false;
    }

    int Read()
    {
        int val = analogRead(_analogPin);
        Serial.println(val);
        return val;
    }

    int ReadPercent()
    {
        float val = Read();

        // adjust values to be tailored to that plant
        if (val < _min)
            _min = val;
        if (val > _max)
            _max = val;

        // normalize
        if (val > _max)
            val = _max;
        else if (val < _min)
            val = _min;

        val = ((val - _min) / (_max - _min)) * 100;

        return val;
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

class Plant
{
private:
    String _name;
    std::vector<CapacitiveSensor*> _sensors;
    Component *_pump;
    int _watering_threshold;
    int _water_amount;

public:
    // TODO add a resevoir reference
    Plant(String name, int num_sensors, uint8_t sensor_pins[], uint8_t pump_pin, int watering_threshold, int water_amount)
    {
        _name = name;

        _sensors = std::vector<CapacitiveSensor*>(num_sensors);
        for (int i = 0; i < num_sensors; i++)
            _sensors[i] = new CapacitiveSensor(sensor_pins[i]);
        _pump = new Component(pump_pin);

        _watering_threshold = watering_threshold;
        _water_amount = water_amount;
    }

    void Begin()
    {
        for (int i = 0; i < _sensors.size(); i++)
            _sensors[i]->Begin();
        _pump->Begin();
    }

    void ToDouse()
    {
        uint8_t to_douse = 0; // 1 get watered
        std::vector<uint32_t> prewatering_moistures = std::vector<uint32_t>();
        std::vector<uint32_t> postwatering_moistures = std::vector<uint32_t>();

        prewatering_moistures = ReadWaterLevels();

        // determine if watering is needed
        for (int i = 0; i < prewatering_moistures.size(); i++)
            to_douse = to_douse && (prewatering_moistures[i] < _watering_threshold);

        if (to_douse)
        {
            // Douse the plant
        }
        else
        {
            // if the plant doesn't need to be watered then they are equal
            postwatering_moistures = prewatering_moistures;
        }
        
    }

    std::vector<uint32_t> ReadWaterLevels()
    {
        std::vector<uint32_t> moistures = std::vector<uint32_t>();
        uint32_t read_end;
        float values;
        float reads;
        for (int i = 0; i < _sensors.size(); i++)
        {
            read_end = millis() + 2000;
            values = 0.f;
            reads = 0.f;
            while (millis() < read_end)
            {
                values = _sensors[i]->ReadPercent();
                reads += 1.f;
                delay(100);
            }
            moistures.push_back(values / reads);
        }

        return moistures;
    }

    String ToString()
    {

    }
};

class PlantWaterer
{
private:
public:
};

const String _ssid = "";
const String _password = "";

const String _host = "192.168.0.39";
const uint16_t _port = 8777;

uint8_t _min_threshhold = 30; // percent in which we water
uint8_t _morning_threshold = 7;  // time to check levels in the morning
uint8_t _evening_threshold = 20; // time to check the levels in the evening
int8_t _tz = -7; // timezone

// uint32_t _sleepTime = 3600000; // This will change based on how often to check the plant
uint32_t _sleepTime = 300000; // This will change based on how often to check the plant

uint16_t _waterDuration = 5000; // TODO measure how much water the pump can pump in a second.
    // That amount wil be the starting point where I can do 1 seocnd to start the pump
    // plus the time that it will take to equal a certain amount of litres
    // 1 + (wanted mL / flow) <-- this is the duration in seconds.
    // according to my old program, 5000ms is about 250mL
uint8_t hugh_pins[] = {0xA0};
Plant hugh = Plant("Hugh", 1, hugh_pins, 14, 250);

void setup()
{
    Serial.begin(115200);

    ConnectToWifi();

    hugh.Begin();
}

void loop()
{
    time_t now = time(nullptr);
    struct tm *p_tm = localtime(&now);
    uint8_t hr = p_tm->tm_hour;

    // if (hr > _morning_threshold && hr < _evening_threshold)
    if (true)
    {

    }
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
	while (time(nullptr) <= 100000)
	{
		Serial.println(".");
		delay(1000);
	}
}
