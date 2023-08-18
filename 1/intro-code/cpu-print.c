#include <unistd.h>
#include <stdio.h>
#include <sys/time.h>

int main(int argc, char *argv[]) 

{
  unsigned int i;
  int count = 0;
  struct timeval tv;
  
  while(count < 10)
    {
      for(i = 0; i < 1000; i++)
	{
	  gettimeofday(&tv, NULL);
	  //printf("%lu sec, %lu usec\n", tv.tv_sec, tv.tv_usec);
	  printf("a");
	}
      count++;
      printf("round %d complete\n", count);
    }
}
