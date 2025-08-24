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
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
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
void end_animation(int unused__)
{
	printf("\r\e[K\033[0m\033[?25h");
	exit(0);
}
extern pid_t loading_animation_pid = 0;
void start_loading_animation(char *msg)
{
	if (rurima_global_config.no_process) {
		return;
	}
	loading_animation_pid = fork();
	if (loading_animation_pid == 0) {
		loading_animation(msg);
		exit(0);
	}
}
void end_loading_animation(void)
{
	if (loading_animation_pid > 0) {
		kill(loading_animation_pid, SIGINT);
		loading_animation_pid = 0;
	}
	waitpid(loading_animation_pid, NULL, 0);
	fflush(stdout);
}
void loading_animation(char *msg)
{
	signal(SIGINT, end_animation);
	printf("\033[?25l");
	char *chr[] = { "(´･ω･`)", "( `･ω･)", "(  `･ω)", "(   `･)", "(    `)", "(`    )", "(･`   )", "(ω･`  )", "(･ω･` )", NULL };
	int ptr = 0;
	while (1) {
		cprintf("\r{base}%s {blue}%s", chr[ptr], msg);
		fflush(stdout);
		usleep(150000);
		ptr++;
		if (chr[ptr] == NULL)
			ptr = 0;
	}
}
bool proot_exist(void)
{
	/*
	 * Test if proot exist.
	 * We use proot to execute ls, so that we can check if proot is really available.
	 */
	const char *cmd[] = { "proot", "ls", NULL };
	char *ret = rurima_fork_execvp_get_stdout(cmd);
	if (ret == NULL) {
		rurima_log("{red}proot not found.\n");
		return false;
	}
	free(ret);
	rurima_log("{green}proot found.\n");
	return true;
}
char *rurima_strstr_ignore_case(const char *_Nonnull haystack, const char *_Nonnull needle)
{
	/*
	 * An implementation of strccasestr().
	 */
	char *haystack_lower = strdup(haystack);
	char *needle_lower = strdup(needle);
	char *ret = NULL;
	for (size_t i = 0; i < strlen(haystack_lower); i++) {
		haystack_lower[i] = (char)tolower(haystack_lower[i]);
	}
	for (size_t i = 0; i < strlen(needle_lower); i++) {
		needle_lower[i] = (char)tolower(needle_lower[i]);
	}
	if (strstr(haystack_lower, needle_lower) == NULL) {
		free(haystack_lower);
		free(needle_lower);
		return NULL;
	}
	ret = (char *)((strstr(haystack_lower, needle_lower) - haystack_lower) + haystack);
	free(haystack_lower);
	free(needle_lower);
	return ret;
}
static char *get_dir_realpath(const char *_Nonnull dir)
{
	/*
	 * Get the realpath of the savedir.
	 * Warning: free() the return value after use.
	 */
	char *ret = malloc(PATH_MAX + strlen(dir) + 1);
	if (strchr(dir, '/') == dir) {
		sprintf(ret, "%s", dir);
	} else {
		getcwd(ret, PATH_MAX);
		strcat(ret, "/");
		if (strstr(dir, "./") == dir) {
			strcat(ret, dir + 2);
		} else {
			strcat(ret, dir);
		}
	}
	return ret;
}
void rurima_check_dir_deny_list(const char *_Nonnull dir)
{
	/*
	 * Check if the directory is in deny list.
	 * If it is, we will refuse to extract the archive.
	 * To protect the host system, we refuse to extract rootfs to unsafe directories.
	 */
	char *path = get_dir_realpath(dir);
	rurima_log("{base}Realpath: {cyan}%s{clear}\n", path);
	if (strcmp(path, "/") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /\n");
	}
	if (strstr(path, "/usr/") == path || strcmp(path, "/usr") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /usr/*\n");
	}
	if (strstr(path, "/etc/") == path || strcmp(path, "/etc") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /etc/*\n");
	}
	if (strstr(path, "/bin/") == path || strcmp(path, "/bin") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /bin/*\n");
	}
	if (strstr(path, "/lib/") == path || strcmp(path, "/lib") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /lib/*\n");
	}
	if (strstr(path, "/lib64/") == path || strcmp(path, "/lib64") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /lib64/*\n");
	}
	if (strstr(path, "/sbin/") == path || strcmp(path, "/sbin") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /sbin/*\n");
	}
	if (strstr(path, "/boot/") == path || strcmp(path, "/boot") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /boot/*\n");
	}
	if (strstr(path, "/dev/") == path || strcmp(path, "/dev") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /dev/*\n");
	}
	if (strstr(path, "/proc/") == path || strcmp(path, "/proc") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /proc/*\n");
	}
	if (strstr(path, "/sys/") == path || strcmp(path, "/sys") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /sys/*\n");
	}
	if (strstr(path, "/snap/") == path || strcmp(path, "/snap") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /snap/*\n");
	}
	if (strcmp(path, "/data") == 0 || strcmp(path, "/data/") == 0 || strcmp(path, "/data/.") == 0 || strcmp(path, "/data/./") == 0) {
		rurima_error("{red}Refuse to extract rootfs to /data\n");
	}
	free(path);
}
void rurima_get_input(char *_Nonnull message, char *_Nonnull buf)
{
	/*
	 * Warning: Not a safe function.
	 */
	cprintf("%s", message);
	fflush(stdout);
	fflush(stdin);
	scanf("%s", buf);
	fflush(stdout);
}
off_t rurima_get_file_size(const char *_Nonnull file)
{
	rurima_log("{base}Getting file size of {cyan}%s\n", file);
	struct stat st;
	if (stat(file, &st) == -1) {
		rurima_log("{red}Failed to get file size!\n");
		return 0;
	}
	rurima_log("{base}File size: {green}%ld\n", st.st_size);
	return st.st_size;
}
char *rurima_get_prefix(void)
{
	/*
	 * Warning: free() the return value after use.
	 */
	char *ret = getenv("PREFIX");
	if (ret == NULL) {
		ret = strdup("/");
	}
	return ret;
}
int rurima_mkdirs(const char *_Nonnull path, mode_t mode)
{
	/*
	 * A very simple implementation of mkdir -p.
	 * I don't know why it seems that there isn't an existing function to do this...
	 */
	remove(path);
	unlink(path);
	DIR *test = opendir(path);
	if (test != NULL) {
		closedir(test);
		return 0;
	}
	char buf[PATH_MAX];
	int ret = 0;
	/* If dir is path/to/mkdir
	 * We do:
	 * ret = mkdir("path",mode);
	 * ret = mkdir("path/to",mode);
	 * ret = mkdir("path/to/mkdir",mode);
	 * return ret;
	 */
	for (size_t i = 1; i < strlen(path); i++) {
		if (path[i] == '/') {
			for (size_t j = 0; j < i; j++) {
				buf[j] = path[j];
				buf[j + 1] = '\0';
			}
			ret = mkdir(buf, mode);
		}
	}
	// If the end of `dir` is not '/', create the last level of the directory.
	if (path[strlen(path) - 1] != '/') {
		ret = mkdir(path, mode);
	}
	return ret;
}
bool rurima_run_with_root(void)
{
	if (geteuid() == 0) {
		return true;
	}
	return false;
}
char *rurima_docker_get_host_arch(void)
{
	/*
	 * Get the cpu arch.
	 * We just need to check the macro we have.
	 *
	 * Do not free() the returned value.
	 */
	char *ret = NULL;
#if defined(__aarch64__)
	ret = "arm64";
#endif
#if defined(__alpha__)
	ret = "alpha";
#endif
#if defined(__arm__)
	ret = "arm";
#endif
#if defined(__armeb__)
	ret = "arm";
#endif
#if defined(__cris__)
	ret = "cris";
#endif
#if defined(__hexagon__)
	ret = "hexagon";
#endif
#if defined(__hppa__)
	ret = "hppa";
#endif
#if defined(__i386__)
	ret = "386";
#endif
#if defined(__loongarch64__)
	ret = "loong64";
#endif
#if defined(__m68k__)
	ret = "m68k";
#endif
#if defined(__microblaze__)
	ret = "microblaze";
#endif
#if defined(__mips__)
	ret = "mips";
#endif
#if defined(__mips64__)
	ret = "mips64";
#endif
#if defined(__mips64el__)
	ret = "mips64el";
#endif
#if defined(__mipsel__)
	ret = "mipsel";
#endif
#if defined(__mipsn32__)
	ret = "mipsn32";
#endif
#if defined(__mipsn32el__)
	ret = "mipsn32el";
#endif
#if defined(__ppc__)
	ret = "ppc";
#endif
#if defined(__ppc64__)
	ret = "ppc64";
#endif
#if defined(__ppc64le__)
	ret = "ppc64le";
#endif
#if defined(__riscv32__)
	ret = "riscv32";
#endif
#if defined(__riscv64__)
	ret = "riscv64";
#endif
#if defined(__s390x__)
	ret = "s390x";
#endif
#if defined(__sh4__)
	ret = "sh4";
#endif
#if defined(__sh4eb__)
	ret = "sh4eb";
#endif
#if defined(__sparc__)
	ret = "sparc";
#endif
#if defined(__sparc32plus__)
	ret = "sparc32plus";
#endif
#if defined(__sparc64__)
	ret = "sparc64";
#endif
#if defined(__x86_64__)
	ret = "amd64";
#endif
#if defined(__xtensa__)
	ret = "xtensa";
#endif
#if defined(__xtensaeb__)
	ret = "xtensaeb";
#endif
	if (ret == NULL) {
		rurima_error("{red}Unknow cpu arch!\n");
	}
	return ret;
}
char *rurima_lxc_get_host_arch(void)
{
	/*
	 * Get the cpu arch.
	 * We just need to check the macro we have.
	 *
	 * Do not free() the returned value.
	 */
	char *ret = NULL;
#if defined(__aarch64__)
	ret = "arm64";
#endif
#if defined(__alpha__)
	ret = "alpha";
#endif
#if defined(__arm__)
	ret = "armhf";
#endif
#if defined(__armeb__)
	ret = "armeabi";
#endif
#if defined(__cris__)
	ret = "cris";
#endif
#if defined(__hexagon__)
	ret = "hexagon";
#endif
#if defined(__hppa__)
	ret = "hppa";
#endif
#if defined(__i386__)
	ret = "i386";
#endif
#if defined(__loongarch64__)
	ret = "loongarch64";
#endif
#if defined(__m68k__)
	ret = "m68k";
#endif
#if defined(__microblaze__)
	ret = "microblaze";
#endif
#if defined(__mips__)
	ret = "mips";
#endif
#if defined(__mips64__)
	ret = "mips64";
#endif
#if defined(__mips64el__)
	ret = "mips64el";
#endif
#if defined(__mipsel__)
	ret = "mipsel";
#endif
#if defined(__mipsn32__)
	ret = "mipsn32";
#endif
#if defined(__mipsn32el__)
	ret = "mipsn32el";
#endif
#if defined(__ppc__)
	ret = "ppc";
#endif
#if defined(__ppc64__)
	ret = "ppc64";
#endif
#if defined(__ppc64le__)
	ret = "ppc64le";
#endif
#if defined(__riscv32__)
	ret = "riscv32";
#endif
#if defined(__riscv64__)
	ret = "riscv64";
#endif
#if defined(__s390x__)
	ret = "s390x";
#endif
#if defined(__sh4__)
	ret = "sh4";
#endif
#if defined(__sh4eb__)
	ret = "sh4eb";
#endif
#if defined(__sparc__)
	ret = "sparc";
#endif
#if defined(__sparc32plus__)
	ret = "sparc32plus";
#endif
#if defined(__sparc64__)
	ret = "sparc64";
#endif
#if defined(__x86_64__)
	ret = "amd64";
#endif
#if defined(__xtensa__)
	ret = "xtensa";
#endif
#if defined(__xtensaeb__)
	ret = "xtensaeb";
#endif
	if (ret == NULL) {
		rurima_error("{red}Unknow cpu arch!\n");
	}
	return ret;
}
bool rurima_rootless_supported(void)
{
	pid_t pid = fork();
	if (pid == 0) {
		if (getuid() == 0) {
			setuid(1145);
		}
		if (unshare(CLONE_NEWUSER) == -1) {
			exit(114);
		}
		exit(0);
	}
	int status;
	waitpid(pid, &status, 0);
	if (WEXITSTATUS(status) == 0) {
		return true;
	}
	return false;
}
off_t rurima_get_dir_file_size(const char *_Nonnull target)
{
	/*
	 * Get the size of the directory or file.
	 */
	const char *command[] = { "du", "-s", target, NULL };
	char *result = rurima_fork_execvp_get_stdout_ignore_err(command);
	if (result == NULL) {
		return 0;
	}
	off_t ret = 0;
	ret = atol(result);
	return ret * 1024;
}
bool rurima_sha256sum_exists(void)
{
	/*
	 * Check if sha256sum exists.
	 */
	const char *command[] = { "sha256sum", "-v", NULL };
	char *result = rurima_fork_execvp_get_stdout_ignore_err(command);
	if (result == NULL) {
		return false;
	}
	free(result);
	return true;
}
bool rurima_jq_exists(void)
{
	/*
	 * Check if jq exists.
	 */
	const char *command[] = { "jq", "--version", NULL };
	char *result = rurima_fork_execvp_get_stdout(command);
	if (result == NULL) {
		return false;
	}
	free(result);
	return true;
}
size_t rurima_split_lines(const char *_Nonnull input, char ***_Nonnull lines)
{
	/*
	 * Split the input into lines.
	 * Warning: free() the lines after use.
	 */
	size_t count = 0;
	char *input_copy = strdup(input);
	char *line = strtok(input_copy, "\n");
	while (line != NULL) {
		if (line[0] == '\0') {
			line = strtok(NULL, "\n");
			continue;
		}
		count++;
		line = strtok(NULL, "\n");
	}
	free(input_copy);
	if (count == 0) {
		return 0;
	}
	*lines = malloc(sizeof(char *) * (count + 2));
	input_copy = strdup(input);
	line = strtok(input_copy, "\n");
	size_t index = 0;
	while (line != NULL && strlen(line) > 0) {
		(*lines)[index++] = strdup(line);
		line = strtok(NULL, "\n");
	}
	free(input_copy);
	rurima_log("{base}lines count: {green}%zu{clear}\n", count);
	for (size_t i = 0; i < count; i++) {
		rurima_log("{base}lines[%zu]: {cyan}%s{clear}\n", i, (*lines)[i]);
	}
	(*lines)[count] = NULL; // Null-terminate the array
	return count;
}
size_t rurima_split_lines_allow_null(const char *_Nonnull input, char ***_Nonnull lines)
{
	/*
	 * Split the input into lines.
	 * Warning: free() the lines after use.
	 */
	size_t count = 0;
	char *input_copy = strdup(input);
	char *p = input_copy;
	char *line = strsep(&p, "\n");
	while (line != NULL) {
		count++;
		line = strsep(&p, "\n");
	}
	free(input_copy);
	if (count == 0) {
		return 0;
	}
	*lines = malloc(sizeof(char *) * (count + 2));
	input_copy = strdup(input);
	p = input_copy;
	line = strsep(&p, "\n");
	size_t index = 0;
	while (line != NULL) {
		if (strlen(line) > 0) {
			(*lines)[index++] = strdup(line);
		} else {
			(*lines)[index++] = NULL; // Allow null lines
		}
		line = strsep(&p, "\n");
	}
	free(input_copy);
	rurima_log("{base}lines count: {green}%zu{clear}\n", count);
	for (size_t i = 0; i < count; i++) {
		if ((*lines)[i] != NULL) {
			rurima_log("{base}lines[%zu]: {cyan}%s{clear}\n", i, (*lines)[i]);
		} else {
			rurima_log("{base}lines[%zu]: {cyan}NULL{clear}\n", i);
		}
	}
	(*lines)[count] = NULL; // Null-terminate the array
	return count;
}