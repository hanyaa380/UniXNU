/* Darwin <mach-o/swap.h> for XNU's SETUP tools on Linux. kextsymboltool swaps only for a target whose byte order
 * differs from the host's; every Linux host it runs on and every XNU target are little-endian, so these refuse. */
#ifndef UNIXNU_COMPAT_MACHO_SWAP_H
#define UNIXNU_COMPAT_MACHO_SWAP_H
#include <err.h>
#include <mach-o/arch.h>
#include <mach-o/loader.h>
#include <mach-o/nlist.h>

#define UNIXNU_NO_SWAP(name, type) \
	static inline void name(type *value, enum NXByteOrder order) { (void)value; (void)order; \
	errx(1, #name ": byte-swapped Mach-O output is not supported on this host"); }
UNIXNU_NO_SWAP(swap_mach_header, struct mach_header)
UNIXNU_NO_SWAP(swap_mach_header_64, struct mach_header_64)
UNIXNU_NO_SWAP(swap_segment_command, struct segment_command)
UNIXNU_NO_SWAP(swap_segment_command_64, struct segment_command_64)
UNIXNU_NO_SWAP(swap_symtab_command, struct symtab_command)
UNIXNU_NO_SWAP(swap_uuid_command, struct uuid_command)
static inline void swap_nlist(struct nlist *value, uint32_t count, enum NXByteOrder order)
{ (void)value; (void)count; (void)order; errx(1, "swap_nlist: byte-swapped Mach-O output is not supported on this host"); }
static inline void swap_nlist_64(struct nlist_64 *value, uint32_t count, enum NXByteOrder order)
{ (void)value; (void)count; (void)order; errx(1, "swap_nlist_64: byte-swapped Mach-O output is not supported on this host"); }
#endif
