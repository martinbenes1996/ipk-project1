
#include <cstring>
#include <exception>
#include <fstream>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

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
    void setPort(const char * port) { mport = std::string(port); }

    // debug
    void printConfig()
    {
      #ifdef CONFIG_DEBUG
        std::cout << "|port:\t" << mport << "\n";
      #endif
    }

    void check()
    {
      if(mport == "") throw std::runtime_error("Argument -p not given.");
    }

  private:
    /* ----------------- DATA ------------------- */
    std::string mport = ""; /*< Host port. */
    /* ------------------------------------------ */

};
typedef std::shared_ptr<Config> ConfigPtr; // config shared ptr

/* ==================================================== */

/**
 * @brief Processes arguments. Generates Config from them.
 */
ConfigPtr ProcessArguments(int argc, char *argv[]);

/**
 * @brief Reads /etc/passwd. Returns wanted strings (it can be printed).
 */
std::vector<std::string> ReadPasswd(char mode, const char * login);

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

  std::vector<std::string> v;
  try {
    v = ReadPasswd(0, "");
  }
  catch(std::exception& e) {
    std::cerr << "ERROR: " << e.what() << '\n';
    exit(1);
  }

}



std::vector<std::string> ReadPasswd(char mode, const char * login)
{

  std::ifstream input( "/etc/passwd" );

  std::vector<std::string> v;
  std::string line;

  switch(mode)
  {
      case FULL_USER:
        break;
      case HOME_DIRECTORY:
        break;
      case USER_LIST:
      /*
        std::regex select(std::string(login)+".*");
        std::regex nick("^[^:]+");
        for(std::string l; getline(input, line); )
        {
          std::smatch match;
          if(std::regex_search(l, match, nick))
          {
            if(std::regex_search(l, match, select))
            v.push_back();
          }

          std::string subject("Name: John Doe");
          std::string result;
          try {
            std::regex re("Name: (.*)");
            std::smatch match;
            if (std::regex_search(subject, match, re) && match.size() > 1) {
              result = match.str(1);
            } else {
              result = std::string("");
            }
          } catch (std::regex_error& e) {
            // Syntax error in the regular expression
          }

        }
        */
        break;
      default:
        throw std::runtime_error("Unknown mode.");
  }
  return v;
}

ConfigPtr ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 3) throw std::runtime_error("Bad count of arguments.");
  // create config
  ConfigPtr c = std::make_shared<Config>(); // shared ptr to config

  // not -p
  if( strcmp(argv[1],"-p") ) throw std::runtime_error("Invalid parameters.");
  c->setPort(argv[2]);

  return c;
}
