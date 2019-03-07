#ifndef __HIGHSCOREMANAGER__
#define __HGIHSCOREMANAGER__
#include "MicroBit.h"

class HighScoreManager{

    public:
    HighScoreManager();
    
    // Populate member variables and calculate the average score
    bool Initialise(MicroBit * uBit);
    // Adds a new time to the highscores list.
    uint16_t AddEntry(uint32_t Time);
    // Resets the NumberOfEntries.
    bool Reset();

    // Gets the next ID to be used in flash
    int GetNextEntryID();
    // Gets the total number of entries in flash
    int GetNumberOfEntries();
    // Gets the score corresponding with the provided ID
    bool GetScore(uint16_t Id, uint32_t * Time);
    // Gets the fastest time
    unsigned int GetBestTime();
    // Gets the current average reaction time.
    double GetAverage();

    private:
    uint16_t NumberOfEntries;
    uint16_t BestTimeID;
    double AverageTime;
    MicroBit * mpuBit;
    // // Runs through all entries and caluclates the average time. 
    void CalculateAverage();

    // // Gets the ID of the highest score from flash.
    // void GetHighestScore();


};

#endif
