//#include <step.h>
#include <position.h>
//#include <btooth.h>
//#include <session.h>

const int k = 1000;

typedef struct {
    int startTime;
    int endTime;
    int stepNum = 0;
    int distance = 0;
    int timeWalking = 0;
    int timeRunning = 0;
} session;

session startSession(void);

// Entry Point

void setup() {

}

void loop() {

    if (watchWithinHouse()) {
        // device is inside thus wait
        delay(10*k)
    } else {
        // outside thus outing begins
        session currentOuting = startSession();
        btoothStartOuting(currentOuting);

        while (!watchWithinHouse()) {
            currentOuting.stepNum += countSteps(20, 10*k); // count until 20 steps or 10 seconds has passed
            updatePosition(currentOuting);
            btoothUpdateOuting(currentOuting);
        }

        currentOuting.endTime = position_time();
        btoothUploadOuting(currentOuting);
    }

}
