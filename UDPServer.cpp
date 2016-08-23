#include <Arduino.h>
#include <Scout.h>
#include <GS.h>

#include <backpacks/wifi/WifiModule.h>

#include "UDPServer.h"

const int          SERVER_PORT = 21105;

const IPAddress    Z21_IP(192, 168, 0, 111);
const int          Z21_PORT = 21105;

const IPAddress    MAC_BOOK_IP(192, 168, 0, 98);
const int          MAC_BOOK_PORT = 21105;

UDPServer UDP;

static void onAssociate(void *data) {
    Serial.println("WiFi associated.");
}

static void onDisassociate(void *data) {
    ((UDPServer*)data)->onGSDisconnected();
    Serial.println("WiFi disassociated.");
    Led.blue();
}

static void onNcmConnect(void *data, GSCore::cid_t cid) {
    Serial.print("UDP Server connected. CID: ");
    Serial.println(cid);
    ((UDPServer*)data)->onGSConnected(cid);
    Led.turnOff();
}

static void onNcmDisconnect(void *data) {
    ((UDPServer*)data)->onGSDisconnected();
    Serial.println("UDP Server disconnected.");
    Led.blue();
}

UDPServer::UDPServer() : gs(NULL){
    clear();
}
void UDPServer::setup() {
    clear();

    pinoccio::WifiBackpack* bp = pinoccio::WifiModule::instance.bp();
    if (bp == NULL) {
        Serial.println("No Backpack.");
        return;
    }
    bp->disableHqConnection = true;
    gs = &(bp->gs);

    gs->onNcmConnect = onNcmConnect;
    gs->onNcmDisconnect = onNcmDisconnect;
    gs->onAssociate = onAssociate;
    gs->onDisassociate = onDisassociate;
    gs->eventData = this;

    gs->setAutoConnectServer(SERVER_PORT, GSModule::GS_UDP);
    // When association fails, keep retrying indefinately (at least it
    // seems that a retry count of 0 means that, even though the
    // documentation says it should be >= 1).
    gs->setNcmParam(GSModule::GS_NCM_L3_CONNECT_RETRY_COUNT, 0);
    gs->setNcm(true, false, false, GSModule::GS_NCM_STATION);
}

void UDPServer::loop() {
    if (!isConnected()) {
        Led.blue();
        return;
    }

    if (left_to_read <= 0) {
        left_to_read = parsePacket();

        if (left_to_read <= 0)
            return;

        if (left_to_read < Z21Packet::MIN_PACKET_SIZE || left_to_read > Z21Packet::MAX_PACKET_SIZE)
            left_to_read = 0; // skip this packet

        if (left_to_read <= 0)
            return;

        packet_size = left_to_read;
        read_to = rx_buffer;
    }

    int count = read(read_to, left_to_read);
    left_to_read -= count;
    read_to += count;

    if (left_to_read > 0)
        return;

    Led.blinkGreen();
    // Could be multiple packets
    byte* z21packet = rx_buffer;
    while(packet_size >= Z21Packet::MIN_PACKET_SIZE) {
        Z21.processPacket(z21packet);
        word len = Z21Packet::getLength(z21packet);
        packet_size -= len;
        z21packet += len;
    }
}

void UDPServer::onGSConnected(GSCore::cid_t cid) {
    clear();
    this->cid = cid;
    if (this->onConnect)
        this->onConnect();
}

void UDPServer::onGSDisconnected() {
    clear();
}
void UDPServer::clear() {
    this->cid = GSCore::INVALID_CID;
    this->rx_frame.length = 0;
    this->read_to = rx_buffer;
    this->packet_size = 0;
    this->left_to_read = 0;
}

int UDPServer::parsePacket() {
    // If there are still bytes pending from the previous packet, drop
    // them. If not all of them are directly available, don't block but
    // instead leave the rest and return no valid packet yet, our caller
    // will probably retry with another parsePacket call which will
    // continue dropping bytes.
    while(rx_frame.length) {
        if (gs->readData(cid) < 0)
              return 0;
        --rx_frame.length;
    }

    rx_frame = gs->getFrameHeader(this->cid);
        
    return rx_frame.length;
}

int UDPServer::read(uint8_t *buf, size_t size) {
    if (!rx_frame.length)
        return 0;
    
    size_t read = gs->readData(cid, buf, size);
    rx_frame.length -= read;
    return read;
}

void UDPServer::sendZ21(Z21Packet& packet) {
    if (!isConnected())
        return;

    if (!gs->writeData(cid, Z21_IP, Z21_PORT, packet.data, packet.length))
        onWriteError();
}

void UDPServer::sendMac(Z21Packet& packet) {
    if (!isConnected())
        return;

    if (!gs->writeData(cid, MAC_BOOK_IP, MAC_BOOK_PORT, packet.data, packet.length))
        onWriteError();
}

void UDPServer::onWriteError() {
    Led.red();
    if (gs->unrecoverableError) {
        // WifiModule::loop() will take care of reboot
        Serial.print("UDP Unrecoverable error. CID: ");
        Serial.println(cid);
        return;
    }

    if (GSCore::MAX_CID == cid) {
        // WifiModule::loop() will take care of reboot
        Serial.print("UDP Unrecoverable error. CID reach the limit: ");
        Serial.println(cid);
        gs->unrecoverableError = true;
        return;
    }

    if (!gs->isAssociated()) {
        // GS NCM auto-associate should take care of this.
        Serial.println("WIFI is not associated.");
        Serial.println("WIFI re-associated.");

        return;
    }

    // GS NCM auto-reconnect should take care of this.
    if (!gs->getConnectionInfo(cid).connected) {
        Serial.print("UDP server is not connected: ");
    } else if (gs->getConnectionInfo(cid).error) {
        Serial.print("UDP server connection has error: ");
        Serial.println(cid);
    } else {
        Serial.print("UDP communication error: ");
    }
}
