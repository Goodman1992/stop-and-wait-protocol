#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define	MAX_FTA	128

// These arguments must be passed to the program
// argv[1]: IP address of server, e.g., "127.0.0.1"
// argv[2]: port number of sever, e.g., "6789"

int main (int argc, char *argv[])
{
	int	sd;
	char	buf[MAX_FTA];
	int	n;
	int	i;

	if (argc < 3) {
		fprintf(stderr, "Usage: %s IP_address port_number\n", argv[0]);
		exit(1);
	}

	// connect to the swap server
	sd = swap_open(inet_addr(argv[1]), htons(atoi(argv[2])));
	if (sd < 0)
	{
		fprintf(stderr, "%s cannot swap_open\n", argv[0]);
		exit(1);
	}

	// send messages to the swap server

	buf[0] = '0';
	swap_write(sd, buf, 1);

	// close the connection to the swap server
	swap_close(sd);
}
