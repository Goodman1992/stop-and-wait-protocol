#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#define MAX_FTA	128

// These arguments must be passed to the program
// argv[1]: port number, e.g., "6789"

int main (int argc, char *argv[])
{
	int	sd, bytes_read;
	char	buf[MAX_FTA],dest_name[256],pack_length;
	FILE *dest;

	if (argc < 2) {
		fprintf(stderr, "Usage: %s port_number\n", argv[0]);
		exit(1);
	}

	sd = swap_wait(htons(atoi(argv[1])));
	if (sd < 0) {
		fprintf(stderr, "%s cannot wait, %d\n", argv[0], sd);
		exit(1);
	}

	printf("Student: Zhiyu Qin\nStudent #: T00034701\n");

	//read file name from client
	bytes_read=swap_read(sd,dest_name);

	dest_name[bytes_read]='\0';
	dest=fopen(dest_name,"w+");

	if(dest==NULL){
		printf("Error, can not open the file");
		exit(-1);
	}

	//read from client
	bytes_read=swap_read(sd,buf);
	buf[bytes_read]='\0';
	printf("%s",buf);
	//print buf to file
	fprintf(dest,"%s",buf);
	
	// close the file and the connection
	fclose(dest);
	swap_close(sd);

	return 0;
}

