#include <stepcounter.h>
#include <position.h>
#include <btooth.h>

//#include <SoftwareSerial.h>

const int k = 1000;

//we'll get these via the app ofc
const double homeLongitude = 0;
const double homeLatitude = 0;
const double homeRadius = 1;

const double runningSpeedpHr = 6.5;

typedef struct {
    packet previousPacket;
    double startTime;
    double endTime;
    int stepNum = 0;
    int distance = 0;
    int isRunningBool = 0;
    double timeWalking = 0;
    double timeRunning = 0;
} session;

// The BlueTooth uses a software serial
SoftwareSerial btoothSerial(2, 3);

// Entry Point

void setup() {
    Serial.begin(9600);
    while(!Serial);
    btoothSerial.begin(9600);
}

void loop() {

    if (watchWithinHouse()) {
        // device is inside thus wait
        delay(10*k);
    } else {
        // outside thus outing begins
        session currentOuting = startSession();
        btoothStartOuting(&currentOuting);

        int prevStepCount = 0;
        while (!watchWithinHouse()) {
            countSteps(&currentOuting, 20, 10*k);
            if (prevStepCount!=currentOuting.stepNum) {
                updatePosition(&currentOuting);
                btoothUpdateOuting(&currentOuting);
                prevStepCount = currentOuting.stepNum;
            } else {
                // no new steps thus nothing to update
                continue;
            }
        }
        currentOuting.endTime = createPacket().time;
        btoothUploadOuting(&currentOuting);
    }

}
