#!/usr/bin/env bash
# Build a clang with XNU's __builtin_xnu_type_signature, __builtin_xnu_type_summary and __builtin_xnu_types_compatible
# for kernel builds on Linux (Apple's clang has them; upstream LLVM does not). Then: export UNIXNU_XNU_CLANG=DIR/bin/clang
#
#   build-xnu-clang-linux.sh DIR
#
# Upstream LLVM at LLVM_TAG with minidarwin's layout builtins (MIT, MINIDARWIN_COMMIT, pkgs/xnu/clang), accepted for
# arm64 besides x86_64: neither signs pointers, so the eight-byte granule kinds are the same. arm64e stays refused.
# The values only place the kernel's own kalloc_type allocations; the original kexts carry the signatures Apple's
# clang gave them. Takes 2 GB and some 20 minutes on 32 cores.
set -euo pipefail
LLVM_TAG=${LLVM_TAG:-llvmorg-21.1.8}
LLVM_COMMIT=2078da43e25a4623cab2d0d60decddf709aaea28
MINIDARWIN_COMMIT=71c5bc1f5b4af3e45f68e6c49be7bf56294beb37
DIR=$(mkdir -p "${1:?usage: build-xnu-clang-linux.sh DIR}" && cd "$1" && pwd)
SRC=$DIR/llvm-project
if [ ! -d "$SRC/.git" ]; then
  git clone -q --depth 1 --branch "$LLVM_TAG" --filter=blob:none --sparse https://github.com/llvm/llvm-project.git "$SRC"
  git -C "$SRC" sparse-checkout set llvm clang cmake third-party
fi
[ "$(git -C "$SRC" rev-parse HEAD)" = "$LLVM_COMMIT" ] || { echo "unexpected LLVM revision" >&2; exit 1; }
if [ ! -d "$DIR/minidarwin/.git" ]; then
  git init -q "$DIR/minidarwin"
  git -C "$DIR/minidarwin" fetch -q --depth 1 https://github.com/V3L0C1T13S/minidarwin "$MINIDARWIN_COMMIT"
  git -C "$DIR/minidarwin" checkout -q FETCH_HEAD
fi
MD=$DIR/minidarwin/pkgs/xnu/clang
if [ ! -f "$SRC/clang/lib/Sema/XnuTypeLayout.cpp" ]; then
  (cd "$SRC/clang" && python3 "$MD/patch.py")
  python3 - "$MD" "$SRC/clang/lib/Sema" <<'PY'
import shutil, sys
from pathlib import Path
md, sema = Path(sys.argv[1]), Path(sys.argv[2])
# A class template specialization (OSValueObject<int> in libkern) is only instantiated once Sema completes it.
impl = (md / 'XnuTypeLayout.cpp').read_text()
call = '  auto Result = TypeLayout(S.Context).signature(T);'
assert impl.count(call) == 1, 'minidarwin XnuTypeLayout.cpp changed'
(sema / 'XnuTypeLayout.cpp').write_text(impl.replace(call, '  if (!T->isDependentType()) S.isCompleteType(Loc, T);\n' + call))
old = '''    // This implementation targets x86_64, which has no authenticated pointers.
    if (C.getTargetInfo().getTriple().getArch() != llvm::Triple::x86_64)
      return std::nullopt;'''
new = '''    // x86_64 and arm64 have no authenticated pointers; arm64e (PAC) needs granule kinds this does not compute.
    const llvm::Triple &Target = C.getTargetInfo().getTriple();
    if (Target.getArch() != llvm::Triple::x86_64 &&
        !(Target.getArch() == llvm::Triple::aarch64 && !Target.isArm64e()))
      return std::nullopt;'''
header = (md / 'XnuTypeLayout.h').read_text()
assert header.count(old) == 1, 'minidarwin XnuTypeLayout.h changed'
(sema / 'XnuTypeLayout.h').write_text(header.replace(old, new))
cmake = sema / 'CMakeLists.txt'
text = cmake.read_text()
assert text.count('  SemaTypeTraits.cpp\n') == 1
cmake.write_text(text.replace('  SemaTypeTraits.cpp\n', '  SemaTypeTraits.cpp\n  XnuTypeLayout.cpp\n'))
# Kernel and kext code keeps C++ global destructors in __mod_term_func, as Apple's clang does; upstream LLVM
# lowers them to __cxa_atexit, which the kernel does not have.
driver = sema.parents[1] / 'lib/Driver/ToolChains/Clang.cpp'
text = driver.read_text()
anchor = '    CmdArgs.push_back("-fregister-global-dtors-with-atexit");\n'
assert text.count(anchor) == 1, 'clang Driver changed'
driver.write_text(text.replace(anchor, anchor + '''  if (KernelOrKext) {
    CmdArgs.push_back("-mllvm");
    CmdArgs.push_back("-disable-atexit-based-global-dtor-lowering");
  }
'''))
PY
fi
cmake -S "$SRC/llvm" -B "$DIR/build" -G Ninja -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PROJECTS=clang \
  -DLLVM_TARGETS_TO_BUILD='AArch64;X86' -DLLVM_ENABLE_ASSERTIONS=OFF -DCMAKE_C_COMPILER=clang -DCMAKE_CXX_COMPILER=clang++ \
  -DLLVM_INCLUDE_TESTS=OFF -DLLVM_INCLUDE_BENCHMARKS=OFF -DLLVM_INCLUDE_EXAMPLES=OFF -DCLANG_INCLUDE_TESTS=OFF \
  -DCLANG_ENABLE_STATIC_ANALYZER=OFF -DCLANG_ENABLE_ARCMT=OFF -DLLVM_ENABLE_ZSTD=OFF -DLLVM_ENABLE_LIBXML2=OFF >/dev/null
ninja -C "$DIR/build" clang
# minidarwin's checks, for the arm64 kernel target.
"$DIR/build/bin/clang" -target arm64-apple-ios17.0 -ffreestanding -nostdinc -fsyntax-only "$MD/layout-test.c"
"$DIR/build/bin/clang++" -target arm64-apple-ios17.0 -ffreestanding -nostdinc -std=c++20 -fsyntax-only "$MD/layout-test.cpp"
mkdir -p "$DIR/bin"
ln -sf ../build/bin/clang "$DIR/bin/clang"
ln -sf ../build/bin/clang++ "$DIR/bin/clang++"
echo "export UNIXNU_XNU_CLANG=$DIR/bin/clang"
