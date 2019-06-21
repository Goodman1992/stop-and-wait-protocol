/*
*	swap_server.c
*/


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>

#define	MAXLINE	128	// maximum characters to receive and send at once
#define MAXFRAME 256


extern int swap_accept(unsigned short port);
extern int swap_disconnect(int sd);
extern int sdp_send(int sd, char *buf, int length);
extern int sdp_receive(int sd, char *buf);
extern int sdp_receive_with_timer(int sd, char *buf, unsigned int expiration);
void delay(unsigned int mseconds);
unsigned int singleCharCheckSum(char *data, int count);
int receive_metadata(int sd);

int session_id = 0;

int R = 0;	// frame number to receive


int swap_wait(unsigned short port)
{
	/*
	*	if the session is already open, then return error
	*/

	if (session_id != 0)
		return -1;

	/*
	*	accept a connection
	*/

	session_id = swap_accept(port);	// in sdp.o

	/*
	*	return a ssession id
	*/

	return session_id;
}


int swap_read(int sd, char *buf)
{
	int	n,i,j,packet_length;
	char frame[MAXFRAME],ack;
	char buffer[4],ack_frame[3];
	unsigned int checksum,selfchecksum;

	if (session_id == 0 || sd != session_id)
		return -1;
	i=0;
	ack=0;

	//receieve length from client
	packet_length=receive_metadata(sd);

	//assume server knows the length of the data
	while(i<packet_length){

		//receive from socket and store in buffer
		//buffer[0] is the data
		//buffer[1] is the seq
		//buffer[2] is left part of checksum
		//buffer[3] is right part of checksum
		n=sdp_receive(sd,buffer);
		//if error, re-receive
		if(n<0){
			continue;
		}
		
		//caculate checksum from buffer and self caculate checksum
		selfchecksum=singleCharCheckSum(buffer,2);
		checksum=((buffer[2]<<8)&0x0000FF00)|(buffer[3]&0x000000FF);

		//if no corruption && seq is expected
		if(selfchecksum==checksum&&buffer[1]==ack){
			//assign data into frame
			frame[ack]=buffer[0];
			//update i and ack
			i++;
			ack++;

			//caculate checksum, send ack with checksum
			checksum=singleCharCheckSum(&ack,1);
			ack_frame[0]=ack;
			ack_frame[1]=checksum>>8;
			ack_frame[2]=(checksum<<8)>>8;
			n=sdp_send(sd,ack_frame,sizeof ack_frame);

			//if error, keep resending
			while(n<0){
				n=sdp_send(sd,ack_frame,sizeof ack_frame);
			}

			//if no corruption but packet is not expected
		}else if (selfchecksum==checksum&&buffer[1]!=ack){
			//caculate checksum for ack and send ack+checksum
			checksum=singleCharCheckSum(&ack,1);
			ack_frame[0]=ack;
			ack_frame[1]=checksum>>8;
			ack_frame[2]=(checksum<<8)>>8;
			n=sdp_send(sd,ack_frame,sizeof ack_frame);

			//if error, keep sending
			while(n<0){
				n=sdp_send(sd,ack_frame,sizeof ack_frame);
			}

			//recevied frame have corruption
			//wait for 40ms and re-receive
		}else{
			delay(40);
			continue;
		}
		
	}

	//assign data from frame to buf
	for(j=0;j<i;j++){
		buf[j]=frame[j];
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
int receive_metadata(int sd){
	int i,n;
	char meta_data[3];
	unsigned int selfchecksum,checksum;

	if (session_id == 0 || sd != session_id)
		return -1;

	while(1){
		n=sdp_receive(sd,meta_data);

		if(n<0){
			delay(40);
			continue;
		}

		selfchecksum=singleCharCheckSum(meta_data,1);
		checksum=((meta_data[1]<<8)|meta_data[2])&0x0000FFFF;

		if(checksum==selfchecksum){
			i=meta_data[0];		
			meta_data[0]=0;
			checksum=singleCharCheckSum(meta_data,1);
			meta_data[1]=checksum>>8;
			meta_data[2]=(checksum<<8)>>8;
			n=sdp_send(sd,meta_data,3);
			while(n<0){
				n=sdp_send(sd,meta_data,3);	
			}
			break;
		}else{
			delay(40);
			continue;
		}

	}
	return i;
}