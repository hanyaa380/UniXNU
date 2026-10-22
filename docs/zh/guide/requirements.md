# 构建环境

## 主机

构建驱动在 macOS 和 Linux 上运行。`--skip-setup` 调用 `hdiutil`；在 Linux 上，驱动在拉取任何内容之前拒绝它。package、固件、负载、启动镜像、IPSW 和 userdata 步骤在两种主机上都能运行。内核步骤在 macOS 上构建；在 Linux 上它需要两个尚无开源替代的工具，见 [Linux 上的内核](#linux-上的内核)。在 Linux 上，userdata 步骤自行展开 UDIF 磁盘镜像，用 `lzfse` 解码 LZFSE 块，并校验每个块表的 CRC32。

| 工具 | 用途 | macOS | Fedora |
| --- | --- | --- | --- |
| macOS 与 Xcode | 内核、skip-setup、seed-data-grow | App Store 或 developer.apple.com | 不可用；见 [Linux 上的内核](#linux-上的内核) |
| Python 3.11 或更新 | 所有步骤 | python.org 或 Homebrew | `python3` |
| git、make | 拉取、固件 | Xcode command line tools 或 Homebrew | `git make` |
| C 与 C++ 编译器、libuuid | EDK II BaseTools | Xcode command line tools | `gcc gcc-c++ libuuid-devel` |
| cmake、ninja | 内核参考源码 | `brew install cmake ninja` | `cmake ninja-build` |
| unifdef | libdispatch 头文件 | `brew install unifdef` | `unifdef` |
| flex、bison、m4、perl、csh | Linux 上的 mig 与 XNU 的 SETUP 工具 | Xcode | `flex bison m4 perl tcsh` |
| LLVM 与 lld | 内核、固件、启动镜像 | `brew install llvm lld` | `clang llvm lld` |
| mtools | 负载卷 | `brew install mtools` | `mtools` |
| dtc | 设备树属性 | `brew install dtc` | `dtc` |
| LZFSE 解码器 | 从 IPSW 取得 kernelcache | `compression_tool`（系统自带） | `lzfse` |
| fastboot | seed-data-grow `--auto`、userdata 刷写 | Android platform tools | `android-tools` |
| QEMU 与 AAVMF | 可选冒烟测试 | `brew install qemu` | `qemu-system-aarch64 edk2-aarch64` |

在 Fedora 上安装最后一列的软件包：

```sh
sudo dnf install python3 git make gcc gcc-c++ libuuid-devel clang llvm lld mtools dtc lzfse \
  cmake ninja-build unifdef flex bison m4 perl tcsh
```

驱动会直接找到 Homebrew 的 `/opt/homebrew/opt/llvm/bin`，无需修改 `PATH`。其他工具（包括 `/usr/bin` 中的 Fedora LLVM）需要在 `PATH` 中。`lld-link` 必须与 `clang` 的 LLVM 主版本相同，因为固件使用 LTO 链接。

### Linux 上的内核

在 macOS 以外，内核步骤使用 `scripts/unixnu/host`：回答 `-find` 与 `--show-sdk-*` 查询的 `xcrun`，`ditto`、`sw_vers`、`plutil`、`install`，把 `-arch` 转为 Darwin 目标的目标 `clang`，以及调用 `ld64.lld` 的 `ld`。`scripts/build-host-tools-linux.sh` 从 `bootstrap_cmds` 构建 `mig`，并为 XNU 的 SETUP 工具准备 Mach 头文件。`libfirehose_kernel` 不经 `xcodebuild` 编译。替代的 iPhoneOS SDK 只含 `TargetConditionals.h`，不使用 Apple 的 SDK。CTF 关闭（`DO_CTFMERGE=0`）；内核步骤生成 `.noctf` 内核。

仍缺两项输入：

| 工具 | 原因 | 状态 |
| --- | --- | --- |
| `iig` | 生成 IOKit 的 DriverKit 头文件和 `*.iig.cpp`。其 `*_Methods` 宏为 `OSObject` 和 `IOService` 增加虚方法，因此输出决定原有 kext 预链接所依赖的虚表。 | 仅 Xcode 提供。没有 `UNIXNU_IIG` 时驱动拒绝内核步骤。已有开源重写（PureDarwin `iig-tools`），尚未与 Apple 的输出核对。 |
| 支持 `__builtin_xnu_type_signature`、`__builtin_xnu_type_summary` 和 `__builtin_xnu_types_compatible` 的 clang | `osfmk/kern/kalloc.h` 的 `kalloc_type` 无回退地使用它们。 | 仅 Apple clang 提供。上游 LLVM 没有；第一个内核编译错误位于 `security/mac_base.c`。 |

在两者都具备之前，用 `macOS build` workflow 构建内核。

`unixnu fetch` 需要网络，用于更新模块子模块并拉取 Apple 源码参考和 Mu_Basecore 树。

没有 Mac 时，`macOS build` workflow 在 GitHub 的 macOS runner 上运行 Mac 专属步骤。见[构建](/zh/building#macos-runner)。

## 外部输入

以下输入位于 UniXNU 仓库之外。

| 输入 | 使用步骤 | 选项 | 环境变量 |
| --- | --- | --- | --- |
| 设备的 Mu-Silicium 仓库 | 固件、启动镜像 | `--uefi DIR` | `UNIXNU_UEFI` |
| 目标 IPSW，本地文件或 HTTPS URL | 内核预链接、userdata | `--ipsw PATH_OR_URL` | `UNIXNU_IPSW` |
| 解压后的 kernelcache | 内核预链接 | `--kernelcache FILE` | `UNIXNU_KERNELCACHE` |
| 编译后的设备树 | 启动镜像 | `--dtb FILE` | `UNIXNU_DTB` |
| trust cache 包 | 负载 | `--trust-cache FILE` | |
| owner trust cache 包 | 负载 | `--owner-trust-cache FILE` | |
| Data ramdisk 种子 | 负载 | `--data-ramdisk FILE` | |
| 平台种子 | 负载 | `--unique-chip-id HEX` | `UNIXNU_UNIQUE_CHIP_ID` |
| 预编译静态库 | 内核 | 负载可用 `--kernel FILE` | |
| edk2 或 Mu_Basecore 树 | package、固件 | `--edk2 DIR` | `UNIXNU_EDK2` |

设备仓库提供板级描述、内核 defconfig 和 `[kernel] archives` 中列出的静态库脚本。

## 设备

设备需要满足：

- bootloader 可解锁并接受 `fastboot boot`。
- UEFI 移植提供 framebuffer、输入、块 I/O 和文件系统。
- bring-up 期间有 UART 或 USB 日志通道。
- 内存足以容纳内核窗口、负载和 userdata 镜像。
- 目标系统恢复镜像和匹配的设备身份。

## 下一步

- [构建](/zh/building)介绍使用这些输入的命令。
- [设备仓库](/zh/porting/device-repository)介绍板级文件。
- [设备](/zh/devices/)列出设备仓库和各自的输入。
