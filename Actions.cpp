#include <Z21.h>

#include "Actions.h"
#include "UDPConnection.h"

long Z21Client::start() {
    UDP.sendZ21(Z21.getStatus());
    UDP.sendZ21(Z21.getVersion());
    UDP.sendZ21(Z21.setBroadcastFlags(Z21_Broadcast::STATUS_LOCO_TURNOUT));
    return 2000; //Z21/GS doesn't respond first 2 seconds to the Info request
}

long Z21Client::progress() {
    Serial.print("Progress: #L: ");
    int count = 0 ;
    Loco** locos = Loco::getAll(count);
    Serial.print(count);
    while(count > 0)
        UDP.sendZ21(Z21.getLocoInfo(locos[--count]->getDCCAddress()));
    
    Accessory** accessories = Accessory::getAll(count);
    Serial.print(", #A: ");
    Serial.println(count);
    while(count > 0)
        UDP.sendZ21(Z21.getAccessoryInfo(accessories[--count]->getDCCAddress()));
    
    return 20000L; // Once 20 seconds
}

void Z21Client::cancel() {
}

long Z21Function::start() {
    UDP.sendZ21(Z21.setLocoFunction(loco.getDCCAddress(), function, on ? Z21_Function_Action::ON : Z21_Function_Action::OFF));
    return interval;
}

long Z21Function::progress() {
    UDP.sendZ21(Z21.setLocoFunction(loco.getDCCAddress(), function, Z21_Function_Action::OFF));
    return ACTION_COMPLETE;
}



long Z21Speed::start() {
    UDP.sendZ21(Z21.setLocoDrive(loco.getDCCAddress(), loco.isForward(), Z21_Speed_Range::STEPS_128, speed128));
    return ACTION_COMPLETE;
}


long Z21Turnout::start() {
    UDP.sendZ21(Z21.setAccessory(address, pos, true, true));
    return ACTION_COMPLETE;
}


