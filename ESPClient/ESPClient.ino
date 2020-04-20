#include <ESP8266WiFi.h>
#include <CapacitiveSensor.h>
#include <time.h>
#include <vector>
#include <string>

/* TODO move this into a documentation file.
// For a message it is of format x;y;j;k
// int type - 0=plant, 1=reservoir, 2=...
// type: plant
//  String name - name of plant
//  uint8_t to water - if it was watered or not
//  uint8_t detectors - number of detectors
//  vector<uint16_t> pre moistures - moisture of each detector before watering
//  vector<uint16_t> post moistures - moisture of each detector after watering
//
// type: reservoir:
//  String description - which reservoir this is 
//  bool status - if there is water or not.
*/

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

// To trick this thing you need to make the send pin the resistor outputting from a pin
// one of the metal bars should be hooked into ground,
// the recieve pin should then be another one of the rods
// send pin, rod, and recieve pin should all be hooked into a single rail.
// see diagram.
class VoltageSensor
{
private:
    CapacitiveSensor *_sensor;

public:
    VoltageSensor(uint8_t send_pin, uint8_t recieve_pin)
    {
        _sensor = new CapacitiveSensor(send_pin, recieve_pin);
        _sensor->set_CS_AutocaL_Millis(0xFFFFFFFF);
    }

    ~VoltageSensor()
    {
        delete _sensor;
    }

    bool Detect()
    {
        long total = 0;
        int reads = 20;
        for (int i = 0; i < reads; i++)
        {
            long v = _sensor->capacitiveSensor(20);
            total += v;
            if (v > 15)
                _sensor->reset_CS_AutoCal();
        }

        if (total / reads >= 4)
            return true;
        
        return false;
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

// Could have a digitalMux to have more solenoids.
class Plant
{
private:
    String _name;
    std::vector<CapacitiveWaterSensor *> _sensors;
    std::vector<uint8_t> _sensor_selector_pins;
    AnalogMux &_sensor_mux;
    Reservoir &_reservoir; // this is so that multiple plants can use the same Reservoir
    // AnalogMux &_solenoid_mux;
    // uint8_t _solenoid_selector_pin;
    Component *_solenoid;
    uint8_t _watering_threshold;
    uint16_t _water_amount;

    std::vector<float> _pre_watering_moistures;
    std::vector<float> _post_watering_moistures;

    bool _to_water;

public:
    Plant(String name, uint8_t analog_pin, uint8_t sensor_count, std::vector<uint8_t> sensor_selector_pins, AnalogMux &sensor_mux, Reservoir &reservoir, uint8_t watering_threshold, uint16_t water_amount, uint8_t solenoid_pin)
        : _reservoir(reservoir), _sensor_mux(sensor_mux) // this is how to reference
    {
        _name = name;

        _sensors = std::vector<CapacitiveWaterSensor *>();
        _sensor_selector_pins = sensor_selector_pins;

        for (int i = 0; i < sensor_count; i++)
            _sensors.push_back(new CapacitiveWaterSensor(analog_pin));
        _solenoid = new Component(solenoid_pin);

        _watering_threshold = watering_threshold;
        _water_amount = water_amount;

        _pre_watering_moistures = std::vector<float>(_sensor_selector_pins.size());
        _post_watering_moistures = std::vector<float>(_sensor_selector_pins.size());

        _to_water = false;
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
        if (ToWater())
        {
            Douse();
            _post_watering_moistures = ReadWaterLevels();
        }
        else
        {
            _post_watering_moistures = _pre_watering_moistures;
        }
    }

    bool ToWater()
    {
        _to_water = 0; // 1 get watered

        // determine if watering is needed
        for (int i = 0; i < _pre_watering_moistures.size(); i++)
            _to_water = _to_water && (_pre_watering_moistures[i] < _watering_threshold);

        return _to_water;
    }

    void Douse()
    {
        _solenoid->TurnOn();
        _reservoir.Douse(250);
        _solenoid->TurnOff();
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

    String Message()
    {
        String msg = "";
        msg.concat(0);
        msg.concat(";");

        msg.concat(_name);
        msg.concat(";");

        msg.concat(_to_water);
        msg.concat(";");

        uint8_t sensors_c = _sensors.size();
        msg.concat(sensors_c);
        msg.concat(";");

        for (int i = 0; i < sensors_c; i++)
        {
            msg.concat(_pre_watering_moistures[i]);
            msg.concat(";");
        }

        for (int i = 0; i < sensors_c; i++)
        {
            msg.concat(_post_watering_moistures[i]);
            msg.concat(";");
        }

        return msg;
    }
};

class Reservoir
{
private:
    String _description;
    Component *_pump;
    VoltageSensor *_sensor;

    Component *_led;
    bool _flash_led;

    int _min_percent;

    bool _status;

public:
    Reservoir(String description, uint8_t pump_pin, uint8_t send_pin, uint8_t recieve_pin, int min_percent, uint8_t led_pin = -1)
    {
        _description = description;
        _pump = new Component(pump_pin);
        _sensor = new VoltageSensor(send_pin, recieve_pin);

        _min_percent = min_percent;

        if (led_pin >= 0)
            _led = new Component(led_pin);
        else
            _led = NULL;

        _flash_led = false;
        _status = false;
    }

    ~Reservoir()
    {
        delete _pump;
        delete _sensor;
        delete _led;
    }

    // solenoid is a valve,
    // amount is in mL
    bool Douse(int amount)
    {
        if (DetectWater())
        {
            // water the plant
            int time_end = millis() + (TIME_PER_MILLILITRE * amount);
            _pump->TurnOn();
            while (millis() < time_end)
                if (!DetectWater())
                    break;
            _pump->TurnOff();
            delay(500); // let water drain back into reservoir TODO find accurate timing.
        }
    }

    bool DetectWater()
    {
        _status = _sensor->Detect();
        return _status;
    }

    void Status()
    {
        if (!DetectWater())
            if (_led != NULL)
                _led->Toggle();
    }

    String Message()
    {
        String msg = "";
        msg.concat(1);
        msg.concat(";");

        msg.concat(_status);

        return msg;
    }
};

class Manager
{
private:
    std::vector<Plant &> _plants;
    std::vector<Reservoir &> _reservoirs;

public:
    Manager()
    {
        _plants = std::vector<Plant &>();
    }

    ~Manager()
    {
        for (int i = 0; i < _plants.size(); i++)
            delete &_plants[i];
    }

    void RegisterPlant(Plant &plant)
    {
        _plants.push_back(plant);
    }

    void RegisterReservoir(Reservoir &reservoir)
    {
        _reservoirs.push_back(reservoir);
    }

    void Manage()
    {
        for (auto plant : _plants)
            plant.Manage();
    }

    void ReservoirStatuses()
    {
        for (auto reservoir : _reservoirs)
            reservoir.Status();
    }

    std::vector<String> Messages()
    {
        std::vector<String> msg = std::vector<String>();
        for (auto plant : _plants)
            msg.push_back(plant.Message());

        for (auto reservoir : _reservoirs)
            msg.push_back(reservoir.Message());

        return msg;
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

uint32_t _prev_time_check = 0;
uint32_t _prev_time = 0;
// uint32_t _sleepTime = 3600000; // This will change based on how often to check the plant
unsigned long _sleep_time = 300000; // 5min This will change based on how often to check the plant

Manager _manager = Manager();

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

    // initial set the time
    InitTime();

    String name = "Hugh";
    uint8_t analog_pin = 0xA0;

    // if we had more than one sensor then we'd need a selector for each.
    uint8_t sensors_c = 1;
    std::vector<uint8_t> hugh_select_pins = {D0};

    std::vector<uint8_t> select_pins = {D0, D1, D2};
    AnalogMux mux = AnalogMux(select_pins);

    String description = "Main";
    Reservoir main_reserovir = Reservoir(description, D3, D4, D5, 20, D10);

    uint8_t watering_threshold = 25; // 25%
    uint16_t water_amount = 250;     // 250mL
    uint8_t solenoid_pin = D6;

    Plant hugh = Plant(name, analog_pin, sensors_c, hugh_select_pins, mux, main_reserovir, watering_threshold, water_amount, solenoid_pin);

    // register reservoirs
    _manager.RegisterReservoir(main_reserovir);

    // register plants
    _manager.RegisterPlant(hugh);
}

void loop()
{

    long curr_time = millis();
    if (curr_time > _prev_time + _sleep_time)
    {
        // check the time every 24 hrs, just to make sure it doesn't get out of sync.
        if (curr_time > _prev_time_check + (unsigned long)86400000)
            InitTime();

        uint8_t hr = CheckTime();

        _prev_time = curr_time;
        // TODO remove
        hr = 10;
        if (hr > _morning_threshold && hr < _evening_threshold)
        {
            // do the watering checks of all of the plants
            _manager.Manage();

            // connext to host
            Serial.println("Connect to host");
            WiFiClient client;
            uint8_t attempts = 0;
            while (!client.connect(_host, _port))
            {
                Serial.print("Attempting to connect to host ");
                Serial.println(9 - attempts);
                if (attempts++ > 9)
                    return; // failed to connect not sending message.
                delay(1000);
            }

            // Send to the server
            if (client.connected())
            {
                std::vector<String> msgs = _manager.Messages();

                for (int i = 0; i < msgs.size(); i++)
                {
                    String msg = msgs[i];
                    Serial.println(msg);
                    Serial.println("Sending to host");

                    client.write(msg.c_str());
                }
            }

            // TODO this part.
            // recieve message and parse
            uint32_t timeout = millis() + 10000;
            bool available = 1;
            while (client.available() == 0)
            {
                if (millis() > timeout)
                {
                    Serial.println("Client Timeout");
                    available = 0;
                    break;
                }
                delay(100);
            }

            if (available)
            {
                Serial.println("Recieving from server");
                while (client.available())
                {
                    char ch = static_cast<char>(client.read());
                    Serial.println(ch);
                }
            }

            // close the connection
            client.stop();
        }
    }
    // sleep for a certain amount of time
    delay(1000);
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

uint8_t CheckTime()
{
    time_t now = time(nullptr);
    struct tm *p_tm = localtime(&now);
    uint8_t hr = p_tm->tm_hour;

    return hr;
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