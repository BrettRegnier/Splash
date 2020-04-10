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
