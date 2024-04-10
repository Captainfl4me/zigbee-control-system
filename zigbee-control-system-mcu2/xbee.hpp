#pragma once
#include "logger.hpp"
#include <cstdint>
#include <vector>

#define BUFFSIZE 64

namespace xBee {
    struct xBeeMessageBytesBuffer {
        uint16_t len;
        uint8_t* msg;
    };
    struct xBeeReceivePacketFrame {
        uint64_t src;
        uint16_t srcShortAddr;
        uint8_t option;
        uint8_t *msg;
        uint16_t len;
    };

    class xBeeMessage {
    private:
        uint8_t type;
    public:
        xBeeMessage(uint8_t type);
        virtual xBeeMessageBytesBuffer as_bytes() = 0;
        uint8_t getType();
    };
    
    class xBeeReceivePacket: protected xBeeMessage {
    private:
        std::vector<uint8_t> msg;

    public:
        xBeeReceivePacket();
        xBeeReceivePacketFrame readFromBufferedSerial(BufferedSerial* sourceBuffer);
        xBeeMessageBytesBuffer as_bytes();
    };
}