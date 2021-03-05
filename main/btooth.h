#include <stdarg.h>
#include <stdlib.h>

typedef struct session;

// Top level functions
void btoothStartOuting(session* currentOuting);
void btoothUpdateOuting(session* currentOuting);
void btoothUploadOuting(session* currentOuting);

// Lower level functions
int pairedBool(char* signifier);


// Top level functions //

void btoothStartOuting(session* currentOuting) {
    btoothSerial.write("Start");
    //pairedBool("Start");
    writeData(currentOuting->startTime);
}

void btoothUpdateOuting(session* currentOuting) {
    btoothSerial.write("Update");
    //pairedBool("Update");
    writeData((double) currentOuting->stepNum,
                (double) currentOuting->distance,
                currentOuting->timeWalking,
                currentOuting->timeRunning);
}

void btoothUploadOuting(session* currentOuting) {
    btoothSerial.write("Upload");
    //pairedBool("Upload");
    writeData(currentOuting->endTime,
                (double) currentOuting->stepNum,
                (double) currentOuting->distance,
                currentOuting->timeWalking,
                currentOuting->timeRunning);
}


// Lower level functions //

int pairedBool(char* signifier) {

    int pairedBool = 0;
    while(!pairedBool) {
        // Wait for a response
        while(!btoothSerial.available());
        // Allow time so signifier to load onto buffer
        delay(500);
        if (btoothSerial.read() == signifier) {
            return 1;
        } else {
            continue;
        }
    }
}

void writeData(double args, ...) {
    va_list vaList;
    va_start(vaList, args);
    while (args) {
        //char writtenString[10];
        //sprintf(writtenString, 10, "%f", args);
        btoothSerial.write(args);
        btoothSerial.write(",");
        args = va_arg(vaList, double);
    }
    btoothSerial.write("*");
}