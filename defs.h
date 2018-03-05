#ifndef DEFS_H
#define DEFS_H

#include <exception>
#include <fstream>
#include <string>
#include <sstream>

#ifdef DEBUG_MODE
  //#define CONFIG_DEBUG
  //#define COMM_DEBUG
  //#define SOCKET_DEBUG
  #define CONCURRENT_DEBUG
#endif // DEBUG_MODE

std::string ReadFile(std::string name)
{
  // open file
  std::ifstream t;
  t.open(name);
  if( !t.is_open() ) throw std::runtime_error(std::string("Can't open file ") + name);

  // read whole file
  std::stringstream stream;
  stream << t.rdbuf();
  std::string str = stream.str();

  // return string
  return str;
}

void WriteToFile(std::string filename, std::string file)
{
  std::ofstream o(filename);
  o << file;
  o.close();
}

#endif // DEFS_H
