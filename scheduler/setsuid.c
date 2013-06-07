
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

int
main(int argc, char* argv[])
{
   if (chown("./sleeper", 0, 0) != 0) 
      perror("chown of sleeper"), exit(-1);

   mode_t mode = S_ISUID | 0777;
   if (chmod("./sleeper", mode) != 0)
      perror("chmod of sleeper"), exit(-1);

   return 0;
}
