#include "mbed.h"
#include <cstdint>
#include "logger.hpp"
#include "xbee.hpp"

#define XBEE_ADDR 0x0000000000000000

BufferedSerial pc(USBTX, USBRX, 115200);
#ifdef MBED_DEBUG
Log::Logger logger(&pc, Log::LogFrameType::DEBUG);
#else 
Log::Logger logger(&pc, Log::LogFrameType::RELEASE);
#endif

#define READ_XBEE_BUFFER_LENGTH 32
#define WRITE_XBEE_BUFFER_LENGTH 32

char read_xbee_buffer[READ_XBEE_BUFFER_LENGTH] = {0};
char write_xbee_buffer[WRITE_XBEE_BUFFER_LENGTH] = {0};
BufferedSerial xbee(PA_9, PA_10, 115200);

Thread thread;
void watchdog_thread(){
    while (true) {
        // Write all log in queue to the output stream (serial communication).
        logger.flushLogToSerial();
        ThisThread::sleep_for(10ms);
    }
}

int main()
{
    // Start watchdog thread, will flush the log queue.
    thread.start(callback(watchdog_thread));

    // Set blocking to serial connection so that its wait for input to be received.
    pc.set_blocking(true);
    logger.addLogToQueue(Log::LogFrameType::INFO, "Program started!");

    uint8_t buff[256];
    xBee::xBeeReceivePacket rcvPacket;
    while (true) {
        if(xbee.readable()) {
            xBee::xBeeReceivePacketFrame read = rcvPacket.readFromBufferedSerial(&xbee);
            
            logger.addLogToQueue(Log::LogFrameType::DEBUG, "START FRAME:");
            for (size_t i = 0; i < rcvPacket.as_bytes().len; i++)
                logger.addLogToQueue(Log::LogFrameType::DEBUG, "%02X", rcvPacket.as_bytes().msg[i]);
            logger.addLogToQueue(Log::LogFrameType::DEBUG, "END FRAME:");
        }
    }
}