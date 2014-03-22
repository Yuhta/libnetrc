#include <stdio.h>
#include <stdlib.h>

#include <error.h>

#include "libnetrc.h"

#define null2str(x) ((x) == NULL ? "NULL" : (x))

int
main (int argc, char ** argv)
{
  if (argc < 2)
    {
      fprintf (stderr, "Usage: %s MACHINE [LOGIN]\n", argv[0]);
      exit (EXIT_FAILURE);
    }

  struct netrc_entry entry;
  entry.machine  = argv[1];
  entry.login    = argc > 2 ? argv[2] : NULL;
  entry.password = NULL;
  char * fn      = getenv("NETRC_FILE");

  int status = search_in_netrc (&entry, fn);
  printf ("Machine:   %s\n"         "Login:     %s\n"
          "Password:  %s\n"         "Port:      %d\n"
          "File Name: %s\n"         "Status:    %d\n",
          null2str (entry.machine),  null2str (entry.login),
          null2str (entry.password), entry.port,
          null2str (fn),             status);

  if (entry.login && argc == 2) free (entry.login);
  if (entry.password)           free (entry.password);
  return 0;
}
