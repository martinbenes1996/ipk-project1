
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>

#include "config.h"
#include "mysocket.h"

#ifdef DEBUG_MODE
  #define CONFIG_DEBUG
#endif // DEBUG_MODE

/**
 * @brief Processes arguments. Generates Config from them.
 */
ConfigPtr ProcessArguments(int argc, char *argv[]);

std::string ReadFile(std::string name);

/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
  try
  {
    // process arguments
    ConfigPtr conf;
    conf = ProcessArguments(argc, argv);

    // create socket
    MySocket sckt( conf->getAddress(), conf->getPort() );

    // read
    if(conf->read())
    {
      // read file
      std::string f = ReadFile( conf->getFile() );
      // send
      sckt.Send(f);
    }
    // write
    else
    {

    }

  } catch(std::exception& ex) {
    std::cerr << "ERROR: " << ex.what() << "!\n";
    exit(444);
  }



}



ConfigPtr ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 7) throw std::runtime_error("Bad count of arguments.");
  // create config
  ConfigPtr c = std::make_shared<Config>(); // shared ptr to config

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

  c->check();
  return c;
}

std::string ReadFile(std::string name)
{
  std::ifstream t(name);
  t.seekg(0, std::ios::end);
  size_t size = t.tellg();
  std::string buffer(size, ' ');
  t.seekg(0);
  t.read(&buffer[0], size);
  return buffer;
}
