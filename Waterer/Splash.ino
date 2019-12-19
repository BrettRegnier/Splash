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

WaterDetector _resevoirDetector(5, A2);
WaterDetector _detectors[DETECTORS] = {WaterDetector(13, A0)};
Component _pumpRelay(2);
Component _powerRelay(3);

uint8_t _detectType = 0;        // 0 is digital, 1 is analog
uint8_t _analogThreshold = 175; // The amount of moisure that should be in the soil
uint8_t _morningThreshold = 7;  // time to check levels in the morning
uint8_t _eveningThreshold = 20; // time to check the levels in the evening

uint32_t _sleepTime = 3600000;

void setup()
{
	Serial.begin(9600);
	_pumpRelay.Begin();
	_powerRelay.Begin();
	for (int i = 0; i < DETECTORS; i++)
		_detectors[i].Begin();
	_resevoirDetector.Begin();
}

void loop()
{
	uint8_t toWater = 0; // 0 doesn't get watered | 1 gets watered.
	uint32_t preMoistures[DETECTORS];
	uint32_t postMoistures[DETECTORS];

	// init moistures stride-1 access
	for (int i = 0; i < DETECTORS; i++)
		preMoistures[i] = 0;
	for (int i = 0; i < DETECTORS; i++)
		postMoistures[i] = 0;

	_powerRelay.TurnOn(); // turn on power to the peripherals
	delay(1000);          // allow for the sensors to start detecting water

	// read the water levels before watering
	ReadWaterLevels(preMoistures);

	int time = 10; // replace this with real time.
	if (time > _morningThreshold && time < _eveningThreshold)
	{
		if (_detectType == 0) // detect using digital
			toWater = WateringDigital();
		else if (_detectType == 1)
			toWater = WateringAnalog(preMoistures);

		if (toWater)
		{
			WaterThePlant();
			ReadWaterLevels(postMoistures);
		}
	}

	Serial.println(toWater);
	_powerRelay.TurnOff(); // turn off power to peripherals

	// client -> server code here.

	// sleep for n time
	delay(_sleepTime);
}

// Reads the current water levels and averages the values
void ReadWaterLevels(uint32_t *moistures)
{
	// init reads array
	uint16_t reads[DETECTORS];
	for (int i = 0; i < DETECTORS; i++)
		reads[i] = 0;

	uint32_t readEnd = millis() + 2000;
	while (millis() < readEnd)
	{
		for (int i = 0; i < DETECTORS; i++)
		{
			moistures[i] += _detectors[i].Read();
			reads[i]++;
		}
	}

	for (int i = 0; i < DETECTORS; i++)
		moistures[i] = moistures[i] / reads[i];
}

uint8_t WateringDigital()
{
	bool needsWatering = true;
	for (int i = 0; i < DETECTORS; i++)
		needsWatering = needsWatering && !_detectors[i].Detect();

	return needsWatering;
}

uint8_t WateringAnalog(uint32_t *moistures)
{
	bool needsWatering = true;
	for (int i = 0; i < DETECTORS; i++)
		needsWatering = needsWatering && (moistures[i] < _analogThreshold);

	return needsWatering;
}

void WaterThePlant()
{
	uint16_t duration = millis() + 5000; // should pump about 250mL

	_pumpRelay.TurnOn();
	delay(100); // this for some reason allows for the sensors to detect correctly...?
	while (millis() < duration)
	{
		if (!_resevoirDetector.Detect())
		{
			_pumpRelay.TurnOff();
			// TODO add in LED blinking variable here...
			break; // can't water, break out of loop
		}
	}
	_pumpRelay.TurnOff();
}