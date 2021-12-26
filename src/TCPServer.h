#ifndef TCP_SERVER_H
#define TCP_SERVER_H

#include <memory>

#include "Arduino.h"
#include "TCPClient.h"

class TCPServer {
   public:
    virtual void begin() = 0;
    virtual std::shared_ptr<TCPClient> accept() = 0;
    virtual void end() = 0;
};

#endif