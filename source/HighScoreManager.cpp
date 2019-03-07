

#include "HighScoreManager.h"
#include "MicroBit.h"

const char NumEntryString[] = "NumEntries";
const char BestTimeIDString[] = "BestTimeID";


HighScoreManager::HighScoreManager() : NumberOfEntries(0), AverageTime(0), BestTimeID(0) {



}

bool HighScoreManager::Initialise(MicroBit * uBit){
    mpuBit = uBit;


    // Check if the uBit has previously stored the initialised header. If not, default values will need to be written to flash.
    KeyValuePair* tempKVP = mpuBit->storage.get("Initialised");
    if (tempKVP == NULL){
        // Header has not been written, write defualt headers
        Reset();
    }else{
        //Headers 'should' exist, check for each.
        
        KeyValuePair* NumEntryKvp = mpuBit->storage.get(NumEntryString);
        KeyValuePair* BestTimeKvp = mpuBit->storage.get(BestTimeIDString);

        // If either of these keys don't exist for some exotic reason then Reset the high scores.
        if (NumEntryKvp == NULL || BestTimeKvp == NULL){
            Reset();


        }

        // They must both be OK, read their values into variables.
        memcpy(&NumberOfEntries, NumEntryKvp->value, sizeof(int));
        memcpy(&BestTimeID, BestTimeKvp->value, sizeof(int));

        // Clean up the mess....
        delete NumEntryKvp;
        delete BestTimeKvp;

    }

    delete tempKVP;
    // Write the Initialised key for next boot.
    int init = 1;
    mpuBit->storage.put("Initialised", (uint8_t *)&init, sizeof(init));




    return true;
}


bool HighScoreManager::Reset(){
        // Write the number of entries
        mpuBit->storage.put(NumEntryString, (uint8_t *)&NumberOfEntries, sizeof(NumberOfEntries));

        // Write the entry ID for the current highest score
        mpuBit->storage.put(BestTimeIDString, (uint8_t *)&BestTimeID, sizeof(BestTimeID));

        return true;
}

uint16_t HighScoreManager::AddEntry(uint32_t Time){
    char tempStore[sizeof(uint16_t)] = {0};

    // Form the ID of the next available entry ID.
    // Entry ID's are indexed based to zero therefore the nextEntryID is equal to the number of entries total.
    uint16_t nextEntryId = NumberOfEntries;

    memcpy(tempStore, &nextEntryId, sizeof(uint16_t));

    mpuBit->storage.put(tempStore,(uint8_t *)&Time, sizeof(Time));

    NumberOfEntries++;
    mpuBit->storage.put(NumEntryString, (uint8_t *)&NumberOfEntries, sizeof(NumberOfEntries));

    // If this is the first entry, the best time is already correct.    
    if (nextEntryId == 0)
        return nextEntryId;

    // Is this the new highest score??
    uint32_t currentBest = 0;
    GetScore(BestTimeID, &currentBest);

    // If the current best time is greater than this, it is worse.
    if (currentBest > Time){
        BestTimeID = nextEntryId;
        mpuBit->storage.put(BestTimeIDString, (uint8_t *)&BestTimeID, sizeof(BestTimeID));
    }
    return nextEntryId;

}

bool HighScoreManager::GetScore(uint16_t Id, uint32_t * Time){
    // Temp space for referring to a score's ID
    char tempStore[sizeof(uint16_t)];
    
    // Attempt to move the Id into the temp store
    memcpy(tempStore, &Id, sizeof(uint16_t));

    // Read this key from storage
    KeyValuePair* Score = mpuBit->storage.get(tempStore);

    if (Score == NULL){
        delete Score;
        *Time = 0;
        return false;
    }
    // Valid key, copy the value from memory
    memcpy(Time, Score->value, sizeof(uint32_t));

    // Success
    return true;

}

void HighScoreManager::CalculateAverage(){
    // Assume that the variables have been initialised.
    double sum = 0;
    uint32_t tempVal = 0;

    // Calculate the current average
    for (int i = 0; i < NumberOfEntries; i++){
        if (GetScore(i,&tempVal) == false){
            AverageTime = 9999;
            return;
        }else{
            sum += tempVal;
        }
    }

    AverageTime = sum / (double) NumberOfEntries;

    
}