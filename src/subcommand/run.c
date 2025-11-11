// SPDX-License-Identifier: MIT
/*
 *
 * This file is part of rurima, with ABSOLUTELY NO WARRANTY.
 *
 * MIT License
 *
 * Copyright (c) 2024 Moe-hacker
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
#include "../include/rurima.h"
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
	if (argc == 1) {
		chdir(argv[0]);
		ruri(2, (char *[]){ "ruri", "./rootfs", NULL });
		exit(0);
	} else {
		if (access(argv[1], F_OK) == 0) {
			rurima_error("{red}%s already exists!\n", argv[1]);
		}
		rurima_mkdirs(argv[1], 0755);
		chdir(argv[1]);
		rurima_fork_rexec((char *[]){ "pull", argv[0], "./rootfs", NULL });
		ruri(2, (char *[]){ "ruri", "./rootfs", NULL });
		exit(0);
	}
}
