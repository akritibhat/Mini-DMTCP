#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <ucontext.h>
#include <sys/mman.h>
#include "hexLibrary.h"
#include "memRegion.h"

void restore();
void restoreFile();
void stack_unmap();
ucontext_t myContext;
char image[30];
char stackstartaddr[20];
char stackendaddr[20];

int main(int argc, char* argv[])
{
   strcpy(image,argv[1]);
  printf("the pid is : %d\n",getpid() );

    restore();
    void* stackAddr=(void *)0x5300000;
    void* stack_ptr=(void *)0x5301000;
    void* addr= mmap(stackAddr, 4096, PROT_EXEC|PROT_WRITE|PROT_READ, MAP_GROWSDOWN|MAP_PRIVATE|MAP_ANONYMOUS,-1, 0); //mmap to allocate memory
       if(addr==MAP_FAILED)
       {
           printf("memory mapping failed..");
           exit(1);
       }

   asm volatile("mov %0,%%rsp;" : : "g" (stack_ptr) : "memory"); //moving stack pointer to the new memory created
   restoreFile();
}



void restore()
{
    int len_line;
    int fileinput;
    char buffer[200]="";
    int i = 0;
    char *fileName="/proc/self/maps"; //reading proc/self/maps to get stack address
  if ((fileinput = open(fileName, O_RDONLY, 0)) != -1) { //if 1
  while (read(fileinput, &buffer[i], 1) > 0) {
  if (buffer[i] == '\n') {
     i=0;
     char *startAddr;
                char *endAddr;
            int posofdash;
            int posofspace;

                char tempString[200];
                char *key = "stack";
                if(strstr(buffer, key) != NULL) { //finding the string with keyword stack
                    strcpy(tempString,buffer);


                      startAddr=strtok(buffer,"-");
                    strcpy(stackstartaddr,startAddr);

                           posofdash = strlen(startAddr)+1;
                   strcpy(buffer,tempString);
                   len_line=strlen(buffer);

                   strncpy(tempString,buffer+posofdash,len_line);
                           endAddr=strtok(tempString," ");
                         strcpy(stackendaddr,endAddr);
                                    break;
                }
              continue;
              }
        i++;
    }

    }
    else
    {
      printf("\nError: file not found");
    }

close(fileinput);
}

void stack_unmap(){
int stack_size=difference(stackendaddr,stackstartaddr) ;
long long hex= getDecimalValue(stackstartaddr);
void *poi = (void *)hex;
munmap(poi, stack_size);
}

void restoreFile()
{
     stack_unmap();
    int fileInput= open(image,O_RDONLY,0600);
    int count;
   read(fileInput,&myContext, sizeof(ucontext_t));
    read(fileInput,&count,sizeof(int));
    printf("%i",count);
    memRegion mem[count];
    for(int i=0;i<count;i++){
    read(fileInput,&mem[i], sizeof(memRegion));
    printf("\n%s",mem[i].startaddr);
               printf(" - %s",mem[i].endaddr);
                 printf("  %d",mem[i].isReadable);
                 printf("  %d",mem[i].size);
                  printf("  %d",i);
      }

         for(int i=0;i<count-1;i++){
					//printf("\nmmap at location %d %s size: %d r:%d", i, mem[i].startaddr, mem[i].size, mem[i].isReadable);
						long long hex= getDecimalValue(mem[i].startaddr);
            void* stackAddr=(void *)hex;
						void * addr= mmap(stackAddr, mem[i].size, PROT_EXEC|PROT_WRITE|PROT_READ,MAP_FIXED|MAP_GROWSDOWN|MAP_PRIVATE|MAP_ANONYMOUS,-1, 0);
            if(addr==MAP_FAILED)
                 {
                     printf("fail");
                 }
			//		printf("size \n %d",read(fileInput,(void *)hex,mem[i].size));
		}

    for(int v=0;v<count-1;v++){
      printf("\nin readable");
    				if(mem[v].isReadable){
           printf("\nat location*%d*%s*size:*%d*r:*%d*", v, mem[v].startaddr, mem[v].size, mem[v].isReadable);
    				   long long starinhexa=getDecimalValue(mem[v].startaddr);
      					 int address =read(fileInput,(void *)starinhexa,mem[v].size);
    						printf(" index:%d:  %d",v,address);

    				}
    }
    setcontext(&myContext);
    exit(0);
}
