class Output
{
    const byte pin;
public:
    int state;
    unsigned long timeout;
    Output();
    Output(byte attachPin);
    void setup();
    void loop();
    virtual void on();
    virtual void off();
    void run(unsigned int run_time_ms);
};

class LatchingValve : public Output
{
    const byte pin1;
    const byte pin2;
public:
    int pulse_time_ms;
    int state;
    unsigned long timeout;
    virtual void on();
    virtual void off();
    LatchingValve(byte attachPin1, byte attachPin2, int attachPulseTime);
};
