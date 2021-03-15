#include <SoftwareSerial.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Typedefs //

typedef struct {
    double longitude;
    double latitude;
    double time;
} packet;

typedef struct coordinates{
    float x;
    float y;
    float z;
    float magnitude;
    //float theta;
    //float phi
} coordinates;

// Functions //

void countSteps(int interuptRunTime);
void getAcceleration(void);

packet createPacket(void);
int watchWithinHouse(void);

void bToothUpdate(packet* currentPacket);

// Variables //

const int k = 1000;

int times=0;
//float distance = 0; // total distance
//coordinates s; // displacement
//coordinates u; // velocity of the last milisecond
//coordinates v; // velocity of this milisecond
coordinates acceleration; // acceleration of this milisecond
//float t = 0.001; // how long the interupt is, 1 milisecond
int steps = 0; // step counter
//int walking = 0; //the boolean one for deciding walking/running
//float angle;
int lasttime;
int high=0;
float x[100];
float y[100];
float z[100];
int runTime = 0;

const double homeLongitude = 50.944000;
const double homeLatitude = 1.399979;
const double homeRadius = 20;

// Misc //

SoftwareSerial GPSserial(4, 7);

// Entry Point

void setup() {
    Serial.begin(9600);
    while(!Serial);
    GPSserial.begin(9600);
    pinMode(9, OUTPUT);
    pinMode(10, OUTPUT);

    pinMode(A0 , INPUT); //a.x
    pinMode(A1 , INPUT); //a.y
    pinMode(A2 , INPUT); //a.z
    pinMode(13, OUTPUT);
    digitalWrite(13, LOW);
    // sets up timer0 interrupt to be called for every millisecond
    cli();
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
}

void loop() {

    if (watchWithinHouse()) {
        // device is inside thus wait
        delay(10*k);
        digitalWrite(9, HIGH);
        digitalWrite(10, LOW);
    } else {
        digitalWrite(9, LOW);
        digitalWrite(10, HIGH);
        // outside thus outing begins
        packet currentPacket = createPacket();
        bToothUpdate(&currentPacket);

        while (!watchWithinHouse()) {
            countSteps(10*k);
            currentPacket = createPacket();
            bToothUpdate(&currentPacket);
        }
        currentPacket = createPacket();
        bToothUpdate(&currentPacket);
        steps = 0;
    }

}


// Function definitions //

void countSteps(int interuptRunTime) {
    sei();
    runTime = 0;
    while(runTime != interuptRunTime);
    cli();
}

SIGNAL(TIMER0_COMPA_vect) {
    cli();
    runTime++;
    getAcceleration();
    sei();
}


void getAcceleration(){
    int totalx, totaly, totalz;
    x[times%25] = analogRead(A0);
    if(times%25==0){
        for(int i=0;i<100;i++){
            totalx+=x[i];
        }
        acceleration.x=totalx/25;
        if(acceleration.x<0){
            acceleration.x=4.65*(9.81/0.33)*(acceleration.x+323)/1023;
        }
        else{
            acceleration.x=4.65*(9.81/0.33)*(acceleration.x-324)/1023;
        }
        totalx=0;
        if(abs(acceleration.x)<0.3){
            acceleration.x=0;
        }
    }
    y[times%25] = analogRead(A1);
    if(times%25==0){
        for(int i=0;i<100;i++){
            totaly+=y[i];
        }
        acceleration.y=(totaly/25)+320;
        acceleration.y=4.65*(9.81/0.33)*(acceleration.y)/1023;
        if(acceleration.y<0){
            acceleration.y+=0.5;
        } else {
        //acceleration.y-=3;
        }
        totaly=0;
        if(abs(acceleration.y)<0.16){
            acceleration.y=0;
        }
    }
    z[times%25] = analogRead(A2);
    if(times%25==0){
        for(int i=0;i<100;i++){
            totalz+=z[i];
        }
        acceleration.z = totalz/25;
        acceleration.z=4.65*(9.81/0.33)*(acceleration.z-270)/1023;
        if(acceleration.z<0){
            acceleration.z+=0.41;
        } else {
        acceleration.z-=0.41;
        }
        totalz=0;
        if(abs(acceleration.z)<0.16){
            acceleration.z=0;
        }
    }
    acceleration.magnitude = sqrt(sq(acceleration.x) + sq(acceleration.y) + sq(acceleration.z));
    if(abs(acceleration.x)>1.3 && times>600){
        digitalWrite(13, HIGH);
        steps++;
        times=0;
        Serial.println(steps);
        Serial.println(acceleration.x);
    } else if (times==500){
        digitalWrite(13, LOW);
        times++;
    } else {
        times++;
    }
}

packet createPacket(void) {

    // locate correct packet type
    const char corIdent[7] = "$GPGGA";
    // if checksum false then find another
    int checksumBool = 0;

    char* dataStr;

    while (!checksumBool) {

        // correct packet
        char ident[7] = "000000";
        int correctPacketBool = 0;
        while (!correctPacketBool) {
            ident[5] = GPSserial.read();
            if (strcmp(ident, corIdent)) {
                correctPacketBool = 1;
            } else {
                ident[0] = ident[1];
                ident[1] = ident[2];
                ident[2] = ident[3];
                ident[3] = ident[4];
                ident[4] = ident[5];
            }
        }

        // extract packet data
        int dataStrSize = 0, dataBool = 1;
        dataStr = (char*) calloc(dataStrSize, dataStrSize);
        while (dataBool) {
            char symbol = GPSserial.read();
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
            char symbol = GPSserial.read();
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
        //checksumBool = checksumPacket(reCreatedPacket, dataStrSize+5, cSum);
        checksumBool = 1;
        free(cSumStr);
    }

    // parse the data
    packet currentPacket;
    char* tokens = strtok(dataStr, ",");
    int tokensIndex = 0;
    while (tokens != NULL) {
        switch (tokensIndex) {
        case 0:
            currentPacket.time = atof(tokens);
        case 1:
            double lat = atof(tokens);
            currentPacket.latitude = (int) (lat/100) + ((lat/100) - ((int) lat /100))*100 / 60;
        case 2:
            if (*tokens == 'S') currentPacket.latitude = -currentPacket.latitude;
        case 3:
            double lon = atof(tokens);
            currentPacket.longitude = (int) (lon/100) + ((lon/100) - ((int) lon /100))*100 / 60;
        case 4:
            if (*tokens == 'W') currentPacket.longitude = -currentPacket.longitude;
        default:
            tokens = strtok(NULL, ",");
        }
        tokensIndex++;
    }
    free(dataStr);
    return currentPacket;
}


int watchWithinHouse(void) {
    packet currentPacket = createPacket();
    const int coef = 14.28595;
    if (abs(homeLatitude-currentPacket.latitude)<homeRadius/coef) {
        return 1;
    } else if (abs(homeLongitude-currentPacket.longitude)<homeRadius/coef) {
        return 1;
    } else {
        return 0;
    }
}

void bToothUpdate(packet* currentPacket) {
    Serial.print("*steps");
    Serial.print(steps);
    Serial.print("time");
    Serial.print(currentPacket->time);
    Serial.print("long");
    Serial.print(currentPacket->longitude);
    Serial.print("lati");
    Serial.print(currentPacket->latitude);
}