/**************************************************************************\
* Pinoccio Library                                                         *
* https://github.com/Pinoccio/library-pinoccio                             *
* Copyright (c) 2014, Pinoccio Inc. All rights reserved.                   *
* ------------------------------------------------------------------------ *
*  This program is free software; you can redistribute it and/or modify it *
*  under the terms of the MIT License as described in license.txt.         *
\**************************************************************************/
#include <SPI.h>
#include <Wire.h>
#include <Scout.h>
#include <GS.h>
#include <bitlash.h>
#include <lwm.h>
#include <js0n.h>
#include <backpacks/wifi/WifiModule.h>
#include <util/memdebug.h>

#include <Z21.h>

#include <Action.h>
#include <ActionSequence.h>
#include <ActionProcessor.h>


#include "Loco.h"
#include "Sensor.h"
#include "Actions.h"
#include "UDPConnection.h"

//!!! MESH-KEY = 14630d84c1e68bbe
//!!! MESH-REPORT for LEADER
// {"type":"mesh","scoutid":1,"troopid":2,"routes":0,"channel":20,"rate":"250 kb/s","power":"3.5 dBm","at":163723}
//!!! MESH-REPORT for STEAM:
// {"type":"mesh","scoutid":2,"troopid":2,"routes":0,"channel":20,"rate":"250 kb/s","power":"3.5 dBm","at":60083}

Loco        MOGUL(12, 2);
Loco        TAURUS(18, 3);

Accessory   TURNOUT_RIGHT(4);
Accessory   TURNOUT_LEFT(5);
Accessory   STATION_AUTO(6);
Accessory   STAGE_STOP(7);
Accessory   STAGE_AUTO(8);

Z21Speed    MOGUL_STOP          (MOGUL, 0);
Z21Speed    MOGUL_SLOW          (MOGUL, 24);
Z21Speed    MOGUL_SLOW_DOWN     (MOGUL, 48);
Z21Speed    MOGUL_GO            (MOGUL, 64);

Z21Function MOGUL_OPEN_DOORS    (MOGUL, Z21_Function::F6, true);
Z21Function MOGUL_CLOSE_DOORS   (MOGUL, Z21_Function::F6, false);

Z21Function MOGUL_BELL_ON       (MOGUL, Z21_Function::F1, true);
Z21Function MOGUL_BELL_OFF      (MOGUL, Z21_Function::F1, false);

Z21Function MOGUL_APPROACH_ON   (MOGUL, Z21_Function::F3, true);
Z21Function MOGUL_APPROACH_OFF  (MOGUL, Z21_Function::F3, false);

Z21Function MOGUL_SQUEAL_ON     (MOGUL, Z21_Function::F7, true);
Z21Function MOGUL_SQUEAL_OFF    (MOGUL, Z21_Function::F7, false);

Z21Function MOGUL_SOUND_ON      (MOGUL, Z21_Function::F20, false);
Z21Function MOGUL_SOUND_OFF     (MOGUL, Z21_Function::F20, true);

Z21Speed    TAURUS_STOP         (TAURUS, 0);
Z21Speed    TAURUS_SLOW         (TAURUS, 24);
Z21Speed    TAURUS_SLOW_DOWN    (TAURUS, 48);
Z21Speed    TAURUS_GO           (TAURUS, 64);

Z21Function TAURUS_OPEN_DOORS   (TAURUS, Z21_Function::F6, true);
Z21Function TAURUS_CLOSE_DOORS  (TAURUS, Z21_Function::F6, false);

Z21Function TAURUS_WISTLE_ON    (TAURUS, Z21_Function::F11, true);
Z21Function TAURUS_WISTLE_OFF   (TAURUS, Z21_Function::F11, false);

Z21Function TAURUS_APPROACH_ON  (TAURUS, Z21_Function::F3, true);
Z21Function TAURUS_APPROACH_OFF (TAURUS, Z21_Function::F3, false);

Z21Function TAURUS_SQUEAL_ON    (TAURUS, Z21_Function::F7, true);
Z21Function TAURUS_SQUEAL_OFF   (TAURUS, Z21_Function::F7, false);

Z21Function TAURUS_SOUND_ON     (TAURUS, Z21_Function::F20, false);
Z21Function TAURUS_SOUND_OFF    (TAURUS, Z21_Function::F20, true);

Z21Turnout  TURNOUT_RIGHT_IN    (TURNOUT_RIGHT, 1);
Z21Turnout  TURNOUT_RIGHT_OUT   (TURNOUT_RIGHT, 0);

Z21Turnout  TURNOUT_LEFT_IN     (TURNOUT_LEFT, 0);
Z21Turnout  TURNOUT_LEFT_OUT    (TURNOUT_LEFT, 1);

Delay        STATION_WAIT     (20000L);
Delay        STATION_ARRIVAL  ( 2000L- Z21_ACTION_CHECK_DELAY);
Delay        STATION_DEPARTURE( 3000L- Z21_ACTION_CHECK_DELAY);

Delay        SWITCH_DELAY     (  500L - Z21_ACTION_CHECK_DELAY);

Delay        MOGUL_BELL_DELAY       ( 2000L - Z21_ACTION_CHECK_DELAY);
Delay        MOGUL_APPROACH_DELAY   ( 2000L - Z21_ACTION_CHECK_DELAY);

Delay        TAURUS_WISTLE_DELAY    ( 1000L - Z21_ACTION_CHECK_DELAY);
Delay        TAURUS_APPROACH_DELAY  ( 2000L - Z21_ACTION_CHECK_DELAY);

Action* MOGUL_APPROACH_LIST[] = { 
        &MOGUL_APPROACH_ON,
        &MOGUL_APPROACH_DELAY,
        &MOGUL_APPROACH_OFF,
        NULL
};

Action* TAURUS_APPROACH_LIST[] = { 
        &TAURUS_APPROACH_ON,
        &TAURUS_APPROACH_DELAY,
        &TAURUS_APPROACH_OFF,
        NULL
};

Action* MOGUL_STATION_LIST[] = { 
        &MOGUL_STOP,
        &STATION_ARRIVAL,
        &MOGUL_OPEN_DOORS,
        &STATION_WAIT,
        &MOGUL_BELL_ON,
        &MOGUL_BELL_DELAY,
        &MOGUL_BELL_OFF,
        &MOGUL_CLOSE_DOORS,
        &STATION_DEPARTURE,
        &MOGUL_GO,
        NULL,
};

Action* TAURUS_STATION_LIST[] = { 
        &TAURUS_STOP,
        &STATION_ARRIVAL,
        &TAURUS_OPEN_DOORS,
        &STATION_WAIT,
        &TAURUS_WISTLE_ON,
        &TAURUS_WISTLE_DELAY,
        &TAURUS_WISTLE_OFF,
        &TAURUS_CLOSE_DOORS,
        &STATION_DEPARTURE,
        &TAURUS_GO,
        NULL,
};

Action* MOGUL_TAURUS_LIST[] = { 
        &MOGUL_STOP,
        &STATION_ARRIVAL,
        &TAURUS_SOUND_ON,
        &STATION_ARRIVAL,
        &MOGUL_SOUND_OFF,
        &STATION_ARRIVAL,
        &TURNOUT_RIGHT_IN,
        &SWITCH_DELAY,
        &TURNOUT_LEFT_IN,
        &STATION_DEPARTURE,
        &TAURUS_GO,
        NULL
};

Action* TAURUS_MOGUL_LIST[] = { 
        &TAURUS_STOP,
        &STATION_ARRIVAL,
        &MOGUL_SOUND_ON,
        &STATION_ARRIVAL,
        &TAURUS_SOUND_OFF,
        &STATION_ARRIVAL,
        &TURNOUT_RIGHT_OUT,
        &SWITCH_DELAY,
        &TURNOUT_LEFT_OUT,
        &STATION_DEPARTURE,
        &MOGUL_GO,
        NULL,
};


ActionSequence MOGUL_APPROACH;
ActionSequence MOGUL_STATION;
ActionSequence MOGUL_TAURUS;

ActionSequence TAURUS_APPROACH;
ActionSequence TAURUS_STATION;
ActionSequence TAURUS_MOGUL;


const Sensor SENSORS[15] = {
    Sensor("01", 0x1e, 0x0020d85fL, onRFID_1),
    Sensor("02", 0x1e, 0x0020e180L, onRFID_2_5),
    Sensor("03", 0x1e, 0x0020e17cL, onRFID_3),
    Sensor("04", 0x1e, 0x0020bc87L, onRFID_4),
    Sensor("05", 0x1e, 0x0020f033L, onRFID_2_5),
    Sensor("06", 0x1e, 0x0020f05eL, onRFID_6),
    Sensor("07", 0x1e, 0x0020d8a4L, onRFID_7),
    Sensor("08", 0x1e, 0x002101edL, onRFID_8),
    Sensor("09", 0x1b, 0x0033a4fdL, onRFID_9),
    Sensor("10", 0x1e, 0x0020fc21L, onRFID_10),
    Sensor("11", 0x1e, 0x0020e12cL, onRFID_11),
    Sensor("12", 0x1e, 0x0020bc71L, onRFID_12),
    Sensor("13", 0x1e, 0x00212350L, onRFID_13),
    Sensor("14", 0x1e, 0x0020aa6aL, onRFID_14),
    Sensor("15", 0x1e, 0x0020c333L, onRFID_15)
};

const int SENSOR_COUNT = sizeof(SENSORS)/sizeof(Sensor);

Z21Client Z21_BACKGROUND;

void setup() {
    Scout.setup();
    Processor.setup();

    //Arduino has no static initialization. Has to call init.
    MOGUL.init();
    TAURUS.init();

    TURNOUT_RIGHT.init();
    TURNOUT_LEFT.init();
    STATION_AUTO.init();
    STAGE_STOP.init();
    STAGE_AUTO.init();

    MOGUL_APPROACH.init(MOGUL_APPROACH_LIST);
    TAURUS_APPROACH.init(TAURUS_APPROACH_LIST);
    MOGUL_STATION.init(MOGUL_STATION_LIST);
    TAURUS_STATION.init(TAURUS_STATION_LIST);
    MOGUL_TAURUS.init(MOGUL_TAURUS_LIST);
    TAURUS_MOGUL.init(TAURUS_MOGUL_LIST);

    UDP.onConnect = onConnect;
    Z21.onLocoInfo = onLocoInfo; 
    Z21.onAccessoryInfo = onAccessoryInfo;        
    
    addBitlashFunction("rfid.report", (bitlash_function)RFIDreport);
    UDP.setup();
}

void loop() {
    Scout.loop();
    UDP.loop();
    Processor.loop();
}

numvar RFIDreport(void) {
    boolean action = getarg(1);
    unumvar scout  = getarg(2); 
    unumvar mfid   = getarg(3);
    unumvar rfid   = getarg(4);
    
    if (!UDP.isConnected())
        return 0;
    
    onRFID(action, scout, mfid, rfid);        

    Z21.packet.setHeader(10, (action ? 0x3701 : 0x3702)); // enter or leave with YG code
    Z21.packet.setByte(4, scout);
    Z21.packet.setByte(5, mfid);
    Z21.packet.setLEuint32(6, rfid);
    UDP.sendMac(Z21.packet);

/*    
    String report = "Scout " + String(scout);
    if (action == 1)
        report += " enter ";
    else
        report += " leave ";

    report += "DEC: ";
    report += String(mfid);
    report += "-";
    report += String(rfid);

    report += ", HEX: ";
    report += String(mfid, HEX);
    report += "-";
    report += String(rfid, HEX);
    report += "\n";
    
    Shell.print(report.c_str());
*/    
    return 0;
}

void onConnect() {
    Processor.start(&Z21_BACKGROUND);
}

void onLocoInfo(uint16_t address,
                boolean busy, boolean  consist, boolean transpond,
                boolean forward, uint8_t speed, uint32_t functions) {
    Loco* loco = Loco::findByDCCAddress(address);
    if (loco != NULL) {
        loco->update(forward, speed, functions);
        Serial.print("LI:");
        Serial.println(address, DEC);
    }
}

void onAccessoryInfo(uint16_t address, uint8_t pos) {
    Accessory* accessory = Accessory::findByDCCAddress(address);
    if (accessory != NULL) {
        accessory->update(pos);
        Serial.print("AI:");
        Serial.println(address, DEC);
    }
}

void onRFID(boolean enter, uint16_t scout, uint16_t mfg, uint32_t rfid) {
    Sensor s(mfg, rfid);
    for (int i = 0; i < SENSOR_COUNT; ++i) {
        if (s == SENSORS[i]) {
            SENSORS[i].onEvent(enter, scout);
            Serial.print("RI: ");
            Serial.print(mfg, DEC);
            Serial.print("-");
            Serial.print(rfid, DEC);
            Serial.print("->");
            Serial.println(scout, DEC);
            /*
            Serial.print("used/free/large: ");
            Serial.print(getMemoryUsed(), DEC);
            Serial.print("/");
            Serial.print(getFreeMemory(), DEC);
            Serial.print("/");
            Serial.println(getLargestAvailableMemoryBlock(), DEC);
            Serial.print("#NWK-Frmaes: ");
            Serial.println(countNwkFrames(), DEC);
            */
            break;        
        }
    }
}
int countNwkFrames() {
    NwkFrame_t *frame = NULL;
    int count = 0;
    while (NULL != (frame = nwkFrameNext(frame))) {
        ++count;
    }
    return count;
}

void onRFID_1(bool enter, word scout) {
    if (!enter)
        return;
        
    if (STAGE_STOP.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;

        if (STAGE_AUTO.getPos() == Z21_Accessory_Pos::P1 && !TAURUS.isForward())
            Processor.start(&MOGUL_TAURUS);
        else
            Processor.start(&MOGUL_STOP);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
            
        Processor.start(&TAURUS_SLOW_DOWN);
        return;
    }
}               
void onRFID_2_5(bool enter, word scout) {
    if (!enter)
        return;

    if (STAGE_STOP.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;
        
        Processor.start(&MOGUL_SLOW);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
            
        Processor.start(&TAURUS_SLOW);
        return;
    }
}

void onRFID_3(bool enter, word scout) {
    if (!enter)
        return;

    if (STAGE_STOP.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;

        Processor.start(&MOGUL_SLOW_DOWN);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;

        Processor.start(&TAURUS_STOP);
        return;
    }
}
void onRFID_4(bool enter, word scout) {
    if (!enter)
        return;

    if (STAGE_STOP.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;

        Processor.start(&MOGUL_STOP);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;

        Processor.start(&TAURUS_SLOW_DOWN);
        return;
    }
}

void onRFID_6(bool enter, word scout) {
    if (!enter)
        return;

    if (STAGE_STOP.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;

        Processor.start(&MOGUL_SLOW_DOWN);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;

        if (STAGE_AUTO.getPos() == Z21_Accessory_Pos::P1 && MOGUL.isForward())  
            Processor.start(&TAURUS_MOGUL);
        else
            Processor.start(&TAURUS_STOP);
        return;
    }
}

void onRFID_7(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;
        Processor.start(&MOGUL_SQUEAL_ON);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
            
        Processor.start(&TAURUS_GO);
        return;
    }
}

void onRFID_8(bool enter, word scout) {
    if (!enter)
        return;
    
    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;
        Processor.start(&MOGUL_APPROACH);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
            
        Processor.start(&TAURUS_SQUEAL_OFF);
        return;
    }
}

void onRFID_9(bool enter, word scout) {
    if (!enter)
        return;
    
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
        Processor.start(&TAURUS_APPROACH);
        return;
    }
}
void onRFID_10(bool enter, word scout) {
    if (!enter)
        return;

    if (STATION_AUTO.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;
        Processor.start(&MOGUL_STATION);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
        Processor.start(&TAURUS_SLOW_DOWN);            
        return;
    }
}
void onRFID_11(bool enter, word scout) {
    if (!enter)
        return;

    if (STATION_AUTO.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;
        
        Processor.start(&MOGUL_SLOW);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;

        Processor.start(&TAURUS_SLOW);
        return;
    }    
}
void onRFID_12(bool enter, word scout) {
    if (!enter)
        return;

    if (STATION_AUTO.getPos() != Z21_Accessory_Pos::P1)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;

        Processor.start(&MOGUL_SLOW_DOWN);
        return;
    }
    
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
        Processor.start(&TAURUS_STATION);            
        return;
    }
}

void onRFID_13(bool enter, word scout) {
    if (!enter)
        return;
    
    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;
        Processor.start(&MOGUL_APPROACH);
        return;
    }
}

void onRFID_14(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;
            
        Processor.start(&MOGUL_SQUEAL_OFF);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
        Processor.start(&TAURUS_APPROACH);
        return;
    }
}
void onRFID_15(bool enter, word scout) {
    if (!enter)
        return;
    
    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isForward())
            return;

        Processor.start(&MOGUL_GO);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (TAURUS.isForward())
            return;
        Processor.start(&TAURUS_SQUEAL_ON);
        return;
    }
}

