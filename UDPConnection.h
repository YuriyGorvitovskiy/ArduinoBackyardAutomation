#ifndef __UDP_CONNECTION_H__
#define __UDP_CONNECTION_H__

#include <GS.h>
#include <Z21.h>

class UDPConnection {
public: 

    void setup();
    void loop();

    boolean isConnected();

    void (*onConnect)();

    void sendZ21(Z21Packet& packet);
    void sendMac(Z21Packet& packet);

    void onWriteError();

protected:

    GSUdpServer* server;
    boolean      connected;

    byte         rx_buffer[Z21Packet::MAX_PACKET_SIZE + 10]; // in case
    int          packet_size;
    byte*        read_to;
    int          left_to_read;
};

extern UDPConnection UDP;


#endif //__Z21_CONNECTION_H__

