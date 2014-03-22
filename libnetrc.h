#ifndef HEADER_LIBNETRC_H
#define HEADER_LIBNETRC_H

#define NETRC_FAILURE          -1
#define NETRC_SUCCESS           0
#define NETRC_MACHINE_NOT_FOUND 1
#define NETRC_LOGIN_NOT_FOUND   2

struct netrc_entry
{
  char *         machine;
  char *         login;
  char *         password;
  unsigned short port;
};

/**
 * @return NETRC_FAILURE
 *         NETRC_SUCCESS
 *         NETRC_MACHINE_NOT_FOUND
 *         NETRC_LOGIN_NOT_FOUND
 */
int search_in_netrc (struct netrc_entry * entry_ptr, char * filename);

#endif
