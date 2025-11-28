// SPDX-License-Identifier: MIT
/*
 *
 * This file is part of cpprintf, with ABSOLUTELY NO WARRANTY.
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
 */
//
// This program has Super Neko Powers
//
// Damn bro, it's only for Linux
#ifndef __linux__
#error "This code is only for Linux"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <limits.h>
#include <dirent.h>
#include <sys/time.h>
#include <float.h>
#include <linux/limits.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sched.h>
#include <syscall.h>
#if __STDC_VERSION__ < 202000L
#ifndef bool
#define bool _Bool
#define true ((_Bool) + 1u)
#define false ((_Bool) + 0u)
#endif
#endif
#define ERROR_NUM 114
#define error(...)                                                                       \
	{                                                                                \
		fprintf(stderr, "Error in %s:%s%d: ", __FILE__, __FUNCTION__, __LINE__); \
		fprintf(stderr, __VA_ARGS__);                                            \
		fprintf(stderr, "\n");                                                   \
		on_exit__(SIGINT);                                                       \
		exit(ERROR_NUM);                                                         \
	}
#define pprintf(...)                 \
	{                            \
		printf(__VA_ARGS__); \
	}
void remove_test_dot_c(void);
void on_exit__(int sig)
{
	pprintf("Exiting %d......\n", getpid());
	while (waitpid(-1, NULL, WNOHANG) > 0)
		;
	remove_test_dot_c();
	exit(ERROR_NUM);
}
int fork_exec(char **argv)
{
	// Return -1 if failed to exec()
	// Or return same as exec()ed program.
	int pid = fork();
	if (pid < 0) {
		perror("fork failed");
		return -1;
	} else if (pid == 0) {
		int fd = open("/dev/null", O_WRONLY);
		dup2(fd, STDOUT_FILENO);
		dup2(fd, STDERR_FILENO);
		dup2(fd, STDIN_FILENO);
		close(fd);
		execvp(argv[0], argv);
		perror("exec failed");
		exit(ERROR_NUM);
	} else {
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			return WEXITSTATUS(status);
		} else {
			return -1;
		}
	}
}
char *fork_execvp_get_stdout(char *argv[])
{
	/*
	 * Warning: free() after use.
	 * We will fork(2) and then execvp(3).
	 * And then we will get the stdout of the child process.
	 * Return the stdout of the child process.
	 * If failed, return NULL.
	 */
	// Create a pipe.
	int pipefd[2];
	if (pipe(pipefd) == -1) {
		return NULL;
	}
	// fork(2) and then execvp(3).
	int pid = fork();
	if (pid == -1) {
		close(pipefd[0]);
		close(pipefd[1]);
		return NULL;
	}
	if (pid == 0) {
		// Close the read end of the pipe.
		close(pipefd[0]);
		// Redirect stdout and stderr to the write end of the pipe.
		dup2(pipefd[1], STDOUT_FILENO);
		int nullfd = open("/dev/null", O_WRONLY);
		dup2(nullfd, STDERR_FILENO);
		close(pipefd[1]);
		execvp(argv[0], (char **)argv);
		exit(ERROR_NUM);
	} else {
		// Close the write end of the pipe.
		close(pipefd[1]);
		// Get the output from the read end of the pipe.
		size_t buffer_size = 1024;
		size_t total_read = 0;
		char *output = malloc(buffer_size);
		if (!output) {
			close(pipefd[0]);
			return NULL;
		}
		ssize_t bytes_read;
		while ((bytes_read = read(pipefd[0], output + total_read, buffer_size - total_read - 1)) > 0) {
			total_read += (size_t)bytes_read;
			if (total_read >= buffer_size - 1) {
				buffer_size *= 2;
				char *new_output = realloc(output, buffer_size);
				if (!new_output) {
					free(output);
					close(pipefd[0]);
					return NULL;
				}
				output = new_output;
			}
		}
		if (bytes_read == -1) {
			free(output);
			close(pipefd[0]);
			return NULL;
		}
		output[total_read] = '\0';
		close(pipefd[0]);
		int status = 0;
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status) != 0) {
			free(output);
			return NULL;
		}
		return output;
	}
	return NULL;
}
// Never mind, this is safe broooo
char **CFLAGS = NULL;
char **LIBS = NULL;
char *CC = "cc";
char *STRIP = "strip";
char *BUILD_DIR = "out";
char *SRC_DIR = NULL;
char **OBJS = NULL;
char *OUTPUT = NULL;
char *COMMIT_ID = NULL;
char *RURI_COMMIT_ID = NULL;
int JOBS = 8;
bool FORCE = false; // Force rebuild all files
// Not only args, but (char **) in fact.
void add_args(char ***argv, const char *arg)
{
	if (*argv == NULL) {
		*argv = malloc(2 * sizeof(char *));
		if (!*argv) {
			error("Error: Failed to allocate memory for args");
		}
		(*argv)[0] = strdup(arg);
		if (!(*argv)[0]) {
			free(*argv);
			*argv = NULL;
			error("Error: Failed to duplicate string for args");
		}
		(*argv)[1] = NULL;
	} else {
		size_t len = 0;
		while ((*argv)[len] != NULL)
			len++;
		char **new_argv = realloc(*argv, (len + 2) * sizeof(char *));
		if (!new_argv) {
			error("Error: Failed to reallocate memory for args");
		}
		*argv = new_argv;
		(*argv)[len] = strdup(arg);
		if (!(*argv)[len]) {
			error("Error: Failed to duplicate string for args");
		}
		(*argv)[len + 1] = NULL;
	}
}
// Not only args, but (char **) in fact.
void free_args(char **arg)
{
	for (int i = 0; arg && arg[i] != NULL; i++) {
		free(arg[i]);
	}
	free(arg);
}
// For testing compile environment
void remove_test_dot_c(void)
{
	remove("test.c");
	unlink("test.c");
	rmdir("test.c");
}
void create_test_dot_c(void)
{
	remove_test_dot_c();
	FILE *fp = fopen("test.c", "w");
	if (!fp) {
		error("Failed to create test.c");
	}
	fprintf(fp, "#include <stdio.h>\n");
	fprintf(fp, "int main(void) {\n");
	fprintf(fp, "    printf(\"Hello, World!\\n\");\n");
	fprintf(fp, "    return 0;\n");
	fprintf(fp, "}\n");
	fclose(fp);
}
// Check if CC supports a specific C flag
bool check_c_flag(const char *flag)
{
	create_test_dot_c();
	char **args = NULL;
	add_args(&args, CC);
	for (int i = 0; CFLAGS && CFLAGS[i] != NULL; i++) {
		add_args(&args, CFLAGS[i]);
	}
	add_args(&args, flag);
	add_args(&args, "-o");
	add_args(&args, "/dev/null");
	add_args(&args, "test.c");
	for (int i = 0; LIBS && LIBS[i] != NULL; i++) {
		add_args(&args, LIBS[i]);
	}
	if (fork_exec(args) != 0) {
		pprintf("Check for flag %s :failed\n", flag);
		free_args(args);
		return false;
	}
	free_args(args);
	pprintf("Check for flag %s :success\n", flag);
	return true;
}
// Check if we can use a specific library
bool check_lib(const char *lib)
{
	create_test_dot_c();
	char **args = NULL;
	add_args(&args, CC);
	for (int i = 0; CFLAGS && CFLAGS[i] != NULL; i++) {
		add_args(&args, CFLAGS[i]);
	}
	add_args(&args, "-o");
	add_args(&args, "/dev/null");
	add_args(&args, "test.c");
	for (int i = 0; LIBS && LIBS[i] != NULL; i++) {
		add_args(&args, LIBS[i]);
	}
	add_args(&args, lib);
	if (fork_exec(args) != 0) {
		pprintf("Check for lib %s :failed\n", lib);
		free_args(args);
		return false;
	}
	free_args(args);
	pprintf("Check for lib %s :success\n", lib);
	return true;
}
// init
void init_env(void)
{
	create_test_dot_c();
	if (getenv("CC")) {
		CC = getenv("CC");
	}
	if (getenv("STRIP")) {
		STRIP = getenv("STRIP");
	}
	if (getenv("CFLAGS")) {
		char *flags = strdup(getenv("CFLAGS"));
		if (!flags) {
			error("Error: Failed to duplicate CFLAGS string");
		}
		char *token = strtok(flags, " ");
		while (token) {
			add_args(&CFLAGS, token);
			token = strtok(NULL, " ");
		}
		free(flags);
	}
	char **arg = NULL;
	add_args(&arg, CC);
	for (int i = 0; CFLAGS && CFLAGS[i] != NULL; i++) {
		add_args(&arg, CFLAGS[i]);
	}
	add_args(&arg, "-o");
	add_args(&arg, "/dev/null");
	add_args(&arg, "test.c");
	if (fork_exec(arg) != 0) {
		error("Error: Compiler %s failed to compile %s\n", CC, "test.c");
		exit(ERROR_NUM);
	}
	pprintf("CC: %s\n", CC);
	free_args(arg);
	char *commit_id = fork_execvp_get_stdout((char *[]){ "git", "-C", "..", "rev-parse", "--short", "HEAD", NULL });
	if (commit_id) {
		if (commit_id[strlen(commit_id) - 1] == '\n') {
			commit_id[strlen(commit_id) - 1] = '\0';
		}
		pprintf("Commit ID: %s\n", commit_id);
	} else {
		pprintf("Warning: failed to get commit ID\n");
	}
	COMMIT_ID = commit_id;
	char *ruri_commit_id = fork_execvp_get_stdout((char *[]){ "git", "-C", "../src/ruri", "rev-parse", "--short", "HEAD", NULL });
	if (ruri_commit_id) {
		if (ruri_commit_id[strlen(ruri_commit_id) - 1] == '\n') {
			ruri_commit_id[strlen(ruri_commit_id) - 1] = '\0';
		}
		pprintf("RURI Commit ID: %s\n", ruri_commit_id);
	} else {
		pprintf("Warning: failed to get RURI commit ID\n");
	}
	RURI_COMMIT_ID = ruri_commit_id;
	if (fork_exec((char *[]){ STRIP, "--version", NULL }) != 0) {
		STRIP = NULL;
	} else {
		pprintf("STRIP: %s\n", STRIP);
	}
}
// Switch to the build directory
void switch_to_build_dir(char *dir)
{
	// Check for src dir
	SRC_DIR = realpath("./src", NULL);
	if (!SRC_DIR) {
		error("Error: failed to resolve path for ./src\n");
	}
	char *basedir = realpath(".", NULL);
	if (!basedir) {
		error("Error: failed to resolve current directory path\n");
	}
	static char out[PATH_MAX];
	sprintf(out, "%s/%s", basedir, "rurima");
	OUTPUT = out;
	free(basedir);
	// Change to the build directory
	mkdir(dir, 0755);
	if (chdir(dir) != 0) {
		error("Error: failed to change directory to %s\n", dir);
	}
	BUILD_DIR = realpath(".", NULL);
	if (!BUILD_DIR) {
		error("Error: failed to resolve build directory path\n");
	}
}
// Get file name without path
char *basename_of(const char *path)
{
	const char *name = strrchr(path, '/');
	return name ? (char *)(name + 1) : (char *)path;
}
int copy_file(const char *src, const char *dest)
{
	int src_fd = open(src, O_RDONLY);
	if (src_fd < 0) {
		perror("Error opening source file");
		return -1;
	}
	int dest_fd = open(dest, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	if (dest_fd < 0) {
		perror("Error opening destination file");
		pprintf("%s\n", dest);
		close(src_fd);
		return -1;
	}
	struct stat st;
	if (fstat(src_fd, &st) < 0) {
		perror("Error getting source file size");
		close(src_fd);
		close(dest_fd);
		return -1;
	}
	ssize_t result = syscall(SYS_sendfile, dest_fd, src_fd, NULL, st.st_size);
	if (result < 0) {
		perror("Error copying file with sendfile");
		close(src_fd);
		close(dest_fd);
		return -1;
	}
	close(src_fd);
	close(dest_fd);
	return 0;
}
bool file_diff(char *file1, char *file2)
{
	/*
	 * Check if two files are different.
	 * Return true if they are different, false if they are the same.
	 */
	int fd1 = open(file1, O_RDONLY);
	if (fd1 < 0) {
		perror("Error opening file1");
		return true;
	}
	int fd2 = open(file2, O_RDONLY);
	if (fd2 < 0) {
		perror("Error opening file2");
		close(fd1);
		return true;
	}
	// Check file size.
	struct stat st1, st2;
	if (fstat(fd1, &st1) < 0 || fstat(fd2, &st2) < 0) {
		close(fd1);
		close(fd2);
		return true;
	}
	if (st1.st_size != st2.st_size) {
		close(fd1);
		close(fd2);
		return true;
	}
	char buf1[4096], buf2[4096];
	ssize_t read1, read2;
	while ((read1 = read(fd1, buf1, sizeof(buf1))) > 0 && (read2 = read(fd2, buf2, sizeof(buf2))) > 0) {
		if (read1 != read2 || memcmp(buf1, buf2, read1) != 0) {
			close(fd1);
			close(fd2);
			return true; // Files are different
		}
	}
	close(fd1);
	close(fd2);
	return false; // Files are the same
}
char *gen_obj_name(const char *file)
{
	/*
	 * Generate object file name from source file name.
	 * For example, if file is "src/main.c", it will return "main.o".
	 */
	char *base = strstr(file, SRC_DIR);
	if (!base) {
		error("Error: Failed to find source directory in file path");
	}
	base += strlen(SRC_DIR) + 1;
	base = strdup(base);
	for (int i = 0; i < strlen(base); i++) {
		if (base[i] == '/') {
			base[i] = '_';
		}
	}
	static char obj_name[PATH_MAX];
	sprintf(obj_name, "%s.o", base);
	free(base);
	return obj_name;
}
// Compile the specified source file to file.o
void compile(char *file)
{
	char **args = NULL;
	add_args(&args, CC);
	for (int i = 0; CFLAGS && CFLAGS[i] != NULL; i++) {
		add_args(&args, CFLAGS[i]);
	}
	add_args(&args, "-c");
	add_args(&args, "-o");
	char *output_file = gen_obj_name(file);
	add_args(&args, output_file);
	// Check if already compiled
	char saved_source_file[PATH_MAX];
	sprintf(saved_source_file, ".%s.c", output_file);
	if (access(saved_source_file, F_OK) == 0 && access(output_file, F_OK) == 0) {
		if (!file_diff(file, saved_source_file) && !FORCE) {
			pprintf("Compile %s :skipped\n", file);
			free_args(args);
			return;
		}
	}
	copy_file(file, saved_source_file);
	// Compile
	remove(output_file);
	unlink(output_file);
	rmdir(output_file);
	add_args(&args, file);
	if (fork_exec(args) != 0) {
		error("Error: Compiler %s failed to compile %s\n", CC, file);
	}
	free_args(args);
	pprintf("Compile %s :success\n", file);
}
int pmcrts(const char *s1, const char *s2)
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
// Find files in a directory
char **find_file(char *dir, const char *end_match, char **blacklist)
{
	DIR *d = opendir(dir);
	if (!d) {
		return NULL;
	}
	struct dirent *entry;
	char **files = NULL;
	while ((entry = readdir(d)) != NULL) {
		if (entry->d_type == DT_REG && pmcrts(entry->d_name, end_match) == 0) {
			bool skip = false;
			for (int i = 0; blacklist && blacklist[i] != NULL; i++) {
				if (pmcrts(entry->d_name, blacklist[i]) == 0) {
					skip = true;
					break;
				}
			}
			if (!skip) {
				char relative_path[PATH_MAX];
				sprintf(relative_path, "%s/%s", dir, entry->d_name);
				char *absolute_path = realpath(relative_path, NULL);
				if (!absolute_path) {
					perror("Error resolving absolute path");
					continue;
				}
				add_args(&files, absolute_path);
				free(absolute_path);
			}
		}
	}
	closedir(d);
	return files;
}
// Compile source files in parallel
void compile_files_parallel(char **files, int max_processes)
{
	if (!files)
		return;
	int file_count = 0;
	while (files[file_count] != NULL)
		file_count++;
	if (file_count == 0)
		return;
	int active_processes = 0;
	int completed = 0;
	int current_file = 0;
	while (completed < file_count) {
		// Start new processes up to the limit
		while (active_processes < max_processes && current_file < file_count) {
			pid_t pid = fork();
			if (pid == 0) {
				// Child process - compile one file
				compile(files[current_file]);
				exit(EXIT_SUCCESS);
			} else if (pid > 0) {
				// Parent process
				active_processes++;
				current_file++;
			} else {
				error("Error: failed to fork process for %s\n", files[current_file]);
			}
		}
		// Wait for at least one child to complete
		if (active_processes > 0) {
			int status;
			pid_t finished_pid = wait(&status);
			if (finished_pid > 0) {
				active_processes--;
				completed++;
				if (WEXITSTATUS(status) != 0) {
					error("Error: compilation failed in child process\n");
				}
			}
		}
	}
	// Wait for any remaining processes
	while (active_processes > 0) {
		int status;
		wait(&status);
		active_processes--;
		if (WEXITSTATUS(status) != 0) {
			error("Error: compilation failed in child process\n");
		}
	}
	// Add compiled object files to OBJS
	for (int i = 0; i < file_count; i++) {
		char *obj_file = gen_obj_name(files[i]);
		add_args(&OBJS, obj_file);
	}
}
// Build
void build()
{
	// compile src/*.c and src/easteregg/*.c
	char **files = find_file(SRC_DIR, ".c", NULL);
	char ruri_src[PATH_MAX];
	sprintf(ruri_src, "%s/ruri/src", SRC_DIR);
	char **ruri_files = find_file(ruri_src, ".c", (char *[]){ "main.c", NULL });
	for (int i = 0; ruri_files && ruri_files[i] != NULL; i++) {
		add_args(&files, ruri_files[i]);
	}
	char easteregg_src[PATH_MAX];
	sprintf(easteregg_src, "%s/ruri/src/easteregg", SRC_DIR);
	char **easteregg_files = find_file(easteregg_src, ".c", NULL);
	for (int i = 0; easteregg_files && easteregg_files[i] != NULL; i++) {
		add_args(&files, easteregg_files[i]);
	}
	char catsh_src[PATH_MAX];
	sprintf(catsh_src, "%s/catsh/src", SRC_DIR);
	char **catsh_files = find_file(catsh_src, ".c", NULL);
	for (int i = 0; catsh_files && catsh_files[i] != NULL; i++) {
		add_args(&files, catsh_files[i]);
	}
	char easteregg_src_rma[PATH_MAX];
	sprintf(easteregg_src_rma, "%s/easteregg", SRC_DIR);
	char **easteregg_rma_files = find_file(easteregg_src_rma, ".c", NULL);
	for (int i = 0; easteregg_rma_files && easteregg_rma_files[i] != NULL; i++) {
		add_args(&files, easteregg_rma_files[i]);
	}
	char ssfx_src[PATH_MAX];
	sprintf(ssfx_src, "%s/ssfx", SRC_DIR);
	char **ssfx_files = find_file(ssfx_src, ".c", (char *[]){ "main.c", NULL });
	for (int i = 0; ssfx_files && ssfx_files[i] != NULL; i++) {
		add_args(&files, ssfx_files[i]);
	}
	compile_files_parallel(files, JOBS);
	free_args(easteregg_files);
	free_args(easteregg_rma_files);
	free_args(catsh_files);
	free_args(files);
	// Link
	char **args = NULL;
	add_args(&args, CC);
	for (int i = 0; CFLAGS && CFLAGS[i] != NULL; i++) {
		add_args(&args, CFLAGS[i]);
	}
	add_args(&args, "-o");
	add_args(&args, OUTPUT);
	for (int i = 0; OBJS && OBJS[i] != NULL; i++) {
		add_args(&args, OBJS[i]);
	}
	for (int i = 0; LIBS && LIBS[i] != NULL; i++) {
		add_args(&args, LIBS[i]);
	}
	if (fork_exec(args) != 0) {
		error("Error: failed to link object files\n");
	}
	pprintf("Link %s :success\n", OUTPUT);
	free_args(args);
	// Strip
	if (STRIP) {
		if (fork_exec((char *[]){ STRIP, OUTPUT, NULL }) != 0) {
			error("Error: failed to strip object files\n");
		} else {
			pprintf("Strip %s :success\n", OUTPUT);
		}
	} else {
		pprintf("Skipping strip: STRIP not set\n");
	}
	pprintf("Output: %s\n", OUTPUT);
}
// As the name said
void check_and_add_cflag(char *flag, bool panic)
{
	if (check_c_flag(flag)) {
		add_args(&CFLAGS, flag);
	} else if (panic) {
		error("Error: C flag %s is not supported\n", flag);
	}
}
// As the name said
void check_and_add_lib(char *lib, bool panic)
{
	if (check_lib(lib)) {
		add_args(&LIBS, lib);
	} else if (panic) {
		error("Error: Library %s is not supported\n", lib);
	}
}
// Default cflags
void default_cflags(void)
{
	check_and_add_cflag("-ftrivial-auto-var-init=pattern", false);
	check_and_add_cflag("-fcf-protection=full", false);
	check_and_add_cflag("-flto=auto", false);
	check_and_add_cflag("-fPIE", false);
	check_and_add_cflag("-pie", false);
	check_and_add_cflag("-Wl,-z,relro", false);
	check_and_add_cflag("-Wl,-z,noexecstack", false);
	check_and_add_cflag("-Wl,-z,now", false);
	check_and_add_cflag("-fstack-protector-all", false);
	check_and_add_cflag("-fstack-clash-protection", false);
	check_and_add_cflag("-mshstk", false);
	check_and_add_cflag("-Wno-unused-result", false);
	check_and_add_cflag("-O2", false);
	check_and_add_cflag("-Wl,--build-id=sha1", false);
	check_and_add_cflag("-ffunction-sections", false);
	check_and_add_cflag("-fdata-sections", false);
	check_and_add_cflag("-Wl,--gc-sections", false);
	check_and_add_cflag("-Wl,--strip-all", false);
	check_and_add_cflag("-U_FORTIFY_SOURCE", false);
	check_and_add_cflag("-D_FORTIFY_SOURCE=3", false);
	check_and_add_cflag("-fvisibility=hidden", false);
	check_and_add_cflag("-fsanitize=cfi", false);
	check_and_add_cflag("-fsanitize=safe-stack", false);
}
// Dev cflags
void dev_cflags(void)
{
	check_and_add_cflag("-g", true);
	check_and_add_cflag("-O0", false);
	check_and_add_cflag("-fno-omit-frame-pointer", false);
	check_and_add_cflag("-Wl,-z,norelro", false);
	check_and_add_cflag("-Wl,-z,execstack", false);
	check_and_add_cflag("-fno-stack-protector", false);
	check_and_add_cflag("-Wall", false);
	check_and_add_cflag("-Wextra", false);
	check_and_add_cflag("-pedantic", false);
	check_and_add_cflag("-Wconversion", false);
	check_and_add_cflag("-Wno-newline-eof", false);
	check_and_add_cflag("-Wno-gnu-zero-variadic-macro-arguments", false);
	check_and_add_cflag("-fsanitize=address,undefined", false);
	check_and_add_cflag("-Wl,--build-id=sha1", false);
	check_and_add_cflag("-ffunction-sections", false);
	check_and_add_cflag("-fdata-sections", false);
	check_and_add_cflag("-Wl,--gc-sections", false);
	check_and_add_cflag("-DRURI_DEBUG", false);
	check_and_add_cflag("-DRURIMA_DEBUG", false);
	check_and_add_cflag("-DRURI_DEV", false);
	check_and_add_cflag("-DRURIMA_DEV", false);
	STRIP = "true";
}
void show_help(void)
{
	char *self_path = realpath("/proc/self/exe", NULL);
	pprintf("Usage: ./%s [OPTION]...\n", basename_of(self_path));
	pprintf("    -h, --help             show help\n");
	pprintf("    -s, --static           compile static binary\n");
	pprintf("    -d, --dev              compile dev version\n");
	pprintf("    -f, --force            force rebuild\n");
	pprintf("    -j, --jobs <num>       number of jobs to run simultaneously\n");
}
// So good brooooo, the program works with magic here.
int main(int argc, char **argv)
{
	signal(SIGINT, on_exit__);
	switch_to_build_dir("out");
	init_env();
	bool cflags_configured = false;
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "--dev") == 0 || strcmp(argv[i], "-d") == 0) {
			dev_cflags();
			cflags_configured = true;
		} else if (strcmp(argv[i], "--force") == 0 || strcmp(argv[i], "-f") == 0) {
			FORCE = true;
		} else if ((strcmp(argv[i], "--jobs") == 0 || strcmp(argv[i], "-j") == 0) && i + 1 < argc) {
			JOBS = atoi(argv[++i]);
			if (JOBS <= 0) {
				error("Error: Invalid number of jobs: %s", argv[i]);
			}
		} else if (strcmp(argv[i], "--static") == 0 || strcmp(argv[i], "-s") == 0) {
			check_and_add_cflag("-static-pie", false);
			if (!check_c_flag("-static-pie")) {
				check_and_add_cflag("-static", true);
			}
		} else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
			show_help();
			exit(0);
		} else if (strcmp(argv[i], "--official-release") == 0 || strcmp(argv[i], "-r") == 0) {
			check_and_add_cflag("-DRURIMA_OFFICIAL_BUILD", false);
		} else {
			show_help();
			error("Error: Unknown option: %s", argv[i]);
		}
	}
	if (!cflags_configured) {
		default_cflags();
	}
	check_and_add_cflag("-std=gnu11", true);
	if (COMMIT_ID) {
		char define_commit[PATH_MAX];
		sprintf(define_commit, "-DRURIMA_COMMIT_ID=\"%s\"", COMMIT_ID);
		check_and_add_cflag(define_commit, false);
	} else {
		check_and_add_cflag("-DRURIMA_COMMIT_ID=unknown", false);
	}
	if (RURI_COMMIT_ID) {
		char define_ruri_commit[PATH_MAX];
		sprintf(define_ruri_commit, "-DRURI_COMMIT_ID=\"%s\"", RURI_COMMIT_ID);
		check_and_add_cflag(define_ruri_commit, false);
	} else {
		check_and_add_cflag("-DRURI_COMMIT_ID=unknown", false);
	}
	check_and_add_lib("-lcap", true);
	check_and_add_lib("-lseccomp", true);
	check_and_add_lib("-lpthread", false);
	build();
	remove_test_dot_c();
	return 0;
}