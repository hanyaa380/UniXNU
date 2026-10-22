/* TargetConditionals.h of the Linux stand-in SDK (scripts/unixnu/host): the TARGET_* macros, from clang's view of
 * the target triple. Only the headers XNU and its kernel archives include are served by this SDK. */
#ifndef UNIXNU_TARGET_CONDITIONALS_H
#define UNIXNU_TARGET_CONDITIONALS_H

#if !defined(__is_target_os) || !defined(__is_target_environment)
#error "the UniXNU stand-in TargetConditionals.h needs clang"
#endif

#define TARGET_OS_MAC 1
#define TARGET_OS_WIN32 0
#define TARGET_OS_WINDOWS 0
#define TARGET_OS_UNIX 0
#define TARGET_OS_LINUX 0
#define TARGET_OS_OSX __is_target_os(macos)
#define TARGET_OS_IOS __is_target_os(ios)
#define TARGET_OS_TV __is_target_os(tvos)
#define TARGET_OS_WATCH __is_target_os(watchos)
#define TARGET_OS_VISION __is_target_os(xros)
#define TARGET_OS_BRIDGE __is_target_os(bridgeos)
#define TARGET_OS_DRIVERKIT __is_target_os(driverkit)
#define TARGET_OS_MACCATALYST __is_target_environment(macabi)
#define TARGET_OS_UIKITFORMAC TARGET_OS_MACCATALYST
#define TARGET_OS_SIMULATOR __is_target_environment(simulator)
#define TARGET_OS_IPHONE (TARGET_OS_IOS || TARGET_OS_TV || TARGET_OS_WATCH || TARGET_OS_VISION || TARGET_OS_BRIDGE)
#define TARGET_OS_EMBEDDED (TARGET_OS_IPHONE && !TARGET_OS_SIMULATOR && !TARGET_OS_MACCATALYST)
/* clang 21 predefines these two for Darwin targets; Apple's clang does not. */
#ifndef TARGET_OS_NANO
#define TARGET_OS_NANO TARGET_OS_WATCH
#endif
#ifndef TARGET_IPHONE_SIMULATOR
#define TARGET_IPHONE_SIMULATOR TARGET_OS_SIMULATOR
#endif

#if defined(__arm64__) || defined(__aarch64__)
#define TARGET_CPU_ARM64 1
#else
#define TARGET_CPU_ARM64 0
#endif
#if defined(__arm__) && !TARGET_CPU_ARM64
#define TARGET_CPU_ARM 1
#else
#define TARGET_CPU_ARM 0
#endif
#if defined(__x86_64__)
#define TARGET_CPU_X86_64 1
#else
#define TARGET_CPU_X86_64 0
#endif
#if defined(__i386__)
#define TARGET_CPU_X86 1
#else
#define TARGET_CPU_X86 0
#endif
#define TARGET_CPU_PPC 0
#define TARGET_CPU_PPC64 0
#define TARGET_CPU_68K 0
#define TARGET_CPU_ALPHA 0
#define TARGET_CPU_MIPS 0
#define TARGET_CPU_SPARC 0
#define TARGET_CPU_HPPA 0

#define TARGET_RT_MAC_CFM 0
#define TARGET_RT_MAC_MACHO 1
#define TARGET_RT_LITTLE_ENDIAN (__BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__)
#define TARGET_RT_BIG_ENDIAN (__BYTE_ORDER__ == __ORDER_BIG_ENDIAN__)
#if defined(__LP64__)
#define TARGET_RT_64_BIT 1
#else
#define TARGET_RT_64_BIT 0
#endif
#endif
