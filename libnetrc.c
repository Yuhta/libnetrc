#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <alloca.h>
#include <error.h>
#include <sys/stat.h>
#include <unistd.h>

#include "libnetrc.h"

static char *
default_path ()
{
  char * home = getenv ("HOME");
  if (!home) error (EXIT_FAILURE, 0, "cannot get home directory");
  int home_len = strlen (home);
  char * p = malloc (home_len + strlen ("/.netrc.gpg") + 1);
  if (!p) error (EXIT_FAILURE, 0, "failed to allocate string");
  strcpy (p, home);

  strcpy (p + home_len, "/.netrc");
  if (access (p, R_OK) == 0) return p;
  strcpy (p + home_len, "/.netrc.gpg");
  if (access (p, R_OK) == 0) return p;
  return NULL;
}

static void
check_permission (const char * path)
{
  if (access (path, F_OK) != 0)
    error (EXIT_FAILURE, 0, "file `%s' doesn't exist", path);
  struct stat s;
  stat (path, &s);
  if ((s.st_mode & 0777) != 0600)
    error (0, 0, "wrong permission for `%s', should be 600", path);
}

static int
suffix (const char * suf, const char * s)
{
  const char * s_tail = s + strlen (s) - strlen (suf);
  if (s_tail < s) return 0;
  return strcmp (s_tail, suf) == 0;
}

static FILE *
decode (const char * path)
{
  char * cmd0 = "gpg --batch --quiet --decrypt ";
  char * cmd  = alloca (strlen (cmd0) + strlen (path) + 1);
  sprintf (cmd, "%s%s", cmd0, path);
  return popen (cmd, "r");
}

enum netrc_machine_lookup_state
{
  NOTHING,
  MACHINE_KEYWORD,
  MACHINE_FOUND
};

static int
parse (struct netrc_entry * entry_ptr, FILE * input)
{
  char * buf         = NULL;
  size_t buf_size    = 0;
  int specific_login = entry_ptr->login && *(entry_ptr->login);
  int result         = NETRC_MACHINE_NOT_FOUND;
  enum netrc_machine_lookup_state state = NOTHING;
  int state_login    = 0, state_login_found = 0;
  int state_password = 0;
  int state_port     = 0;

  while (getline (&buf, &buf_size, input) != -1)
    {
      char * tok_save_ptr;
      char * tok = strtok_r (buf, " \t\n", &tok_save_ptr);
      while (tok)
        {
          switch (state)
            {
            case NOTHING:
              if (strcmp (tok, "machine") == 0) state = MACHINE_KEYWORD;
              break;
            case MACHINE_KEYWORD:
              if (strcmp (tok, entry_ptr->machine) == 0)
                {
                  state  = MACHINE_FOUND;
                  result = NETRC_SUCCESS;
                }
              else state = NOTHING;
              break;
            case MACHINE_FOUND:
              if (state_login)
                {
                  if (specific_login)
                    state_login_found =
                      strcmp (tok, entry_ptr->login) == 0;
                  else
                    {
                      if (entry_ptr->login) free (entry_ptr->login);
                      entry_ptr->login = strdup (tok);
                      if (!entry_ptr->login)
                        {
                          error (0, 0, "failed to allocate string");
                          result = NETRC_FAILURE;
                          goto end;
                        }
                    }
                  state_login = 0;
                }
              else if (state_password)
                {
                  if (state_login_found || !specific_login)
                    {
                      if (entry_ptr->password)
                        free (entry_ptr->password);
                      entry_ptr->password = strdup (tok);
                      if (!entry_ptr->password)
                        {
                          error (0, 0, "failed to allocate string");
                          result = NETRC_FAILURE;
                          goto end;
                        }
                    }
                  state_password = 0;
                }
              else if (state_port)
                {
                  if (state_login_found || !specific_login)
                    entry_ptr->port = atoi (tok);
                  state_port = 0;
                }
              else if (strcmp (tok, "login")    == 0)
                state_login    = 1;
              else if (strcmp (tok, "password") == 0)
                state_password = 1;
              else if (strcmp (tok, "port")     == 0)
                state_port     = 1;
              else if (strcmp (tok, "machine")  == 0 &&
                       entry_ptr->login    && *(entry_ptr->login) &&
                       entry_ptr->password && *(entry_ptr->password))
                goto end;
              break;
            }
          tok = strtok_r (NULL, " \t\n", &tok_save_ptr);
        }
    }

 end:
  if (buf) free (buf);
  if (!(entry_ptr->password && *(entry_ptr->password)))
    result = NETRC_LOGIN_NOT_FOUND;
  return result;
}

int
search_in_netrc (struct netrc_entry * entry_ptr, char * filename)
{
  int filename_alloc = 0;
  if (filename == NULL)
    {
      filename       = default_path ();
      filename_alloc = 1;
    }
  check_permission (filename);

  FILE * in;
  int encoded = suffix (".gpg", filename);
  int status;
  if (encoded) in = decode (filename);
  else         in = fopen (filename, "r");
  if (in == NULL)
    {
      error (0, 0, "failed to open file: %s", filename);
      status = NETRC_FAILURE;
    }
  else
    {
      status = parse (entry_ptr, in);
      if ((encoded ? pclose (in) : fclose (in)) != 0)
        {
          error (0, 0, "error closing netrc file");
          status = NETRC_FAILURE;
        }
    }

  if (filename_alloc) free (filename);
  return status;
}
