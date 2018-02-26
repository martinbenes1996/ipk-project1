#ifndef CONFIG_H
#define CONFIG_H

#include <string>

/**
 * @brief Config class. Represents given arguments.
 */
class Config
{
  public:
    /** @brief Constructor. */
    Config() {}

    // Host setters
    void setAddress(const char * addr) { maddr = std::string(addr); }
    void setPort(const char * port)
    {
      try{
        mport = std::stoi(port);
      } catch(std::exception& e) {
        throw std::runtime_error("Not valid port number.");
      }
    }
    // Mode setters
    void setWrite(const char * file) { mread = false; mfile = std::string(file); }
    void setRead(const char * file) { mread = true; mfile = std::string(file); }

    std::string getAddress() { return maddr; }
    int getPort() { return mport; }
    bool read() { return mread; }
    bool write() { return !mread; }
    std::string getFile() { return mfile; }

    void checkClient()
    {
      if(maddr == "") throw std::runtime_error("Argument -h not given.");
      if(mport == -1) throw std::runtime_error("Argument -p not given.");
      if(mfile == "") throw std::runtime_error("Any of arguments [-w|-r] not given.");
    }

    void checkServer()
    {
      if(mport == -1) throw std::runtime_error("Argument -p not given.");
    }

    // debug
    void printConfig()
    {
      #ifdef CONFIG_DEBUG
        std::cout << "|address:\t" << maddr << "\n" <<
                     "|port:\t" << mport << "\n" <<
                     "|mode:\t" << mfile << (mread)?" R":" W" << "\n";
      #endif
    }

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


typedef std::shared_ptr<Config> ConfigPtr; // config shared ptr


#endif // CONFIG_H
