# Requirements

## Host

The build driver runs on macOS and Linux. The `--skip-setup` option invokes `hdiutil`; on Linux the driver refuses it before it fetches anything. The package, firmware, payload, boot image, IPSW and userdata steps run on both hosts. The kernel step builds on both; on Linux it needs a toolchain built once and Apple's iig outputs from CI, see [Kernel on Linux](#kernel-on-linux). On Linux the userdata step expands UDIF disk images itself, decoding LZFSE chunks with `lzfse`, and checks each block table CRC32.

| Tool | Used by | macOS | Fedora |
| --- | --- | --- | --- |
| macOS with Xcode | kernel, skip-setup, seed-data-grow | App Store or developer.apple.com | kernel: see [Kernel on Linux](#kernel-on-linux) |
| Python 3.11 or newer | every step | python.org or Homebrew | `python3` |
| git, make | fetch, firmware | Xcode command line tools or Homebrew | `git make` |
| C and C++ compiler, libuuid | EDK II BaseTools | Xcode command line tools | `gcc gcc-c++ libuuid-devel` |
| cmake, ninja | kernel references | `brew install cmake ninja` | `cmake ninja-build` |
| unifdef | libdispatch headers | `brew install unifdef` | `unifdef` |
| flex, bison, m4, perl, csh | mig and XNU's SETUP tools on Linux | Xcode | `flex bison m4 perl tcsh` |
| LLVM with lld | kernel, firmware, boot image | `brew install llvm lld` | `clang llvm lld` |
| mtools | payload volume | `brew install mtools` | `mtools` |
| dtc | device tree properties | `brew install dtc` | `dtc` |
| LZFSE decoder | kernelcache from the IPSW | `compression_tool` (built in) | `lzfse` |
| fastboot | seed-data-grow `--auto`, userdata flash | Android platform tools | `android-tools` |
| QEMU and AAVMF | optional smoke tests | `brew install qemu` | `qemu-system-aarch64 edk2-aarch64` |

On Fedora, install the packages of the last column:

```sh
sudo dnf install python3 git make gcc gcc-c++ libuuid-devel clang llvm lld mtools dtc lzfse \
  cmake ninja-build unifdef flex bison m4 perl tcsh
```

The driver finds the keg-only Homebrew LLVM at `/opt/homebrew/opt/llvm/bin` without `PATH` changes. Other tools, including the Fedora LLVM in `/usr/bin`, must be on `PATH`. `lld-link` must have the same LLVM major version as `clang`, because the firmware is linked with LTO.

### Kernel on Linux

Off macOS the kernel step replaces what Xcode provides. Apple's SDKs are never used.

| Xcode provides | On Linux |
| --- | --- |
| `xcrun`, `ditto`, `sw_vers`, `plutil`, `install`, `ld` | `scripts/unixnu/host` |
| `mig` | built from `bootstrap_cmds` by `scripts/build-host-tools-linux.sh` |
| clang with `__builtin_xnu_type_signature`, `__builtin_xnu_type_summary` and `__builtin_xnu_types_compatible` | `scripts/build-xnu-clang-linux.sh DIR`: LLVM 21.1.8 with minidarwin's layout builtins, enabled for arm64 (`UNIXNU_XNU_CLANG`) |
| `ld64` | `scripts/build-ld64-linux.sh DIR`: ld64-951.9 and cctools from cctools-port (`UNIXNU_LD64`). `ld64.lld` ignores `-static`, `-segaddr`, `-segment_order`, `-image_base` and `-alias_list` |
| `iig` | the outputs of Apple's `iig` from the `macOS build` workflow (artifact `unixnu-iig-outputs`), replayed when each `.iig` has the recorded sha256 (`UNIXNU_IIG_OUTPUTS`) |
| `libclang_rt.cc_kext` | `scripts/build-cc-kext-linux.py`: compiler-rt's builtins and kernel profile runtime |
| SDK headers | XNU's installed headers, the Libc, libpthread, libdispatch, libmalloc and dyld headers, and a stand-in `TargetConditionals.h` |
| `libfirehose_kernel` | compiled without `xcodebuild` |

`patches/xnu-linux` adapts the source to upstream clang: `xnu_usage_semantics` becomes `annotate`, which the layout builtins read, and the arm64 crypto assembly uses the `v` register syntax. The driver applies these patches only on Linux. CTF is off (`DO_CTFMERGE=0`); the step produces the `.noctf` kernel.

```sh
scripts/build-xnu-clang-linux.sh ~/unixnu-toolchain/clang   # some 20 minutes on 32 cores
scripts/build-ld64-linux.sh ~/unixnu-toolchain/ld64
export UNIXNU_XNU_CLANG=~/unixnu-toolchain/clang/bin/clang
export UNIXNU_LD64=~/unixnu-toolchain/ld64/bin/arm64-apple-darwin-ld
export UNIXNU_IIG_OUTPUTS=~/unixnu-iig-outputs              # gh run download RUN -n unixnu-iig-outputs -D ...
./unixnu build kernel
```

The driver checks each of these before it fetches anything. The kernel built this way links and prelinks. Against the macOS build of the same sources it has the same segment layout and the same exported symbols, apart from those of the iig outputs when they come from another iig. The local symbols differ: Xcode's clang splits cold code (`.cold.N`) and outlines functions; upstream clang does not. It has not been booted. `UNIXNU_IIG` accepts another `iig` for build tests; PureDarwin's `iig-tools` builds, but its RPC message layouts differ from Apple's.

Network access is required for `unixnu fetch`, which updates the module submodules and fetches Apple source references and a Mu_Basecore tree.

Without a Mac, the `macOS build` workflow runs the Mac-only steps on a GitHub macOS runner. See [Build](/building#macos-runner).

## External inputs

These inputs live outside the UniXNU repositories.

| Input | Used by | Option | Environment |
| --- | --- | --- | --- |
| Mu-Silicium checkout for the device | firmware, boot image | `--uefi DIR` | `UNIXNU_UEFI` |
| Target IPSW, local file or HTTPS URL | kernel prelink, userdata | `--ipsw PATH_OR_URL` | `UNIXNU_IPSW` |
| Decompressed kernelcache | kernel prelink | `--kernelcache FILE` | `UNIXNU_KERNELCACHE` |
| Compiled device tree | boot image | `--dtb FILE` | `UNIXNU_DTB` |
| Trust cache bundle | payload | `--trust-cache FILE` | |
| Owner trust cache bundle | payload | `--owner-trust-cache FILE` | |
| Data ramdisk seed | payload | `--data-ramdisk FILE` | |
| Platform seed | payload | `--unique-chip-id HEX` | `UNIXNU_UNIQUE_CHIP_ID` |
| Prebuilt static archives | kernel | `--kernel FILE` for the payload, archives in the build tree | |
| edk2 or Mu_Basecore tree | package, firmware | `--edk2 DIR` | `UNIXNU_EDK2` |

The device repository supplies the board description, the kernel defconfig and the archive builders listed in `[kernel] archives`.

## Device

The device needs:

- An unlockable bootloader that accepts `fastboot boot`.
- A UEFI port that provides a framebuffer, input, block I/O and file system access.
- A UART or USB transport for logs during bring-up.
- Enough RAM for the kernel window, the payload and the userdata image.
- The target OS restore image and a matching device identity.

## Next steps

- [Build](/building) describes the commands that consume these inputs.
- [Device repository](/porting/device-repository) describes the board file.
- [Devices](/devices/) lists device repositories and their specific inputs.
