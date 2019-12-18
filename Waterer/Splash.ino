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

const int DETECTORS = 1;

WaterDetector resevoirDetector(5, A2);
WaterDetector detectors[DETECTORS] = {WaterDetector(13, A0)};
Component pumpRelay(2);
Component powerRelay(3);

int sleepTime = 0;

void setup()
{
    Serial.begin(9600);
    pumpRelay.Begin();
    powerRelay.Begin();
    for (int i = 0; i < DETECTORS; i++)
        detectors[i].Begin();
    resevoirDetector.Begin();
}

void loop()
{
    // Turn on power to sensors
    // and check readings
    unsigned long wateringEnd = millis();
    unsigned long waterDuration = 5000;

    unsigned long readEnd = millis();
    unsigned long readDuration = 2000;
    unsigned int readCnt = 0;
    unsigned int moisture = 0;
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
                readEnd = wateringEnd + waterDuration; // read after watering
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
    moisture = moisture / readCnt;

    // make sure the pump relay has been turned off after the allotted time.
    pumpRelay.TurnOff();
    // turn off the power relay
    powerRelay.TurnOff();

    // Sleep for an hour
    delay(3600000);
}
