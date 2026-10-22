# Requirements

## Host

The build driver runs on macOS and Linux. The `--skip-setup` option invokes `hdiutil`; on Linux the driver refuses it before it fetches anything. The package, firmware, payload, boot image, IPSW and userdata steps run on both hosts. The kernel step builds on macOS; on Linux it needs two tools that have no open equivalent yet, see [Kernel on Linux](#kernel-on-linux). On Linux the userdata step expands UDIF disk images itself, decoding LZFSE chunks with `lzfse`, and checks each block table CRC32.

| Tool | Used by | macOS | Fedora |
| --- | --- | --- | --- |
| macOS with Xcode | kernel, skip-setup, seed-data-grow | App Store or developer.apple.com | not available; see [Kernel on Linux](#kernel-on-linux) |
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

Off macOS the kernel step uses `scripts/unixnu/host`: an `xcrun` that answers `-find` and the `--show-sdk-*` queries, `ditto`, `sw_vers`, `plutil`, `install`, a target `clang` that turns `-arch` into a Darwin target, and `ld` for `ld64.lld`. `scripts/build-host-tools-linux.sh` builds `mig` from `bootstrap_cmds` and the Mach headers of XNU's SETUP tools. `libfirehose_kernel` is compiled without `xcodebuild`. The stand-in iPhoneOS SDK holds only `TargetConditionals.h`; Apple's SDK is not used. CTF is off (`DO_CTFMERGE=0`); the kernel step produces the `.noctf` kernel.

Two inputs are still missing:

| Tool | Why | State |
| --- | --- | --- |
| `iig` | Generates the DriverKit headers and `*.iig.cpp` files of IOKit. Its `*_Methods` macros add virtual methods to `OSObject` and `IOService`, so the output sets the vtables that the original kexts are prelinked against. | Xcode only. The driver refuses the kernel step without `UNIXNU_IIG`. Open reimplementations exist (PureDarwin `iig-tools`); none is checked against Apple's output. |
| clang with `__builtin_xnu_type_signature`, `__builtin_xnu_type_summary` and `__builtin_xnu_types_compatible` | `kalloc_type` in `osfmk/kern/kalloc.h` uses them without a fallback. | Apple clang only. Upstream LLVM lacks them; the first kernel compile error is in `security/mac_base.c`. |

Until both exist, build the kernel with the `macOS build` workflow.

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
