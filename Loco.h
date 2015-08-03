#ifndef __LOCO_H__
#define __LOCO_H__

#include <Arduino.h>

class Loco {
    public:
        Loco(uint16_t dccAddress, uint16_t idScout) : 
            dccAddress(dccAddress), 
            idScout(idScout),
            forward(false),
            speed128(0),
            functions(0) {};

        void init();
        
        void update(boolean forward, uint8_t speed128, uint32_t functions);
        
        uint16_t getDCCAddress()            {return dccAddress;};
        uint16_t getIDScout()               {return idScout;}
        
        boolean isForward()                  {return forward;}
        uint8_t getSpeed128()                {return speed128;}
        boolean isFunctionON(uint8_t func)   {return (functions & (1L << func)) != 0;}

    public:
        static Loco* findByDCCAddress(uint16_t dccAddress);
        static Loco* findByIDScout(uint16_t idScout);

        static Loco** getAll(int& count);
        

    protected:
    
        uint16_t    dccAddress;
        uint16_t    idScout;
        
        boolean     forward;
        uint8_t     speed128;
        uint32_t    functions;
};

#endif //__LOCO_H__
