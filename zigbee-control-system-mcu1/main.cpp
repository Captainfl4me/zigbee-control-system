#include "mbed.h"
#include <cstdint>
#include "logger.hpp"
#include "xbee.hpp"

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

// void BuildMessage(char *xbeeMsg, char *msg, int len)
// {
//     char startByte[]        = {0x7E};
//     char msgLen[]           = {0x00, 0x0E};
//     char type[]             = {0x10};
//     char frameID[]          = {0x01};
//     char destAddr[]         = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
//     char destAdd2[]         = {0xFF, 0xFE};
//     char broadcastRad[]     = {0x00};
//     char options[]          = {0x00};

//     int templen = 0;
//     int finalLen = 17+len;
//     int temp = 0;
//     char temp2[1] = {0};
//     char _xbeeMsg[BUFFSIZE] = {0};
    
//     //Calculate checksum
//     for(int i=0;i<msgLen[1];i++)
//     {
//         temp += ChksmPreamble[i];
//     }
//     for(int i=0;i<len;i++)
//     {
//         temp += msg[i];
//     }
//     msgLen[1] += len;
//     temp2[0] = (0xFF-temp) & 0xFF;

//     finalLen = msgLen[1]+4;                                                     //Calculate full message length
//                                        //Write the checksum to USB UART

//     //Prepare full Xbee message frame
//     char _xbeeMsg2[finalLen];
//     _xbeeMsg2[0] = startByte[0];
//     _xbeeMsg2[1] = msgLen[0];
//     _xbeeMsg2[2] = msgLen[1];
//     _xbeeMsg2[3] = type[0];
//     _xbeeMsg2[4] = frameID[0];
//     _xbeeMsg2[5] = destAddr[0];
//     _xbeeMsg2[6] = destAddr[1];
//     _xbeeMsg2[7] = destAddr[2];
//     _xbeeMsg2[8] = destAddr[3];
//     _xbeeMsg2[9] = destAddr[4];
//     _xbeeMsg2[10] = destAddr[5];
//     _xbeeMsg2[11] = destAddr[6];
//     _xbeeMsg2[12] = destAddr[7];
//     _xbeeMsg2[13] = destAdd2[0];
//     _xbeeMsg2[14] = destAdd2[1];
//     _xbeeMsg2[15] = broadcastRad[0];
//     _xbeeMsg2[16] = options[0];
//     for(int i=0;i<len;i++)
//     {
//         _xbeeMsg2[i+17] = msg[i];
//     }
//     _xbeeMsg2[finalLen-1] = temp2[0];

//     xbee.write(_xbeeMsg2, finalLen);                                //Send to xbee
    

    // length = snprintf(buffer, BUFFSIZE, "Hex: ");
    // pc.write(buffer, length);
    // for(int i=0;i<finalLen;i++)
    // {
    //     length = snprintf(buffer, BUFFSIZE, "%#x,", _xbeeMsg2[i]);
    //     pc.write(buffer, length);
    // }
// }

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

    uint8_t payload[] = {0xab};
    xBee::xBeeTransmitRequest req(0x0013A20041B109B7, 0xfffe, payload, 1);
    while (true) {
        xBee::xBeeMessageBytesBuffer frame = req.as_bytes();
        xbee.write(frame.msg, frame.len);
        ThisThread::sleep_for(1s);
    }
}

