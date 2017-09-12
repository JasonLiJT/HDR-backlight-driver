#ifndef SERIALCLASS_H_INCLUDED
#define SERIALCLASS_H_INCLUDED

#define ARDUINO_WAIT_TIME 2000

#include <windows.h>
#include <stdio.h>
#include <stdlib.h>

class SerialPortWindows {
   private:
    //Serial comm handler
    HANDLE hSerial;
    //Connection status
    bool connected;
    //Get various information about the connection
    COMSTAT status;
    //Keep track of last error
    DWORD errors;

   public:
    //Initialize Serial communication with the given COM port
    auto serialport_init(const char *serialport = "\\\\.\\COM4", int baud = 9600);

    //Close the connection
    void serialport_close(HANDLE placeholder);

    //Read data in a buffer, if nbChar is greater than the
    //maximum number of bytes available, it will return only the
    //bytes available. The function return -1 when nothing could
    //be read, the number of bytes actually read.
    int ReadData(char *buffer, unsigned int nbChar);
    int serialport_readByte(auto placeholder, auto timeout);
    //Writes data from a buffer through the Serial connection
    //return true on success.
    bool WriteData(const char *buffer, unsigned int nbChar);
    bool serialport_write(auto placeholder, const char *buffer);
    bool serialport_writeBuffer(auto placeholder, const uint8_t *buffer, int len);
    bool serialport_writebyte(auto placeholder, uint8_t byte_to_send);
    //Check if we are actually connected
    bool IsConnected();
};

#endif  // SERIALCLASS_H_INCLUDED