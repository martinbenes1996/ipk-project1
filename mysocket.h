#ifndef MYSOCKET_H
#define MYSOCKET_H

#include <climits>
#include <cmath>
#include <cstdio>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "defs.h"

class Socket
{
  public:
    Socket(int domain)
    {
      msocket = socket(domain, SOCK_STREAM, 0);
      #ifdef SOCKET_DEBUG
        std::cerr << "Create socket " << msocket << ".\n";
      #endif
      if(msocket <= 0)
        throw std::runtime_error("Opening a socket failed");
    }

    void SendMessage(NetString msg)
    {
      if(msg.getSize() >= SIZE_MAX)
        throw std::runtime_error("Message too big");

      size_t size = msg.getSize();

      // send
      if( send(msocket, (const char *)&size, sizeof(size_t), 0) < 0 ) // send a size of a message
        throw std::runtime_error("Sending a size of a message failed");

      size_t total = 0;
      while ( total != size ) {
        #ifdef SOCKET_DEBUG
          std::cerr << "received " << total << "/" << size << "\n";
        #endif
        ssize_t d = send(msocket, (void *)&msg.getData()[total], size - total, 0);
        if( d < 0 )
          throw std::runtime_error("Sending a message failed");
        total += d;
      }

      #ifdef SOCKET_DEBUG
        std::cerr << "Done:" << total << "/" << size << "\n";
      #endif

    }

    NetString ReceiveMessage()
    {
      #ifdef SOCKET_DEBUG
        std::cerr << "Receiving message.\n";
      #endif
      size_t size;

      // receive
      if( recv(msocket, &size, sizeof(size_t), 0) < 0 ) // receive a size of a message
        throw std::runtime_error("Receiving a size of a message failed");
      char *buff = new char[size];

      size_t total = 0;
      while ( total != size ) {
        #ifdef SOCKET_DEBUG
          std::cerr << "received " << total << "/" << size << "\n";
        #endif
        ssize_t d = recv(msocket, &buff[total], size - total, 0);
        if( d < 0 )
          throw std::runtime_error("Sending a message failed");
        total += d;
      }

      #ifdef SOCKET_DEBUG
        std::cerr << "Done:" << total << "/" << size << "\n";
      #endif

      NetString msg(buff, size);
      delete [] buff;
      return msg;
    }

    void SendByte(unsigned char byte)
    {
      #ifdef SOCKET_DEBUG
        std::cerr << "Send byte " << (int)byte << " to " << msocket << "!\n";
      #endif

      if( send(msocket, (const char*)&byte, sizeof(unsigned char), 0) < 0 )
        throw std::runtime_error("Sending a metainfo failed");
    }

    unsigned char ReceiveByte()
    {
      #ifdef SOCKET_DEBUG
        std::cerr << "Receiving byte from " << msocket << ".\n";
      #endif
      unsigned char byte;

      // receive
      if( recv(msocket, &byte, sizeof(unsigned char), 0) < 0 )
        throw std::runtime_error("Receiving a metainfo failed");

      #ifdef SOCKET_DEBUG
        std::cerr << "Received byte " << (int)byte << "!\n";
      #endif
      return byte;
    }

    ~Socket()
    {
      #ifdef SOCKET_DEBUG
        std::cerr << "Close socket: " << msocket << "\n";
      #endif
      close(msocket);
    }

  protected:
    int msocket = -1;

    std::string mfilename;
    std::string mfile;
};

class ClientSocket: public Socket
{
  public:
    ClientSocket(std::string address, int port): Socket(AF_INET)
    {
      #ifdef SOCKET_DEBUG
        std::cerr << "Create client socket.\n";
      #endif
      addr = getAddr(address, port);

      #ifdef SOCKET_DEBUG
        std::cout << "Server " << inet_ntoa(addr.sin_addr) << " : " << ntohs(addr.sin_port) << "\n";
      #endif

      if( connect( msocket, (const struct sockaddr*)&addr, sizeof(addr) ) != 0 )
        throw std::runtime_error("Connecting failed");
    }

  private:
    struct sockaddr_in addr;

    struct sockaddr_in getAddr(std::string address, int port)
    {
      // address
      const char * host = address.c_str();

      // server
      struct hostent *server;

      // server address
      struct sockaddr_in server_address;

      // DNS request
      if ((server = gethostbyname(host)) == NULL) {
        throw std::runtime_error("Unknown host");
      }

      // set up sockaddr_in structure
      bzero((char *) &server_address, sizeof(server_address));
      server_address.sin_family = AF_INET;
      bcopy((char *)server->h_addr, (char *)&server_address.sin_addr.s_addr, server->h_length);
      server_address.sin_port = htons(port);

      // return
      return server_address;
    }
};

class ServerSocket: public Socket
{
  public:
    ServerSocket(int port): Socket(PF_INET6)
    {
      #ifdef SOCKET_DEBUG
        std::cerr << "Create server socket.\n";
      #endif
      addr = getAddr(port);

      #ifdef SOCKET_DEBUG
        std::cerr << "Binding.\n";
      #endif
      if( bind( msocket, (const struct sockaddr*)&addr, sizeof(addr) ) < 0 )
      {
        throw std::runtime_error("Binding failed");
      }

      #ifdef SOCKET_DEBUG
        std::cerr << "Listening.\n";
      #endif
      if( listen( msocket, 1) < 0 )
        throw std::runtime_error("Listening failed");
    }

    NetString ReceiveMessage()
    {
      if( !Connected() ) throw std::runtime_error("Connection not established");

      int tmp = msocket;
      msocket = mcomm_socket;

      NetString result = Socket::ReceiveMessage();

      msocket = tmp;
      return result;
    }

    void SendMessage(NetString msg)
    {
      if( !Connected() ) throw std::runtime_error("Connection not established");

      int tmp = msocket;
      msocket = mcomm_socket;

      Socket::SendMessage(msg);

      msocket = tmp;
    }

    void SendByte(unsigned char byte)
    {
      if( !Connected() ) throw std::runtime_error("Connection not established");

      int tmp = msocket;
      msocket = mcomm_socket;

      Socket::SendByte(byte);

      msocket = tmp;
    }

    unsigned char ReceiveByte()
    {
      if( !Connected() ) throw std::runtime_error("Connection not established");

      int tmp = msocket;
      msocket = mcomm_socket;

      unsigned char result = Socket::ReceiveByte();

      msocket = tmp;
      return result;
    }

    int WaitForConnection()
    {
      // close previous connection
      CloseConnection();

      // create communication socket
      struct sockaddr_in6 client_addr;
      socklen_t caddr_len = sizeof(client_addr);
      mcomm_socket = accept(msocket, (struct sockaddr*)&client_addr, &caddr_len);
      if(mcomm_socket <= 0) return false;

      int pid = fork();
      if(pid < 0) throw std::runtime_error("fork() call failed!");

      if(pid == 0)
      {
        close(msocket);

        // connect to the client
        char str[INET6_ADDRSTRLEN];
        if(inet_ntop(AF_INET6, &client_addr.sin6_addr, str, sizeof(str)))
        {
          #ifdef DEBUG_MODE
            std::cerr << "- Connected: " << str << ":" << ntohs(client_addr.sin6_port) << "\n";
          #endif
        }
      }
      return pid;
    }

    void CloseConnection()
    {
      if(mcomm_socket != -1)
      {
        #ifdef SOCKET_DEBUG
          std::cerr << "Close comm socket: " << mcomm_socket << ".\n";
        #endif
        close(mcomm_socket);
        mcomm_socket = -1;
      }
    }

    bool Connected() { return mcomm_socket != -1; }

    ~ServerSocket() {
      CloseConnection();
    }



  private:
    struct sockaddr_in6 getAddr(int port)
    {
      struct sockaddr_in6 addr;

    	memset(&addr,0,sizeof(addr));
    	addr.sin6_family = AF_INET6;
    	addr.sin6_addr = in6addr_any;
    	addr.sin6_port = htons(port);

      return addr;
    }

    int mcomm_socket = -1;
    struct sockaddr_in6 addr;

};

#endif // MYSOCKET_H
