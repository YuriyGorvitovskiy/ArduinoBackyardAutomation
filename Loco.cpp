#include "Loco.h"

const int  MAX_LOCO_COUNT = 10;

Loco* ALL_LOCOS[MAX_LOCO_COUNT];
int   ALL_LOCOS_COUNT = 0;

Loco::Loco(uint16_t dccAddress, uint16_t idScout) : 
    dccAddress(dccAddress), 
    idScout(idScout) {
        
    if (ALL_LOCOS_COUNT < MAX_LOCO_COUNT)        
        ALL_LOCOS[ALL_LOCOS_COUNT++] = this;
};
        
void Loco::update(boolean fwd, uint8_t speed, uint32_t func) {
    this->forward = fwd;
    this->speed128 = speed;
    this->functions = func;   
}

Loco* Loco::findByDCCAddress(uint16_t dccAddress) {
    for (int i = 0; i < ALL_LOCOS_COUNT; ++i) {
        if (ALL_LOCOS[i]->dccAddress == dccAddress)
            return ALL_LOCOS[i];
    }
    return NULL;

}

Loco* Loco::findByIDScout(uint16_t idScout) {
    for (int i = 0; i < ALL_LOCOS_COUNT; ++i) {
        if (ALL_LOCOS[i]->idScout == idScout)
            return ALL_LOCOS[i];
    }
    return NULL;
}

Loco** Loco::getAllLocos(int& count) {
    count = ALL_LOCOS_COUNT;
    return ALL_LOCOS;
}

