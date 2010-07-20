#include <stdio.h>
#include <stdlib.h>

#include <sys/time.h>
#include <ldap.h>

struct ldap_config {
  int enabled;
  int auth_method;
  int desired_version;
  char *ldap_url;
  char *binddn;
  char *bindpw;
  char *basedn;
  char *attr_hwaddr;
  char *attr_clid;
  char *attr_hostname;
  char *attr_ip;
};

struct ldap_connection {
  int status;
  LDAP *ld;
  LDAPMessage *result;
  LDAPMessage *e;
  struct dhcp_config *ldap_dhcp_config;
};

struct ldap_connection ldapconn;
struct ldap_config     ldapconf;

int open_ldap_conn (void);
int search_ldap_attr (char* attrname, char* value);
void close_ldap_conn (void);

int main(int argc, char *argv[])
{
    char buffer[200];

    ldapconf.enabled = 1;
    ldapconf.auth_method     = LDAP_AUTH_SIMPLE;
    ldapconf.desired_version = LDAP_VERSION3;

    ldapconf.attr_hostname = "associatedDomain";
    ldapconf.attr_ip = "aRecord";
    ldapconf.attr_hwaddr = "dhcpHWAddr";
    ldapconf.attr_clid = "dhcpCLID";
    ldapconf.ldap_url = "ldap://ldap.scalaxy.ru";
    ldapconf.binddn = "uid=dhcpd,ou=virtual,ou=users,o=scalaxy";
    ldapconf.basedn = "ou=hosts,o=scalaxy";
    ldapconf.bindpw = "976a0cd522dbb06fdd8063dc74c7b1dc";

    open_ldap_conn();

    search_ldap_attr(ldapconf.attr_clid, buffer);
    fprintf(stderr, "waking\n");

    fprintf(stderr, "sleeping\n");
    sleep(1);
    fprintf(stderr, "waking\n");
    search_ldap_attr(ldapconf.attr_clid, buffer);
    fprintf(stderr, "sleeping\n");
    sleep(1);
    fprintf(stderr, "waking\n");
    search_ldap_attr(ldapconf.attr_clid, buffer);
    close_ldap_conn();
    return 0;
}

int open_ldap_conn (void)
{
    int rc;
    struct timeval timeout = { 0, 500000 };
    struct berval passwd;

  passwd.bv_val = ldapconf.bindpw;
  passwd.bv_len = strlen( passwd.bv_val );

  ldapconn.status = 0;
  if (!ldapconn.ld) // Create new if it is empty
    {
      fprintf(stderr, "connecting LDAP server %s\n", ldapconf.ldap_url);
      if ((rc = ldap_initialize(&(ldapconn.ld), ldapconf.ldap_url)) != LDAP_SUCCESS )
        {
          fprintf(stderr, "LDAP init failed: %s\n", ldap_err2string(rc));
          return 1;
        }
    }
  if (ldap_set_option(ldapconn.ld, LDAP_OPT_PROTOCOL_VERSION, (void *)&(ldapconf.desired_version)) != LDAP_OPT_SUCCESS)
    {
      fprintf(stderr, "Setting LDAP option failed\n");
      return 2;
    }
  if (ldap_set_option(ldapconn.ld,  LDAP_OPT_NETWORK_TIMEOUT, (void *)&timeout) != LDAP_OPT_SUCCESS)
    {
      fprintf(stderr, "Setting LDAP option failed\n");
      return 2;
    }

  fprintf(stderr, "binding LDAP server %s as user %s\n", ldapconf.ldap_url, ldapconf.binddn);
  rc = ldap_sasl_bind_s(ldapconn.ld, ldapconf.binddn, LDAP_SASL_SIMPLE, &passwd, NULL, NULL, NULL);
  if (rc != LDAP_SUCCESS)
    {
      fprintf(stderr, "Could not bind to LDAP: %s\n", ldap_err2string(rc));
      return 3;
    }

  ldapconn.status = 1;
  fprintf(stderr, "LDAP server connected\n");

}

void close_ldap_conn (void)
{
  fprintf( stderr, "unbinding LDAP\n" );
  ldap_msgfree (ldapconn.result);
  ldap_unbind_ext_s(ldapconn.ld, NULL, NULL);
  ldapconn.status = 0;
}


// search for entry by pair attribute-value
int search_ldap_attr (char* attrname, char* value)
{
  char *filter, *buf;
  int rc = LDAP_SERVER_DOWN;
  struct timeval timeout = { 0, 500000 };
  // cleanup

  filter = (char *) malloc (strlen(attrname) + strlen(value) + 10);
  sprintf(filter, "(%s=%s)", attrname, value);

  if (ldapconn.result) ldap_msgfree (ldapconn.result);
  ldapconn.result = NULL;
  ldapconn.e = NULL;
  // perform search
    fprintf(stderr, "search\n");
  rc = ldap_search_ext_s(ldapconn.ld, ldapconf.basedn, LDAP_SCOPE_SUBTREE, filter,
                     NULL, 0, NULL, NULL, &timeout, 0, &ldapconn.result);
  if (rc != LDAP_SUCCESS)
    {
      fprintf(stderr, "LDAP search failed: %s\n", ldap_err2string(rc));
      free(ldapconn.ld);
      ldapconn.ld = NULL;
      open_ldap_conn();
      free(filter);
      return -1;
    }

  // get first entry
  ldapconn.e = ldap_first_entry(ldapconn.ld, ldapconn.result);
  if (ldapconn.e == NULL)
    {
      fprintf(stderr, "LDAP: filter %s: no entries found\n", filter);
      free(filter);
      return 0;
    }
  buf = ldap_get_dn(ldapconn.ld, ldapconn.e);
  fprintf(stderr, "LDAP: first found entry DN on filter %s: %s\n", filter, buf);
  free(filter);
  ldap_memfree (buf);
  if (ldapconn.result) ldap_msgfree (ldapconn.result);
  return 1;
}

