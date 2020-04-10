class CapacitiveSensor
{
private:
    uint8_t _analogPin;
    float _min;
    float _max;
    bool _state;

public:
    CapacitiveSensor(uint8_t analogPin, float min, float max)
    {
        _analogPin = analogPin;
        _min = min;
        _max = max;
    }

    void Begin()
    {
        _state = false;
    }

    int Read()
    {
        int val = analogRead(_analogPin);
        return val;
    }

    int ReadPercent()
    {
        float val = Read();

        // normalize
        if (val > _max)
            val = _max;
        else if (val < _min)
            val = _min;

        val -= _max;
        if (val < 0)
            val *= -1;

        val /= (_max - _min);
        val *= 100;
        return val;
    }
};