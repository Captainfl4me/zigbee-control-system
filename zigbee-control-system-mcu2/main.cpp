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

PwmOut servo(D9);

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
    servo.period_ms(20);
    servo.pulsewidth_ms(1);

    xbee.set_blocking(false);
    logger.addLogToQueue(Log::LogFrameType::INFO, "Program started!");

    uint8_t buff[256];
    xBee::xBeeReceivePacket rcvPacket;
    while (true) {
        if(xbee.readable()) {
            xBee::xBeeReceivePacketFrame read = rcvPacket.readFromBufferedSerial(&xbee);
            if (read.len == 0) {
                continue;
            }

            switch (read.msg[0]) {
                case 0x01:
                    servo.pulsewidth_ms(2);
                break;
                case 0x02:
                    servo.pulsewidth_us(1500);
                break;
                default:
                    servo.pulsewidth_ms(1);
            }
        }
    }
}