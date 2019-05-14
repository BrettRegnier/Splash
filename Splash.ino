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

    void Read(int *data, int readTime)
    {
      // TODO fix this garbage.
      // not returning the pointer correctly and causing the program to crash
      for (int i = 0; i < readTime; i++)
      {
        data[i] = analogRead(_analogPin);
        Serial.println(data[i]);
      }
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

const int DETECTORS = 2;

WaterDetector resevoirDetector(5, A2);
WaterDetector detectors[DETECTORS] = {WaterDetector(12, A1), WaterDetector(13, A0)};
Component pumpRelay(2);
Component powerRelay(3);

int sleepTime = 0;

void setup()
{
  pumpRelay.Begin();
  powerRelay.Begin();
  for (int i = 0; i < DETECTORS; i++)
    detectors[i].Begin();
  resevoirDetector.Begin();
  Serial.begin(9600);
}

void loop()
{
  // Turn on power to sensors
  // and check readings
  unsigned long _time = millis();
  unsigned long _waterTime = 0;  
  
  powerRelay.TurnOn();
  delay(900); // allow for the sensors to start detecting water

  int readTime = 10;
  int data[readTime];
  detectors[0].Read(&data[0], readTime);
  
  do
  {
    // There must be water in the resevoir to proceed
    if (resevoirDetector.Detect())
    {
      bool needsWatering = true;
      for (int i = 0; i < DETECTORS; i++)
        needsWatering = (needsWatering && !detectors[i].Detect());

      if (needsWatering && _waterTime <= 0)
      {
        pumpRelay.TurnOn();
        _waterTime = 5000;
      }
    }
    else
    {
      pumpRelay.TurnOff();
      _waterTime = 0 ;
    }
  } while (millis() < _time + _waterTime);

  //  for (int i = 0; i < readTime; i++)
  //    Serial.println(data[i]);


  // turn off the power relay
  powerRelay.TurnOff();

  // Sleep for an hour
  delay(3600000);
}
