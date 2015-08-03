#ifndef __ACCESSORY_H__
#define __ACCESSORY_H__

#include <Arduino.h>

class Accessory {
    public:
        Accessory(uint16_t dccAddress) : 
            dccAddress(dccAddress),
            pos(0){}
            
        void init();
        
        void update(uint8_t pos);
        
        uint16_t getDCCAddress()  {return dccAddress;};
        uint8_t  getPos()         {return pos;}

    public:
        static Accessory*  findByDCCAddress(uint16_t dccAddress);
        static Accessory** getAll(int& count);
        

    protected:
    
        uint16_t    dccAddress;
        uint8_t     pos;
};

#endif //__ACCESSORY_H__
