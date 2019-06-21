/*
*	swap_client.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>

#define	MAXLINE 128	// maximum characters to receive and send at once
#define	MAXFRAME 256


extern int swap_connect(unsigned int addr, unsigned short port);
extern int swap_disconnect(int sd);
extern int sdp_send(int sd, char *buf, int length);
extern int sdp_receive(int sd, char *buf);
extern int sdp_receive_with_timer(int sd, char *buf, unsigned int expiration);
void delay(unsigned int mseconds);
unsigned int singleCharCheckSum(char *data, int count);
int send_metadata(int sd, int length);

int session_id = 0;

int S = 0;	// frame number sent


int swap_open(unsigned int addr, unsigned short port)
{
	int	sockfd;		// sockect descriptor
	struct	sockaddr_in	servaddr;	// server address
	char	buf[MAXLINE];
	int	len, n;

	/*
	*	if the session is already open, then return error
	*/

	if (session_id != 0)
		return -1;

	/*
	*	connect to a server
	*/

	session_id = swap_connect(addr, port);	// in sdp.o

	/*
	*	return the seesion id
	*/

	return session_id;
}


int swap_write(int sd, char *buf, int length)
{
	int n,i;
	char frame[MAXFRAME];
	char combined_frame[MAXFRAME][4],buffer[3];
	unsigned int selfchecksum,checksum;


	if (session_id == 0 || sd != session_id)
		return -1;
	
	//send length to server prior dataframes
	send_metadata(sd,length);

	//assign values from buf to frames
	for(i=0;i<length;i++){
		frame[i]=buf[i];
	}
	//for each combined_frame[i]
	//combined_frame[i][0] is the data
	//combined_frame[i][1] is the seq number
	//combined_frame[i][2] is the left part of checksum
	//combined_frame[i][3] is the right part of checksum
	for(i=0;i<length;i++){
		combined_frame[i][0]=frame[i];
		combined_frame[i][1]=i;
		//get checksum and assign left and right part of it
		selfchecksum=singleCharCheckSum(combined_frame[i],2);
		combined_frame[i][2]=(selfchecksum>>8);
		combined_frame[i][3]=(selfchecksum<<8)>>8;
	}

	//while ack<length
	i=0;
	while(i<length){
		//send data, seq and seperated checksum all together
		
		n=sdp_send(sd,combined_frame[i],sizeof combined_frame[i]);
		selfchecksum=singleCharCheckSum(combined_frame[i],2);

		//if error, resend the frame;
		if(n<0){
			continue;
		}

		//receive ack with timer 
		//buffer[0] is ack number
		//buffer[1] is left part of checksum
		//buffer[2] is right part of checksum
		n=sdp_receive_with_timer(sd,buffer,40);


		//if error, resend current frame
		if(n<0){
			continue;
		}

		//get checksum from frame and compare to self caculated check sum
		selfchecksum=singleCharCheckSum(buffer,1);
		checksum=(buffer[1]<<8)|buffer[2];

		//if no error, update and continue
		//else, resend current frame
		if(selfchecksum==checksum){
			i=buffer[0];
			continue;
		}else{
			continue;
		}
	}
	return i;
	
}


void swap_close(int sd)
{
	if (session_id == 0 || sd != session_id)
		return;

	else
		session_id = 0;

	swap_disconnect(sd);	// in sdp.o
}

//used for flow control
void delay(unsigned int mseconds){
	clock_t goal=mseconds+clock();
	while (goal>clock());
}

//8-bit Fletcher checksum function
//data: any array for chars
//count: # of chars should be counted
//return: 16-bit of unsigned int 
unsigned int singleCharCheckSum(char *data, int count){
	unsigned int L=0;
    unsigned int R=0;
    int i;
    for(i=0;i<count;i++){
        R=(R+data[i])%255;
        L=(L+R)%255;
    }
    return (L<<8)|R;
}

//send length of packet before send out data frames
int send_metadata(int sd, int length){

	int n=0;
	char meta_data[3],buf[3];
	unsigned int selfchecksum,checksum;

	if (session_id == 0 || sd != session_id)
		return -1;

	meta_data[0]=length;

	selfchecksum=singleCharCheckSum(meta_data,1);
	meta_data[1]=selfchecksum>>8;
	meta_data[2]=(selfchecksum<<8)>>8;
	while(1){
		n=sdp_send(sd,meta_data,3);

		if(n<0){
			continue;
		}
		n=sdp_receive_with_timer(sd,buf,40);
		if(n<0){
			continue;
		}
		
		selfchecksum=singleCharCheckSum(buf,1);
		checksum=((buf[1]<<8)|buf[2])&0x0000FFFF;
		
		if(selfchecksum==checksum&&buf[0]==0){
			break;
		}else{
			continue;
		}
	}
	return 1;
}