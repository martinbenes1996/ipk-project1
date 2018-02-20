#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <cstdio>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

class MySocket
{
  public:
    MySocket(std::string address, int port)
    {
      msocket = socket(AF_INET, SOCK_STREAM, 0);
      if(msocket <= 0)
        throw std::runtime_error("Opening a socket failed");

      struct sockaddr_in addr = getAddr(address, port);
      if( connect( msocket, /*(const struct sockaddr *)*/&addr, address.size() ) != 0)
        throw std::runtime_error("Connecting to an address failed");
    }

    struct sockaddr_in getAddr(std::string address, int port)
    {
      const char * host = address.c_str();
      struct sockaddr_in result{};

      struct hostent * server;
      if ( (server = gethostbyname(host)) == NULL)
        throw std::runtime_error("Unknown host");

      bzero((char *)&result, sizeof(result));
      result.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&result.sin_addr.s_addr, server->h_length);
      result.sin_port = htons(port);

      return result;
    }

    void Send(std::string s)
    {
      if( send(msocket, s.c_str(), s.size(), 0) < 0)
        throw std::runtime_error("Sending a file failed");
    }

    ~MySocket()
    {
      close(msocket);
    }

  private:
    int msocket = -1;
};


#endif // MYSOCKET_H
