#include <ESP8266WiFi.h>
#include <CapacitiveSensor.h>
#include <time.h>
#include <vector>
#include <string>

const int TIME_PER_MILLILITRE = 20;

class CapacitiveWaterSensor
{
private:
    uint8_t _analog_pin;
    float _min;
    float _max;
    int _read_time;

public:
    CapacitiveWaterSensor(uint8_t analog_pin)
    {
        _analog_pin = analog_pin;
        _min = 500.f;
        _max = 700.f;
        _read_time = 2000;
    }

    float Read()
    {
        uint32_t read_end;
        float values;
        float reads;

        read_end = millis() + _read_time;
        values = 0.f;
        reads = 0.f;
        while (millis() < read_end)
        {
            values += analogRead(_analog_pin);
            reads += 1.f;
            // delay(100); // TOOD do I need this?
        }

        return values / reads;
    }

    float ReadPercent()
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
    std::vector<uint8_t> _selector_pins;

public:
    AnalogMux(std::vector<uint8_t> selector_pins)
    {
        _selector_pins = selector_pins;
    }

    // selector is an integer value between 0-max_selectors
    void Select(byte selector)
    {
        for (int i = 0; i < 3; i++)
        {
            if (selector & (1 << i))
                digitalWrite(_selector_pins[i], HIGH);
            else
                digitalWrite(_selector_pins[i], HIGH);
        }
    }
};

class VoltageSensor
{
private:
    bool _state;
    
public:
    VoltageSensor(uint8_t _send_pin, uint8_t _recieve_pin)
    {

    }
}

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
    std::vector<CapacitiveWaterSensor *> _sensors;
    std::vector<uint8_t> _sensor_selector_pins;
    AnalogMux &_sensor_mux;
    Reservoir &_reservoir; // this is so that multiple plants can use the same Reservoir
    AnalogMux &_solenoid_mux;
    uint8_t _solenoid_selector_pin;
    Component *_solenoid;
    int _watering_threshold;
    int _water_amount;

    std::vector<float> _pre_watering_moistures;
    std::vector<float> _post_watering_moistures;

public:
    Plant(String name, uint8_t analog_pin, std::vector<uint8_t> sensor_selector_pins, AnalogMux &sensor_mux, Reservoir &reservoir, int watering_threshold, int water_amount, int solenoid_pin, uint8_t solenoid_selector_pin, AnalogMux &solenoid_mux)
        : _reservoir(reservoir), _sensor_mux(sensor_mux), _solenoid_mux(solenoid_mux) // this is how to reference
    {
        _name = name;

        _sensors = std::vector<CapacitiveWaterSensor *>();
        _sensor_selector_pins = sensor_selector_pins;

        for (int i = 0; i < _sensor_selector_pins.size(); i++)
            _sensors.push_back(new CapacitiveWaterSensor(analog_pin));
        _solenoid = new Component(solenoid_pin);

        _watering_threshold = watering_threshold;
        _water_amount = water_amount;

        _pre_watering_moistures = std::vector<float>(_sensor_selector_pins.size());
        _post_watering_moistures = std::vector<float>(_sensor_selector_pins.size());
    }

    ~Plant()
    {
        for (auto sensor : _sensors)
            delete sensor;
        delete _solenoid;
    }

    void Manage()
    {
        _pre_watering_moistures = ReadWaterLevels();
        if (ToDouse())
        {
            Douse();
            _post_watering_moistures = ReadWaterLevels();
        }
        else
        {
            _post_watering_moistures = _pre_watering_moistures;
        }
    }

    bool ToDouse()
    {
        bool to_douse = 0; // 1 get watered

        // determine if watering is needed
        for (int i = 0; i < _pre_watering_moistures.size(); i++)
            to_douse = to_douse && (_pre_watering_moistures[i] < _watering_threshold);

        return to_douse;
    }

    void Douse()
    {
        _solenoid_mux.Select(_solenoid_selector_pin);
        _reservoir.Douse(*_solenoid, 250);
    }

    std::vector<float> ReadWaterLevels()
    {
        std::vector<float> moistures = std::vector<float>();

        for (int i = 0; i < _sensors.size(); i++)
        {
            _sensor_mux.Select(_sensor_selector_pins[i]);
            moistures.push_back(_sensors[i]->ReadPercent());
        }

        return moistures;
    }

    String ToString()
    {
        // TODO this
    }
};

class Reservoir
{
private:
    Component *_pump;
    CapacitiveWaterSensor *_sensor;
    uint8_t _selector_pin;
    AnalogMux &_mux;

    Component *_led;
    bool _flash_led;

    int _min_percent;

public:
    Reservoir(uint8_t pump_pin, uint8_t analog_pin, uint8_t selector_pin, AnalogMux &mux, int min_percent, uint8_t led_pin = -1)
        : _mux(mux)
    {
        _pump = new Component(pump_pin);
        _sensor = new CapacitiveWaterSensor(analog_pin);
        _selector_pin = selector_pin;

        _min_percent = min_percent;

        if (led_pin >= 0)
            _led = new Component(led_pin);
        else
            _led = NULL;

        _flash_led = false;
    }

    ~Reservoir()
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
            _pump->TurnOff();
            delay(500); // let water drain back into reservoir TODO find accurate timeing.
            solenoid.TurnOff();
            // delay(100);
        }
    }

    bool DetectWater()
    {
        _mux.Select(_selector_pin);
        float percent = _sensor->ReadPercent();

        // if its about the minimum level then there is water.
        if (percent > _min_percent)
            return true;

        return false;
    }

    void Status()
    {
        if (!DetectWater())
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

    String ToString()
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

const uint8_t MAIN_RESERVOIR = 14;

// uint32_t _sleepTime = 3600000; // This will change based on how often to check the plant
uint32_t _sleepTime = 300000; // This will change based on how often to check the plant

uint16_t _waterDuration = 5000; // TODO measure how much water the pump can pump in a second.
    // That amount wil be the starting point where I can do 1 seocnd to start the pump
    // plus the time that it will take to equal a certain amount of litres
    // 1 + (wanted mL / flow) <-- this is the duration in seconds.
    // according to my old program, 5000ms is about 250mL
Plant *hugh;
AnalogMux *mux;

// TODO ask the server for infromation first, if nothing is recieved,
// use default values or current set values.

// TODO change the code for reservoir, I can juse use the CapacitiveSensor library
// for it and then I'll need to calibrate the water level expectation.
// this means I will not need the MUX in the reservoir at all.
// Also, as expandability perhaps having mux inside of the plant is determintal,
// or I could have a default value of none so that i do a check if needed.

void setup()
{
    Serial.begin(115200);

    ConnectToWifi();

    std::vector<uint8_t> select_pins = {D0, D1, D2};
    mux = new AnalogMux(select_pins);

    Reservoir main_reserovir = Reservoir(MAIN_RESERVOIR, 0xA0, 0, *mux, 20, D10);

    uint8_t hugh_pins[] = {0xA0};
    hugh = new Plant("Hugh", 0xA0, hugh_pins, main_reserovir, );
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

static const uint8_t D0 = 16;
static const uint8_t D1 = 5;
static const uint8_t D2 = 4;
static const uint8_t D3 = 0;
static const uint8_t D4 = 2;
static const uint8_t D5 = 14;
static const uint8_t D6 = 12;
static const uint8_t D7 = 13;
static const uint8_t D8 = 15;
static const uint8_t D9 = 3;
static const uint8_t D10 = 1;