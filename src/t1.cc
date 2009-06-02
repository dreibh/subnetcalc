#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char** argv)
{
   for(int i = 30;i <= 39;i++) {
      printf("\x1b[%dmThis is foreground colour #%d!\n", i, i);
   }
   printf("\x1b[0m");
   for(int i = 40;i <= 49;i++) {
      printf("\x1b[%dmThis is background colour #%d!\n", i, i);
   }
   puts("\x1b[0m");
   return(0);
}
