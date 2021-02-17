#include <stdlib.h>
#include <string.h>

typedef struct session;

typedef struct {
    int longitude;
    int latitude;
    int time;
} packet;

packet createPacket(void);
int checksumPacket(char* data, int len);
int watchWithinHouse(void);
session startSession(void);
void updatePosition(session* currentSession);

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
        //checksum = checksumPacket(dataStr, dataStrSize, cSum);
        checksumBool = 1;
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

int checksumPacket(char* data, int len) { //broken

    // Re-create original packet
    char packetData[len+2];
    strcpy(packetData, data);
    char* packet = strcat("GPGGA", packetData); // fails on this line idk why

    // checksum
    int cSum = 0;
    for (int i=0; i<len+5; i++) {
        cSum ^= packet[i];
    }

    return cSum;
}

int watchWithinHouse(void) {
    packet currentPacket = createPacket();
    if (( (currentPacket.longitude<homeLongitude+homeRadius) && (currentPacket.longitude>homeLongitude-homeRadius) )
        || ( (currentPacket.latitude<homeLatitude+homeRadius) && (currentPacket.latitude>homeLatitude-homeRadius) )) {
        return 1;
    } else {
        return 0;
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


    currentSession->previousPacket = currentPacket;
}