#include "xbee.hpp"
#include <cstdint>

using namespace xBee;

xBeeMessage::xBeeMessage(uint8_t type): type(type) {};
uint8_t xBeeMessage::getType() { return this->type; };

xBeeTransmitRequest::xBeeTransmitRequest(uint64_t dest, uint16_t destShortAddr, uint8_t *msg, uint16_t len): xBeeMessage(0x10), dest(dest), destShortAddr(destShortAddr), msg(msg), len(len) {};

void uint16SliceIntoArray(uint8_t* arr, uint16_t startIdx, uint16_t val) {
    for(uint8_t i = 0; i < 2; i++) {
        arr[startIdx + i] = val << (8*i);
    }
}

xBeeMessageBytesBuffer xBeeTransmitRequest::as_bytes() {
    uint16_t xbeeMsgLen = 18+len;
    uint8_t* xbeeMsg = new uint8_t[xbeeMsgLen]; // 17 bytes for headers + len + checksum byte
    std::memset(xbeeMsg, 0, xbeeMsgLen);

    // startByte
    xbeeMsg[0] = 0x7E;
    // msgLen
    uint16_t xbeeMsgLenInside= xbeeMsgLen -4;
    uint8_t* msgLen8Arr = static_cast<uint8_t*>(static_cast<void*>(&xbeeMsgLenInside));
    std::reverse(msgLen8Arr, msgLen8Arr+2);
    std::memcpy(xbeeMsg+1, msgLen8Arr, 2);
    // type
    xbeeMsg[3] = this->getType();
    // FrameID
    xbeeMsg[4] = 0x01;
    // destAddr
    uint64_t dest = this->dest;
    msgLen8Arr = static_cast<uint8_t*>(static_cast<void*>(&dest));
    std::reverse(msgLen8Arr, msgLen8Arr+8);
    std::memcpy(xbeeMsg+5, msgLen8Arr, 8);
    // destAdd2
    uint16_t destShortAddr = this->destShortAddr;
    msgLen8Arr = static_cast<uint8_t*>(static_cast<void*>(&destShortAddr));
    std::reverse(msgLen8Arr, msgLen8Arr+2);
    std::memcpy(xbeeMsg+13, msgLen8Arr, 2);
    // broadcastRad
    xbeeMsg[15] = 0x00;
    // options
    xbeeMsg[16] = 0x30; // Enable APS + Secure Session Encryption

    // Copy msg
    if (len > 0) {
        std::memcpy(xbeeMsg+17, msg, len);
    }

    // Compute checksum
    for (uint16_t i = 3; i < xbeeMsgLen-1; i++) {
        xbeeMsg[xbeeMsgLen-1] += xbeeMsg[i];
    }
    xbeeMsg[xbeeMsgLen-1] = 0xFF - xbeeMsg[xbeeMsgLen-1];

    return xBeeMessageBytesBuffer {
        .len = xbeeMsgLen,
        .msg = xbeeMsg
    };
}