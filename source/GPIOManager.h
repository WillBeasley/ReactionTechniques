#ifndef __GPIOMANAGER__
#define __GPIOMANAGER__
#include "MicroBit.h"



union BuffStruct{
    uint16_t Value;
    char Data[2];
};

class GPIOManager{
    public:
    GPIOManager();
    void Init(MicroBit * uBit);

    void digitalWrite(int pin, bool val);

    bool digitalRead(int pin);

    void pinMode(int pin);

    char ReadPortB();

    char readRegister(char addr);

    bool isBitSet(char data, int bit);

    private:
    BuffStruct CommsBuffer;
    MicroBit * mpuBit;

    void SendCommand(uint16_t Command);



};



enum CommandCodes : uint16_t{
    SeparateBanks = 0xE20A,
    PortAOutput = 0x0000,
    PortBInput = 0xFF10,
    PortALow = 0x0009,
    PortAHigh = 0xFF09,
    PortBPullups = 0xFF16,
    PortBPolarity = 0xFF11,
    DisablePortAInterrupt = 0x0002,
    EnablePortBInterrupt = 0xFF12
};







#endif