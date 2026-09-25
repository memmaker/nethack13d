# NetHack 1.3d — RVIP

## RVIP progress
- Stage 4 done (2026-09-25). Next: stage 5 (launcher + shortcut). Tiles as Hack:
  DawnLike default, NetHack 3.6 switchable (`HACK_TILESET=nethack`).
  - `port/mktiles.py` (from Hack; reads monst.c, dog.c etc., objects.h incl.
    SPELL) writes `port/tiles-dawn.png/.rgba`, `port/tiles.png/.rgba`,
    `port/tilemap.h` (394 slots; DawnLike misses only egg; unknown
    appearances = stable hashed pick in class). NetHack sources: `port/nethack/`.
    Credits `port/TILES-CREDITS.txt`. Makefile: port objs depend on tilemap.h.
  - `port/tiles.c` from Hack plus: 12 roles (`P:`), polyself (`u.usym` ->
    `mons[u.umonnum]`), fountain, throne (typ THRONE, else `\` object), web
    (trap WEB, else `"` amulet), `+` door only if typ DOOR/LDOOR (else
    spellbook), traps pit/spiked/squeaky/magic/level tele/anti-magic/rust.
  - `be_x11.c` unchanged (cell 18, nearest-neighbour); env `HACK_TILES`,
    `HACK_CELL`, `HACK_TEXT`, `HACK_POS`. Sheets read from `port/` rel. cwd.
  - Tested live: both sets, inventory box over map, 300 random keys alive.
- Stage 3 done (2026-09-25). Next: stage 4 (tiles: DawnLike default + NetHack switchable, as Hack).
  - Ported from Hack's `port/rl.c`: Enter = `cmd_menu()` (parses `Commands:`
    lines of `help`, "\t<key>\t<text>") plus every `extcmdlist` entry as
    "#  name: desc" (choosing pushes "name\n" via `vt_push` after `#`).
    help gained A, V, @, # lines.
  - `i` = `inv_menu()` / `item_menu()` / `act()`; getobj() (invent.c) returns
    `rl_obj` once and its first key comes from `rl_pick(lets)`. 1.3d extras:
    spellbook main action `X` (transcribe), `#dip` in every item menu.
  - Tested live: Enter menu (scrolls), #pray from menu, i -> bow -> wield,
    `t` prompt cursor list; 400 random keys, game alive.
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
