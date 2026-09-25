#!/bin/sh
# Build NetHack 1.3d for the browser (Emscripten + Asyncify) into web/dist;
# port/be_web.c + web/nethack.js draw the screen and tiles. Deploy: web/deploy.sh.
set -e
cd "$(dirname "$0")/.."
OUT=web/dist SEED=web/seed
rm -rf "$OUT" "$SEED" && mkdir -p "$OUT" "$SEED"
[ -f save/data ] || make rumors data
cp save/help save/hh save/data save/rumors "$SEED/"
SRCS=$(sed -n '/^HACKCSRC/,/[^\\]$/p' Makefile.unix | tr -d '\\' | sed 's/HACKCSRC =//' | sed 's/main\.c/unixmain.c/; s/tty\.c/unixtty.c/; s/unix\.c/unixunix.c/')
emcc -O2 $EMFLAGS -std=gnu99 -w -D_GNU_SOURCE -fcommon -Dusleep=hk_usleep \
	-Wno-incompatible-function-pointer-types -Wno-return-mismatch -Wno-implicit-function-declaration -Wno-implicit-int -Wno-incompatible-pointer-types \
	-Iport/web-inc -include port/compat.h -include port/web-inc/hkio.h -I. \
	$SRCS port/vt.c port/rl.c port/tiles.c port/termcap-web.c port/be_web.c \
	--preload-file "$SEED@/seed" -o "$OUT/nethack-core.js" \
	-sASYNCIFY -sASYNCIFY_STACK_SIZE=65536 -sSTACK_SIZE=1048576 \
	-sALLOW_MEMORY_GROWTH -sEXIT_RUNTIME=1 -sINITIAL_MEMORY=32MB \
	-sEXPORTED_FUNCTIONS=_main \
	-sEXPORTED_RUNTIME_METHODS=FS,IDBFS,ENV,HEAPU32,HEAP32,addRunDependency,removeRunDependency \
	-sFORCE_FILESYSTEM -lidbfs.js -sENVIRONMENT=web
rm -rf "$SEED"
cp web/index.html web/nethack.js port/tiles-dawn.png port/tiles.png "$HOME/Games/rvip-tools/web/rvip-wm.js" "$OUT/"
python3 web/make-help.py > "$OUT/help.html"
ls -la "$OUT"
