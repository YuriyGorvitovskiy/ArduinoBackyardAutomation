#ifndef SENSOR_H
#define SENSOR_H

#include <Arduino.h>

typedef void (*OnRFIDEvent)(bool enter, word scout);

class Sensor {
public:
    Sensor(byte mfid, long rfid) : 
        name(NULL), mfid(mfid), rfid(rfid), onRFIDEvent(NULL) {}

    Sensor(const char* name, byte mfid, long rfid) : 
        name(name), mfid(mfid), rfid(rfid), onRFIDEvent(NULL) {}
        
    Sensor(const char* name, byte mfid, long rfid, OnRFIDEvent onRFIDEvent) : 
        name(name), mfid(mfid), rfid(rfid), onRFIDEvent(onRFIDEvent) {}

    boolean operator==(const Sensor& b) {return this->mfid == b.mfid && this->rfid == b.rfid; }

    const char* getName() {return name;}
    void onEvent(bool enter, word scout) const { if (onRFIDEvent != NULL) onRFIDEvent(enter, scout); }
    
    
protected:    
    const char*     name;
    const byte      mfid;
    const long      rfid;

    OnRFIDEvent     onRFIDEvent;
};

#endif

