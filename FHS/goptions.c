#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main (int argc, char **argv)
{
  int aflag = 0;
  int bflag = 0;
  char *cvalue = NULL;
  int index;
  int c;

  opterr = 0;


  while ((c = getopt (argc, argv, "cmp:")) != -1)
    switch (c)
      {
      case 'c':
        aflag = 1;
        break;
      case 'm':
        bflag = 1;
        break;
      case 'b':
        bflag = 1;
        break;
      case 'v':
        bflag = 1;
        break;
      case 'p':
        cvalue = optarg;
        break;
      case '?':
        if (optopt == 'p')
          fprintf (stderr, "Option -%c requires an argument pid\n", optopt);
        else if (isprint (optopt)) // check for any printable character including space
          fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
          fprintf (stderr,
                   "Unknown option character `\\x%x'.\n",
                   optopt);
        return 1;
      default:
        abort ();
      }


  printf ("aflag = %d, bflag = %d, cvalue = %s\n",
          aflag, bflag, cvalue);

  for (index = optind; index < argc; index++)
    printf ("Non-option argument %s\n", argv[index]);
  return 0;
}

