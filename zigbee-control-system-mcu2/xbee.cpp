#include "xbee.hpp"
#include <cstdint>
#include <cstring>

using namespace xBee;

xBeeMessage::xBeeMessage(uint8_t type): type(type) {};
uint8_t xBeeMessage::getType() { return this->type; };

xBeeReceivePacket::xBeeReceivePacket(): xBeeMessage(0x90) {};

void uint16SliceIntoArray(uint8_t* arr, uint16_t startIdx, uint16_t val) {
    for(uint8_t i = 0; i < 2; i++) {
        arr[startIdx + i] = val << (8*i);
    }
}

xBeeMessageBytesBuffer xBeeReceivePacket::as_bytes() {
    xBeeMessageBytesBuffer buf;
    buf.msg = &this->msg.front();
    buf.len = this->msg.size();
    return buf;
}

xBeeReceivePacketFrame xBeeReceivePacket::readFromBufferedSerial(BufferedSerial* sourceBuffer) {
    Log::Logger::getInstance()->addLogToQueue(Log::LogFrameType::INFO, "Start reading...");
    xBeeReceivePacketFrame rcv_packet;
    const uint8_t CHUNK_SIZE = 64;
    uint8_t chunk[CHUNK_SIZE] = {0x00};

    // Fetching and storing complete buffer for later
    this->msg.clear();
    while (sourceBuffer->readable()) {
        uint8_t read_len = sourceBuffer->read(&chunk, CHUNK_SIZE);
        this->msg.insert(this->msg.end(), chunk, chunk + read_len);
        ThisThread::sleep_for(1ms);
    }

    size_t current_byte = 0;
    if (this->msg[current_byte++] != 0x7E) {
        Log::Logger::getInstance()->addLogToQueue(Log::LogFrameType::ERROR, "Expect 0x7E as frame start!");
        return rcv_packet;
    }
    
    uint16_t frame_length = this->msg[current_byte] << 8 | this->msg[current_byte+1];
    current_byte += 2;
    if (frame_length < 12) {
        Log::Logger::getInstance()->addLogToQueue(Log::LogFrameType::ERROR, "Frame length is %d, length below 12 are suspicious!", frame_length);
        return rcv_packet;
    }
    
    uint16_t payload_size = frame_length - 12;
    
    if (this->msg[current_byte++] != 0x90) {
        Log::Logger::getInstance()->addLogToQueue(Log::LogFrameType::ERROR, "Expect 0x90 as frame type, got %02X!", this->msg[current_byte-1]);
        return rcv_packet;
    }

    for (int8_t i = 7; i >= 0; i--) {
        rcv_packet.src += this->msg[current_byte++] << 8*i;
    }

    rcv_packet.srcShortAddr = this->msg[current_byte] << 8 | this->msg[current_byte+1];
    current_byte += 2;

    rcv_packet.option = this->msg[current_byte++];

    rcv_packet.msg = new uint8_t[payload_size];
    rcv_packet.len = payload_size;
    memcpy(rcv_packet.msg, &this->msg.front() + current_byte, payload_size);
    current_byte += payload_size;

    // Check checksum is the same
    uint8_t checksum = 0x00;
    for (uint16_t i = 3; i < this->msg.size()-1; i++) {
        checksum += this->msg[i];
    }
    checksum = 0xFF - checksum;
    if (this->msg.back() != checksum) {
        Log::Logger::getInstance()->addLogToQueue(Log::LogFrameType::ERROR, "Compute checksum is %d, which is different from received frame!", checksum);
    }
    
    return rcv_packet;
}