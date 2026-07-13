// SPDX-License-Identifier: MIT
/*
 *
 * This file is part of rurima, with ABSOLUTELY NO WARRANTY.
 *
 * MIT License
 *
 * Copyright (c) 2026 Moe-hacker
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 */
#include "include/rurima.h"
/*
 * run
 */
void rurima_run(int argc, char **_Nonnull argv)
{
	if (argc == 0) {
		cprintf("Usage: \n");
		cprintf("  rurima run <image:tag> <bundle_path>\n");
		cprintf("  rurima run <bundle_path>\n");
		return;
	}
	int pidfile_fd = memfd_create("test", MFD_CLOEXEC);
	fchmod(pidfile_fd, S_IRUSR | S_IWUSR);
	if (pidfile_fd == -1) {
		rurima_error("{red}Failed to create pidfile: %s\n", strerror(errno));
	}
	char proc_fs_fd_path[PATH_MAX];
	snprintf(proc_fs_fd_path, sizeof(proc_fs_fd_path), "/proc/%d/fd/%d", getpid(), pidfile_fd);
	//
	close(pidfile_fd);
	rurima_error("{red}rurima run is WIP, it should not be used by users.\n");
}
void rurima_create(int argc, char **_Nonnull argv)
{
	if (argc == 0) {
		cprintf("Usage: \n");
		cprintf("  rurima create <image:tag> <bundle_path>\n");
		return;
	}
	rurima_error("{red}rurima create is WIP, it should not be used by users.\n");
}