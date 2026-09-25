#!/bin/sh
# NetHack 1.3d (X11 tiles). ./play.sh [nethack] picks the NetHack tile set
# (default DawnLike). Playground (saves, record): save/.
cd "$(dirname "$0")"
export XAUTHORITY="${XAUTHORITY:-$HOME/.Xauthority}" TERM=vt100
export HACK_TILESET="${1:-${HACK_TILESET:-dawn}}" HACKDIR="$PWD/save"
[ -f save/data ] || make rumors data >/dev/null
exec ./nethack
