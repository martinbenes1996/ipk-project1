
#include <csignal>
#include <cstring>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

#include "config.h"
#include "defs.h"

void sigChildEnded(int)
{
	int pid = wait(NULL);
  #ifdef CONCURRENT_DEBUG
    std::cerr << "Connection " << pid << " ended.\n";
  #endif
  (void)pid; // without CONCURRENT_DEBUG it causes warning (not used).
}

void sigInt(int)
{
  exit(0);
}

struct sockaddr_in6 getAddr(int port)
{
	struct sockaddr_in6 addr;
	memset(&addr,0,sizeof(addr));
	addr.sin6_family = AF_INET6;
	addr.sin6_addr = in6addr_any;
	addr.sin6_port = htons( port );
	return addr;
}

/**
 * @brief Processes arguments. Generates Config from them.
 */
Config ProcessArguments(int argc, char *argv[]);

/**
 * @brief Performs read from the server. Noreturn function.
 * @param sckt 			Communication socket.
 */
void PerformRead(int sckt);

/**
 * @brief Performs write to the server. Noreturn function.
 * @param sckt 			Communication socket.
 */
void PerformWrite(int sckt);

/**
 * @brief Main function.
 */
int main(int argc, char *argv[])
{
  // process arguments
  Config conf;
  try { conf = ProcessArguments(argc, argv); }
  catch(std::exception& e) { std::cerr << e.what() << "\n"; exit(1); }


  // create socket
	int welcome;
	try {
		welcome = createSocket(PF_INET6);
		struct sockaddr_in6 addr = getAddr(conf.getPort());
		if( bind(welcome, (const struct sockaddr*)&addr, sizeof(addr)) < 0 ) throw std::runtime_error("bind() failed");
		if( listen(welcome, 1) < 0 ) throw std::runtime_error("listen() failed");
	}
  catch(std::exception& e) {
		std::cerr << e.what() << "!\n";
		exit(1);
	}


	// reset signal handlers
  signal(SIGCHLD,sigChildEnded);
  signal(SIGINT, sigInt);


	// repeat until SIGINT
  while(true)
  {

		// create communication socket
		int comm = 0;
		try {
			struct sockaddr_in6 client_addr;
			socklen_t caddr_len = sizeof(client_addr);
			if( (comm = accept(welcome, (struct sockaddr*)&client_addr, &caddr_len)) <= 0) throw std::runtime_error("accept() error");


			// fork
			int pid = fork();
			if(pid < 0) throw std::runtime_error("fork() call failed!"); // error

			// ------------------ sub process --------------------
			if(pid == 0)
			{
				close(welcome);

				// connect to the client
				char str[INET6_ADDRSTRLEN];
				if(inet_ntop(AF_INET6, &client_addr.sin6_addr, str, sizeof(str)))
				{
					#ifdef DEBUG_MODE
						std::cerr << "- Connected: " << str << ":" << ntohs(client_addr.sin6_port) << "\n";
					#endif
				}

				// receive mode (read/write)
				unsigned char mode;
				recv(comm, &mode, sizeof(mode), 0);
	      if( mode == 0xFF ) PerformRead(comm); // read
	      else PerformWrite(comm);              // write
				// no return here
			}
			// --------------------------------------------------

	    // main process
	    else
			{
	      #ifdef CONCURRENT_DEBUG
	        std::cerr << "Process " << pid << " spawned.\n";
	      #endif
				close(comm);
			}

		// error
		} catch(std::runtime_error& e) {
			std::cerr << e.what() << "!\n";
			close(comm);
			exit(1);
		}

  } return 0; // dead code (ends with Ctrl+C)
}


Config ProcessArguments(int argc, char *argv[])
{
  // bad arguments count
  if(argc != 3) throw std::runtime_error("Bad count of arguments.");

  Config c; // create config
  // -p
  if( !strcmp(argv[1],"-p") ) c.setPort(argv[2]);
  // error
  else throw std::runtime_error("Invalid parameters");

  c.checkServer();
  return c;
}

void PerformRead(int comm)
{
	#ifdef CONCURRENT_DEBUG
    std::cerr << "- Performing read.\n";
  #endif

	FILE *f = NULL;

  try {

    /* ----------------- SERVER READ PROTOCOL ------------------- */
		// filename
		size_t size;
		if(recv(comm, &size, sizeof(size_t), 0) < 0) throw std::runtime_error("connection failed");
		char * filename = new char[size];
		if(recv(comm, filename, size, 0) < 0) throw std::runtime_error("connection failed");

		Debug_Comm(std::string("Asked filename: ") + filename);

		// open the file
		f = fopen(filename, "rb");
		delete [] filename;
		if(f == NULL) throw std::runtime_error("file could not be opened");
		unsigned char stat = (f==NULL)?0x00:0xFF;
		if(send(comm, &stat, sizeof(char), 0) < 0) throw std::runtime_error("connection failed");
		if(f == NULL) throw std::runtime_error("file could not be opened");

		// size of file
		struct stat st;
		fstat(fileno(f), &st);
		size_t msgsize = st.st_size;
		if(send(comm, &msgsize, sizeof(size_t), 0) < 0) throw std::runtime_error("connection failed");

		Debug_Comm("File size: " + std::to_string(msgsize) + "B");

		// send file
		char pckt[BUFFER_SIZE];
		size_t rest = msgsize;
		int cnt = 0;
		do {
			struct timeval start, stop;
			gettimeofday(&start, NULL);

			size_t blksize = (rest > BUFFER_SIZE) ? BUFFER_SIZE : rest;
			size_t readBytes = fread(pckt, sizeof(char), blksize, f);
			if(readBytes != blksize) throw std::runtime_error("EOF unexpected");
			if(send(comm, pckt, blksize, 0) < 0) throw std::runtime_error("connection failed");

			unsigned char ack;
			if(recv(comm, &ack, sizeof(char), 0) < 0) throw std::runtime_error("connection failed");
			rest -= blksize;

			//Debug_Comm("Sent " + std::to_string(msgsize - rest) + " / " + std::to_string(msgsize) + " B [" + std::to_string(int((msgsize - rest) / (double)msgsize)) + "%]");

			gettimeofday(&stop, NULL);
			double speed = blksize / (((double)(stop.tv_usec - start.tv_usec)/1000000) + (double)(stop.tv_sec - start.tv_sec));
			cnt++;
      if(cnt == 200)
      {
				std::cout << int(((msgsize - rest) / (double)msgsize)*100) << "%\t" << int(speed/1000.) << " kB/s\n";
				cnt = 0;
			}
		} while(rest > 0);

    /* ----------------------------------------------------------- */

    #ifdef CONCURRENT_DEBUG
      std::cerr << "- Read success.\n";
    #endif
    close(comm);
		fclose(f);
    exit(0);

	// error
  } catch(std::exception& e) {
    std::cerr << "Server: PerformRead(): " << e.what() << "!\n";
		if(f != NULL) fclose(f);
    close(comm);
    exit(1);
  }

}

void PerformWrite(int comm)
{
  #ifdef CONCURRENT_DEBUG
    std::cerr << "- Performing write.\n";
  #endif

	FILE * f = NULL;
  try {

    /* ----------------- SERVER WRITE PROTOCOL ------------------- */
		// filename
		size_t size;
		recv(comm, &size, sizeof(size_t), 0);
		char * filename = new char[size];
		recv(comm, filename, size, 0);

		// open the file
		f = fopen(filename, "wb");
		delete [] filename;
		if(f == NULL) throw std::runtime_error("file could not be opened");

		// size of file
		size_t msgsize;
		recv(comm, &msgsize, sizeof(size_t), 0);

		char pckt[BUFFER_SIZE];
		size_t rest = msgsize;
		int cnt = 0; struct timeval start, stop; // speed measure
		do {
			if(++cnt == 200) gettimeofday(&start, NULL);

			size_t blksize = (rest > BUFFER_SIZE) ? BUFFER_SIZE : rest;
			recv(comm, pckt, blksize, 0);
			fwrite(pckt, sizeof(char), blksize, f);

			unsigned char ack = 0xFF;
			send(comm, &ack, sizeof(char), 0);
			rest -= blksize;

			//Debug_Comm("Sent " + std::to_string(msgsize - rest) + " / " + std::to_string(msgsize) + " B [" + std::to_string(int((msgsize - rest) / (double)msgsize)) + "%]");
	    if(cnt == 200)
      {
        gettimeofday(&stop, NULL);
  			double speed = blksize / (((double)(stop.tv_usec - start.tv_usec)/1000000) + (double)(stop.tv_sec - start.tv_sec));
				std::cout << int(((msgsize - rest) / (double)msgsize)*100) << "%\t" << int(speed/1000.) << " kB/s\n";
				cnt = 0;
			}

		} while(rest > 0);

    /* ----------------------------------------------------------- */

    close(comm);
		fclose(f);
    exit(0);

	// error
  } catch(std::exception& e) {
    std::cerr << "Server: PerformWrite(): " << e.what() << "!\n";
    close(comm);
		if(f != NULL) fclose(f);
    exit(1);
  }
}
