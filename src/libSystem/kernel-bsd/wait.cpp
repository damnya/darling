#include "config.h"
#include "wait.h"
#include "libc/errno.h"
#include "libc/darwin_errno_codes.h"
#include "libc/signals.h"
#include "log.h"
#include <sys/time.h>
#include <sys/resource.h>

int __darwin_kill(pid_t pid, int sig)
{
	int rv = kill(pid, Darling::signalDarwinToLinux(sig));
	if (rv == -1)
		errnoOut();
	return rv;
}

pid_t __darwin_wait(int *stat_loc)
{
	return __darwin_waitpid(-1, stat_loc, 0);
}

// options are compatible
pid_t __darwin_wait3(int *stat_loc, int options, struct rusage *rusage)
{
	pid_t rv = __darwin_waitpid(-1, stat_loc, options);
	if (rv && rusage)
		getrusage(RUSAGE_CHILDREN, rusage); // TODO: may not be correct
	return rv;
}

pid_t __darwin_wait4(pid_t pid, int *stat_loc, int options, struct rusage *rusage)
{
	pid_t rv = __darwin_waitpid(pid, stat_loc, options);
	if (rv && rusage)
		getrusage(RUSAGE_CHILDREN, rusage); // TODO: may not be correct
	return rv;
}

pid_t __darwin_waitpid(pid_t pid, int *stat_loc, int options)
{
	pid_t rv = waitpid(pid, stat_loc, options);
	if (WIFSIGNALED(*stat_loc))
	{
		// rewrite the signal number
		// 0x7f on Linux
		int signum = (*stat_loc) & (0x7f);
		signum = Darling::signalLinuxToDarwin(signum);
		*stat_loc &= ~(0x7f);
		*stat_loc |= signum;
	}
	return rv; // TODO: check if compatible
}

