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
};

const int DETECTORS = 2;

WaterDetector resevoirDetector(8);
WaterDetector detectors[2] = {WaterDetector(7), WaterDetector(13)};
Component relay(7);

void setup()
{
	wd.Begin();
	relay.Begin();
	Serial.begin(1000);
}

void loop()
{
	// Water is not detected
	if (resevoirDetector.Detect())
	{
		bool needsWatering = true;
		for (int i = 0; i < 2; i++)
		{
			needsWatering = (needsWatering && !detectors[i].Detect()); 
		}
		
		if (needsWatering)
		{
			relay.Toggle(1000);
		}	
	}
	
	// Sleep for an hour
	delay(3600000);
}