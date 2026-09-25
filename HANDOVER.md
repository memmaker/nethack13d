# NetHack 1.3d — RVIP

## RVIP progress
- Stage 2 done (2026-09-25). Next: stage 3 (Enter menu + inventory).
  - X11 window: `port/vt.c`, `vt.h`, `be_x11.c` from Hack (title
    "NetHack 1.3d"); vt.c also strips termcap padding `$<n>`.
    `port/tiles.c` = glyph-only `vt_map` + `vt_cooked` (tiles: stage 4).
    Makefile.unix: `POBJ` built with `-std=gnu99`, links X11/Xft.
  - `port/rl.c`: `_` = explore (`x` is spells in 1.3d), `<`/`>` off the
    stairs walk to known ones. Hook: `rhack()` in cmd.c calls `rl_parse()`
    instead of `parse()`; one step key per turn while a mode runs.
  - Known grid = own `known[][]` OR'd from `levl[x][y].seen`; reset per
    `dlevel`. Boulders (object `ENORMOUS_ROCK`) and seen traps block paths.
    Tame monsters may be walked into (swap or "You stop..." message stops
    the walk); other adjacent displayed monsters block the first step.
  - Stops: new message (`vt_msgs`), key, no movement, visible hostile
    (explore only). help/hh document `_`, `<`, `>`.
  - Tested live (X11): explore mapped a whole level, `>` walked across it
    and descended, `<` walked to the upstairs.
  - Test: `-D` needs `getlogin()` == "wizard", so no wizard mode.
    xsend keysyms: `underscore`, `less`, `greater`.
  - Cosmetic, later: prompts before the map (character pick) drawn in
    map-cell spacing (rows 1-22 are map rows).
- Stage 1 done (2026-09-25). Next: stage 2 (explore + stairs).
  - Source: github.com/bhaak/NetHack-1.3d (full history; upstream = bcec982).
  - Case O, like Hack (`~/Games/hack`, O-Hack in RVIP.md): termcap game,
    flat source dir, `Makefile` -> `Makefile.unix`; `unixmain.c`,
    `unixtty.c`, `unixunix.c` are copied to main.c/tty.c/unix.c by make
    (edit the unix* files, never the copies).
  - Build: `make nethack CC="cc -std=gnu89 -w -Wno-incompatible-function-pointer-types -Wno-return-mismatch -include port/compat.h"`
    then `make rumors data`. Run: `HACKDIR=$PWD/save TERM=vt100 ./nethack`
    (save/ = playground: help hh rumors data perm record, save/save/).
  - Port fixes: `#define BSD` (sgtty.h), `port/compat.h` declares libc
    (64-bit pointers were cut to int), `getdate` -> `hgetdate`, varargs
    `panic/error/impossible` take `char *`, `iso8601` declared in dump.c,
    makedefs/link use `$(CC)`.
  - ASan found: startup SEGV (pointer truncation, fixed by compat.h),
    `let_to_name` buffer overflow with vt100 HI/HE (upstream; buf 64).
    Random-key ASan runs afterwards clean.
  - Quirks: argv[0] must contain a `/` (gethdate stats it via PATH);
    termcap padding `$<2>` printed literally — VT layer must strip it;
    a killed game leaves `save/<user>.0` lock.
  - Tiles (user): DawnLike default + NetHack switchable, as Hack.
