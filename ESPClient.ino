#include <ESP8266WiFi.h>
#include <time.h>

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

const char *_ssid = "";
const char *_password = "";

const char *_host = "192.168.0.24";
const uint16_t _port = 8777;

const char *_name = "Hugh";

const int DETECTORS = 1;
WaterDetector _resevoirDetector(5);
WaterDetector _detectors[DETECTORS] = {WaterDetector(13, 0xA0)};
Component _pumpRelay(2);
Component _powerRelay(3);

uint8_t _detectType = 0;		// 0 is digital, 1 is analog
uint8_t _analogThreshold = 175; // The amount of moisure that should be in the soil
uint8_t _morningThreshold = 7;  // time to check levels in the morning
uint8_t _eveningThreshold = 20; // time to check the levels in the evening

uint32_t _sleepTime = 3600000; // This will change based on how often to check the plant

uint8_t _tz = -7;

uint16_t _waterDuration = 5000;

void setup()
{
	Serial.begin(115200);

	ConnectToWifi();
	InitTime();
	InitComponents();

	Serial.println(_waterDuration);
}

void loop()
{
	// Get the current hour of the day
	time_t now = time(nullptr);
	struct tm *p_tm = localtime(&now);
	uint8_t hr = p_tm->tm_hour;

	uint8_t toWater = 0; // 0 doesn't get watered | 1 gets watered.
	uint32_t preMoistures[DETECTORS];
	uint32_t postMoistures[DETECTORS];

	// init moistures stride-1 access
	for (int i = 0; i < DETECTORS; i++)
		preMoistures[i] = 0;
	for (int i = 0; i < DETECTORS; i++)
		postMoistures[i] = 0;

	_powerRelay.TurnOn(); // turn on power to the peripherals
	delay(1000);		  // allow for the sensors to start detecting water

	// read the water levels before watering
	ReadWaterLevels(preMoistures);

	// time should be within a certain time period. 7am < time <10pm
	if (hr > _morningThreshold && hr < _eveningThreshold)
	{
		if (_detectType == 0) // detect using digital
			toWater = WateringDigital();
		else if (_detectType == 1)
			toWater = WateringAnalog(preMoistures);

		if (toWater)
		{
			// Water the plant.
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
			ReadWaterLevels(postMoistures);
		}
	}

	Serial.println(toWater);
	int x = 10;
	Serial.println(sizeof(x));
	_powerRelay.TurnOff(); // turn off power to peripherals

	// client -> server code here.

	// Use WiFiClient class to create TCP connections
	Serial.println("Connect to host");
	WiFiClient client;
	if (!client.connect(_host, _port))
	{
		// connection failed, so just try again next loop.
		delay(5000);
		return;
	}

	// Send data to the server in a big byte array
	/* Send:
	** _name = name of plant
	** _detectors = number of detectors
	** preMoistures = moisture before watering
	** toWater = 0 not watered | 1 watered, effects the next send.
	** postMoistures = moisture after watering
	*/
	if (client.connected())
	{
		// for testing
		preMoistures[0] = 200;
		toWater = 1;
		postMoistures[0] = 100;

		int sizeName = 0;
		while (_name[sizeName] != '\0')
			sizeName++;

		int sizeDetectors = 0;
		char *s_detectors = ConvertIntToCharLiteral(DETECTORS, sizeDetectors);

		int sizeToWater = 0;
		char *s_toWater = ConvertIntToCharLiteral(toWater, sizeToWater);

		// Convert data into *char
		int totalSizePreMoist = 0;
		int sizePreMoist[DETECTORS];
		char *s_preMoistures[DETECTORS];
		for (int i = 0; i < DETECTORS; i++)
		{
			s_preMoistures[i] = ConvertIntToCharLiteral(preMoistures[i], sizePreMoist[i]);
			totalSizePreMoist += sizePreMoist[i];
		}

		int totalSizePostMoist = 0;
		int sizePostMoist[DETECTORS];
		char *s_postMoistures[DETECTORS];
		if (toWater)
			for (int i = 0; i < DETECTORS; i++)
			{
				s_postMoistures[i] = ConvertIntToCharLiteral(postMoistures[i], sizePostMoist[i]);
				totalSizePostMoist += sizePostMoist[i];
			}

		// // for testing.
		// Serial.println("preMoistures");
		// for (int i = 0; i < DETECTORS; i++)
		// 	for (int j = 0; j < sizePreMoist[i]; j++)
		// 		Serial.println(s_preMoistures[i][j]);

		int sizeSeperators = 3; // 3 seperators ';'
		if (toWater)
			sizeSeperators = 4; // 4 seperators ';'

		size_t s = sizeof(char) * (sizeName +
								   totalSizePreMoist +
								   sizeToWater +
								   totalSizePostMoist +
								   sizeSeperators);

		int idx = 0;
		char *msg = (char *)malloc(s);

		int i = 0;
		int j = 0;

		for (i = 0; i < sizeName; i++)
			msg[idx + i] = _name[i];
		idx = sizeName;

		msg[idx] = ';'; // seperator
		idx++;

		for (i = 0; i < sizeDetectors; i++)
			msg[idx + i] = s_detectors[i];
		idx += sizeDetectors;

		msg[idx] = ';'; // seperator
		idx++;

		for (i = 0; i < sizeToWater; i++)
			msg[idx + i] = s_toWater[i];
		idx += sizeToWater;

		msg[idx] = ';'; // seperator
		idx++;

		for (i = 0; i < DETECTORS; i++)
		{
			for (j = 0; j < sizePreMoist[i]; j++)
				msg[idx + j] = s_preMoistures[i][j];
			idx += sizePreMoist[i];

			msg[idx] = ';'; // seperator
			idx++;
		}

		if (toWater)
			for (i = 0; i < DETECTORS; i++)
			{
				for (j = 0; j < sizePostMoist[i]; j++)
					msg[idx + j] = s_postMoistures[i][j];
				idx += sizePostMoist[i];

				msg[idx] = ';'; // seperator
				idx++;
			}

		// Serial.println(msg);

		client.write(msg);
	}
	
	// TODO take server arguments.

	// Close the connection
	client.stop();

	// sleep for n time
	delay(_sleepTime);
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

void InitComponents()
{
	_pumpRelay.Begin();
	_powerRelay.Begin();
	for (int i = 0; i < DETECTORS; i++)
		_detectors[i].Begin();
	_resevoirDetector.Begin();
}

void PrintConnection(const char *host, uint16_t port)
{
	Serial.print("connecting to ");
	Serial.print(host);
	Serial.print(':');
	Serial.println(port);
}

void PrintTime()
{
	time_t now = time(nullptr);
	struct tm *p_tm = localtime(&now);
	Serial.print(p_tm->tm_mday);
	Serial.print("/");
	Serial.print(p_tm->tm_mon + 1);
	Serial.print("/");
	Serial.print(p_tm->tm_year + 1900);

	Serial.print(" ");

	Serial.print(p_tm->tm_hour);
	Serial.print(":");
	Serial.print(p_tm->tm_min);
	Serial.print(":");
	Serial.println(p_tm->tm_sec);
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

char *ConvertIntToCharLiteral(int x, int &figs)
{
	int tmp = x;
	figs = 0;

	// get the number of figures in the int.
	do
	{
		tmp = (tmp / 10);
		figs++;
	} while (tmp > 0);

	tmp = x;
	char *st = (char *)malloc(sizeof(char) * figs);
	// st[figs];
	for (int i = 0; i < figs; i++)
	{
		// convert a single value into a char literal.
		st[(figs - 1) - i] = '0' + (tmp % 10); // store in reverse order.
		tmp = tmp / 10;
	}

	return st;
}

// static const uint8_t D0   = 16;
// static const uint8_t D1   = 5;
// static const uint8_t D2   = 4;
// static const uint8_t D3   = 0;
// static const uint8_t D4   = 2;
// static const uint8_t D5   = 14;
// static const uint8_t D6   = 12;
// static const uint8_t D7   = 13;
// static const uint8_t D8   = 15;
// static const uint8_t D9   = 3;
// static const uint8_t D10  = 1;