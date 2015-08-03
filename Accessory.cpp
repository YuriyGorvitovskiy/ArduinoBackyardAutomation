#include "Accessory.h"

const int  MAX_ACCESSORY_COUNT = 10;

Accessory*  ALL_ACCESSORIES[MAX_ACCESSORY_COUNT];
int         ALL_ACCESSORIES_COUNT = 0;

void Accessory::init() {
    if (ALL_ACCESSORIES_COUNT < MAX_ACCESSORY_COUNT)        
        ALL_ACCESSORIES[ALL_ACCESSORIES_COUNT++] = this;
}
        
void Accessory::update(uint8_t pos) {
    this->pos = pos;
}

Accessory* Accessory::findByDCCAddress(uint16_t dccAddress) {
    for (int i = 0; i < ALL_ACCESSORIES_COUNT; ++i) {
        if (ALL_ACCESSORIES[i]->dccAddress == dccAddress)
            return ALL_ACCESSORIES[i];
    }
    return NULL;

}

Accessory** Accessory::getAll(int& count) {
    count = ALL_ACCESSORIES_COUNT;
    return ALL_ACCESSORIES;
}

