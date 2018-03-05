
#include <csignal>
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

#include "config.h"
#include "defs.h"
#include "mysocket.h"

std::unique_ptr<ServerSocket> sckt;

void sigChildEnded(int)
{
	int pid = wait(NULL);
  #ifdef CONCURRENT_DEBUG
    std::cerr << "Subprocess " << pid << " caught.\n";
  #endif
}

void sigInt(int)
{
  sckt.reset(nullptr);
  exit(0);
}

/**
 * @brief Processes arguments. Generates Config from them.
 */
Config* ProcessArguments(int argc, char *argv[]);

/**
 * @brief Performs read from the server. Noreturn function.
 *
 * @param sckt  Socket to send the file to.
 */
void PerformRead(std::unique_ptr<ServerSocket>& sckt);

/**
 * @brief Performs write to the server. Noreturn function.
 *
 * @param sckt  Socket to receive the file from.
 */
void PerformWrite(std::unique_ptr<ServerSocket>& sckt);

/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
  // process arguments
  std::unique_ptr<Config> conf;
  try { conf.reset( ProcessArguments(argc, argv) ); }
  catch(std::exception& e) { std::cerr << e.what() << "\n"; }

  // create socket
  try { sckt.reset( new ServerSocket(conf->getPort()) ); }
  catch(std::exception& e) { std::cerr << e.what() << "\n"; }

  signal(SIGCHLD,sigChildEnded);
  signal(SIGINT, sigInt);
  conf.reset(nullptr);

  while(true)
  {
    // wait for connection, forks inside
    int pid = sckt->WaitForConnection();

    // subprocess
    if(pid == 0)
    {
      // the subprocess will exit in one of those
      // read
      if( sckt->ReceiveByte() == 0xFF ) PerformRead(sckt);
      // write
      else { PerformWrite(sckt); }
    }

    // main process
    else{
      #ifdef CONCURRENT_DEBUG
        std::cerr << "Process " << pid << " spawned.\n";
      #endif
    }

  }

  sckt.reset(nullptr);
  return 0;
}


Config* ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 3) throw std::runtime_error("Bad count of arguments.");
  // create config
  Config* c = new Config; // ptr to config

  // -p
  if( !strcmp(argv[1],"-p") )
  {
    c->setPort(argv[2]);
  }
  // error
  else
  {
    throw std::runtime_error("Invalid parameters.");
  }

  c->checkServer();
  return c;
}

void PerformWrite(std::unique_ptr<ServerSocket>& sckt)
{
  #ifdef CONCURRENT_DEBUG
    std::cerr << "- Performing write.\n";
  #endif

  try {
    /* ----------------- SERVER WRITE PROTOCOL ------------------- */
    std::string filename = sckt->ReceiveMessage(); // receive filename

    std::string file = sckt->ReceiveMessage(); // receive file
    /* ----------------------------------------------------------- */

    // write the file
    WriteToFile(filename, file);

    #ifdef CONCURRENT_DEBUG
      std::cerr << "- Write success.\n";
    #endif
    #ifdef DEBUG_MODE
    std::cerr << "- Received file " << filename << ".\n";
    #endif
    sckt.reset(nullptr);
    exit(0);

  } catch(std::exception& e) {
    #ifdef CONCURRENT_DEBUG
      std::cerr << "- Write fail.\n";
    #endif

    std::cerr << e.what() << "\n";
    sckt.reset(nullptr);
    exit(1);
  }
}

void PerformRead(std::unique_ptr<ServerSocket>& sckt)
{
  #ifdef CONCURRENT_DEBUG
    std::cerr << "- Performing read.\n";
  #endif

  try {
    /* --------------- SERVER READ PROTOCOL -------------------- */
    std::string filename = sckt->ReceiveMessage(); // receive filename

    // read file
    std::string file;
    bool fail = false;
    try { file = ReadFile(filename); }
    catch(std::exception& ex) { fail = true; }
    sckt->SendByte( ((fail) ? 0x00 : 0xFF) );

    sckt->SendMessage( file ); // send file
    /* --------------------------------------------------------- */

    #ifdef CONCURRENT_DEBUG
      std::cerr << "- Read success.\n";
    #endif
    #ifdef DEBUG_MODE
    std::cerr << "- Sent file " << filename << ".\n";
    #endif
    sckt.reset(nullptr);
    exit(0);

  } catch(std::exception& e) {
    #ifdef CONCURRENT_DEBUG
      std::cerr << "- Read fail.\n";
    #endif

    std::cerr << "- " << e.what() << "\n";
    sckt.reset(nullptr);
    exit(1);
  }
}
