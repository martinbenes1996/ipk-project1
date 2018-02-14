
#include <cstring>
#include <exception>
#include <iostream>
#include <memory>
#include <string>

#ifdef DEBUG_MODE
  #define CONFIG_DEBUG
#endif // DEBUG_MODE


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

/* ==================================================== */

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
