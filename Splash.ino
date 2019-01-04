class WaterDetector
{
  private:
	uint8_t _pin;
	bool _state; // true, water detected

  public:
	WaterDetector(uint8_t pin)
	{
		_pin = pin;
	}

	void Begin()
	{
		_state = false;
		pinMode(_pin, INPUT);
	}

	bool Detect()
	{
		// water detected t = LOW, and t = HIGH water undetected
		uint8_t t = digitalRead(_pin);
		_state = (t == LOW);
		return _state;
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
};

const int DETECTORS = 2;

WaterDetector resevoirDetector(3);
WaterDetector detectors[DETECTORS] = {WaterDetector(12), WaterDetector(13)};
Component relay(2);

void setup()
{
	relay.Begin();
	for (int i = 0; i < DETECTORS; i++)
		detectors[i].Begin();
	resevoirDetector.Begin();
	Serial.begin(3000);
}

void loop()
{
	// There must be water in the resevoir to proceed
	if (resevoirDetector.Detect())
	{
		bool needsWatering = true;
		for (int i = 0; i < DETECTORS; i++)
			needsWatering = (needsWatering && !detectors[i].Detect());

		if (needsWatering)
		{
			relay.Toggle(5000);
		}
	}
	else
	{
		relay.TurnOff();
	}

	// Sleep for an hour
	delay(360000);
}