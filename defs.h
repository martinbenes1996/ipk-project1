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
#define WINDOW_SIZE 8 // firstly i wanted to do it using windows

/**
 * @brief     Prints given message, if COMM_DEBUG is enabled
 * @param str         String to print.
 */
void Debug_Comm(std::string str)
{
  (void)str;
  #ifdef COMM_DEBUG
    std::cerr << str << "\n";
  #endif
}

/**
 * @brief     Prints given message, if CONCURRENT_DEBUG is enabled
 * @param str         String to print.
 */
void Debug_Concurr(std::string str)
{
  (void)str;
  #ifdef CONCURRENT_DEBUG
    std::cerr << str << "\n";
  #endif
}

/**
 * @brief Creates a socket (used in client as well as server).
 * @param mode          Mode in which the socket is opened (IPv4, IPv6).
 * @returns Descriptor of socket.
 */
int createSocket(int mode)
{
  int msocket;
  if( (msocket = socket(mode, SOCK_STREAM, 0)) <= 0) throw std::runtime_error("opening a socket failed");
  #ifdef SOCKET_DEBUG
    std::cerr << "Create socket " << msocket << ".\n";
  #endif
  return msocket;
}



/**
 * @brief Config class. Represents given arguments.
 */
class Config
{
  public:
    /* ----------------- Constructor ------------------ */
    /** @brief Constructor. */
    Config() {}
    /* ------------------- Setters -------------------- */
    // HOST SETTERS
    /**
     * @brief Adress setter.
     * @param addr        New address.
     */
    void setAddress(const char * addr) { maddr = std::string(addr); }
    /**
     * @brief Port setter.
     * @param port        New port.
     */
    void setPort(const char * port)
    {
      try { mport = std::stoi(port); }
      catch(std::exception& e) { throw std::runtime_error("Not valid port number."); }
    }
    // MODE SETTERS
    /**
     * @brief Write file setter.
     * @param file        File, that will be sent to server.
     */
    void setWrite(const char * file) { mread = false; mfile = std::string(file); }
    /**
     * @brief Read file setter.
     * @param file        File, that will be received from server.
     */
    void setRead(const char * file) { mread = true; mfile = std::string(file); }

    /* -------------------- Getters -------------------- */
    /**
     * @brief Adress getter.
     * @returns Address.
     */
    std::string getAddress() { return maddr; }
    /**
     * @brief Port getter.
     * @returns Port.
     */
    int getPort() { return mport; }
    /**
     * @brief Read indicator getter.
     * @returns True, if -r was given.
     */
    bool read() { return mread; }
    /**
     * @brief Write indicator getter.
     * @returns True, if -w was given.
     */
    bool write() { return !mread; }
    /**
     * @brief File getter.
     * @returns Name of the file, as it was given.
     */
    std::string getFile() { return mfile; }
    /**
     * @brief Filename getter.
     * @returns Name of the file, without a path.
     */
    std::string getFilename() { return mfile.substr(mfile.find_last_of("/\\") + 1); }

    /* -------------------- Checkers -------------------- */
    /**
     * @brief Client checker. Throws runtime_error,
     *        if any client parameter is missing.
     */
    void checkClient()
    {
      if(maddr == "") throw std::runtime_error("Argument -h not given.");
      if(mport == -1) throw std::runtime_error("Argument -p not given.");
      if(mfile == "") throw std::runtime_error("Any of arguments [-w|-r] not given.");
    }
    /**
     * @brief Server checker. Throws runtime_error,
     *        if any server parameter is missing.
     */
    void checkServer()
    {
      if(mport == -1) throw std::runtime_error("Argument -p not given.");
    }

    /* -------------------- Debug -------------------- */
    /**
     * @brief Prints contents of Config, if CONFIG_DEBUG is enabled.
     */
    void printConfig()
    {
      #ifdef CONFIG_DEBUG
        std::cout << "|address:\t" << maddr << "\n" <<
                     "|port:\t" << mport << "\n" <<
                     "|mode:\t" << mfile << (mread)?" R":" W" << "\n";
      #endif
    }
    /* ----------------------------------------------- */

  private:

    /* ----------------- DATA ------------------- */
    // host
    std::string maddr = "";   /*< Host address. */
    int mport = -1; /*< Host port. */
    // mode
    bool mread = true; /* Whether read (true), or write (false). */
    std::string mfile = ""; /* File to write/read. */
    /* ------------------------------------------ */

};

#endif // DEFS_H
