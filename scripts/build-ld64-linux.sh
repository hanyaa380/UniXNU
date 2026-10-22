#!/usr/bin/env bash
# Build Apple's ld64 (and cctools) for kernel builds on Linux. Then: export UNIXNU_LD64=DIR/bin/arm64-apple-darwin-ld
#
#   build-ld64-linux.sh DIR
#
# ld64.lld cannot link XNU: it ignores -static, -segaddr, -segment_order, -image_base and -alias_list. This builds
# ld64-951.9 (Xcode 15, the Darwin 23 linker) from tpoechtrager/cctools-port, with its libdispatch and blocks runtime
# (tpoechtrager/apple-libdispatch; Fedora's libdispatch-devel works as well). Needs clang and cmake; a few minutes.
set -euo pipefail
CCTOOLS_COMMIT=7d84f3ef414e8f4da6cb99ca7ad5c79fde5f47af   # branch 1010.6-ld64-951.9
DISPATCH_COMMIT=323b9b4e0ca05d6c56a0c2f2d7d8d47363e612b7
DIR=$(mkdir -p "${1:?usage: build-ld64-linux.sh DIR}" && cd "$1" && pwd)
fetch() {   # fetch URL COMMIT DIR: one commit, shallow
  [ -d "$3/.git" ] && return
  git init -q "$3"
  git -C "$3" fetch -q --depth 1 "$1" "$2"
  git -C "$3" checkout -q FETCH_HEAD
}
fetch https://github.com/tpoechtrager/apple-libdispatch "$DISPATCH_COMMIT" "$DIR/apple-libdispatch"
fetch https://github.com/tpoechtrager/cctools-port "$CCTOOLS_COMMIT" "$DIR/cctools-port"
cmake -S "$DIR/apple-libdispatch" -B "$DIR/apple-libdispatch/build" -G Ninja -DCMAKE_BUILD_TYPE=Release \
  -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_INSTALL_PREFIX="$DIR" >/dev/null
ninja -C "$DIR/apple-libdispatch/build" install >/dev/null
cd "$DIR/cctools-port/cctools"
CC=clang CXX=clang++ ./configure --prefix="$DIR" --target=arm64-apple-darwin \
  --with-libdispatch="$DIR" --with-libblocksruntime="$DIR" >/dev/null
make -j"${JOBS:-$(nproc)}" >/dev/null
make install >/dev/null
"$DIR/bin/arm64-apple-darwin-ld" -v 2>&1 | head -1
echo "export UNIXNU_LD64=$DIR/bin/arm64-apple-darwin-ld"
