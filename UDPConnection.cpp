#include <Arduino.h>
#include <Scout.h>
#include <GS.h>

#include <backpacks/wifi/WifiModule.h>

#include "UDPConnection.h"

const int          SERVER_PORT = 21105;

const IPAddress    Z21_IP(192, 168, 0, 111);
const int          Z21_PORT = 21105;

const IPAddress    MAC_BOOK_IP(192, 168, 0, 98);
const int          MAC_BOOK_PORT = 21105;

UDPConnection UDP;



void UDPConnection::setup() {
    connected == false;
    server == NULL;
    read_to = rx_buffer;
    packet_size = 0;
    left_to_read = 0;

    bp = pinoccio::WifiModule::instance.bp();
    if (bp == NULL) {
        Serial.println("No Backpack.");   
        return;
    }
    bp->disableHqConnection = true;
    bp->gs.setAutoConnectServer(SERVER_PORT, GSModule::GS_UDP);
    bp->gs.setNcm(true, true, false, GSModule::GS_NCM_STATION);        

    server = new GSUdpServer(bp->gs);
    
}

void UDPConnection::loop() {
    if (server == NULL)
        return;

    if (!connected) {
        Led.blue();
        connected = server->begin(SERVER_PORT);
        if (!connected)
            return;

        Led.blinkGreen();
        Serial.print("UDP Connected. CID: ");   
        Serial.println(server->cid);   
            
        if (onConnect != NULL)            
            onConnect();    
    }

    if (!connected)
        return;

    if (left_to_read <= 0) {
        left_to_read = server->parsePacket();
        
        if (left_to_read <= 0) 
            return;

        if (left_to_read < Z21Packet::MIN_PACKET_SIZE || left_to_read > Z21Packet::MAX_PACKET_SIZE)
            left_to_read = 0; // skip this packet

        if (left_to_read <= 0) 
            return;

        packet_size = left_to_read;
        read_to = rx_buffer;
    }
    
    int read = server->read(read_to, left_to_read);
    left_to_read -= read;
    read_to += read;

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

boolean UDPConnection::isConnected() {
    return connected;
}

void UDPConnection::sendZ21(Z21Packet& packet) {
    if (!connected)
        return;

//bypass realloc/free code
//change library to expose GSUdpServer::gs, GSUdpServer::cid
    if (!server->gs.writeData(server->cid, Z21_IP, Z21_PORT, packet.data, packet.length))
        onWriteError();
}

void UDPConnection::sendMac(Z21Packet& packet) {
    if (!connected)
        return;

//bypass realloc/free code
//change library to expose GSUdpServer::gs, GSUdpServer::cid
    if (!server->gs.writeData(server->cid, MAC_BOOK_IP, MAC_BOOK_PORT, packet.data, packet.length))
        onWriteError();
}

void UDPConnection::onWriteError() {
    Led.blue();
    if (server->gs.unrecoverableError) {
        Serial.print("UDP Unrecoverable error. CID: "); 
        Serial.println(server->cid); 
        return;
    }
    if (GSCore::MAX_CID == server->cid) {
        Serial.print("UDP Unrecoverable error. CID reach the limit: "); 
        Serial.println(server->cid); 
        server->gs.unrecoverableError = true;
        return;
    }

    if (!server->gs.isAssociated()) {
        Serial.println("WIFI is not associated.");
        Serial.println("WIFI re-associated.");
        server->stop(); 
        connected = false;
        bp->associate();
        Led.blue();
        return;
    }
        
    if (!server->gs.getConnectionInfo(server->cid).connected) {
        Serial.print("UDP server is not connected: "); 
    } else if (server->gs.getConnectionInfo(server->cid).error) {
        Serial.print("UDP server connection has error: "); 
        Serial.println(server->cid); 
    } else {
        Serial.print("UDP communication error: "); 
    }
    Serial.println(server->cid); 

    Serial.println("UDP server reconnecting."); 
    Led.blue();
    server->stop();
    connected = false;
}

