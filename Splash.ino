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
      _state = false;
      TurnOff();
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
      _state = false;
      digitalWrite(_pin, HIGH);
    }

    void TurnOn()
    {
      _state = true;
      digitalWrite(_pin, LOW);
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
  unsigned long _time = millis();
  unsigned long _waterTime = 0;
  
  unsigned long _read = _time;
  unsigned long _readTime = 1000;
  
  powerRelay.TurnOn();
  delay(900); // allow for the sensors to start detecting water

// New loop
  do 
  {
    bool needsWatering = true;
    for (int i = 0; i < DETECTORS; i++)
    {
      // Check if watering is needed and the analog levels.
      needsWatering = (needsWatering && !detectors[i].Detect());
      Serial.println(detectors[0].Read());
    }

    do 
    {
      if (resevoirDetector.Detect() && needsWatering)
      {
          pumpRelay.TurnOn();
          _time = millis();
          _waterTime = 5000;
      }
      else 
      {
        // TODO could add in a flashing LED and set a global variable such that the resevoir needs water
        pumpRelay.TurnOff();
        _waterTime = 0;
      }
      
    } while (millis() < _time + waterTime);

  } while (millis() < _read + readTime);

  // Old loop, try removing sometime.
  do
  {
    while (millis() < _read + _readTime)
    {
      Serial.println(detectors[0].Read());
    }
    
    // There must be water in the resevoir to proceed
    if (resevoirDetector.Detect())
    {
      bool needsWatering = true;
      for (int i = 0; i < DETECTORS; i++)
      {
        needsWatering = (needsWatering && !detectors[i].Detect());
      }

      if (needsWatering && _waterTime <= 0)
      {
        pumpRelay.TurnOn();
        _time = millis();
        _waterTime = 5000;
      }
    }
    else
    {
      pumpRelay.TurnOff();
      _waterTime = 0;
    }
  } while (millis() < _time + _waterTime);

  // make sure the pump relay has been turned off after the allotted time.
  pumpRelay.TurnOff();
  // turn off the power relay
  powerRelay.TurnOff();

  // Sleep for an hour
  delay(3600000);
}
