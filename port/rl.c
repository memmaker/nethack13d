/* RVIP: auto-explore (_), walk-to-stairs (< >), the command menu (Enter),
 * the inventory with a cursor (i) and the cursor list in item prompts.
 * rhack() calls rl_parse() instead of parse(); getobj() calls rl_pick()
 * and takes rl_obj when the inventory menu already chose the object. */
#include "hack.h"
#include "vt.h"

extern char *parse(), sdir[];
extern schar xdir[], ydir[];
extern struct obj *o_at(), *sobj_at();
extern struct gold *g_at();
extern struct trap *t_at();
extern struct monst *m_at();
extern int canseemon();
extern char obj_to_let();
extern char *doname();
#include "func_tab.h"

static char mode;                   /* 0, '_', '<' or '>' */
static char known[COLNO][ROWNO], stood[COLNO][ROWNO];
static int lvl = -1, lastx, lasty, lastmsg;

static int ok(int x, int y) { return x >= 0 && x < COLNO && y >= 0 && y < ROWNO; }

static int pass(int x, int y)
{
    struct trap *t;
    return ok(x, y) && known[x][y] && ACCESSIBLE(levl[x][y].typ) &&
           !((t = t_at(x, y)) && t->tseen) && !sobj_at(ENORMOUS_ROCK, x, y);
}

static int target(int x, int y)
{
    int i;
    if (mode == '<') return x == xupstair && y == yupstair;
    if (mode == '>') return x == xdnstair && y == ydnstair;
    if (stood[x][y]) return 0;
    if (o_at(x, y) || g_at(x, y)) return 1;
    for (i = 0; i < 8; i++)
        if (ok(x + xdir[i], y + ydir[i]) && !known[x + xdir[i]][y + ydir[i]]) return 1;
    return 0;
}

/* BFS from the player; returns the key for the first step, 0 if none. */
static int path(void)
{
    static short q[COLNO * ROWNO];
    static schar from[COLNO][ROWNO];
    int h = 0, t = 0, i, x, y, nx, ny;
    struct monst *m;

    memset(from, -1, sizeof from);
    from[u.ux][u.uy] = 8;
    q[t++] = u.ux * ROWNO + u.uy;
    while (h < t) {
        x = q[h] / ROWNO, y = q[h++] % ROWNO;
        if ((x != u.ux || y != u.uy) && target(x, y)) {
            while (from[x][y] != 8) {   /* walk back to the first step */
                i = from[x][y];
                nx = x - xdir[i], ny = y - ydir[i];
                if (nx == u.ux && ny == u.uy) return sdir[i];
                x = nx, y = ny;
            }
        }
        for (i = 0; i < 8; i++) {
            nx = x + xdir[i], ny = y + ydir[i];
            if (!pass(nx, ny) || from[nx][ny] != -1) continue;
            if (h == 1 && (m = m_at(nx, ny)) && m->mdispl && !m->mtame) continue;   /* pets swap; others ask or fight */
            if (xdir[i] && ydir[i] && (levl[x][y].typ == DOOR || levl[nx][ny].typ == DOOR)) continue;
            from[nx][ny] = i;
            q[t++] = nx * ROWNO + ny;
        }
    }
    return 0;
}

static int threat(void)
{
    struct monst *m;
    for (m = fmon; m; m = m->nmon)
        if (!m->mtame && !m->mpeaceful && !m->mimic && canseemon(m)) return 1;
    return 0;
}

static int step(void)
{
    int x, y;

    if (dlevel != lvl) return 0;
    for (x = 0; x < COLNO; x++)
        for (y = 0; y < ROWNO; y++) known[x][y] |= levl[x][y].seen;
    known[u.ux][u.uy] = stood[u.ux][u.uy] = 1;
    if (mode == '<' && u.ux == xupstair && u.uy == yupstair) return mode = 0, '<';
    if (mode == '>' && u.ux == xdnstair && u.uy == ydnstair) return mode = 0, '>';
    if (vt_msgs != lastmsg || be_getkey(0) >= 0) return 0;
    if (lastx == u.ux && lasty == u.uy) return 0;   /* last step did not move */
    if (mode == '_' && threat()) return 0;   /* stairs walk: messages stop it */
    lastx = u.ux, lasty = u.uy;
    if (!(x = path()) && mode == '_') pline("Nothing left to explore. Search (s) for hidden doors.");
    return x;
}

static void start(char c)
{
    mode = c, lastx = lasty = -1, lastmsg = vt_msgs;
    if (dlevel != lvl) memset(known, 0, sizeof known), memset(stood, 0, sizeof stood), lvl = dlevel;
}

/* ---- Enter menu: the command lines of the help file ("\t<key>\t<text>") ---- */
static int cmd_menu(void)
{
    static char line[100][80], key[100];
    static const char *ext[100];   /* "#cast": pushed after the # */
    static const char *it[100];
    static int n, cur;
    char b[200], *t;
    FILE *f;

    if (!n && (f = fopen(HELP, "r"))) {
        int on = 0;
        while (n < 100 && fgets(b, sizeof b, f)) {
            if (!strncmp(b, "Commands:", 9)) on = 1;
            if (!on || b[0] != '\t' || b[1] == '\t') continue;
            int ctl = b[1] == '^' && b[2] != '\t';  /* "^Z" but not "^" */
            t = b[2 + ctl] == '\t' ? b + 3 + ctl : 0;
            if (!t) continue;                        /* "kjhlyubn - ..." */
            key[n] = ctl ? b[2] & 037 : b[1];
            t[strcspn(t, "\n")] = 0;
            snprintf(line[n], 80, "%-2.*s %.66s", 1 + ctl, b + 1, t);
            it[n] = line[n];
            n++;
        }
        fclose(f);
        for (struct ext_func_tab *e = extcmdlist; e->ef_txt && n < 100; e++)
            if (*e->ef_txt != '?')
                key[n] = '#', ext[n] = e->ef_txt, snprintf(line[n], 80, "#  %s: %.60s", e->ef_txt, e->ef_desc), it[n] = line[n], n++;
    }
    if (!n) return 0;
    cur = vt_menu(it, n, cur);
    if (vt_menukey == '\n' || vt_menukey == ' ' || vt_menukey == '5') {
        if (ext[cur]) vt_push(ext[cur]), vt_push("\n");
        return key[cur];
    }
    for (int i = 0; i < n; i++) if (key[i] == vt_menukey) return cur = i, key[i];
    return 0;
}

/* ---- inventory: letter = main action, Shift = drop, Ctrl = examine ---- */
struct obj *rl_obj;          /* getobj() returns this once */
static int reopen;

static struct obj *nth(int i) { struct obj *o = invent; while (o && i--) o = o->nobj; return o; }
static struct obj *bylet(int c) { struct obj *o; for (o = invent; o; o = o->nobj) if (obj_to_let(o) == c) return o; return 0; }

static int worn(struct obj *o) { return (o->owornmask & (W_ARMOR | W_RING)) != 0; }

static int main_act(struct obj *o)
{
    switch (o->olet) {
    case FOOD_SYM: return 'e';
    case POTION_SYM: return 'q';
    case SCROLL_SYM: return 'r';
    case WAND_SYM: return 'z';
    case TOOL_SYM: return 'a';
    case WEAPON_SYM: return 'w';
    case ARMOR_SYM: return worn(o) ? 'T' : 'W';
    case RING_SYM: return worn(o) ? 'R' : 'P';
    case SPBOOK_SYM: return 'X';
    }
    return '*';
}

static const char *act_name(int k)
{
    switch (k) {
    case 'e': return "eat";      case 'q': return "quaff";   case 'r': return "read";
    case 'z': return "zap";      case 'a': return "apply";   case 'w': return "wield";
    case 'T': return "take off"; case 'W': return "wear";    case 'R': return "remove";
    case 'P': return "put on";   case 't': return "throw";   case 'd': return "drop";
    case 'X': return "transcribe"; case '#': return "dip";
    }
    return "examine";
}

/* run command k on o: '*' examines, else getobj() gets o through rl_obj */
static char *act(int k, struct obj *o)
{
    static char b[2];
    reopen = 1;
    if (k == '*') { pline("%s", doname(o)); return 0; }
    rl_obj = o;
    if (k == '#') vt_push("dip\n");
    if (k == 'R' && uleft && uright) vt_push(o == uleft ? "l" : "r");
    b[0] = k;
    return b;
}

static char *item_menu(struct obj *o)
{
    static char line[8][40];
    const char *it[8];
    int k[8], n = 0, m = main_act(o), c;

    k[n++] = m;
    if (m != 'w' && !worn(o) && o != uwep) k[n++] = 'w';
    if (!worn(o)) k[n++] = 't', k[n++] = '#', k[n++] = 'd';
    if (m != '*') k[n++] = '*';
    for (int i = 0; i < n; i++) snprintf(line[i], 40, "%c  %s", k[i], act_name(k[i])), it[i] = line[i];
    c = vt_menu(it, n, 0);
    if (vt_menukey == '\n' || vt_menukey == ' ' || vt_menukey == '5' || vt_menukey == BE_RIGHT || vt_menukey == '6')
        return act(k[c], o);
    if (vt_menukey == '+') return act(m, o);
    if (vt_menukey == '-' && !worn(o)) return act('d', o);
    if (vt_menukey == '*') return act('*', o);
    for (int i = 0; i < n; i++) if (k[i] == vt_menukey) return act(k[i], o);
    return 0;
}

static char *inv_menu(void)
{
    static char line[52][80];
    static int cur;
    const char *it[52];
    struct obj *o;
    int n = 0, k;

    reopen = 0;
    for (o = invent; o && n < 52; o = o->nobj, n++)
        snprintf(line[n], 80, "%c - %.74s", obj_to_let(o), doname(o)), it[n] = line[n];
    if (!n) { pline("You are empty handed."); return 0; }
    for (;;) {
        if (cur >= n) cur = n - 1;
        cur = vt_menu(it, n, cur);
        k = vt_menukey, o = nth(cur);
        if (k == '\n' || k == ' ' || k == '5' || k == BE_RIGHT || k == '6') {
            char *r = item_menu(o);
            if (r) return r;
            continue;
        }
        if (k == '+') return act(main_act(o), o);
        if (k == '-') return act('d', o);
        if (k == '*') return act('*', o), (char *)0;
        if (k >= 1 && k <= 26 && (o = bylet(k + 'a' - 1))) return act('*', o), (char *)0;
        if ((o = bylet(k))) return act(main_act(o), o);
        if (k >= 'A' && k <= 'Z' && (o = bylet(k - 'A' + 'a'))) return act('d', o);
        if (k == 033 || k == '0' || k == '.' || k == BE_LEFT || k == '4' || k == 'i') return 0;
        if (k < 0x100) { static char b[2]; b[0] = k; return b; }   /* a normal command */
    }
}

/* item prompt: cursor list of the allowed letters, else the typed key */
int rl_pick(const char *lets)
{
    const char *it[52];
    static char line[52][80];
    char let[52];
    struct obj *o;
    int n = 0, c;

    extern char readchar(void);
    if (vt_queued() || !lets) return readchar();
    for (o = invent; o && n < 52; o = o->nobj)
        if (index(lets, obj_to_let(o)))
            let[n] = obj_to_let(o), snprintf(line[n], 80, "%c - %.74s", let[n], doname(o)), it[n] = line[n], n++;
    if (!n) return readchar();
    c = vt_menu(it, n, 0);
    if (vt_menukey == '\n' || vt_menukey == ' ' || vt_menukey == '5') return let[c];
    if (vt_menukey == '0' || vt_menukey == BE_LEFT) return 033;
    return vt_menukey < 0x100 ? vt_menukey : 033;
}

int rl_saved, rl_at_prompt;
static char *parse1(void);
char *rl_parse(void)
{
    char *c;
    rl_saved = 0;
    c = parse1();
    rl_saved = c[0] == 'S' && !c[1];    /* web: keep the save file at exit */
    return c;
}

#include <fcntl.h>
/* Web autosave (X11: HACK_AUTOSAVE=1 tests it at every prompt): dosave0()
   writes the save and tears the game down, so restore it at once and put
   the file back (dorecover() deletes it). */
void rl_autosave(void)
{
    extern char SAVEF[];
    extern int dosave0(), dorecover();
    extern int redotoplin(), done();
    int fd, k, ph = flags.moonphase, tl = flags.toplin;
    long n = 0;
    char *buf = NULL, *d0[NROFOBJECTS + 2];
    FILE *f;

    flags.toplin = tl ? 2 : 0;          /* docrt() in dorecover: no --More-- */
    if (!dosave0(1)) { flags.toplin = tl; return; }
    if ((f = fopen(SAVEF, "rb"))) {
        fseek(f, 0, SEEK_END); n = ftell(f); rewind(f);
        if ((buf = malloc(n))) n = fread(buf, 1, n, f);
        fclose(f);
    }
    for (k = 0; k < NROFOBJECTS + 2; k++) d0[objects[k].oc_descr_i] = objects[k].oc_descr;
    for (k = 0; k < NROFOBJECTS + 2; k++) objects[k].oc_descr = d0[k];   /* restnames expects unshuffled */
    uarm = uarm2 = uarmh = uarms = uarmg = uwep = uball = uchain = uleft = uright = 0;  /* freed; setworn() */
    if ((fd = open(SAVEF, 0)) < 0 || !dorecover(fd)) { free(buf); done("tricked"); }
    if (tl) redotoplin();               /* the message stays up */
    flags.toplin = tl;
    flags.moonphase = ph;
    if (ph == FULL_MOON) u.uluck++;     /* dosave0 took it */
    if (buf && (f = fopen(SAVEF, "wb"))) { fwrite(buf, 1, n, f); fclose(f); }
    free(buf);
}

static char *parse1(void)
{
    static char b[2];
    char *cmd;

    rl_obj = 0;   /* the last command is done */
    for (;;) {
        if (reopen && !threat() && !mode) { if ((cmd = inv_menu())) return cmd; continue; }
        reopen = 0;
        if (mode) {
            if ((b[0] = step())) {
                lastmsg = vt_msgs;
                return b;
            }
            mode = 0;
        }
        rl_at_prompt = 1;
        cmd = parse();
        rl_at_prompt = 0;
        if (cmd[1] || multi) return cmd;
        if (cmd[0] == '\n' || cmd[0] == '\r') { int k = cmd_menu(); if (!k) continue; b[0] = k; cmd = b; }
        if (cmd[0] == 'i') { if ((cmd = inv_menu())) return cmd; continue; }
        if (cmd[0] == '_') start('_');
        else if ((cmd[0] == '<' && (u.ux != xupstair || u.uy != yupstair) && levl[xupstair][yupstair].seen) ||
                 (cmd[0] == '>' && (u.ux != xdnstair || u.uy != ydnstair) && levl[xdnstair][ydnstair].seen))
            start(cmd[0]);
        else return cmd;
    }
}

/* web Inventory window: one line per item, at the command prompt only */
/* Angband's colour for an object class (RVIP W0: colours come from the game) */
const char *rl_css(int olet)
{
    switch (olet) {
    case AMULET_SYM: return "#ff9000";
    case FOOD_SYM:   return "#d09050";
    case WEAPON_SYM: return "#b0b0b8";
    case TOOL_SYM:   return "#c0c0c0";
    case ARMOR_SYM:  return "#a07040";
    case POTION_SYM: return "#40a0ff";
    case SCROLL_SYM: return "#ffffff";
    case WAND_SYM:   return "#40d040";
    case RING_SYM:   return "#ff4040";
    case GEM_SYM:    return "#ff60ff";
#ifdef SPELLS
    case SPBOOK_SYM: return "#60e0e0";
#endif
    }
    return "";
}

/* Inventory window: "<css colour>\ta - item" per line */
char *rl_invtext(void)
{
    static char b[52 * 80];
    struct obj *o;
    int n = 0;
    b[0] = 0;
    for (o = invent; o && n < (int)sizeof b - 80; o = o->nobj)
        n += snprintf(b + n, 80, "%s\t%c - %.64s\n", rl_css(o->olet), obj_to_let(o), doname(o));
    return b;
}
