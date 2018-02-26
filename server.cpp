
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

#include "config.h"
#include "defs.h"
#include "mysocket.h"

#ifdef DEBUG_MODE
  #define CONFIG_DEBUG
#endif // DEBUG_MODE

/**
 * @brief Processes arguments. Generates Config from them.
 */
ConfigPtr ProcessArguments(int argc, char *argv[]);



/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
  // process arguments
  ConfigPtr conf;
  try {
    conf = ProcessArguments(argc, argv);

    // create socket
    ServerSocket sckt( conf->getPort() );

    while( sckt.WaitForConnection() )
    {

      // read
      if( sckt.ReceiveByte() == 0xFF )
      {

        /* --------------- SERVER READ PROTOCOL -------------------- */
        std::string filename = sckt.ReceiveMessage(); // receive filename

        sckt.SendMessage( ReadFile(filename) ); // send file
        /* --------------------------------------------------------- */

        std::cout << "Sent file: " << filename << "\n";


      }
      // write
      else
      {

      }

    }


  }
  catch(std::exception& e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    exit(1);
  }

  return 0;
}


ConfigPtr ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 3) throw std::runtime_error("Bad count of arguments.");
  // create config
  ConfigPtr c = std::make_shared<Config>(); // shared ptr to config

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
