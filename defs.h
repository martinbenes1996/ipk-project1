#ifndef DEFS_H
#define DEFS_H

#include <algorithm>
#include <exception>
#include <fstream>
#include <iostream>
#include <iterator>
#include <string>
#include <sstream>

#ifdef DEBUG_MODE
  //#define CONFIG_DEBUG
  #define COMM_DEBUG
  #define SOCKET_DEBUG
  //#define CONCURRENT_DEBUG
#endif // DEBUG_MODE

class NetString
{
  public:
    NetString(const char * str, size_t size)
    {
      for(size_t i = 0; i < size; i++) { mstr.push_back(str[i]); }
    }
    NetString(std::string str): NetString(str.c_str(), str.size()) {}
    NetString() = default;

    void push_back(char c) { mstr.push_back(c); }
    std::string getString() const
    {
      std::string str;
      for(auto& it: mstr) { str.push_back(it); }
      return str;
    }
    const char * getData() const { return mstr.data(); }
    size_t getSize() const { return mstr.size(); }

    void ReadFile(std::string name)
    {
      std::ifstream t( name, std::ios::binary );
      if(!t.is_open())
        throw std::runtime_error(std::string("Can't open file ") + name);

      mstr = std::vector<char>(std::istreambuf_iterator<char>(t),
                               std::istreambuf_iterator<char>());
    }

    void Write2File(std::string name)
    {
      std::ofstream o(name, std::ios::binary);
      o.write(getData(), getSize());
    }
    void Write2File(NetString name) { Write2File(name.getString()); }

  private:
    std::vector<char> mstr;
};

void WriteToFile(std::string filename, std::string file)
{
  std::ofstream o(filename);
  o << file;
  o.close();
}

#endif // DEFS_H
