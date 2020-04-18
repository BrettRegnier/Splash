#include <ESP8266WiFi.h>
#include <time.h>
#include <vector>
#include <string>

const int TIME_PER_MILLILITRE = 20;

class CapacitiveSensor
{
private:
    AnalogMux &_mux;
    float _min;
    float _max;
    int _read_time;

public:
    CapacitiveSensor(AnalogMux &mux) : _mux(mux)
    {
        _min = 500.f;
        _max = 700.f;
        _read_time = 2000;
    }

    int Read()
    {
        uint32_t read_end;
        float values;
        float reads;

        read_end = millis() + _read_time;
        values = 0.f;
        reads = 0.f;
        while (millis() < read_end)
        {
            values += analogRead(_analogPin);
            reads += 1.f;
            // delay(100); // TOOD do I need this?
        }

        return values / reads;
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

class AnalogMux
{
private:
    uint8_t _read_pin;
    std::vector<uint8_t> _activate_pins;

public:
    AnalogMux(uint8_t read_pin, std::vector<uint8_t> activate_pins)
    {
        _read_pin = read_pin;
        _activate_pins = activate_pins;
    }

    // selector is an integer value between 0-max_selectors
    float AnalogRead(uint8_t selector)
    {
        // TODO finish this

        return analogRead(_read_pin)
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
    std::vector<CapacitiveSensor *> _sensors;
    Resevoir &_resevoir; // this is so that multiple plants can use the same resevoir
    Component *_solenoid;
    int _watering_threshold;
    int _water_amount;

public:
    Plant(String name, int num_sensors, uint8_t sensor_pins[], Resevoir &reservoir, int solenoid_pin, int watering_threshold, int water_amount)
        : _resevoir(reservoir) // this is how to reference
    {
        _name = name;

        _sensors = std::vector<CapacitiveSensor *>(num_sensors);
        for (int i = 0; i < num_sensors; i++)
            _sensors[i] = new CapacitiveSensor(sensor_pins[i]);
        _solenoid = new Component(solenoid_pin);

        _watering_threshold = watering_threshold;
        _water_amount = water_amount;
    }

    ~Plant()
    {
        for (auto sensor : _sensors)
            delete sensor;
        delete _solenoid;
    }

    bool ToDouse()
    {
        bool to_douse = 0; // 1 get watered
        std::vector<uint32_t> prewatering_moistures = std::vector<uint32_t>();
        std::vector<uint32_t> postwatering_moistures = std::vector<uint32_t>();

        prewatering_moistures = ReadWaterLevels();

        // determine if watering is needed
        for (int i = 0; i < prewatering_moistures.size(); i++)
            to_douse = to_douse && (prewatering_moistures[i] < _watering_threshold);

        if (!to_douse)
            // if the plant doesn't need to be watered then they are equal
            postwatering_moistures = prewatering_moistures;

        return to_douse;
    }

    std::vector<uint32_t> ReadWaterLevels()
    {
        std::vector<uint32_t> moistures = std::vector<uint32_t>();

        for (int i = 0; i < _sensors.size(); i++)
            moistures.push_back(_sensors[i]->ReadPercent());

        return moistures;
    }

    String ToString()
    {
        // TODO this
    }
};

class Resevoir
{
private:
    Component *_pump;
    CapacitiveSensor *_sensor;

    Component *_led;
    bool _flash_led;

    int _min_percent;

    bool DetectWater()
    {
        float percent = _sensor->ReadPercent();

        // if its about the minimum level then there is water.
        if (percent > _min_percent)
            _flash_led = false;
        return true;

        _flash_led = true;
        return false;
    }

public:
    Resevoir(uint8_t pump_pin, CapacitiveSensor sensor_pin, int min_percent, uint8_t led_pin = -1)
    {
        _pump = new Component(pump_pin);
        _sensor = new CapacitiveSensor(sensor_pin);

        _min_percent = min_percent;

        if (led_pin >= 0)
            _led = new Component(led_pin);
        else
            _led = NULL;

        _flash_led = false;
    }

    ~Resevoir()
    {
        delete _pump;
        delete _sensor;
        delete _led;
    }

    // solenoid is a valve,
    // amount is in mL
    bool Douse(Component &solenoid, int amount)
    {
        if (DetectWater())
        {
            // water the plant
            int time_end = millis() + (TIME_PER_MILLILITRE * amount);
            solenoid.TurnOn();
            _pump->TurnOn();
            while (millis() < time_end)
                if (!DetectWater())
                    break;
            // delay(100);
        }
    }

    void Status()
    {
        if (_flash_led)
            if (_led != NULL)
                _led->Toggle();
    }

    String ToString()
    {
    }
};

class PlantWaterer
{
private:
    std::vector<Plant &> _plants;

public:
    PlantWaterer()
    {
        _plants = std::vector<Plant &>();
    }

    ~PlantWaterer()
    {
        for (int i = 0; i < _plants.size(); i++)
            delete &_plants[i];
    }

    void RegisterPlant(Plant &plant)
    {
        _plants.push_back(plant);
    }

    void Manage()
    {
    }
};

const String _ssid = "";
const String _password = "";

const String _host = "192.168.0.39";
const uint16_t _port = 8777;

uint8_t _min_threshhold = 30;    // percent in which we water
uint8_t _morning_threshold = 7;  // time to check levels in the morning
uint8_t _evening_threshold = 20; // time to check the levels in the evening
int8_t _tz = -7;                 // timezone

// uint32_t _sleepTime = 3600000; // This will change based on how often to check the plant
uint32_t _sleepTime = 300000; // This will change based on how often to check the plant

uint16_t _waterDuration = 5000; // TODO measure how much water the pump can pump in a second.
    // That amount wil be the starting point where I can do 1 seocnd to start the pump
    // plus the time that it will take to equal a certain amount of litres
    // 1 + (wanted mL / flow) <-- this is the duration in seconds.
    // according to my old program, 5000ms is about 250mL
Plant *hugh;

// TODO ask the server for infromation first, if nothing is recieved,
// use default values or current set values.

void setup()
{
    Serial.begin(115200);

    ConnectToWifi();

    Resevoir main_reserovir = Resevoir(14, )

        uint8_t hugh_pins[] = {0xA0};
    hugh = new Plant("Hugh", 1, hugh_pins, );
}

void loop()
{
    time_t now = time(nullptr);
    struct tm *p_tm = localtime(&now);
    uint8_t hr = p_tm->tm_hour;

    // if (hr > _morning_threshold && hr < _evening_threshold)
    if (true)
    {
        // do the watering checks of all of the plants

        // Send to the server
    }

    // sleep for a certain amount of time
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
