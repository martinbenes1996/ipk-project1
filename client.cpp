
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#include "config.h"

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
    conf = ProcessArguments(argc, argv); /* DELETE */conf->printConfig();
  }
  catch(std::exception& e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    exit(1);
  }

  // something to do.
}



ConfigPtr ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 6 && argc != 7) throw std::runtime_error("Bad count of arguments.");
  // create config
  ConfigPtr c = std::make_shared<Config>(); // shared ptr to config

  int it = 1;
  while(it < argc)
  {
    // bad arguments
    if(it == argc-1 && strcmp(argv[it], "-l") ) throw std::runtime_error("Invalid parameters.");

    if( !strcmp(argv[it], "-h") )
    {
      c->setIP(argv[it+1]);
    }
    else if( !strcmp(argv[it], "-p") )
    {
      c->setPort(argv[it+1]);
    }
    else if( !strcmp(argv[it], "-n") )
    {
      c->setFullUser(argv[it+1]);
    }
    else if( !strcmp(argv[it], "-l") )
    {
      c->setUserList( (it != argc-1) ? argv[it+1] : "" );
      if(it == argc-1) break;
    }
    else if( !strcmp(argv[it], "-f") )
    {
      c->setHomeDirectory(argv[it+1]);
    }
    else
    {
      throw std::runtime_error("Unknown parameter.");
    }
    it += 2;
  }

  return c;


}
