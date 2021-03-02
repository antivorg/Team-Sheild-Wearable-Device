
/*************************************************************************
*
*   GPS test code
*
*   The following is a test script, intended to:
*   - Verify data can be read from the GPS module
*   - The correct packets are parsed into usable data
*   - From this data, determine if the user is inside of outside
*
*   Before compilation the tester must define their test location
*   on lines 27 and 28. Once compiled the tester can evalute by confirming:
*   - The red LED is switched on only when within the defined "home"
*   - The first green LED is switched on only when outside the "home"
*   - The reamining green LED is on and flashes only when outside
*
*************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <math.h>

const int k = 1000;

// GET FROM GOOGLE MAPS
const double homeLongitude = 0;
const double homeLatitude = 0;
const double homeRadius = 20;

// LEDS
const int greenLED0 = 4;
const int redLED = 5;
const int greenLED1 = 6;

// type defs
typedef struct {
    double longitude;
    double latitude;
    double time;
} packet;

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

// Functions
// Top level
int watchWithinHouse(void);
session startSession(void);
void updatePosition(session* currentSession);

// Lower level
packet createPacket(void);
int checksumPacket(char* data, int len, int recievedCSum);
double greatCircleDistance(double long1, double lat1,
                           double long2, double lat2);


// Main //

void setup() {
    Serial.begin(9600);
    while(!Serial);
    pinMode(greenLED0, OUTPUT);
    pinMode(redLED, OUTPUT);
    pinMode(greenLED1, OUTPUT);
}

void loop() {

    if (watchWithinHouse()) {
        // device is inside thus wait
        digitalWrite(greenLED0, LOW);
        digitalWrite(redLED, HIGH);
        digitalWrite(greenLED1, LOW);
        delay(2*k);
    } else {
        // outside thus outing begins
        session currentOuting = startSession();
        digitalWrite(greenLED1, HIGH);
        digitalWrite(redLED, LOW);
        digitalWrite(greenLED1, HIGH);
        int flashState = HIGH;

        while (!watchWithinHouse()) {
            updatePosition(&currentOuting);
            if (flashState == LOW) {
                flashState = HIGH;
            } else {
                flashState = LOW;
            }
            digitalWrite(greenLED1, flashState);
        }
        delay(2*k);
        currentOuting.endTime = createPacket().time;
    }

}

// End of main //

// Top level functions //

int watchWithinHouse(void) {
    packet currentPacket = createPacket();
    if (greatCircleDistance(homeLongitude, homeLatitude,
                            currentPacket.latitude,
                            currentPacket.longitude) > homeRadius) {
        return 0;
    } else {
        return 1;
    }
}

session startSession(void) {
    packet currentPacket = createPacket();
    session currentSession;
    currentSession.startTime = currentPacket.time;
    currentSession.previousPacket = currentPacket;
    return currentSession;
}

void updatePosition(session* currentSession) {
    packet currentPacket = createPacket();
    currentSession->distance = greatCircleDistance(
                                        currentSession->previousPacket.longitude,
                                        currentSession->previousPacket.latitude,
                                        currentPacket.longitude, currentPacket.latitude);
    if (currentSession->isRunningBool) {
        currentSession->timeWalking = currentPacket.time-currentSession->previousPacket.time;
    } else {
        currentSession->timeRunning = currentPacket.time-currentSession->previousPacket.time;
    }
    currentSession->previousPacket = currentPacket;
}


// Lower level functions //

packet createPacket(void) {

    // locate correct packet type
    const char corIdent[7] = "$GPGGA";
    // if checksum false then find another
    int checksumBool = 0;

    char* dataStr;

    while (!checksumBool) {

        char ident[7] = "000000";
        int correctPacketBool = 0;
        while (!correctPacketBool) {
            ident[5] = Serial.read();
            correctPacketBool = 1;
            for (int i=0; i<6; i++) {
                if (ident[i] != corIdent[i]) correctPacketBool = 0;
                if (i<5) ident[i] = ident[i+1];
            }
        }

        // extract packet data
        int dataStrSize = 0, dataBool = 1;
        dataStr = (char*) calloc(dataStrSize, dataStrSize);
        while (dataBool) {
            char symbol = Serial.read();
            if (symbol=='*') { // checksum begins
                dataBool = 0;
            } else {
                dataStr = (char*) realloc(dataStr, ++dataStrSize);
                dataStr[dataStrSize-1] = symbol;
            }
        }
        dataStr[dataStrSize] = '\0';

        // extract packet checksum
        char* cSumStr;
        int cSumStrSize = 0, cSumBool = 1;
        cSumStr = calloc(cSumStrSize, cSumStrSize);
        while (cSumBool) {
            char symbol = Serial.read();
            if (symbol=='\r') { // packets delimited by \r\n
                cSumBool = 0;
            } else {
                cSumStr = (char*) realloc(cSumStr, ++cSumStrSize);
                cSumStr[cSumStrSize-1] = symbol;
            }
        }
        cSumStr[cSumStrSize] = '\0';

        int cSum = atoi(cSumStr);
        char reCreatedPacket[] = "GPGGA";
        strcat(reCreatedPacket, (const char*) dataStr);
        checksumBool = checksumPacket(reCreatedPacket, dataStrSize+5, cSum);
        //checksumBool = 1;
        free(cSumStr);
    }

    // parse the data
    packet currentPacket;
    char* tokens = strtok(dataStr, ",");
    int tokensIndex = 0;
    while (tokens != NULL) {
        switch (tokensIndex) {
        case 1:
            currentPacket.time = atof(tokens);
            break;
        case 3:
            currentPacket.latitude = atof(tokens);
            break;
        case 6:
            currentPacket.longitude = atof(tokens);
            break;
        default:
            tokens = strtok(NULL, ",");
            break;
        }
        tokensIndex++;
    }
    free(dataStr);
    return currentPacket;
}

int checksumPacket(char* data, int len, int recievedCSum) {
    int cSum = 0;
    for (int i=0; i<len; i++) {
        cSum ^= data[i];
    }
    return cSum==recievedCSum;
}

double greatCircleDistance(double long1, double lat1,
                           double long2, double lat2) {

    const int earthRadius = 6371*k;
    const double pi = 3.141592654;

    // uses haversine, accurate for "small distances" (the earth is big yknow)
    double cos_lat1 = cos(pi*lat1/180);
    double cos_lat2 = cos(pi*lat2/180);
    double sin2_dlat = sin(pi*(lat2-lat1)/360);
    sin2_dlat = sin2_dlat*sin2_dlat;
    double sin2_dlong = sin(pi*(long2-long1)/360);
    sin2_dlong = sin2_dlong*sin2_dlong;

    double dSigma = 2*asin(sqrt(sin2_dlat+cos_lat1*cos_lat2*sin2_dlong));
    return dSigma*earthRadius*k;

}