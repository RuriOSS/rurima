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
 * OTA
 */
void rurima_ota(void)
{
#ifndef RURIMA_OFFICIAL_BUILD
	rurima_error("{red}You should remove rurima from your package manager, and use the official release to use this feature\n");
#endif
	start_loading_animation("Fetching latest commit id...");
#ifdef RURIMA_COMMIT_ID
	char *commit_id_local = RURIMA_COMMIT_ID;
#else
	char *commit_id_local = "unknown";
#endif
	char *cmd[] = { "curl", "-sL", "https://github.com/RuriOSS/rurima/releases/latest/download/commit-id.txt", NULL };
	char *commit_id_remote = rurima_fork_execvp_get_stdout(cmd);
	end_loading_animation();
	if (!commit_id_remote) {
		rurima_error("{red}Failed to get remote commit id, please check your network\n");
	}
	// Remove \n
	for (int i = 0; commit_id_remote[i]; i++) {
		if (commit_id_remote[i] == '\n') {
			commit_id_remote[i] = 0;
			break;
		}
	}
	if (strcmp(commit_id_local, commit_id_remote) == 0) {
		cprintf("{base}You are already using the latest version: {cyan}%s\n", commit_id_local);
		free(commit_id_remote);
		return;
	}
	cprintf("{base}New version available: {cyan}%s\n{base}You are using {cyan}%s\n", commit_id_remote, commit_id_local);
	char *tmpdir = getenv("TMPDIR");
	if (!tmpdir)
		tmpdir = "/tmp";
	chdir(tmpdir);
	// Download.
	char URL[PATH_MAX];
	char *hostarch = "unknown";
	// Map hostarch。
#if defined(__aarch64__)
	hostarch = "aarch64";
#elif defined(__arm__)
	hostarch = "armhf";
#elif defined(__armv7__)
	hostarch = "armv7";
#elif defined(__i386__)
	hostarch = "i386";
#elif defined(__loongarch64)
	hostarch = "loongarch64";
#elif defined(__ppc64le__)
	hostarch = "ppc64le";
#elif defined(__riscv) && (__riscv_xlen == 64)
	hostarch = "riscv64";
#elif defined(__s390x__)
	hostarch = "s390x";
#elif defined(__x86_64__)
	hostarch = "x86_64";
#else
	hostarch = "unknown";
#endif
	sprintf(URL, "https://github.com/RuriOSS/rurima/releases/latest/download/%s.tar", hostarch);
	cprintf("{base}Downloading for {cyan}%s\n", hostarch);
	if (rurima_download_file(URL, "rurima.tar", NULL, -1) != 0) {
		rurima_error("{red}Failed to download new version, please check your network\n");
	}
	rurima_extract_archive("rurima.tar", ".");
	char *self_path = realpath("/proc/self/exe", NULL);
#if defined(RURIMA_DEBUG) || defined(RURIMA_DEV)
	if (rurima_fork_execvp((char *[]){ "cp", "-f", "rurima-dbg", self_path, NULL }) != 0) {
		rurima_error("{red}Failed to replace old version, please try to run with sudo\n");
	}
#else
	if (rurima_fork_execvp((char *[]){ "cp", "-f", "rurima", self_path, NULL }) != 0) {
		rurima_error("{red}Failed to replace old version, please try to run with sudo\n");
	}
#endif
	rurima_fork_execvp((char *[]){ "rm", "-f", "rurima.tar", NULL });
	rurima_fork_execvp((char *[]){ "rm", "-f", "rurima", NULL });
	rurima_fork_execvp((char *[]){ "rm", "-f", "rurima-dbg", NULL });
	rurima_fork_execvp((char *[]){ "rm", "-f", "LICENSE", NULL });
	return;
}
