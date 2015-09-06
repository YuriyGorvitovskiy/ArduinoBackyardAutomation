#ifndef __ACTIONS_H__
#define __ACTIONS_H__

#include <Action.h>
#include <Z21.h>
#include "Loco.h"
#include "Accessory.h"

const int  Z21_ACTION_ATTEMPTS    = 3;    
const long Z21_ACTION_CHECK_DELAY = 200L; //200ms


class Z21Client : public Action {
public:    
    virtual long start();
    virtual long progress();
    virtual void cancel();
};

class Z21Speed : public RedundantSingleAction {
public:
    Z21Speed(Loco& loco, byte speed128) : RedundantSingleAction(Z21_ACTION_ATTEMPTS, Z21_ACTION_CHECK_DELAY), loco(loco), speed128(speed128) {}

    virtual void execute();
    virtual boolean check();

protected:

    Loco&   loco;
    byte    speed128;
};

class Z21Function : public RedundantSingleAction {
public:
    // Complete immidiatly
    Z21Function(Loco& loco, byte function, bool on) : RedundantSingleAction(Z21_ACTION_ATTEMPTS, Z21_ACTION_CHECK_DELAY), loco(loco), function(function), on(on) {}

    virtual void execute();
    virtual boolean check();

protected:

    Loco&   loco;
    byte    function;
    bool    on;
};


class Z21Turnout : public RedundantSingleAction {
public:
    Z21Turnout(Accessory& accessory, byte pos) : RedundantSingleAction(Z21_ACTION_ATTEMPTS, Z21_ACTION_CHECK_DELAY), accessory(accessory), pos(pos) {}

    virtual void execute();
    virtual boolean check();

protected:

    Accessory&  accessory;
    byte        pos;
};


#endif //__ACTIONS_H__

