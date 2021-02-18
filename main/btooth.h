
typedef struct session;

void btoothStartOuting(session* currentOuting);
void btoothUpdateOuting(session* currentOuting);
void btoothUploadOuting(session* currentOuting);

void btoothStartOuting(session* currentOuting) {
    btoothSerial.write("signifier");
}

void btoothUpdateOuting(session* currentOuting) {
    btoothSerial.write("signifier");
}

void btoothUploadOuting(session* currentOuting) {
    btoothSerial.write("signifier");
}