// This #include statement was automatically added by the Particle IDE.
#include <SparkFun_SCD30_Arduino_Library.h>
// This #include statement was automatically added by the Particle IDE.
#include <DHT22Gen3_RK.h>
// This #include statement was automatically added by the Particle IDE.
#include <MCP23017-RK.h>

#include <Particle.h>
#include "outputs.h"

PRODUCT_ID(11095);
PRODUCT_VERSION(1);

#define BLOWER_PIN 2
#define FAN1_PIN 0
#define FAN2_PIN 1
#define IRRIGATION_PIN1 6
#define IRRIGATION_PIN2 7

SCD30 airSensor;
MCP23017 gpio(Wire, 0);
float seconds;
int digitalwritepin;
int pulse_valve_length;
float co2;
float hum_in;
float hum_ext;
float temp_in;
float temp_ext;
int irrigation_pulse_len = 15; // by default pulse the valve for 15ms to open or close.
unsigned long old_time = millis();
// How often to check the co2 in milliseconds
const unsigned long CHECK_INTERVAL = 300000;
unsigned long lastCheck = 0;
// The two parameters are any available GPIO pins. They will be used as output but the signals aren't
// particularly important for DHT11 and DHT22 sensors. They do need to be valid pins, however.
DHT22Gen3 dht(D7, D8);
void sampleCallback(DHTSample sample);

Output::Output() : pin(0){};
Output::Output(byte attachPin) : pin(attachPin){};

void Output::setup()
{
    gpio.pinMode(pin, OUTPUT);
    state = LOW;
};
void Output::loop()
{
    if (state == HIGH && millis() >= timeout)
    {
        off();
    }
};
void Output::on()
{
    state = HIGH;
    gpio.digitalWrite(pin, HIGH);
}
void Output::off()
{
    state = LOW;
    gpio.digitalWrite(pin, LOW);
}
void Output::run(unsigned int run_time_ms)
{
    on();
    timeout = millis() + run_time_ms;
};

LatchingValve::LatchingValve(byte attachPin1, byte attachPin2, int attachPulseTime) : pin1(attachPin1), pin2(attachPin2), pulse_time_ms(attachPulseTime){};

void LatchingValve::on()
{
    gpio.digitalWrite(pin1, HIGH);
    delay(pulse_time_ms);
    gpio.digitalWrite(pin1, LOW);
    state = HIGH;
}
void LatchingValve::off()
{
    gpio.digitalWrite(pin2, HIGH);
    delay(pulse_time_ms);
    gpio.digitalWrite(pin2, LOW);
    state = LOW;
}

Output blower(BLOWER_PIN);
Output fan1(FAN1_PIN);
Output fan2(FAN2_PIN);
LatchingValve irrigation(IRRIGATION_PIN1, IRRIGATION_PIN2, 15);

void setup()
{
    //Functions for manual control of the outputs
    Particle.function("Open_seconds", open_seconds);
    Particle.function("Close_seconds", close_seconds);
    Particle.function("pulse_pin", pulse_pin);
    Particle.function("fans_seconds", fans_seconds);
    Particle.function("blower_seconds", blower_seconds);
    Particle.function("sprinkler_seconds", sprinkler_seconds);
    Particle.function("irrigate_seconds", irrigate_seconds);
    Particle.function("irrigation_pulse_length", irrigation_pulse_length); //set the length of a pulse used to open or close the valve
    Serial.begin(9600);                                                    // for the mcp23017
    delay(5000);
    gpio.begin(); //gpio expander instance (mcp23017)
    Wire.begin();
    if (airSensor.begin() == false)
    {
        Serial.println("Air sensor not detected. Please check wiring. Freezing...");
        //Particle.publish("frozen because co2 sensor is not connected");
        while (1)
            ;
    }
    dht.setup();
    blower.setup();
    fan1.setup();
    fan2.setup();
    irrigation.setup();
}
void loop()
{
    blower.loop();
    fan1.loop();
    fan2.loop();
    irrigation.loop();

    dht.loop();
    if (millis() - lastCheck >= CHECK_INTERVAL)
    {
        lastCheck = millis();
        co2 = airSensor.getCO2();
        delay(1000);
        dht.getSample(A3, [](DHTSample sample1) {
            dht.getSample(A4, [sample1](DHTSample sample2) {
                if (sample1.isSuccess() && sample2.isSuccess())
                {
                    hum_in = sample1.getHumidity();
                    temp_in = sample1.getTempC();
                    hum_ext = sample2.getHumidity();
                    temp_ext = sample2.getTempC();
                }
                else
                {
                    Log.info("sample is not valid");
                }
            });
        });
        send_data();
    }
}
int send_data()
{
    if (Particle.connected())
    {
        Particle.publish("co2", String(co2));
        delay(1000);
        Particle.publish("climate_reading_in", String(temp_in) + "," + String(hum_in), PRIVATE);
        delay(1000);
        Particle.publish("climate_reading_ext", String(temp_ext) + "," + String(hum_ext), PRIVATE);
        char buf[256];
        snprintf(buf, sizeof(buf), "{\"temp_in\":%.1f,\"hum_in\":%1.f,\"temp_ext\":%.1f,\"hum_ext\":%1.f,\"co2\":%1.f}", temp_in, hum_in, temp_ext, hum_ext, co2);
        Particle.publish("send_data", buf, PRIVATE);
    }
    else
    {
        //log an error if you want.
    }
    return 0;
}

/* PARTICLE FUNCTIONS */

int blower_seconds(String seconds)
{
    int ms = atoi(seconds) * 1000;
    blower.run(ms);
    return 0;
}

int fans_seconds(String seconds)
{
    int ms = atoi(seconds) * 1000;
    fan1.run(ms);
    fan2.run(ms);
    return 0;
}

/*
int irrigate_seconds(String seconds)
{
    int ms = atoi(seconds) * 1000;
    irrigation.run(ms);
    return 0;
}
*/
int irrigate_seconds(String command)
{
    seconds = atof(command)*1000;
	gpio.pinMode(6, OUTPUT);
	gpio.digitalWrite(6, HIGH);//pulse opens the valve (2-a wire pulsed high for 15ms)
    delay(irrigation_pulse_len);
    gpio.digitalWrite(6, LOW);
    delay(seconds);//technically this is ms now.
    gpio.pinMode(7, OUTPUT);
    gpio.digitalWrite(7, HIGH);//pulse closes the valve (2-b wire pulsed high for 15ms)
    delay(irrigation_pulse_len);
    gpio.digitalWrite(7, LOW);
}

int sprinkler_seconds(String command)
{
    seconds = atof(command) * 1000;
    gpio.pinMode(4, OUTPUT);
    gpio.digitalWrite(4, HIGH); //pulse opens the valve (1-a wire pulsed high for 15ms)
    delay(15);
    gpio.digitalWrite(4, LOW);
    delay(seconds); //technically this is ms now.
    gpio.pinMode(5, OUTPUT);
    gpio.digitalWrite(5, HIGH); //pulse closes the valve (1-b wire pulsed high for 15ms)
    delay(15);
    gpio.digitalWrite(5, LOW);
    return 0;
}

int pulse_pin(String command)
{
    digitalwritepin = atoi(command);
    gpio.pinMode(digitalwritepin, OUTPUT);
    gpio.digitalWrite(digitalwritepin, HIGH);
    delay(15);
    gpio.digitalWrite(digitalwritepin, LOW);
    return 0;
}
int open_seconds(String command)
{
    seconds = atof(command) * 1000;
    gpio.pinMode(9, OUTPUT);
    gpio.digitalWrite(9, HIGH);
    delay(seconds);
    gpio.digitalWrite(9, LOW);
    return 0;
}
int close_seconds(String command)
{
    seconds = atof(command) * 1000;
    gpio.pinMode(8, OUTPUT);
    gpio.digitalWrite(8, HIGH);
    delay(seconds);
    gpio.digitalWrite(8, LOW);
    return 0;
}
int irrigation_pulse_length(String command)
{
    irrigation.pulse_time_ms = atoi(command);
    return 0;
}
