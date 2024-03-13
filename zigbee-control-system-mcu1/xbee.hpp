#pragma once
#include "logger.hpp"

#define BUFFSIZE 64

namespace xBee {
    struct xBeeMessageBytesBuffer {
        uint16_t len;
        uint8_t* msg;
    };

    class xBeeMessage {
    private:
        uint8_t type;
    public:
        xBeeMessage(uint8_t type);
        virtual xBeeMessageBytesBuffer as_bytes() = 0;
        uint8_t getType();
    };

    class xBeeTransmitRequest: protected xBeeMessage {
    private:
        uint64_t dest;
        uint16_t destShortAddr;
        uint8_t *msg;
        uint16_t len;

    public:
        xBeeTransmitRequest(uint64_t dest, uint16_t destShortAddr, uint8_t *msg, uint16_t len);

        xBeeMessageBytesBuffer as_bytes();
    };
}