#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/socket.h>
#include <errno.h>


#define MAX_SIZE  5
#define CALL(x) if(!(x)&&count++)error(#x,__LINE__,errno,count)

int count=2;

void error(char *lit, int line, int erno, int cnt){
	fprintf(stderr,"this line:\n%s\ngenerated error NO:%d in line %d\n",lit, erno, line);
	printf("%s\n", strerror(errno));
	exit(cnt);
}

char* duble(char *val){
	int len=strlen(val);
	char *newVal=malloc(len+2);
	newVal[len+1]='\0';
	int c, cy=0;
	for(int i=len-1;i>=0;i--){
		c=2*(val[i]-'0')+cy;
		cy=c>=10;
		newVal[i+1]=(c%10)+'0';
	}
	if(cy){
		newVal[0]='1';
	}
	else{
		memmove(newVal, newVal+1,len+1);
	}
	return newVal;
}

int main(int argc, char **argv){
	unsigned int first=0;
	if (argc==5){
		CALL(sscanf(argv[4], "%u", &first)==1);
	}
	struct protoent *protoStruct=NULL;
	CALL((protoStruct=getprotobyname("udp"))!=NULL);
	int protoNo=protoStruct->p_proto;

	int myPortNo=0;
	CALL(sscanf(argv[1], "%d", &myPortNo)==1&&myPortNo>0&&myPortNo<65335);

	int recvPortNo=0;
	CALL((sscanf(argv[3], "%d", &recvPortNo))==1&&recvPortNo>0&&recvPortNo<65335);


	int mySocket=0;
	CALL((mySocket=socket(AF_INET, SOCK_DGRAM, protoNo))>0);
	
	struct hostent *IPstruct;
	CALL(IPstruct=gethostbyname(argv[2]));
	char *recvIP=IPstruct->h_addr;

	struct sockaddr_in myStruct=((struct sockaddr_in){
	.sin_family=AF_INET,	
	.sin_port=htons(myPortNo),
	.sin_addr.s_addr=htonl(INADDR_ANY)
	});
	memset(myStruct.sin_zero, 0,8);

	CALL((bind(mySocket, (struct sockaddr *)&myStruct,sizeof(myStruct))==0));

	struct sockaddr_in recieverStruct;
	recieverStruct.sin_family=AF_INET;	
	recieverStruct.sin_port=htons(recvPortNo);
	recieverStruct.sin_addr=*((struct in_addr *) IPstruct->h_addr);
	memset(recieverStruct.sin_zero, 0,8);
	
	char *response=NULL, *buff=NULL;
	for(;;){
		if(first){
			response=malloc(16);
			sprintf(response,"%u",first);
			first=0;
		}
		else{
			buff=malloc(MAX_SIZE+1);
			int recvSize=0;
			CALL(recvSize=recvfrom(mySocket, buff,MAX_SIZE , 0, NULL, NULL));
			buff[recvSize]='\0';
			printf("Received: %s\n", buff);
			if(recvSize>MAX_SIZE){break;}
			sleep(1);
			response=duble(buff);
			free(buff);
	}
	
		CALL(sendto(mySocket, response, strlen(response), 0, (struct sockaddr *)&recieverStruct, sizeof(recieverStruct))>0);
		printf("Sent: %s\n", response);
		if(strlen(response)>MAX_SIZE){
			break;
		}
		free(response);
}
	close(mySocket);
	return 0;
}

