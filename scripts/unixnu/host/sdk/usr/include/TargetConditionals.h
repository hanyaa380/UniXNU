/* TargetConditionals.h of the Linux stand-in SDK (scripts/unixnu/host): the TARGET_* macros, from clang's view of
 * the target triple. Only the headers XNU and its kernel archives include are served by this SDK. */
/* clang 19 and later predefine the TARGET_OS_* macros for Darwin targets (-fdefine-target-os-macros);
   Apple's clang does not. Each definition here applies only where the compiler has none. */
#ifndef UNIXNU_TARGET_CONDITIONALS_H
#define UNIXNU_TARGET_CONDITIONALS_H

#if !defined(__is_target_os) || !defined(__is_target_environment)
#error "the UniXNU stand-in TargetConditionals.h needs clang"
#endif

#ifndef TARGET_OS_MAC
#define TARGET_OS_MAC 1
#endif
#ifndef TARGET_OS_WIN32
#define TARGET_OS_WIN32 0
#endif
#ifndef TARGET_OS_WINDOWS
#define TARGET_OS_WINDOWS 0
#endif
#ifndef TARGET_OS_UNIX
#define TARGET_OS_UNIX 0
#endif
#ifndef TARGET_OS_LINUX
#define TARGET_OS_LINUX 0
#endif
#ifndef TARGET_OS_OSX
#define TARGET_OS_OSX __is_target_os(macos)
#endif
#ifndef TARGET_OS_IOS
#define TARGET_OS_IOS __is_target_os(ios)
#endif
#ifndef TARGET_OS_TV
#define TARGET_OS_TV __is_target_os(tvos)
#endif
#ifndef TARGET_OS_WATCH
#define TARGET_OS_WATCH __is_target_os(watchos)
#endif
#ifndef TARGET_OS_VISION
#define TARGET_OS_VISION __is_target_os(xros)
#endif
#ifndef TARGET_OS_BRIDGE
#define TARGET_OS_BRIDGE __is_target_os(bridgeos)
#endif
#ifndef TARGET_OS_DRIVERKIT
#define TARGET_OS_DRIVERKIT __is_target_os(driverkit)
#endif
#ifndef TARGET_OS_MACCATALYST
#define TARGET_OS_MACCATALYST __is_target_environment(macabi)
#endif
#ifndef TARGET_OS_UIKITFORMAC
#define TARGET_OS_UIKITFORMAC TARGET_OS_MACCATALYST
#endif
#ifndef TARGET_OS_SIMULATOR
#define TARGET_OS_SIMULATOR __is_target_environment(simulator)
#endif
#ifndef TARGET_OS_IPHONE
#define TARGET_OS_IPHONE (TARGET_OS_IOS || TARGET_OS_TV || TARGET_OS_WATCH || TARGET_OS_VISION || TARGET_OS_BRIDGE)
#endif
#ifndef TARGET_OS_EMBEDDED
#define TARGET_OS_EMBEDDED (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR && !TARGET_OS_MACCATALYST)
#endif
#ifndef TARGET_OS_NANO
#define TARGET_OS_NANO TARGET_OS_WATCH
#endif
#ifndef TARGET_IPHONE_SIMULATOR
#define TARGET_IPHONE_SIMULATOR TARGET_OS_SIMULATOR
#endif

#if defined(__arm64__) || defined(__aarch64__)
#ifndef TARGET_CPU_ARM64
#define TARGET_CPU_ARM64 1
#endif
#else
#ifndef TARGET_CPU_ARM64
#define TARGET_CPU_ARM64 0
#endif
#endif
#if defined(__arm__) && !TARGET_CPU_ARM64
#ifndef TARGET_CPU_ARM
#define TARGET_CPU_ARM 1
#endif
#else
#ifndef TARGET_CPU_ARM
#define TARGET_CPU_ARM 0
#endif
#endif
#if defined(__x86_64__)
#ifndef TARGET_CPU_X86_64
#define TARGET_CPU_X86_64 1
#endif
#else
#ifndef TARGET_CPU_X86_64
#define TARGET_CPU_X86_64 0
#endif
#endif
#if defined(__i386__)
#ifndef TARGET_CPU_X86
#define TARGET_CPU_X86 1
#endif
#else
#ifndef TARGET_CPU_X86
#define TARGET_CPU_X86 0
#endif
#endif
#ifndef TARGET_CPU_PPC
#define TARGET_CPU_PPC 0
#endif
#ifndef TARGET_CPU_PPC64
#define TARGET_CPU_PPC64 0
#endif
#ifndef TARGET_CPU_68K
#define TARGET_CPU_68K 0
#endif
#ifndef TARGET_CPU_ALPHA
#define TARGET_CPU_ALPHA 0
#endif
#ifndef TARGET_CPU_MIPS
#define TARGET_CPU_MIPS 0
#endif
#ifndef TARGET_CPU_SPARC
#define TARGET_CPU_SPARC 0
#endif
#ifndef TARGET_CPU_HPPA
#define TARGET_CPU_HPPA 0
#endif

#ifndef TARGET_RT_MAC_CFM
#define TARGET_RT_MAC_CFM 0
#endif
#ifndef TARGET_RT_MAC_MACHO
#define TARGET_RT_MAC_MACHO 1
#endif
#ifndef TARGET_RT_LITTLE_ENDIAN
#define TARGET_RT_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#endif
#ifndef TARGET_RT_BIG_ENDIAN
#define TARGET_RT_BIG_ENDIAN (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#endif
#if defined(__LP64__)
#ifndef TARGET_RT_64_BIT
#define TARGET_RT_64_BIT 1
#endif
#else
#ifndef TARGET_RT_64_BIT
#define TARGET_RT_64_BIT 0
#endif
#endif
#endif
