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

#include <Z21.h>

#include <Action.h>
#include <ActionSequence.h>
#include <ActionProcessor.h>


#include "Loco.h"
#include "Sensor.h"
#include "Actions.h"
#include "UDPConnection.h"

Loco        MOGUL(12, 2);
Loco        TAURUS(18, 3);

Z21Speed    MOGUL_STOP(MOGUL, 0);
Z21Speed    MOGUL_GO  (MOGUL, 80);

Z21Function MOGUL_OPEN_DOORS (MOGUL, Z21_Function::F6, true);
Z21Function MOGUL_CLOSE_DOORS(MOGUL, Z21_Function::F6, false);

Z21Function MOGUL_BELL       (MOGUL, Z21_Function::F1, 2000L);

Z21Function MOGUL_APPROACH   (MOGUL, Z21_Function::F3, 2000L);

Z21Function MOGUL_SOUND_ON   (MOGUL, Z21_Function::F8, true);
Z21Function MOGUL_SOUND_OFF  (MOGUL, Z21_Function::F8, false);

Z21Speed    TAURUS_STOP(TAURUS, 0);
Z21Speed    TAURUS_GO  (TAURUS, 80);

Z21Function TAURUS_OPEN_DOORS (TAURUS, Z21_Function::F6, true);
Z21Function TAURUS_CLOSE_DOORS(TAURUS, Z21_Function::F6, false);

Z21Function TAURUS_WISTLE     (TAURUS, Z21_Function::F1, 1000L);
Z21Function TAURUS_HORN       (TAURUS, Z21_Function::F2,  500L);

Z21Function TAURUS_APPROACH   (TAURUS, Z21_Function::F3, 2000L);

Z21Function TAURUS_SOUND_ON   (TAURUS, Z21_Function::F8, true);
Z21Function TAURUS_SOUND_OFF  (TAURUS, Z21_Function::F8, false);

Z21Turnout  TURNOUT_RIGHT_IN (4, 0);
Z21Turnout  TURNOUT_RIGHT_OUT(4, 1);

Z21Turnout  TURNOUT_LEFT_IN  (5, 0);
Z21Turnout  TURNOUT_LEFT_OUT (5, 1);

Delay        STATION_WAIT     (20000L);
Delay        STATION_ARRIVAL  ( 2000L);
Delay        STATION_DEPARTURE( 3000L);

Delay        HORN_REPEAT      (  500L);

Delay        SWITCH_DELAY     (  500L);

Action* MOGUL_STATION_LIST[] = { 
        &MOGUL_STOP,
        &STATION_ARRIVAL,
        &MOGUL_OPEN_DOORS,
        &STATION_WAIT,
        &MOGUL_BELL,
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
        &TAURUS_WISTLE,
        &TAURUS_CLOSE_DOORS,
        &STATION_DEPARTURE,
        &TAURUS_HORN,
        &HORN_REPEAT,
        &TAURUS_HORN,
        &TAURUS_GO,
        NULL,
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
        &TAURUS_HORN,
        &HORN_REPEAT,
        &TAURUS_HORN,
        &TAURUS_GO,
        NULL
};

ActionSequence MOGUL_STATION;
ActionSequence MOGUL_TAURUS;

ActionSequence TAURUS_STATION;
ActionSequence TAURUS_MOGUL;


const Sensor SENSORS[15] = {
    Sensor("01", 0x1e, 0x0020d85fL, onRFID_1),
    Sensor("02", 0x1e, 0x0020e180L, onRFID_2_5),
    Sensor("03", 0x1e, 0x0020e17cL, onRFID_3),
    Sensor("04", 0x1e, 0x0020bc87L, onRFID_4),
    Sensor("05", 0x1e, 0x0020f033L, onRFID_2_5),
    Sensor("06", 0x1e, 0x0020f05eL, onRFID_6),
    Sensor("07", 0x1e, 0x0020d8a4L),
    Sensor("08", 0x1e, 0x002101edL, onRFID_8),
    Sensor("09", 0x1b, 0x0033a4fdL, onRFID_9),
    Sensor("10", 0x1e, 0x0020fc21L, onRFID_10),
    Sensor("11", 0x1e, 0x0020e12cL, onRFID_11),
    Sensor("12", 0x1e, 0x0020bc71L, onRFID_12),
    Sensor("13", 0x1e, 0x00212350L, onRFID_13),
    Sensor("14", 0x1e, 0x0020aa6aL, onRFID_14),
    Sensor("15", 0x1e, 0x0020c333L)
};

const int SENSOR_COUNT = sizeof(SENSORS)/sizeof(Sensor);

Z21Client Z21_BACKGROUND;

void setup() {
    Scout.setup();
    UDP.setup();
    Processor.setup();

    MOGUL_STATION.init(MOGUL_STATION_LIST);
    TAURUS_STATION.init(TAURUS_STATION_LIST);
    MOGUL_TAURUS.init(MOGUL_TAURUS_LIST);
    TAURUS_MOGUL.init(TAURUS_MOGUL_LIST);

    UDP.onConnect = onConnect;
    Z21.onLocoInfo = onLocoInfo;        
    
    addBitlashFunction("rfid.report", (bitlash_function)RFIDreport);
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
    if (loco != NULL)
        loco->update(forward, speed, functions);
}

void onRFID(boolean enter, uint16_t scout, uint16_t mfg, uint32_t rfid) {
    Sensor s(mfg, rfid);
    for (int i = 0; i < SENSOR_COUNT; ++i) {
        if (s == SENSORS[i]) {
            SENSORS[i].onEvent(enter, scout);
            break;        
        }
    }
}

void onRFID_1(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F27))
            return; 

        if (!MOGUL.isForward())
            return;

        if (TAURUS.isFunctionON(Z21_Function::F27) && !TAURUS.isForward())
            Processor.start(&MOGUL_TAURUS);
        else
            UDP.sendZ21(Z21.setLocoDrive(MOGUL.getDCCAddress(), MOGUL.isForward(), Z21_Speed_Range::STEPS_128, 0));
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F27))
            return;
        
        if (TAURUS.isForward())
            return;
            
        UDP.sendZ21(Z21.setLocoDrive(TAURUS.getDCCAddress(), TAURUS.isForward(), Z21_Speed_Range::STEPS_128, 48));
        return;
    }
}               
void onRFID_2_5(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F27))
            return; 

        if (!MOGUL.isForward())
            return;
        
        UDP.sendZ21(Z21.setLocoDrive(MOGUL.getDCCAddress(), MOGUL.isForward(), Z21_Speed_Range::STEPS_128, 24));
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F27))
            return;
        
        if (TAURUS.isForward())
            return;
            
        UDP.sendZ21(Z21.setLocoDrive(TAURUS.getDCCAddress(), TAURUS.isForward(), Z21_Speed_Range::STEPS_128, 24));
        return;
    }
}

void onRFID_3(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F27))
            return; 

        if (!MOGUL.isForward())
            return;
        
        UDP.sendZ21(Z21.setLocoDrive(MOGUL.getDCCAddress(), MOGUL.isForward(), Z21_Speed_Range::STEPS_128, 48));
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F27))
            return;
        
        if (TAURUS.isForward())
            return;
            
        UDP.sendZ21(Z21.setLocoDrive(TAURUS.getDCCAddress(), TAURUS.isForward(), Z21_Speed_Range::STEPS_128, 0));
        return;
    }
}
void onRFID_4(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F27))
            return; 

        if (!MOGUL.isForward())
            return;
        
        UDP.sendZ21(Z21.setLocoDrive(MOGUL.getDCCAddress(), MOGUL.isForward(), Z21_Speed_Range::STEPS_128, 0));
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F27))
            return;
        
        if (TAURUS.isForward())
            return;
            
        UDP.sendZ21(Z21.setLocoDrive(TAURUS.getDCCAddress(), TAURUS.isForward(), Z21_Speed_Range::STEPS_128, 48));
        return;
    }
}

void onRFID_6(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F27))
            return; 

        if (!MOGUL.isForward())
            return;

        UDP.sendZ21(Z21.setLocoDrive(MOGUL.getDCCAddress(), MOGUL.isForward(), Z21_Speed_Range::STEPS_128, 48));
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F27))
            return;
        
        if (TAURUS.isForward())
            return;

        if (MOGUL.isFunctionON(Z21_Function::F27) && MOGUL.isForward())  
            Processor.start(&TAURUS_MOGUL);
        else
            UDP.sendZ21(Z21.setLocoDrive(TAURUS.getDCCAddress(), TAURUS.isForward(), Z21_Speed_Range::STEPS_128, 0));
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
}

void onRFID_9(bool enter, word scout) {
    if (!enter)
        return;
    
    if (scout == TAURUS.getIDScout()) {
        if (!MOGUL.isForward())
            return;
        Processor.start(&TAURUS_APPROACH);
        return;
    }
}
void onRFID_10(bool enter, word scout) {
    if (!enter)
        return;
    
    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F26))
            return; 
        if (!MOGUL.isForward())
            return;
        Processor.start(&MOGUL_STATION);
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F26))
            return;
        if (TAURUS.isForward())
            return;
        UDP.sendZ21(Z21.setLocoDrive(TAURUS.getDCCAddress(), TAURUS.isForward(), Z21_Speed_Range::STEPS_128, 48));
        return;
    }
}
void onRFID_11(bool enter, word scout) {
    if (!enter)
        return;

    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F26))
            return; 

        if (!MOGUL.isForward())
            return;
        
        UDP.sendZ21(Z21.setLocoDrive(MOGUL.getDCCAddress(), MOGUL.isForward(), Z21_Speed_Range::STEPS_128, 24));
        return;
    }
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F26))
            return;
        
        if (TAURUS.isForward())
            return;
            
        UDP.sendZ21(Z21.setLocoDrive(TAURUS.getDCCAddress(), TAURUS.isForward(), Z21_Speed_Range::STEPS_128, 24));
        return;
    }    
}
void onRFID_12(bool enter, word scout) {
    if (!enter)
        return;
    
    if (scout == MOGUL.getIDScout()) {
        if (!MOGUL.isFunctionON(Z21_Function::F26))
            return; 
        if (!MOGUL.isForward())
            return;
        UDP.sendZ21(Z21.setLocoDrive(MOGUL.getDCCAddress(), MOGUL.isForward(), Z21_Speed_Range::STEPS_128, 48));
        return;
    }
    
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isFunctionON(Z21_Function::F26))
            return;
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
    
    if (scout == TAURUS.getIDScout()) {
        if (!TAURUS.isForward())
            return;
        Processor.start(&TAURUS_APPROACH);
        return;
    }
}

