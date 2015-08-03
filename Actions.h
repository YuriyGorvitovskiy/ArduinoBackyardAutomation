#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#include <Action.h>
#include <Z21.h>
#include "Loco.h"
#include "Accessory.h"

class Z21Client : public Action {
public:    
    virtual long start();
    virtual long progress();
    virtual void cancel();
};

class Z21Speed : public SingleAction {
public:
    Z21Speed(Loco& loco, byte speed128) : loco(loco), speed128(speed128) {}
    virtual long start();

protected:

    Loco&   loco;
    byte    speed128;
};

class Z21Function : public SingleAction {
public:
    // Complete immidiatly
    Z21Function(Loco& loco, byte function, bool on) : 
        loco(loco), function(function), on(on), interval(ACTION_COMPLETE) {}

    // Complete turn Function on, wait interval, and then turn off
    Z21Function(Loco& loco, byte function, long interval) : 
        loco(loco), function(function), on(true), interval(interval) {}

    virtual long start();
    virtual long progress();

protected:

    Loco&   loco;
    byte    function;
    bool    on;
    long    interval;
};

class Z21Turnout : public SingleAction {
public:
    Z21Turnout(word address, byte pos) : address(address), pos(pos) {}
    virtual long start();

protected:

    word address;
    byte pos;
};


#endif //__ACTIONS_H__

