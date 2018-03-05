
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>

#include <string>

#include "config.h"
#include "defs.h"
#include "mysocket.h"

/**
 * @brief Processes arguments. Generates Config from them.
 */
Config * ProcessArguments(int argc, char *argv[]);

std::string ReadFile(std::string name);

/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
  try
  {
    // process arguments
    std::unique_ptr<Config> conf;
    conf.reset( ProcessArguments(argc, argv) );

    // create socket
    ClientSocket sckt( conf->getAddress(), conf->getPort() );

    // read
    if(conf->read())
    {
      /* --------------- CLIENT READ PROTOCOL -------------------- */
      sckt.SendByte(0xFF); // send -r

      sckt.SendMessage(conf->getFile()); // send filename

      if(sckt.ReceiveByte() == 0x00) throw std::runtime_error("Receiving a file failed.");

      std::string file = sckt.ReceiveMessage(); // receive file
      /* --------------------------------------------------------- */

      // write the file
      WriteToFile(conf->getFilename(), file);

      std::cout << "File " << conf->getFile() << " successfully received.\n";

    }
    // write
    else
    {

      /* -------------- CLIENT WRITE PROTOCOL -------------------- */
      sckt.SendByte(0x00); // send -w

      sckt.SendMessage(conf->getFilename()); // send filename

      sckt.SendMessage( ReadFile(conf->getFile()) ); // send file
      /* --------------------------------------------------------- */

      std::cout << "File " << conf->getFile() << " successfully sent.\n";

    }



  } catch(std::exception& ex) {
    std::cerr << "ERROR: " << ex.what() << "!\n";
    exit(444);
  }

  return 0;
}



Config * ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 7) throw std::runtime_error("Bad count of arguments.");
  // create config
  Config* c = new Config; // shared ptr to config

  int it = 1;
  while(it < argc)
  {
    // bad arguments
    if(it == argc-1 && strcmp(argv[it], "-l") ) throw std::runtime_error("Invalid parameters.");

    // -h
    if( !strcmp(argv[it], "-h") )
    {
      c->setAddress(argv[it+1]);
    }
    // -p
    else if( !strcmp(argv[it], "-p") )
    {
      c->setPort( argv[it+1] );
    }
    // -r
    else if( !strcmp(argv[it], "-r") )
    {
      c->setRead(argv[it+1]);
    }
    // -w
    else if( !strcmp(argv[it], "-w") )
    {
      c->setWrite( argv[it+1] );
    }
    else
    {
      throw std::runtime_error(std::string("Unknown parameter '") + std::string(argv[it]) + "'.");
    }
    it += 2;
  }

  c->checkClient();
  return c;
}
