/*
The MIT License (MIT)

Copyright (c) 2016 British Broadcasting Corporation.
This software is provided by Lancaster University by arrangement with the BBC.

Permission is hereby granted, free of charge, to any person obtaining a
copy of this software and associated documentation files (the "Software"),
to deal in the Software without restriction, including without limitation
the rights to use, copy, modify, merge, publish, distribute, sublicense,
and/or sell copies of the Software, and to permit persons to whom the
Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
DEALINGS IN THE SOFTWARE.
*/
#include "MicroBit.h"
#include "HighScoreManager.h"
#include "GPIOManager.h"
MicroBit uBit;


HighScoreManager Highscores;

GPIOManager IOManager;

/// <summary>
/// Structure for pairing up which IO refer to which buttons.
/// </summary>
struct ButtonStruct
{
	int InputPin;
	// ReSharper disable once CppInconsistentNaming
	int LEDPin;
};

// List of all the button LED and Input pin assignments
const ButtonStruct Buttons[] = {
	{1,2},
	{2,1},
	{3,0}
};

void Game1();

void Game2();


int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();

    // Wait for external devices to power up
    wait_ms(2000);
    
    // Read the header information & calculate averages.
    Highscores.Initialise(&uBit);

    // Setup the GPIO expander for buttons
    IOManager.Init(&uBit);
    
    // Check to see if button 2 is being held during startup.
    // This wil erase flash.
    if (IOManager.ReadPortB()){
        Highscores.Reset();
    }

    int modeSelect = 1;
    // Main Loop
    while(1){
        // Select the game mode that we want to play.
        while(1){
            //uBit.display.printAsync("Press any button to start!");
            uBit.display.print(modeSelect);
            if(uBit.buttonB.isPressed()){
                if (modeSelect != 3){
                    modeSelect++;
                }
                while(uBit.buttonB.isPressed());
            }
            if(uBit.buttonA.isPressed()){
                if (modeSelect != 1){
                    modeSelect--;
                }
                while(uBit.buttonA.isPressed());
            }
            if (IOManager.ReadPortB()){
                break;
            }

        }

        switch (modeSelect)
        {
            case 1:
                Game1();
                wait_ms(300);
                break;
            case 2:
                Game2();
                wait_ms(300);
                break;
            default:
            uBit.display.print("!");
            wait_ms(500);
                break;
        }

        
    }

    // If main exits, there may still be other fibers running or registered event handlers etc.
    // Simply release this fiber, which will mean we enter the scheduler. Worse case, we then
    // sit in the idle task forever, in a power efficient sleep.
    release_fiber();
}

void Game1(){
    uBit.display.stopAnimation();
        uBit.display.clear();
        int pin = 2;
        int button = 0;
        int oldButton = 0;
        uint32_t sum = 0;
        for(int i=0; i< 10; i++){
            
            // This will clear pending interrupt
            //IOManager.ReadPortB();
            //uBit.serial.send("GO!\n\r");
            
            // Choose button in range 0-2
            while (button == oldButton){
                button = rand() % 3;
            }
            oldButton = button;


            IOManager.digitalWrite(Buttons[button].LEDPin, true);
            
            uint32_t time1 = us_ticker_read();
            
            while(1){
                
                // Wait for them to press the button
                while(!uBit.io.P8.getDigitalValue());

                // Check which input changed
                char inputFlag = IOManager.ReadPortB();
                
                // Check if the chosen pin was pressed
                if (IOManager.isBitSet(inputFlag, Buttons[button].InputPin)){
                    // Calculate the time it took for the button to be 
                    uint32_t time2 = us_ticker_read();
                    uint32_t result = (time2-time1);
                    //uBit.serial.send((int)result);
                    //uBit.serial.send("\n\r");
                    sum += result;
                    // Wait for the pin to be let go
                    while(IOManager.digitalRead(Buttons[button].InputPin));
                    // Turn off button LED
                    IOManager.digitalWrite(Buttons[button].LEDPin, false);
                    
                    break;
                }else{

                }
            }

        }

        sum /= 10000;
        uBit.serial.send("AVG:");
        uBit.serial.send((int)sum);
        uBit.serial.send("\n\r");
        uBit.display.scroll((int)sum);
}

// Count how many within timeframe
void Game2(){
    // How many butons can you press in 10 seconds.
    uBit.display.stopAnimation();
    uBit.display.clear();
    int pin = 2;
    int button = 0;
    int oldButton = 0;
    uint32_t sum = 0;
    int count = 0;
    // Set timout for ten seconds time
    uint32_t timeout = us_ticker_read() + (10 * 1000000);
    while(us_ticker_read() < timeout){
       
        // Choose button in range 0-2
        while (button == oldButton){
            button = rand() % 3;
        }
        oldButton = button;


        IOManager.digitalWrite(Buttons[button].LEDPin, true);
        
        uint32_t time1 = us_ticker_read();
        
        while(1){
            
            // Wait for them to press the button
            while(!uBit.io.P8.getDigitalValue());

            // Check which input changed
            char inputFlag = IOManager.ReadPortB();
            
            // Check if the chosen pin was pressed
            if (IOManager.isBitSet(inputFlag, Buttons[button].InputPin)){
                // Calculate the time it took for the button to be 
                uint32_t time2 = us_ticker_read();
                uint32_t result = (time2-time1);
                //uBit.serial.send((int)result);
                //uBit.serial.send("\n\r");
                sum += result;
                // Wait for the pin to be let go
                while(IOManager.digitalRead(Buttons[button].InputPin));
                // Turn off button LED
                IOManager.digitalWrite(Buttons[button].LEDPin, false);
                count ++;

                break;
            }else{

            }
        }

    }

    sum /= 10000;
    uBit.serial.send("AVG:");
    uBit.serial.send((int)sum);
    uBit.serial.send("\n\r");
    uBit.display.scroll(count);

}