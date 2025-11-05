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
#include "include/rurima.h"
bool disable_rurima_log = false;
// clang-format off
struct RURIMA_CONFIG rurima_global_config = {
	.docker_mirror = "registry-1.docker.io",
	.lxc_mirror = "images.linuxcontainers.org",
	.quiet = false,
	.no_progress = false,
};
// clang-format on
static void show_help(void)
{
	cprintf("{base}Usage: rurima [subcommand] [options]\n");
	cprintf("{base}Subcommands:\n");
	cprintf("{base}  docker, d : DockerHub support.\n");
	cprintf("{base}  lxc, l    : LXC mirror support.\n");
	cprintf("{base}  pull, p   : Pull image, as a wrap of rurima docker/lxc pull.\n");
	cprintf("{base}  unpack, u : Unpack rootfs.\n");
	cprintf("{base}  backup, b : Backup rootfs.\n");
	cprintf("{base}  ruri, r   : Built-in ruri command.\n");
	cprintf("{base}  help, h   : Show help message.\n");
	cprintf("{base}  dep       : Check dependencies.\n");
// OTA is only available in official build.
#ifdef RURIMA_OFFICIAL_BUILD
	cprintf("{base}  ota       : Update rurima.\n");
#endif
	cprintf("{base}Options:\n");
	cprintf("{base}  -q, --quiet: Disable all unnecessary output.\n");
	cprintf("{base}  -h, --help: Show help message.\n");
	cprintf("{base}  -v, --version: Show version info.\n");
	cprintf("{base}  -V, --version-code: Show version code.\n");
	cprintf("{base}See rurima [subcommand] help for further information.\n");
	cprintf("{base}Use `rurima -q subcommand` to disable log in rurima-dbg.\n");
}
static void rurima_dep_info(void)
{
	char *tar_command[] = { "tar", "--version", NULL };
	char *curl_command[] = { "curl", "--version", NULL };
	char *file_command[] = { "file", "--version", NULL };
	char *gz_command[] = { "gzip", "--version", NULL };
	char *xz_command[] = { "xz", "-V", NULL };
	char *file_command_2[] = { "file", "--brief", "--mime-type", "/proc/self/exe", NULL };
	char *proot_command[] = { "proot", "-V", NULL };
	char *sha256_command[] = { "sha256sum", "--version", NULL };
	char *jq_command[] = { "jq", "--version", NULL };
	char *result = NULL;
	result = rurima_fork_execvp_get_stdout(tar_command);
	if (result == NULL) {
		cprintf("{base}tar : {red}not found\n");
	} else {
		cprintf("{base}tar :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(curl_command);
	if (result == NULL) {
		cprintf("{base}curl : {red}not found\n");
	} else {
		cprintf("{base}curl :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(file_command);
	if (result == NULL) {
		cprintf("{base}file : {red}not found\n");
	} else {
		cprintf("{base}file :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(file_command_2);
	if (result == NULL) {
		cprintf("{base}file : {red}not support --brief --mime-type\n");
	} else {
		cprintf("\n{blue}file --brief --mime-type /proc/self/exe\n%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(gz_command);
	if (result == NULL) {
		cprintf("{base}gzip : {red}not found\n");
	} else {
		cprintf("{base}gzip :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(xz_command);
	if (result == NULL) {
		cprintf("{base}xz : {red}not found\n");
	} else {
		cprintf("{base}xz :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(proot_command);
	if (result == NULL) {
		cprintf("{base}proot : {yellow}not found\n");
	} else {
		cprintf("{base}proot :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(sha256_command);
	if (result == NULL) {
		cprintf("{base}sha256sum : {yellow}not found\n");
	} else {
		cprintf("{base}sha256sum :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
	result = rurima_fork_execvp_get_stdout(jq_command);
	if (result == NULL) {
		cprintf("{base}jq : {yellow}not found\n");
	} else {
		cprintf("{base}jq :{clear}{green} found\n{blue}%s", result);
	}
	free(result);
}
static void detect_suid_or_capability(void)
{
	struct stat st;
	if (stat("/proc/self/exe", &st) == 0) {
		if (((st.st_mode & S_ISUID) || (st.st_mode & S_ISGID)) && (geteuid() == 0 || getegid() == 0)) {
			rurima_error("{red}SUID or SGID bit detected on rurima, this is unsafe desu QwQ\n");
		}
	}
	cap_t caps = cap_get_file("/proc/self/exe");
	if (caps == NULL) {
		return;
	}
	char *caps_str = cap_to_text(caps, NULL);
	if (caps_str == NULL) {
		return;
	}
	if (strlen(caps_str) > 0) {
		rurima_error("{red}Capabilities detected on rurima, this is unsafe desu QwQ\n");
	}
	cap_free(caps);
	cap_free(caps_str);
}
static int pmcrts(const char *s1, const char *s2)
{
	/*
	 *
	 * Compare two strings, but s2 is in the end of s1.
	 * Return the same value as strcmp() s2 and the end of s1.
	 * If s1 is shorter than s2, it will return -1.
	 *
	 */
	size_t len1 = strlen(s1);
	size_t len2 = strlen(s2);
	if (len1 < len2) {
		return -1; // s1 is shorter than s2
	}
	return strcmp(s2, s1 + len1 - len2);
}
int main(int argc, char **argv)
{
	// Check for argv[0], if it's ruri, we will only call built-in ruri.
	if (argc > 0 && pmcrts(argv[0], "ruri") == 0) {
		rurima_log("{green}Detected ruri mode, calling built-in ruri.\n");
		return ruri(argc, argv);
	}
	detect_suid_or_capability();
#ifdef RURIMA_DEV
	rurima_warning("{red}You are using dev/debug build, if you think this is wrong, please rebuild rurima or get it from release page.\n");
#endif
	rurima_register_signal();
	// Check for terminal width.
	struct winsize size;
	if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &size) == -1) {
		rurima_global_config.no_progress = true;
	}
	if (size.ws_col <= 10 || size.ws_col >= 500) {
		rurima_global_config.no_progress = true;
	}
	// Parse args.
	if (argc == 1) {
		show_help();
		return 0;
	}
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
			disable_rurima_log = true;
			rurima_global_config.quiet = true;
			rurima_global_config.no_progress = true;
			continue;
		}
		if (strcmp(argv[i], "docker") == 0 || strcmp(argv[i], "d") == 0) {
			rurima_check_dep();
			if (i + 1 >= argc) {
				rurima_error("{red}No subcommand specified!\n");
			}
			rurima_docker(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "lxc") == 0 || strcmp(argv[i], "l") == 0) {
			rurima_check_dep();
			if (i + 1 >= argc) {
				rurima_error("{red}No subcommand specified!\n");
			}
			rurima_lxc(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "pull") == 0 || strcmp(argv[i], "p") == 0) {
			rurima_check_dep();
			rurima_pull(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "backup") == 0 || strcmp(argv[i], "b") == 0) {
			rurima_check_dep();
			rurima_backup(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "unpack") == 0 || strcmp(argv[i], "u") == 0) {
			rurima_check_dep();
			rurima_unpack(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "dep") == 0) {
			rurima_dep_info();
			return 0;
		}
		if (strcmp(argv[i], "help") == 0 || strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			show_help();
			return 0;
		}
		if (strcmp(argv[i], "version") == 0 || strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			rurima_show_version_info();
			return 0;
		}
		if (strcmp(argv[i], "-V") == 0 || strcmp(argv[i], "--version-code") == 0) {
			rurima_show_version_code();
			return 0;
		}
		if (strcmp(argv[i], "ruri") == 0 || strcmp(argv[i], "r") == 0) {
			ruri_clear_env(argv);
			ruri(argc - i, &argv[i]);
			return 0;
		}
		if (strcmp(argv[i], "ota") == 0) {
			rurima_ota();
			return 0;
		}
		if (strcmp(argv[i], "run") == 0) {
			rurima_run(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "netns") == 0) {
			rurima_netns_wrapper(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		if (strcmp(argv[i], "load") == 0) {
			rurima_load_rootfs(argc - i - 1, &argv[i + 1]);
			return 0;
		}
		show_help();
		rurima_error("{red}Invalid subcommand `%s`!\n", argv[i]);
		return 1;
	}
	return 0;
}
