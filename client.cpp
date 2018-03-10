
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <string>
#include <unistd.h>

#include "config.h"
#include "defs.h"

Config conf;

/**
 * @brief Processes arguments. Generates Config from them.
 */
Config ProcessArguments(int argc, char *argv[]);

std::string ReadFile(std::string name);

/**
 * @brief Performs read from the server.
 * @param sckt    Socket.
 */
void PerformRead(int);

/**
 * @brief Performs write to the server.
 * @param sckt    Socket
 */
void PerformWrite(int);

/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
  try
  {
    // process arguments
    conf = ProcessArguments(argc, argv);

    // create socket
    int sckt = createSocket(AF_INET);
    struct sockaddr_in server_addr;
    struct hostent *server;
    // DNS request
    if( (server = gethostbyname(conf.getAddress().c_str())) == NULL) throw std::runtime_error("Unknown host");
    // set up sockaddr_in structure
    bzero((char *) &server_addr, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr, (char *)&server_addr.sin_addr.s_addr, server->h_length);
    server_addr.sin_port = htons(conf.getPort());

    // connect to the server
    if(connect(sckt, (const struct sockaddr*)&server_addr, sizeof(server_addr)) != 0) throw std::runtime_error("connection failed");

    // send mode (read/write)
    unsigned char mode = (conf.read())?0xFF:0x0F;
    send(sckt, &mode, sizeof(char), 0);
    if(conf.read()) PerformRead(sckt); // read
    else PerformWrite(sckt);           // write

  } catch(std::exception& ex) {
    std::cerr << ex.what() << "!\n";
    exit(444);
  }

  return 0;
}


void PerformRead(int sckt)
{
  /* ----------------- CLIENT READ PROTOCOL ------------------- */

  // filename
  size_t size = conf.getFile().size() + 1;
  send(sckt, &size, sizeof(size_t), 0);
  send(sckt, conf.getFile().c_str(), conf.getFile().size() + 1, 0);

  Debug_Comm("send filename");

  unsigned char stat;
  recv(sckt, &stat, sizeof(char), 0);
  if(stat == 0x00) throw std::runtime_error("file could not be read");

  Debug_Comm("file found");

  // open the file
  FILE * f = fopen(conf.getFilename().c_str(), "wb");
  if(f == NULL) throw std::runtime_error("file could not be opened");

  // size of message
  size_t msgsize;
  recv(sckt, &msgsize, sizeof(size_t), 0);

  Debug_Comm(std::string("file size ") + std::to_string(msgsize));

  // receive file
  char pckt[BUFFER_SIZE];
  size_t rest = msgsize;
  do {
    size_t blksize = (rest > BUFFER_SIZE) ? BUFFER_SIZE : rest;
    recv(sckt, pckt, blksize, 0);
    fwrite(pckt, sizeof(char), blksize, f);

    unsigned char ack = 0xFF;
    send(sckt, &ack, sizeof(char), 0);
    rest -= blksize;
    Debug_Comm(std::to_string(msgsize - rest) + "/" + std::to_string(msgsize));

  } while(rest > 0);

  close(sckt);

  /* ----------------------------------------------------------- */
}


void PerformWrite(int sckt)
{
  /* ----------------- CLIENT WRITE PROTOCOL ------------------- */
  // filename
  size_t size = conf.getFile().size() + 1;
  send(sckt, &size, sizeof(size_t), 0);
  send(sckt, conf.getFile().c_str(), conf.getFile().size() + 1, 0);

  // open the file
  FILE * f = fopen(conf.getFile().c_str(), "rb");
  if(f == NULL) throw std::runtime_error("file could not be opened");

  // size of file
  fseek(f, 0, SEEK_END);
  size_t msgsize = ftell(f);
  fseek(f, 0, SEEK_SET);
  send(sckt, &msgsize, sizeof(size_t), 0);

  // send file
  char pckt[BUFFER_SIZE];
  size_t rest = msgsize;
  do {
    size_t blksize = (rest > BUFFER_SIZE) ? BUFFER_SIZE : rest;
    size_t readBytes = fread(pckt, sizeof(char), blksize, f);
    if(readBytes != blksize) { std::cerr << readBytes << ", but expected " << blksize << "!\n"; throw std::runtime_error("EOF unexpected");}
    send(sckt, pckt, blksize, 0);

    unsigned char ack;
    recv(sckt, &ack, sizeof(char), 0);
    rest -= blksize;

  } while(rest > 0);

  close(sckt);
  /* ----------------------------------------------------------- */
}



Config ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 7) throw std::runtime_error("Bad count of arguments.");
  // create config
  Config c; // shared ptr to config

  int it = 1;
  while(it < argc)
  {
    // bad arguments
    if(it == argc-1 && strcmp(argv[it], "-l") ) throw std::runtime_error("Invalid parameters.");

    // -h
    if( !strcmp(argv[it], "-h") )
    {
      c.setAddress(argv[it+1]);
    }
    // -p
    else if( !strcmp(argv[it], "-p") )
    {
      c.setPort( argv[it+1] );
    }
    // -r
    else if( !strcmp(argv[it], "-r") )
    {
      c.setRead(argv[it+1]);
    }
    // -w
    else if( !strcmp(argv[it], "-w") )
    {
      c.setWrite( argv[it+1] );
    }
    else
    {
      throw std::runtime_error(std::string("Unknown parameter '") + std::string(argv[it]) + "'.");
    }
    it += 2;
  }

  c.checkClient();
  return c;
}
