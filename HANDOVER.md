# NetHack 1.3d — handover

Port of NetHack 1.3d (all RVIP stages done): web build live at
https://ruzzoli.de/roguelikes/nethack13d/, plus a native X11 tiles build.
Procedure: `~/Games/rvip-tools/RVIP.md`. Sister port: `~/Games/hack` (most
`port/` code came from there).

## Source and repo
- github.com/bhaak/NetHack-1.3d (full history; upstream = `bcec982`). Repo
  https://github.com/memmaker/nethack13d (remote `memmaker`, branch `master`);
  README links the compare view.
- Case O, like Hack: termcap game, flat source dir, `Makefile` → `Makefile.unix`;
  `unixmain.c`, `unixtty.c`, `unixunix.c` are copied to main.c/tty.c/unix.c by
  make (edit the unix* files, never the copies).

## Build and deploy
- Web: `sh web/build.sh` → `web/dist` (runs makedefs natively itself, then emcc
  + Asyncify; no separate native build needed). `web/deploy.sh` deploys.
  Shared page code from the parent folder: `../rvip-wm.js`, `../rvip-app.js`.
- Native X11 (for testing/ASan; `./play.sh [nethack]`, Desktop shortcut
  `~/Desktop/Games/Roguelikes/NetHack 1.3d.app`):
  `make nethack CC="cc -std=gnu89 -w -Wno-incompatible-function-pointer-types -Wno-return-mismatch -include port/compat.h"`
  then `make rumors data`. `save/` = playground (git-ignored). X11 env knobs:
  `HACK_TILESET`, `HACK_TILES`, `HACK_CELL`, `HACK_TEXT`, `HACK_POS`,
  `HACK_AUTOSAVE=1` (autosave at every prompt).

## File map
- `port/rl.c`: explore `_` (`x` is spells in 1.3d), `<`/`>` off stairs walk to
  known ones (press again to take them), Enter = `cmd_menu()` (parses `help`
  + `extcmdlist`), `i` = `inv_menu()`/`item_menu()`/`act()`, `rl_autosave()`.
  Hook: `rhack()` in cmd.c calls `rl_parse()` instead of `parse()`.
- `port/tiles.c` (C picks tiles; JS only blits), `port/mktiles.py` →
  `port/tiles-dawn*.png`, `port/tiles.png`, `port/tilemap.h`; credits
  `port/TILES-CREDITS.txt`. DawnLike default, NetHack 3.6 set switchable.
- `port/vt.c`/`vt.h` (VT layer, strips termcap padding `$<n>`), `port/be_web.c`
  (web backend, `js_key(rl_at_prompt)`, beacon `js_beacon`), `port/be_x11.c`,
  `port/termcap-web.c`, `port/web-inc/`, `port/compat.h` (libc declarations;
  64-bit pointers were cut to int), `port/proto.h` (K&R prototypes for wasm).
- `web/nethack.js` (draws), `web/index.html`, `web/make-help.py`.

## Facts and gotchas
- Web saves live in this game's own IndexedDB folder (`RvipApp.dir`); player
  name and tile set are stored there too (no localStorage). User decision: old
  saves in the former shared `/hack` DB are abandoned (no migration).
- `rl_autosave()` unshuffles `oc_descr` before `dorecover` (restnames expects it).
- `auto_more` on by default (topl.c).
- `link()` unsupported on the web → macro in `getlock` (unixunix.c).
- argv[0] must contain a `/`; a killed native game leaves a `save/<user>.0` lock.
- `-D` needs `getlogin()` == "wizard": no wizard mode.
- 1.3d has no open/close: doorways draw floor. No sound upstream, none on the web.

## Open
- Native X11: prompts before the map (character pick) are drawn in map-cell
  spacing (cosmetic).
