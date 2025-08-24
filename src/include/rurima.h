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
// Enable Linux features.
#ifndef __linux__
#error "This program is only for linux."
#else
#define _GNU_SOURCE
#endif
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <limits.h>
#include <dirent.h>
#include <sys/time.h>
#include <float.h>
// This program need be linked with -lcap.
#include <sys/capability.h>
// Bool!!!
#if __STDC_VERSION__ < 202000L
#ifndef bool
#define bool _Bool
#define true ((_Bool) + 1u)
#define false ((_Bool) + 0u)
#endif
#endif
#ifndef _Nullable
#define _Nullable
#endif
#ifndef _Nonnull
#define _Nonnull
#endif
#include "version.h"
#include "../ruri/src/include/ruri.h"
struct RURIMA_CONFIG {
	char *_Nonnull docker_mirror;
	char *_Nonnull lxc_mirror;
	char *_Nullable hook_script;
	bool quiet;
	bool no_process;
};
extern struct RURIMA_CONFIG rurima_global_config;
struct RURIMA {
	/*
	 * This is full rurima config.
	 */
	// rootfs source: docker, lxc, rootfs or hostdir.
	char *_Nonnull rootfs_source;
	// For docker.
	char *_Nullable rootfs_image;
	// For docker.
	char *_Nullable rootfs_tag;
	// For docker/lxc.
	char *_Nullable rootfs_arch;
	// For lxc.
	char *_Nullable rootfs_mirror;
	// For lxc.
	char *_Nullable rootfs_os;
	// For lxc.
	char *_Nullable rootfs_version;
	// For lxc.
	char *_Nullable rootfs_type;
	// For rootfs.
	char *_Nullable rootfs_path;
	// For rootfs.
	char *_Nullable host_dir;
	// Hook script, will be copied into /tmp.
	char *_Nullable hook_script;
	// Hook command.
	char *_Nullable hook_command[RURI_MAX_COMMANDS + 1];
	// Full ruri container config.
	struct RURI_CONTAINER container;
};
struct RURIMA_DOCKER {
	/*
	 * This is part of docker config that we need.
	 */
	// Workdir.
	char *_Nullable workdir;
	// ENV.
	char *_Nullable env[RURI_MAX_ENVS + 2];
	// Command.
	char *_Nullable command[RURI_MAX_COMMANDS + 1];
	// Entry point.
	char *_Nullable entrypoint[RURI_MAX_COMMANDS + 1];
	// Architecture.
	char *_Nullable architecture;
};
// Warnings.
#define rurima_warning(format, ...)                                                                         \
	{                                                                                                   \
		if (!rurima_global_config.quiet) {                                                          \
			cfprintf(stderr, "{yellow}In %s() in %s line %d:\n", __func__, __FILE__, __LINE__); \
			cfprintf(stderr, format, ##__VA_ARGS__);                                            \
		}                                                                                           \
	}
// Show error msg and exit.
#define rurima_error(format, ...)                                                                                      \
	{                                                                                                              \
		end_loading_animation();                                                                               \
		cfprintf(stderr, "{red}In %s() in %s line %d:\n", __func__, __FILE__, __LINE__);                       \
		cfprintf(stderr, format, ##__VA_ARGS__);                                                               \
		cfprintf(stderr, "{base}%s{clear}\n", "  .^.   .^.");                                                  \
		cfprintf(stderr, "{base}%s{clear}\n", "  /⋀\\_ﾉ_/⋀\\");                                                \
		cfprintf(stderr, "{base}%s{clear}\n", " /ﾉｿﾉ\\ﾉｿ丶)|");                                                \
		cfprintf(stderr, "{base}%s{clear}\n", " ﾙﾘﾘ >  x )ﾘ");                                                 \
		cfprintf(stderr, "{base}%s{clear}\n", "ﾉノ㇏  ^  ﾉ|ﾉ");                                                \
		cfprintf(stderr, "{base}%s{clear}\n", "      ⠁⠁");                                                     \
		cfprintf(stderr, "{base}%s{clear}\n", "RURIMA ERROR MESSAGE");                                         \
		cfprintf(stderr, "{base}%s{clear}\n", "Hint:");                                                        \
		cfprintf(stderr, "{base}%s{clear}\n", "  If you have network problems for lxc or docker subcommand,"); \
		cfprintf(stderr, "{base}%s{clear}\n", "  please use -m option to change the mirror.");                 \
		cfprintf(stderr, "{base}%s{clear}\n", "  For docker subcommand, try -f to enable fallback mode.");     \
		cfprintf(stderr, "{base}%s{clear}\n", "If you think something is wrong, please report at:");           \
		cfprintf(stderr, "\033[4m{base}%s{clear}\n", "https://github.com/Moe-hacker/rurima/issues");           \
		exit(114);                                                                                             \
	}
// Log system.
// We use a global variable to disable log in runtime.
extern bool disable_rurima_log;
#if defined(RURIMA_DEBUG)
#define rurima_log(format, ...)                                                                                                               \
	{                                                                                                                                     \
		if (!disable_rurima_log) {                                                                                                    \
			struct timeval tv;                                                                                                    \
			gettimeofday(&tv, NULL);                                                                                              \
			cfprintf(stderr, "{green}[%ld.%06ld] in %s() in %s line %d:\n", tv.tv_sec, tv.tv_usec, __func__, __FILE__, __LINE__); \
			cfprintf(stderr, format, ##__VA_ARGS__);                                                                              \
		}                                                                                                                             \
	}
#else
#define rurima_log(format, ...)
#endif

struct RURIMA_DOCKER_PULL {
	const char *image;
	const char *tag;
	const char *architecture;
	const char *savedir;
	const char *mirror;
	bool fallback;
	int skip_layer;
};
// Functions.
int rurima_fork_execvp(const char *_Nonnull argv[]);
char *rurima_fork_execvp_get_stdout(const char *_Nonnull argv[]);
int rurima_extract_archive(const char *_Nonnull file, const char *_Nonnull dir);
off_t rurima_get_file_size(const char *_Nonnull file);
char *rurima_get_prefix(void);
int rurima_mkdirs(const char *_Nonnull path, mode_t mode);
bool rurima_run_with_root(void);
int rurima_docker_search(const char *_Nonnull image, const char *_Nonnull page_size, bool quiet, const char *_Nullable mirror);
int rurima_docker_search_tag(const char *_Nonnull image, const char *_Nonnull page_size, const char *_Nullable architecture, bool quiet, const char *_Nullable mirror);
struct RURIMA_DOCKER *rurima_docker_pull(struct RURIMA_DOCKER_PULL *_Nonnull action);
void rurima_register_signal(void);
char *rurima_docker_get_host_arch(void);
char *rurima_lxc_get_host_arch(void);
void rurima_lxc_pull_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nonnull version, const char *_Nullable architecture, const char *_Nullable type, const char *_Nonnull savedir);
void rurima_lxc_get_image_list(const char *_Nullable mirror, const char *_Nullable architecture);
void rurima_lxc_search_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nullable architecture);
void rurima_docker(int argc, char **_Nonnull argv);
void rurima_lxc(int argc, char **_Nonnull argv);
void rurima_unpack(int argc, char **_Nonnull argv);
struct RURIMA *rurima_init_config(void);
void rurima_get_input(char *_Nonnull message, char *_Nonnull buf);
void rurima_check_dep(void);
struct RURIMA_DOCKER *rurima_get_docker_config(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nullable architecture, const char *_Nullable mirror, bool fallback);
void rurima_show_docker_config(struct RURIMA_DOCKER *_Nonnull config, char *_Nullable savedir, char *_Nullable runtime, bool quiet);
void rurima_free_docker_config(struct RURIMA_DOCKER *_Nonnull config);
void rurima_lxc_search_arch(const char *_Nullable mirror, const char *_Nonnull os);
int rurima_docker_search_arch(const char *_Nonnull image, const char *_Nonnull tag, char *_Nullable mirror, bool fallback);
void rurima_show_version_info(void);
void rurima_show_version_code(void);
void rurima_check_dir_deny_list(const char *_Nonnull dir);
char *rurima_strstr_ignore_case(const char *_Nonnull haystack, const char *_Nonnull needle);
int rurima_fork_rexec(char **_Nonnull argv);
void rurima_add_argv(char ***_Nonnull argv, char *_Nonnull arg);
void *rurima_default_hook(const char *_Nonnull container_dir);
void rurima_read_global_config(void);
bool rurima_rootless_supported(void);
off_t rurima_get_dir_file_size(const char *_Nonnull target);
int rurima_backup_dir(const char *_Nonnull file, const char *_Nonnull dir);
void rurima_backup(int argc, char **_Nonnull argv);
char *rurima_fork_execvp_get_stdout_ignore_err(const char *_Nonnull argv[]);
bool rurima_sha256sum_exists(void);
void rurima_pull(int argc, char **_Nonnull argv);
bool rurima_lxc_have_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nonnull version, const char *_Nullable architecture, const char *_Nullable type);
int rurima_download_file(const char *_Nonnull url, const char *_Nonnull file, const char *_Nullable token, ssize_t size);
bool proot_exist(void);
bool rurima_jq_exists(void);
char *rurima_fork_execvp_get_stdout_with_input(const char *_Nonnull argv[], const char *_Nonnull input);
char *rurima_call_jq(const char *_Nonnull argv[], const char *_Nonnull input);
size_t rurima_split_lines(const char *_Nonnull input, char ***_Nonnull lines);
size_t rurima_split_lines_allow_null(const char *_Nonnull input, char ***_Nonnull lines);
void loading_animation(char *msg);
void start_loading_animation(char *msg);
void end_loading_animation(void);
extern pid_t loading_animation_pid;