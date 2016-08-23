#ifndef __UDP_SERVER_H__
#define __UDP_SERVER_H__

#include <GS.h>
#include <Z21.h>
#include <backpacks/wifi/WifiBackpack.h>

class UDPServer {
public:
    
    UDPServer();
    
    void setup();
    void loop();

    boolean isConnected();

    void (*onConnect)();

    void sendZ21(Z21Packet& packet);
    void sendMac(Z21Packet& packet);

    void onGSConnected(GSCore::cid_t cid);
    void onGSDisconnected();
protected:

    void onWriteError();
    int  parsePacket();
    int  read(uint8_t *buf, size_t size);
    void clear();

    GSModule*       gs;
    GSCore::cid_t   cid;
    GSCore::RXFrame rx_frame;


    byte            rx_buffer[Z21Packet::MAX_PACKET_SIZE + 10]; // in case
    int             packet_size;
    byte*           read_to;
    int             left_to_read;
};

inline boolean UDPServer::isConnected() {
  return cid != GSCore::INVALID_CID;
}

extern UDPServer UDP;

#endif //__Z21_SERVER_H__
