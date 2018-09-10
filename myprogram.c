#include <unistd.h>
#include <stdio.h>

int main(){
    printf("the pid is : %d\n",getpid() );
int i;

while(1){
    printf("%d \n", i);
sleep(1);
i++;
}
}
