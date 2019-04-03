#include "MicroBit.h"
#include "HighScoreManager.h"
#include "GPIOManager.h"

// Shortcut for finding how big an array is
#define DIM(x) sizeof(x) / sizeof(x[0])

// Microbit object for tieing together all microbit functionality.
MicroBit uBit;

// Class for managing the highscores stored in memory.
HighScoreManager Highscores;

// Manages interaction with external GPIO MCP23017 via i2c
GPIOManager IOManager;

// Structure for pairing up which IO refer to which buttons.
struct ButtonStruct
{
    // Input pin on the GPIO for the button
    int InputPin;

    // PIN used to lighting up the buttons LED
    int LEDPin;
};

// List of all the button LED and Input pin assignments
const ButtonStruct Buttons[] = {
    {1, 4},
    {2, 3},
    {3, 2},
    {4, 1},
    {6, 0}};

enum GameModes
{
    ReactionTime = 0x01,
    ButtonCount = 0x02,
    Versus = 0x03
};

// Test average reaction times
void ReactionTimerGame();

// How many buttons can be pressed in the time.
void ButtonCountGame();

// Two player vs mode
void VersusGame();

// Entry point for the program.
int main()
{
    // Initialise the micro:bit runtime.
    uBit.init();
    uBit.display.scrollAsync("Starting up!!");
    // Wait for external devices to power up
    wait_ms(5000);

    // Read the header information & calculate averages.
    Highscores.Initialise(&uBit);

    // Setup the GPIO expander for buttons
    IOManager.Init(&uBit);

    // Check to see if button 2 is being held during startup.
    // This wil erase flash.
    if (IOManager.ReadPortB())
    {
        Highscores.Reset();
    }

    // Clear the display, in case the microbit is still scrolling the statup message
    uBit.display.stopAnimation();
    uBit.display.clear();

    // Set all the outputs to be off.
    IOManager.writeRegister(0x09, 0x00);

    wait_ms(200);

    // Set default gamemode
    int modeSelect = GameModes::ReactionTime;

    // Main Loop
    while (1)
    {

        // Select the game mode that we want to play.
        while (1)
        {

            // Light up all the buttons on the main menu
            IOManager.digitalWrite(Buttons[0].LEDPin, true);
            IOManager.digitalWrite(Buttons[2].LEDPin, true);
            IOManager.digitalWrite(Buttons[4].LEDPin, true);

            // Update the display to show the currently selected mode.
            uBit.display.print(modeSelect);

            // Mode selection uses Button 1 and Button 5 (Most left and Most right) buttons.
            // The white button (button 3) confirms the selection.
            char inputFlag = IOManager.ReadPortB();

            // Which button was pressed
            if (IOManager.isBitSetExclusive(inputFlag, Buttons[0].InputPin))
            {
                // Left Button
                if (modeSelect != 1)
                {
                    modeSelect--;
                }
                // Wait until they let go of the button.
                while (IOManager.ReadPortB())
                    ;
            }
            else if (IOManager.isBitSetExclusive(inputFlag, Buttons[4].InputPin))
            {
                // Right Button
                if (modeSelect != 3)
                {
                    modeSelect++;
                }
                // Wait until they let go of the button.
                while (IOManager.ReadPortB())
                    ;
            }
            else if (IOManager.isBitSetExclusive(inputFlag, Buttons[2].InputPin))
            {
                // Middle Button
                IOManager.writeRegister(0x09, 0x00);

                // Wait until they let go of the button.
                while (IOManager.ReadPortB())
                    ;

                // Exit the loop
                break;
            }
        }

        // Start the selected game.
        switch (modeSelect)
        {
        case GameModes::ReactionTime:
            ReactionTimerGame();
            wait_ms(300);
            break;
        case GameModes::ButtonCount:
            ButtonCountGame();
            wait_ms(300);
            break;
        case GameModes::Versus:
            VersusGame();
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

void ReactionTimerGame()
{
    // Clear anything off the display
    uBit.display.stopAnimation();
    uBit.display.clear();

    // The current and old button. This prevents repeated buttons
    int currentButton = 0;
    int oldButton = 0;

    // The running sum used to calucate average reaction time.
    uint32_t sum = 0;

    // Loop thorugh 10 times.
    for (int i = 0; i < 10; i++)
    {

        // Choose button in range of Buttons
        while (currentButton == oldButton)
        {
            currentButton = rand() % DIM(Buttons);
        }
        oldButton = currentButton;

        // Light up the chosen button's LED
        IOManager.digitalWrite(Buttons[currentButton].LEDPin, true);

        // Save the current time.
        uint32_t time1 = us_ticker_read();

        // Needs to loop for the case where the incorrect button is pressed.
        while (1)
        {

            // Wait for them to press the button
            while (!uBit.io.P8.getDigitalValue())
                ;

            // Check which input changed
            char inputFlag = IOManager.ReadPortB();

            // Check if the chosen pin was pressed
            if (IOManager.isBitSetExclusive(inputFlag, Buttons[currentButton].InputPin))
            {
                // They have pressed the correct button.

                // Calculate the time it took for the button to be pressed, add it to the current total
                sum += (us_ticker_read() - time1);

                // Wait for the pin to be let go
                while (IOManager.digitalRead(Buttons[currentButton].InputPin))
                    ;

                // Turn off button LED
                IOManager.digitalWrite(Buttons[currentButton].LEDPin, false);

                break;
            }
            else
            {
            }
        }
    }

    // Game finished, display the average on the display (in ms).
    sum /= 10000;
    uBit.serial.send("AVG:");
    uBit.serial.send((int)sum);
    uBit.serial.send("\n\r");
    uBit.display.scroll((int)sum);
}

// Count how many buttons can be pressed within a 10 second timeframe.
void ButtonCountGame()
{
    // Clear anything off the display
    uBit.display.stopAnimation();
    uBit.display.clear();

    // Store the current an old buttons, used to make sure repeat buttons are not possible.
    int currentButton = 0;
    int oldButton = 0;

    // Count of how many buttons can be pressed within the time.
    int count = 0;

    // Set timout for ten seconds time
    uint32_t timeout = us_ticker_read() + (10 * 1000000);

    // While the timeout has not been elapsed.
    while (us_ticker_read() < timeout)
    {

        // Choose button in range of Buttons
        while (currentButton == oldButton)
        {
            currentButton = rand() % DIM(Buttons);
        }

        // Store the chosen button for the next loop.
        oldButton = currentButton;

        // Enable the chosen button's LED
        IOManager.digitalWrite(Buttons[currentButton].LEDPin, true);

        while (1)
        {

            // Wait for them to press the button
            while (!uBit.io.P8.getDigitalValue())
                ;

            // Check which input changed
            char inputFlag = IOManager.ReadPortB();

            // Check if the chosen pin was pressed
            if (IOManager.isBitSetExclusive(inputFlag, Buttons[currentButton].InputPin))
            {
                // The correct button was pressed!

                // Wait for the pin to be let go.
                while (IOManager.digitalRead(Buttons[currentButton].InputPin))
                    ;

                // Turn off button LED
                IOManager.digitalWrite(Buttons[currentButton].LEDPin, false);

                // Increment the counter
                count++;

                break;
            }
            else
            {
                // The wrong button was pressed. Decrement the Counter >:)
                //count--;
            }
        }
    }

    uBit.display.scroll(count);
}

// Two player verses
void VersusGame()
{
    // In this mode, there are two buttons "Left" and "Right". This refers to the Red/Blue Yellow/Green button combos for two player
    uBit.display.stopAnimation();
    uBit.display.clear();

    // Fake loading time
    wait_ms(500);

    int button1 = 0;
    int button2 = 0;

    int player1Score = 0;
    int player2Score = 0;

    // Do this 5 times (best of 5 then innit)
    for (int i = 0; i < 5; i++)
    {

        // Choose button in range 0-1
        button1 = rand() % 2;
        // Translate to button2
        button2 = button1 + 3;

        for (int i = 3; i > 0; i--)
        {
            uBit.display.print(i);
            wait_ms(1000);
        }

        // Wait between 0.25-2 seconds.
        wait_ms(250 + ((rand() % 2) * 1000));

        // Change display
        uBit.display.print("!");

        // Turn on the chosen buttons
        IOManager.digitalWrite(Buttons[button1].LEDPin, true);
        IOManager.digitalWrite(Buttons[button2].LEDPin, true);

        while (1)
        {

            // Wait for them to press the button
            while (!uBit.io.P8.getDigitalValue())
                ;

            // Check which input changed
            char inputFlag = IOManager.ReadPortB();

            bool button1Pressed = IOManager.isBitSet(inputFlag, Buttons[button1].InputPin);
            bool button2Pressed = IOManager.isBitSet(inputFlag, Buttons[button2].InputPin);

            if (button1Pressed && button2Pressed)
            {
                // No idea who hit it first
                uBit.display.print("?");
                continue;
            }
            if (button1Pressed && !button2Pressed)
            {
                // Player 1 Wins!
                uBit.display.print("<");
                player1Score++;
                while (IOManager.digitalRead(Buttons[button1].InputPin))
                    ;
            }
            if (!button1Pressed && button2Pressed)
            {
                // Player 2 Wins!
                uBit.display.print(">");
                player2Score++;
                while (IOManager.digitalRead(Buttons[button2].InputPin))
                    ;
            }
            if (!button1Pressed && !button2Pressed)
            {
                // Nobody has pressed the correct button yet.
                continue;
            }

            // Wait here for a few seconds to display who won
            wait_ms(2000);

            // Turn off button LEDs
            IOManager.digitalWrite(Buttons[button1].LEDPin, false);
            IOManager.digitalWrite(Buttons[button2].LEDPin, false);
            break;
        }
    }

    uBit.display.print("!");
    wait_ms(1000);

    // Game Over, display the score. Do it a few times
    for (int i = 0; i < 2; i++)
    {
        uBit.display.print("<");
        wait_ms(1000);
        uBit.display.print(player1Score);
        wait_ms(1000);
        uBit.display.print(">");
        wait_ms(1000);
        uBit.display.print(player2Score);
        wait_ms(1000);
    }

    for (int i = 0; i < 5; i++)
    {
        if (player1Score > player2Score)
        {
            uBit.display.print("<");
        }
        else
        {
            uBit.display.print(">");
        }
        wait_ms(500);
        uBit.display.clear();
        wait_ms(500);
    }
}