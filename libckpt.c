#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <ucontext.h>
#include "hexLibrary.h"
#include "memRegion.h"

int flag;
void sig_handler(int signum);
void __attribute__((constructor))myconstructor();
void myconstructor(){
		signal(SIGUSR2, sig_handler);
		//signal(SIGINT, sig_handler);    //signal handler
}


void sig_handler(int signum){

	ucontext_t context;

	int pid=getpid();

	getcontext(&context); //saving context
	if(getpid()!=pid){ //setting pid as a flag
		printf("i am back in libckpt!!!");
			return ;
	}

		char addrstr[80];
		int count=1;
		 int co=0;
		char line[400];
	  int tempChar;
	  unsigned int tempCharIdx = 0U;
		strcpy(addrstr, "/proc/self");
		strcat(addrstr,"/maps");
	  
	  int filep,fileOutput;
	  char c;
	  filep= open(addrstr,O_RDONLY,0600);

while(read(filep, &c, 1)>0){
if (c == '\n')
				 count = count + 1;
}
close(filep);

filep= open(addrstr,O_RDONLY,0600); //opening proc/self/maps

remove("myckpt");

if((fileOutput =open("myckpt", O_CREAT|O_WRONLY,0600))<0)  //creating mychpkt image
		{
						 printf("An error occured.");
		}

memRegion *memArray=(memRegion*)malloc(count*sizeof(memRegion));

while(read(filep, &tempChar, 1)>0){
					if (tempChar == '\n') {
					            line[tempCharIdx] = '\0';
					            tempCharIdx = 0U;

					 	  char *startAddr;
					  	char *endAddr;
						  int posofdash;
						  int posofspace;
						  char linetemp[200];

							strcpy(linetemp,line);
							startAddr=strtok(line,"-");

							//printf("\n%i: %s-",co,startAddr);

							strcpy(memArray[co].startaddr,startAddr);
							posofdash = strlen(startAddr) +1;
							strcpy(line,linetemp);

							int lenofline=strlen(line);

							strncpy(linetemp,line+posofdash,lenofline);
							endAddr=strtok(linetemp," ");

							posofspace= posofdash+strlen(endAddr) +1;
							char lineper[4];

							lenofline=posofspace+4;

							//printf("%s",endAddr);

							strcpy(memArray[co].endaddr,endAddr);

							//printf("\n %s %s",memArray[co].startaddr, memArray[co].endaddr);
							//memArray[co].size = difference(memArray[co].endaddr,memArray[co].startaddr) ;
							int diff= difference(memArray[co].endaddr,memArray[co].startaddr);
							//printf("\n %i size:", diff);
							memArray[co].size=diff;
						 printf("\n after copy size  %i",memArray[co].size);

							strcpy(linetemp,line+posofspace);

							strncpy(lineper,linetemp,4);

							char *token;
						  	token=strchr (lineper, 'r');

							if (token!=NULL){
								memArray[co].isReadable=1;
							}

							token=strchr (lineper, 'w');

							if (token!=NULL){
								memArray[co].isWritable=1;
							}

							token=strchr (lineper, 'x');

							if (token!=NULL){
								memArray[co].isExecutable=1;
							}

							co++;
										continue;
										}
										        else
										            line[tempCharIdx++] = (char)tempChar;
										    	}

													for(int i=0;i<count;i++){
														//printf("\n %s %s",memArray[i].startaddr, memArray[i].endaddr);
													}


													 write(fileOutput,&context,sizeof(ucontext_t)); //writing ucontext to file

													 if( write(fileOutput,&count,sizeof(int))!= sizeof(int))
													 {
														 printf("could not write to file");
													 }

													 for(int i=0;i<count;i++){
														 printf("\n %s %s ",memArray[i].startaddr, memArray[i].endaddr); //writing data of proc/self/maps to fle
 													if(write(fileOutput, &memArray[i], sizeof(memRegion))!=sizeof(memRegion)){
														printf("erro copying mem address");
													}
 													}

													for(int i=0;i<count-1;i++){
														printf("\n copying memory %s %s",memArray[i].startaddr, memArray[i].endaddr);
												    if(memArray[i].isReadable){
												    long long hexa= getDecimalValue(memArray[i].startaddr);
														printf("%llx ---- ",hexa );
														//writing data within memory segments to the file
												 if(write(fileOutput,(void *)hexa,memArray[i].size)!=memArray[i].size){
												 printf("error copying memory at %s\n",memArray[i].startaddr);
													}

												  }
													else
									    		{
									    			printf("the memory is non readable\n");
									    		}
												  }

													close(fileOutput);
													exit(0);
}
