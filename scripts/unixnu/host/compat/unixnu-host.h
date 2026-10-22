/* BSD calls of XNU's SETUP tools that glibc lacks; host-cc includes this in every SETUP compile on Linux. */
#ifndef UNIXNU_COMPAT_HOST_H
#define UNIXNU_COMPAT_HOST_H
#include <errno.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

extern char *program_invocation_short_name;
#define getprogname() ((const char *)program_invocation_short_name)

static inline size_t
unixnu_strlcat(char *dst, const char *src, size_t size)
{
	size_t used = strnlen(dst, size), length = strlen(src);
	if (used == size) {
		return size + length;
	}
	size_t copy = length < size - used - 1 ? length : size - used - 1;
	memcpy(dst + used, src, copy);
	dst[used + copy] = '\0';
	return used + length;
}
#define strlcat unixnu_strlcat

/* setmode/getmode for the octal modes XNU passes to installfile -m; symbolic modes are refused. */
static inline void *
unixnu_setmode(const char *text)
{
	char *end;
	unsigned long value = strtoul(text, &end, 8);
	mode_t *mode;
	if (!*text || *end || value > 07777 || !(mode = malloc(sizeof(*mode)))) {
		errno = EINVAL;
		return NULL;
	}
	*mode = (mode_t)value;
	return mode;
}
static inline mode_t
unixnu_getmode(const void *set, mode_t base)
{
	(void)base;
	return *(const mode_t *)set;
}
#define setmode unixnu_setmode
#define getmode unixnu_getmode
#endif
