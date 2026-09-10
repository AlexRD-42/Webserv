#define MAIN_FILE

#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>

#include "core.hpp"
#include "Server.hpp"

int	main(int argc, char** argv, char** envp)
{
	(void) argc, (void)argv, (void) envp;
	if (argc != 2)
		PERR_RETURN(1, "Error: Usage -> ./webserv <config_file>");

	static Server server = {};
	server.init(argv[1]);
	server.run();
	return 0;
}
