# NetHack 1.3d — RVIP

## RVIP progress
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
