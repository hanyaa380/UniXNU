/* Darwin <mach-o/arch.h> for XNU's SETUP tools on Linux: NXArchInfo and the local (little-endian) host. */
#ifndef UNIXNU_COMPAT_MACHO_ARCH_H
#define UNIXNU_COMPAT_MACHO_ARCH_H
#include <mach/machine.h>

#include <architecture/byte_order.h>   /* enum NXByteOrder */

typedef struct {
	const char *name;
	cpu_type_t cputype;
	cpu_subtype_t cpusubtype;
	enum NXByteOrder byteorder;
	const char *description;
} NXArchInfo;

static inline const NXArchInfo *
NXGetLocalArchInfo(void)
{
#if defined(__aarch64__)
	static const NXArchInfo local = { "arm64", 0x0100000c, 0, NX_LittleEndian, "arm64" };
#else
	static const NXArchInfo local = { "x86_64", 0x01000007, 3, NX_LittleEndian, "x86_64" };
#endif
	return &local;
}
#endif
