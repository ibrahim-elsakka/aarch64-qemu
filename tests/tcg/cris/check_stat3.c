/* Simulator options:
#sim: --sysroot=@exedir@
*/
#include <sys/types.h>
#include <sys/stat.h>
#ifndef _MSC_VER
#include <unistd.h>
#endif
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main (int argc, char *argv[])
{
  char path[1024] = "/";
  struct stat buf;

  strncat(path, argv[0], sizeof(path) - 2);
  if (stat (".", &buf) != 0
      || !S_ISDIR (buf.st_mode))
    abort ();
  if (stat (path, &buf) != 0
      || !S_ISREG (buf.st_mode))
    abort ();
  printf ("pass\n");
  exit (0);
}
