#include "mbed.h"
#include <cstdint>
#include "logger.hpp"
#include "xbee.hpp"

#define XBEE_ADDR 0x0013A20041B109B7

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

AnalogIn ain(A0);

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

    uint8_t payload[] = {0x00};
    xBee::xBeeTransmitRequest req(XBEE_ADDR, 0xfffe, NULL, 0);
    while (true) {

        float analog_in = ain.read();
        logger.addLogToQueue(Log::LogFrameType::DEBUG, "Analog value: %.5f%%", analog_in);

        if (analog_in <= 0.30) {
            payload[0] = 0x01;
        } else if (analog_in <= 0.70) {
            payload[0] = 0x02;
        } else {
            payload[0] = 0x03;
        }

        logger.addLogToQueue(Log::LogFrameType::DEBUG, "Payload value: %d", payload[0]);

        req.updateMsg(payload, 1);
        xBee::xBeeMessageBytesBuffer frame = req.as_bytes();
        xbee.write(frame.msg, frame.len);
        delete frame.msg;
        ThisThread::sleep_for(20ms);
    }
}

