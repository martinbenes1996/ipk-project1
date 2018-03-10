#ifndef DEFS_H
#define DEFS_H

#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <sys/types.h>

#ifdef DEBUG_MODE
  //#define CONFIG_DEBUG
  //#define COMM_DEBUG
  //#define SOCKET_DEBUG
  //#define CONCURRENT_DEBUG
#endif // DEBUG_MODE

#define BUFFER_SIZE 1024
#define WINDOW_SIZE 8

void Debug_Comm(std::string str)
{
  (void)str;
  #ifdef COMM_DEBUG
    std::cerr << str << "\n";
  #endif
}

void Debug_Concurr(std::string str)
{
  (void)str;
  #ifdef CONCURRENT_DEBUG
    std::cerr << str << "\n";
  #endif
}

int createSocket(int mode)
{
  int msocket;
  if( (msocket = socket(mode, SOCK_STREAM, 0)) <= 0) throw std::runtime_error("opening a socket failed");
  #ifdef SOCKET_DEBUG
    std::cerr << "Create socket " << msocket << ".\n";
  #endif
  return msocket;
}

class Buffer
{
  public:
    Buffer()
    {
      #ifdef BUFFER_DEBUG
        std::cerr << "Create buffer.\n";
      #endif
    }

    void insert(size_t index, const char * buff, size_t size = BUFFER_SIZE)
    {
      if(size > BUFFER_SIZE) throw std::runtime_error("too big buffer");
      if(index >= WINDOW_SIZE) throw std::runtime_error("index out of range");
      if( getTaken(index) ) throw std::runtime_error("buffer already taken");

      #ifdef BUFFER_DEBUG
        std::cout << "Buffer: inserting " << size << "B string into " << index*BUFFER_SIZE + 1 << ".\n";
      #endif

      strncpy(&mbuff[index*BUFFER_SIZE + 1], buff, size);
      setTaken(index);
      msize[index] = (char)index;
    }

    char * get(size_t index = 0)
    {
      if(!validData()) throw std::runtime_error("invalid data");
      if(index >= WINDOW_SIZE) throw std::runtime_error("index out of range");
      if(!getTaken(index)) throw std::runtime_error("empty index");
      return &mbuff[index*BUFFER_SIZE];
    }

    char * handle(size_t index = 0)
    {
      return &get(index)[1];
    }

    size_t getSize()
    {
      size_t size = 0;
      for(int i = 0; i < WINDOW_SIZE; i++)
        size += msize[i];
      return size;
    }

  private:
    char mbuff[WINDOW_SIZE * (BUFFER_SIZE + 1)];
    size_t msize[WINDOW_SIZE] = {};
    unsigned char mmask = 0x00;

    bool getTaken(size_t index) { return mmask & (0x01 << index); }
    void setTaken(size_t index)
    {
      std::cout << +mmask << " | " << +(0x01 << index) << " -> ";
      mmask = mmask | (0x01 << index);
      std::cout << +mmask << "\n";
    }
    bool validData()
    {
      return ((mmask == 0x01) || (mmask == 0x03) || (mmask == 0x07) || (mmask == 0x0F) || (mmask == 0x1F) || (mmask == 0x3F) || (mmask == 0x7F) || (mmask == 0xFF));
    }
};

#endif // DEFS_H
