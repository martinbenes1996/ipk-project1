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
    void setIP(const char * ip) { mip = std::string(ip); }
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

    // debug
    void printConfig()
    {
      #ifdef CONFIG_DEBUG
        std::cout << "|IP:\t" << mip << "\n" <<
                     "|port:\t" << mport << "\n" <<
                     "|mode:\t" << mfile << (mread)?" R":" W" << "\n";
      #endif
    }

    void check()
    {
      if(mip == "") throw std::runtime_error("Argument -h not given.");
      if(mport == -1) throw std::runtime_error("Argument -p not given.");
      if(mfile == "") throw std::runtime_error("Any of arguments [-w|-r] not given.");
    }

  private:
    /* ----------------- DATA ------------------- */
    // host
    std::string mip = "";   /*< Host IP. */
    int mport = -1; /*< Host port. */
    // mode
    bool mread = true; /* Whether read (true), or write (false). */
    std::string mfile = ""; /* File to write/read. */
    /* ------------------------------------------ */

};


typedef std::shared_ptr<Config> ConfigPtr; // config shared ptr


#endif // CONFIG_H
