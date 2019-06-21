#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include<time.h>

unsigned int singleCharCheckSum(char *data, int count);
void delay(unsigned int mseconds);

int main(void) {
    /*char frame[4],test[2],ack;
    unsigned int checksum, self_checksum;
    int i;
    frame[0]=49;
    frame[1]=0;
    frame[2]="\0";
    printf("%i \n",frame[1]==frame[2]);
    printf("%i \n",frame[2]);*/
    /*FILE *fptr;

    fptr=fopen("testfile.txt","r");
    if(fptr==NULL){
        printf("Error, can not open the file");
    }*/
    if(0){
        printf("0\n");
    }

    
    return 0;
}

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
void delay(unsigned int mseconds){
	clock_t goal=mseconds+clock();
	while (goal>clock());
}