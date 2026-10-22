/* Darwin <mach/mach_init.h> for XNU's SETUP tools on Linux. kextsymboltool only rounds its symbol-set layout to the
 * host page; this uses 4 KiB, the page of an Intel Mac. It changes the padding of the symbol sets, not the kernel. */
#ifndef UNIXNU_COMPAT_MACH_INIT_H
#define UNIXNU_COMPAT_MACH_INIT_H
#include <mach/vm_types.h>
#define UNIXNU_HOST_PAGE_MASK ((mach_vm_offset_t)4095)
#define mach_vm_round_page(x) (((mach_vm_offset_t)(x) + UNIXNU_HOST_PAGE_MASK) & ~UNIXNU_HOST_PAGE_MASK)
#define mach_vm_trunc_page(x) ((mach_vm_offset_t)(x) & ~UNIXNU_HOST_PAGE_MASK)
#endif
