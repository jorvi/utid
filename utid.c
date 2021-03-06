/* utid: set default handlers for document types based on a settings file. */

#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include "handler.h"

extern int nroles;
int verbose = 0;

struct roles rtm[] = {
    {"none", kLSRolesNone},
    {"viewer", kLSRolesViewer},
    {"editor", kLSRolesEditor},
    {"shell", kLSRolesShell},
    {"all", kLSRolesAll},
};

int main(int ac, char *av[]) {
  struct stat st = {0};
  int c = 0, err = 0, set = 0;
  int (*handler_f)(char *);
  char *path = NULL;
  char *p = NULL;

  extern int optind;
  extern char *optarg;

  while ((c = getopt(ac, av, "d:e:hl:o:st:u:Vvx:")) != -1) {
    switch (c) {
      case 'd': /* show default handler for UTI */
        return (uti_handler_show(optarg, 0));

      case 'e': /* UTI declarations for extension */
        return (utid_utis_for_extension(optarg));

      case 'h': /* help */
      default:
        err++;
        break;

      case 'l': /* list all handlers for UTI */
        return (uti_handler_show(optarg, 1));

      case 's': /* set handler */
        set = 1;
        break;

      case 'o': /* list URLs of applications able to handle file */
        return(utid_urls_for_url(optarg));

      case 't': /* info for type */
        return(utid_default_app_for_type(optarg));
        break;

      case 'u': /* UTI declarations */
        return (utid_utis(optarg));

      case 'V': /* version */
        printf("%s\n", UTID_VERSION);
        exit(0);

      case 'v': /* verbose */
        verbose = 1;
        break;

      case 'x': /* info for extension */
        return (utid_default_app_for_extension(optarg));
    }
  }

  nroles = sizeof(rtm) / sizeof(rtm[0]);

  switch ((ac - optind)) {
    case 0: /* read from stdin */
      if (set) {
        err++;
      }
      break;
    case 1: /* read from file or directory */
      if (set) {
        err++;
      }
      path = av[optind];
      break;
    case 2: /* set URI handler */
      if (set) {
        return (utid_handler_set(av[optind], av[optind + 1], NULL));
      }
      /* this fallthrough works because set == 0 */
    case 3: /* set UTI handler */
      if (set) {
        return (utid_handler_set(av[optind], av[optind + 1], av[optind + 2]));
      }
      /* fallthrough to error */
    default: /* error */
      err++;
      break;
  }

  if (err) {
  fprintf(stderr, "usage: %s [ -hvV ] [ -d uti ] [ -e ext ] [ -l uti ] [ -o path ] [ -t type ] [ -u uti ] [ -x ext ] [ settings_path ]\n", av[ 0 ]);
  fprintf(stderr, "usage: %s -s bundle_id url_scheme\n", av[ 0 ]);
  fprintf(stderr, "usage: %s -s bundle_id uti role\n", av[ 0 ]);
  exit(1);
    }

  /* by default, read from a FILE stream */
  handler_f = fsethandler;

  if (path) {
    if (stat(path, &st) != 0) {
      fprintf(stderr, "stat %s: %s\n", path, strerror(errno));
      exit(2);
    }
    switch (st.st_mode & S_IFMT) {
      case S_IFDIR: /* directory of settings files */
        handler_f = dirsethandler;
        break;

      case S_IFREG: /* settings file or plist */
        if ((p = strrchr(path, '.')) != NULL) {
          p++;
          if (strcmp(p, "plist") == 0) {
            handler_f = psethandler;
          }
        }
        break;

      default:
        fprintf(stderr, "%s: not a supported settings path\n", path);
        exit(1);
    }
  }

  return (handler_f(path));
}
