#ifndef CONFIG_H
#define CONFIG_H


#define NO_MODE 0x00
#define FULL_USER 0x01      // -n
#define HOME_DIRECTORY 0x02 // -f
#define USER_LIST 0x04      // -l

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
    void setPort(const char * port) { mport = std::string(port); }
    // Mode setters
    void setFullUser(const char * login) { setMode(FULL_USER, login); }
    void setHomeDirectory(const char * login) { setMode(HOME_DIRECTORY, login); }
    void setUserList(const char * login) { setMode(USER_LIST, login); }

    // debug
    void printConfig()
    {
      #ifdef CONFIG_DEBUG
        std::cout << "|IP:\t" << mip << "\n" <<
                     "|port:\t" << mport << "\n" <<
                     "|mode:\t" << +mmode << " -> " << mlogin << "\n";
      #endif
    }

    void check()
    {
      if(mip == "") throw std::runtime_error("Argument -h not given.");
      if(mport == "") throw std::runtime_error("Argument -p not given.");
      if(mmode == NO_MODE) throw std::runtime_error("Any of arguments [-l|-n|-f] not given.");
    }

  private:
    /* ----------------- DATA ------------------- */
    // host
    std::string mip = "";   /*< Host IP. */
    std::string mport = ""; /*< Host port. */
    // mode
    char mmode = NO_MODE;    /*< Mode of display. */
    std::string mlogin = ""; /*< User's login. */
    /* ------------------------------------------ */

    void setMode(const char mode, const char * login)
    {
      // set mode and login
      mmode = mode;
      mlogin = std::string(login);
    }

};


typedef std::shared_ptr<Config> ConfigPtr; // config shared ptr


#endif // CONFIG_H
