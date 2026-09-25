/* Web build: no tty; gtty/stty fail as they do on the X11 build (port/vt.c). */
#include <sys/ioctl.h>
struct sgttyb { char sg_ispeed, sg_ospeed, sg_erase, sg_kill; int sg_flags; };
struct ltchars { char t_suspc, t_dsuspc, t_rprntc, t_flushc, t_werasc, t_lnextc; };
#define TIOCGLTC 0x7474
#define TIOCSLTC 0x7475
#ifndef ECHO
#define ECHO 0x08
#endif
#define CBREAK 0x02
#define XTABS 0x0c00
#define CRMOD 0x10
static int gtty(int fd, struct sgttyb *b) { return -1; }
static int stty(int fd, struct sgttyb *b) { return -1; }
