#include "stdio.h"


int main(void){

  int x = -1;

  int i = sizeof(int) * 8;

  printf("size = %d\n",i);
 
  for(int j = i; j > 0; j--){
    printf("%c",((1 << j) & x)+'0');
  }

  x = 0x80000000;
  printf("\nx=%d",x);

  return 0;
}
