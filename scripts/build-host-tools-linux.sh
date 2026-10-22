#!/usr/bin/env bash
# Host tools of the kernel step on Linux (macOS uses Xcode's):
#   build/host-tools/include   Mach and Mach-O headers of refer/xnu for host programs (mig, XNU's SETUP tools)
#   build/host-tools/bin       mig and migcom, built from refer/bootstrap_cmds
set -euo pipefail
ROOT=$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)
SRC=$ROOT/refer/bootstrap_cmds/migcom.tproj
XNU=$ROOT/refer/xnu
OUT=$ROOT/build/host-tools
BIN=$OUT/bin
[ -x "$OUT/libexec/migcom" ] && [ "$OUT/libexec/migcom" -nt "$0" ] && exit 0
mkdir -p "$OUT/mig" "$BIN"
INC=$OUT/include
rm -rf "$INC"
mkdir -p "$INC/machine" "$INC/sys" "$INC/i386" "$INC/os" "$INC/libkern/i386"
cp -R "$XNU"/osfmk/mach "$INC/mach"
cp -R "$XNU"/EXTERNAL_HEADERS/mach-o "$XNU"/EXTERNAL_HEADERS/architecture "$INC/"
# Darwin-only headers the Mach headers include; none of them shadows a glibc header.
cp -R "$XNU"/bsd/sys/_types "$XNU"/bsd/sys/appleapiopts.h "$INC/sys/"
cp "$XNU"/osfmk/i386/eflags.h "$INC/i386/"
# The Darwin base types, on glibc's (Darwin's own _types.h redefine glibc's __int64_t and friends).
cat > "$INC/sys/_types.h" <<'EOF'
#include <sys/types.h>
#include <stdint.h>
#include <stddef.h>
typedef unsigned int __darwin_natural_t;
typedef __darwin_natural_t __darwin_mach_port_name_t;
typedef __darwin_mach_port_name_t __darwin_mach_port_t;
typedef unsigned char __darwin_uuid_t[16];
typedef char __darwin_uuid_string_t[37];
typedef size_t __darwin_size_t;
typedef pid_t __darwin_pid_t;
EOF
cp "$INC/sys/_types.h" "$INC/i386/_types.h"
cp "$INC/sys/_types.h" "$INC/machine/_types.h"
cp "$XNU"/libkern/os/base.h "$XNU"/libkern/os/overflow.h "$INC/os/"
cp "$XNU"/libkern/libkern/OSByteOrder.h "$XNU"/libkern/libkern/_OSByteOrder.h "$INC/libkern/"
cp "$XNU"/libkern/libkern/i386/_OSByteOrder.h "$XNU"/libkern/libkern/i386/OSByteOrder.h "$INC/libkern/i386/"
cp "$XNU"/EXTERNAL_HEADERS/Availability.h "$XNU"/EXTERNAL_HEADERS/AvailabilityInternal.h "$XNU"/EXTERNAL_HEADERS/AvailabilityMacros.h "$INC/"
printf '#include <limits.h>\n' > "$INC/machine/limits.h"
printf '#include <sys/types.h>\n#include <stdint.h>\ntypedef uint64_t user_addr_t, user_size_t;\n' > "$INC/machine/types.h"
printf '#define TARGET_OS_MAC 1\n#define TARGET_OS_OSX 1\n#define TARGET_OS_IPHONE 0\n#define TARGET_OS_SIMULATOR 0\n#define TARGET_OS_EMBEDDED 0\n' > "$INC/TargetConditionals.h"
cat > "$INC/compat.h" <<'EOF'
/* Darwin definitions the Mach headers expect on a Linux host; host programs only need their constants and types. */
#define _DEFAULT_SOURCE 1
#include <sys/cdefs.h>
#include <sys/types.h>
#include <stdint.h>
#ifndef __DARWIN_ALIAS
#define __DARWIN_ALIAS(x)
#endif
#define __API_AVAILABLE(...)
#define __API_DEPRECATED(...)
#define __OSX_AVAILABLE(...)
#define __IOS_AVAILABLE(...)
#define __OS_AVAILABILITY(...)
#define __XNU_PRIVATE_EXTERN
#define __kernel_ptr_semantics
#define __options_decl(name, type, ...) enum __VA_ARGS__; typedef type name
#define __options_closed_decl(name, type, ...) enum __VA_ARGS__; typedef type name
#define __enum_decl(name, type, ...) enum __VA_ARGS__; typedef type name
#define __enum_closed_decl(name, type, ...) enum __VA_ARGS__; typedef type name
#define __kernel_data_semantics
#define __kernel_dual_semantics
#define __private_extern__ __attribute__((visibility("hidden")))
#ifndef __has_include
#define __has_include(x) 0
#endif
#define KERNEL_SERVER 0
EOF
cd "$OUT/mig"
bison -y -d -o y.tab.c "$SRC/parser.y"
flex -o lexxer.c "$SRC/lexxer.l"
CFLAGS=(-O2 -w -std=gnu99 -include "$INC/compat.h" -I. -I"$SRC" -I"$INC" -DMIG_VERSION="\"bootstrap_cmds-linux\"")
OBJECTS=()
# The sources of the migcom target of mig.xcodeproj (handler.c is not built).
for source in "$SRC"/{error,global,header,mig,routine,server,statement,string,type,user,utils}.c y.tab.c lexxer.c; do
  object=$(basename "${source%.c}").o
  clang "${CFLAGS[@]}" -c "$source" -o "$object"
  OBJECTS+=("$object")
done
mkdir -p "$OUT/libexec"
clang -o "$OUT/libexec/migcom" "${OBJECTS[@]}"
ln -sf ../libexec/migcom "$BIN/migcom"
# mig.sh finds migcom in ../libexec and, without /usr/bin/xcrun, preprocesses with the cc next to it.
# Linux clang ignores -arch, so that cc turns -arch ARCH into a Darwin target.
cp "$SRC/mig.sh" "$BIN/mig"
cat > "$BIN/cc" <<'CC'
#!/bin/sh
for arg; do
  shift
  if [ -n "$arch" ]; then set -- "$@" -target "$arg-apple-darwin23"; arch=; continue; fi
  case $arg in -arch) arch=1 ;; *) set -- "$@" "$arg" ;; esac
done
exec clang "$@"
CC
chmod +x "$BIN/mig" "$BIN/cc"
echo "mig: $BIN/mig"
