/* Force-included (-include): prototypes for K&R functions whose callers
 * guess the wrong signature. Native builds shrug; wasm traps on them. */
#include <stdarg.h>
#include <time.h>
struct monst;
void dump_init(void), dump_blockquote_start(void), dump_blockquote_end(void);
void dump(const char *, const char *), dump_line(const char *, const char *);
void dump_title(char *), dump_subtitle(const char *), dump_header_html(const char *);
void livelog_wish(char *), livelog_bones_killed(struct monst *), livelog_game_action(const char *);
void livelog_shoplifting(const char *, const char *, long), livelog_game_started(const char *, const char *);
void livelog_genocide(const char *, int);
void set_occupation(int (*)(), char *, int), savech(int), pushch(int);
long yyyymmdd(time_t), hhmmss(time_t);
char *iso8601(time_t);
time_t current_epoch(void);
int pline(const char *, ...), panic(const char *, ...), impossible(const char *, ...), error(const char *, ...);
