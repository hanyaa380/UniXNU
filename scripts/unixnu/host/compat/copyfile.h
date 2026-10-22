/* Darwin <copyfile.h> for XNU's SETUP tools on Linux: (f)copyfile with COPYFILE_DATA, file data only. */
#ifndef UNIXNU_COMPAT_COPYFILE_H
#define UNIXNU_COMPAT_COPYFILE_H
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>

typedef void *copyfile_state_t;
typedef unsigned int copyfile_flags_t;
#define COPYFILE_DATA (1 << 3)

static inline int
fcopyfile(int in, int out, copyfile_state_t state, copyfile_flags_t flags)
{
	char buffer[1 << 16];
	ssize_t count;
	(void)state;
	if (flags != COPYFILE_DATA) {
		errno = ENOTSUP;
		return -1;
	}
	while ((count = read(in, buffer, sizeof(buffer))) > 0) {
		if (write(out, buffer, (size_t)count) != count) {
			return -1;
		}
	}
	return count < 0 ? -1 : 0;
}

static inline int
copyfile(const char *from, const char *to, copyfile_state_t state, copyfile_flags_t flags)
{
	int in = open(from, O_RDONLY), out = in < 0 ? -1 : open(to, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	int result = out < 0 ? -1 : fcopyfile(in, out, state, flags);
	if (in >= 0) {
		close(in);
	}
	if (out >= 0) {
		close(out);
	}
	return result;
}
#endif
