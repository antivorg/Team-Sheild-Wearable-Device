#include <stdlib.h>
#include <string.h>
#include <math.h>

typedef struct session;

typedef struct {
    int longitude;
    int latitude;
    int time;
} packet;

// Top level functions
int watchWithinHouse(void);
session startSession(void);
void updatePosition(session* currentSession);

// Lower level functions
packet createPacket(void);
int checksumPacket(char* data, int len);
double greatCircleDistance(double long1, double lat1, double long2, double lat2);


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
        dataStr = (char*) malloc(dataStrSize);
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
        cSumStr = malloc(cSumStrSize);
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
        checksum = checksumPacket(&dataStr, dataStrSize, cSum);
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

int checksumPacket(char* data, int len, int recievedCSum) { //broken

    // Re-create original packet
    char packetData[len+2];
    strcpy(packetData, data);
    char* packet = strcat("GPGGA", packetData); // fails on this line idk why

    // checksum
    int cSum = 0;
    for (int i=0; i<len+5; i++) {
        cSum ^= packet[i];
    }

    return cSum==recievedCSum;
}

double greatCircleDistance(double long1, double lat1, double long2, double lat2) {

    const int earthRadius = 6371*k;

    // uses haversine, accurate for "small distances" (the earth is big yknow)
    double cos_lat1 = cos(lat1);
    double cos_lat2 = cos(lat2);
    double sin2_dlat = sin((lat2-lat1)/2);
    sin2_dlat = sin2_dlat*sin2_dlat;
    double sin2_dlong = sin((long2-long1)/2);
    sin2_dlong = sin2_dlong*sin2_dlong;

    double dSigma = 2*asin(sqrt(sin2_dlat+cos_lat1*cos_lat2*sin2_dlong));
    return dSigma*earthRadius;

}