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
	char	buf[MAX_FTA], *lineptr,pack_length,pack[1024][MAX_FTA];
	int	n;
	int	i;
	int test_length;
	FILE *source;

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

	printf("Student: Zhiyu Qin\nStudent #: T00034701\n");

	//open file
	source=fopen(argv[3],"r");
	if(source==NULL){
		printf("Error, can not open the file");
		exit(-1);
	}
	//send dest file name to server 
	swap_write(sd, argv[4], strlen(argv[4]));
	
	//get line from file
	lineptr=fgets(buf,127,source);
	if(lineptr==NULL){
		printf("Error, file is empty");
		exit(1);
	}
	//write line to socket
	swap_write(sd,buf,strlen(buf));

	fclose(source);
	swap_close(sd);
	return 0;
}
