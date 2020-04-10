class WaterDetector
{
private:
	uint8_t _digitalPin;
	uint8_t _analogPin;
	bool _state; // true, water detected

public:
	WaterDetector(uint8_t digitalPin, uint8_t analogPin = -1)
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
		if (_analogPin >= 0)
			return analogRead(_analogPin);
		else
			return -1;
	}
};
