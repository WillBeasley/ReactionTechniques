
#include "GPIOManager.h"


GPIOManager::GPIOManager(){
}

void GPIOManager::Init(MicroBit * uBit){
    mpuBit = uBit;
    // Set the comms buffer to set Port A and Port B to separate Control.
    SendCommand(CommandCodes::SeparateBanks);
    // Set Port A to Outputs
    SendCommand(CommandCodes::PortAOutput);
    // Set Port B to Outputs
    SendCommand(CommandCodes::PortBInput);
    // Set Port A and Port B to Low
    SendCommand(CommandCodes::PortALow);
    // Enable pullups on the GPIO
    SendCommand(CommandCodes::PortBPullups);
    SendCommand(CommandCodes::PortBPolarity);
    SendCommand(CommandCodes::EnablePortBInterrupt);
    SendCommand(CommandCodes::DisablePortAInterrupt);
    
    ReadPortB();
}

void GPIOManager::digitalWrite(int pin, bool val){
    // Pins 0-7 are on port A
    // Pins 8-15 are on port B
    
    // Determine wich bit in the mask needs to be set
    char tempMask = 1 << pin;


    char currentMask = readRegister(0x09);



    if (val){
        // OR the current mask with the new pin
        currentMask |= tempMask;


    }else{
        currentMask &= ~tempMask;

    }
    
        // Write the mask to the output
        CommsBuffer.Data[0] = 0x09;
        CommsBuffer.Data[1] = currentMask;
        mpuBit->i2c.write(0x40, CommsBuffer.Data, 2);
        
        // CommsBuffer.Data[0] = 0x19;
        // CommsBuffer.Data[1] = Mask.Data[1];
        // mpuBit->i2c.write(0x40, CommsBuffer.Data, 2);
}

bool GPIOManager::digitalRead(int pin){
    
    // Range 0-7 Will always read port B
    return (ReadPortB() >> pin) & 0x01;
}


void GPIOManager::SendCommand(uint16_t Command){
    CommsBuffer.Value = Command;
    mpuBit->i2c.write(0x40, CommsBuffer.Data, 2);
}

char GPIOManager::ReadPortB(){

return readRegister(0x19);
    // char port = 0x19;
    
    // // Select port B
    // mpuBit->i2c.write(0x40, &port, 1);
    
    // char ReadByte = 0;
    // // Read the byte
    // mpuBit->i2c.read(0x40, &ReadByte, 1);

    // return ReadByte;

}

char GPIOManager::readRegister(char addr){

      
    // Select address
    mpuBit->i2c.write(0x40, &addr, 1);
    
    char ReadByte = 0;
    // Read the byte
    mpuBit->i2c.read(0x40, &ReadByte, 1);

    return ReadByte;

}

bool GPIOManager::isBitSet(char data, int bit){
    data >>= bit;
    return data & 0x01;


}