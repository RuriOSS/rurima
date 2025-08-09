
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
static size_t get_max_pipe_size(void)
{
	int fd = open("/proc/sys/fs/pipe-max-size", O_RDONLY);
	if (fd < 0) {
		perror("open /proc/sys/fs/pipe-max-size");
		return 65536; // Default pipe size
	}
	char buffer[16];
	ssize_t bytes_read = read(fd, buffer, sizeof(buffer) - 1);
	if (bytes_read < 0) {
		perror("read /proc/sys/fs/pipe-max-size");
		close(fd);
		return 65536; // Default pipe size
	}
	buffer[bytes_read] = '\0'; // Null-terminate the string
	size_t max_size = strtoul(buffer, NULL, 10);
	close(fd);
	if (max_size == 0) {
		return 65536; // Default pipe size
	}
	rurima_log("{base}Maximum pipe size: {green}%zu{base} bytes\n", max_size);
	return max_size;
}
int rurima_fork_execvp(const char *_Nonnull argv[])
{
	/*
	 * fork(2) and then execvp(3).
	 * Return the exit status of the child process.
	 */
	int pid = fork();
	if (pid == 0) {
		execvp(argv[0], (char **)argv);
		// If execvp(3) failed, exit as error status 114.
		exit(114);
	}
	int status = 0;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}
char *rurima_fork_execvp_get_stdout_ignore_err(const char *_Nonnull argv[])
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
		perror("pipe");
		return NULL;
	}
	// fork(2) and then execvp(3).
	int pid = fork();
	if (pid == -1) {
		perror("fork");
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
		exit(114);
	} else {
		// Close the write end of the pipe.
		close(pipefd[1]);
		// Get the output from the read end of the pipe.
		size_t buffer_size = 1024;
		size_t total_read = 0;
		char *output = malloc(buffer_size);
		output[0] = '\0';
		ssize_t bytes_read;
		while ((bytes_read = read(pipefd[0], output + total_read, buffer_size - total_read - 1)) > 0) {
			total_read += (size_t)bytes_read;
			if (total_read >= buffer_size - 1) {
				buffer_size *= 2;
				char *new_output = realloc(output, buffer_size);
				output = new_output;
			}
		}
		if (bytes_read == -1) {
			perror("read");
			free(output);
			close(pipefd[0]);
			return NULL;
		}
		output[total_read] = '\0';
		close(pipefd[0]);
		int status = 0;
		waitpid(pid, &status, 0);
		rurima_log("{base}Exec {green}%s{base} result: {purple}\n%s\n", argv[0], output);
		return output;
	}
	return NULL;
}
char *rurima_fork_execvp_get_stdout(const char *_Nonnull argv[])
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
		perror("pipe");
		return NULL;
	}
	// fork(2) and then execvp(3).
	int pid = fork();
	if (pid == -1) {
		perror("fork");
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
		exit(114);
	} else {
		// Close the write end of the pipe.
		close(pipefd[1]);
		// Get the output from the read end of the pipe.
		size_t buffer_size = 1024;
		size_t total_read = 0;
		char *output = malloc(buffer_size);
		ssize_t bytes_read;
		while ((bytes_read = read(pipefd[0], output + total_read, buffer_size - total_read - 1)) > 0) {
			total_read += (size_t)bytes_read;
			if (total_read >= buffer_size - 1) {
				buffer_size *= 2;
				char *new_output = realloc(output, buffer_size);
				output = new_output;
			}
		}
		if (bytes_read == -1) {
			perror("read");
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
		rurima_log("{base}Exec {green}%s{base} result: {purple}\n%s\n", argv[0], output);
		return output;
	}
	return NULL;
}
void rurima_add_argv(char ***_Nonnull argv, char *_Nonnull arg)
{
	/*
	 * Add an argument to the argv array.
	 * Warning: make sure length of argv is enough.
	 */
	int argc = 0;
	while ((*argv)[argc] != NULL) {
		argc++;
	}
	(*argv)[argc] = arg;
	(*argv)[argc + 1] = NULL;
}
int rurima_fork_rexec(char **_Nonnull argv)
{
	/*
	 * Fork and execv self with argv.
	 */
	pid_t pid = fork();
	if (pid == -1) {
		rurima_error("{red}Fork failed!\n");
	}
	if (pid == 0) {
		size_t argc = 0;
		while (argv[argc] != NULL) {
			argc++;
		}
		char **new_argv = malloc(sizeof(char *) * (argc + 2));
		new_argv[0] = "/proc/self/exe";
		for (size_t i = 0; i < argc; i++) {
			rurima_log("{base}Argv[%zu]: %s\n", i, argv[i]);
			new_argv[i + 1] = argv[i];
		}
		new_argv[argc + 1] = NULL;
		execv(new_argv[0], new_argv);
		rurima_error("{red}Execv() failed!\n");
		free(new_argv);
		exit(1);
	}
	int status;
	waitpid(pid, &status, 0);
	return WEXITSTATUS(status);
}
char *rurima_fork_execvp_get_stdout_with_input(const char *_Nonnull argv[], const char *_Nonnull input)
{
	/*
	 * Warning: free() after use.
	 * We will fork(2) and then execvp(3).
	 * And then we will get the stdout of the child process.
	 * Return the stdout of the child process.
	 * If failed, return NULL.
	 */
	// Create a pipe.
	int pipefd_in[2];
	if (pipe(pipefd_in) == -1) {
		perror("pipe");
		return NULL;
	}
	// Set the maximum pipe size.
	size_t max_pipe_size = get_max_pipe_size();
	fcntl(pipefd_in[1], F_SETPIPE_SZ, max_pipe_size);
	int pipefd_out[2];
	if (pipe(pipefd_out) == -1) {
		perror("pipe");
		return NULL;
	}
	// fork(2) and then execvp(3).
	int pid = fork();
	if (pid == -1) {
		perror("fork");
		return NULL;
	}
	if (pid == 0) {
		// Close the read end of the pipe.
		close(pipefd_out[0]);
		// Redirect stdout and stderr to the write end of the pipe.
		dup2(pipefd_out[1], STDOUT_FILENO);
		int nullfd = open("/dev/null", O_WRONLY);
		dup2(nullfd, STDERR_FILENO);
		close(pipefd_out[1]);
		// Close the write end of the input pipe.
		close(pipefd_in[1]);
		// Redirect stdin to the read end of the input pipe.
		// This will allow the child process to read from the input pipe.
		dup2(pipefd_in[0], STDIN_FILENO);
		// Close the read end of the input pipe.
		close(pipefd_in[0]);
		rurima_log("{base}Exec {green}%s{base}\n", argv[0]);
		execvp(argv[0], (char **)argv);
		exit(114);
	} else {
		usleep(100); // Give the child process some time to set up.
		// Close the write end of the pipe.
		close(pipefd_out[1]);
		// Close the read end of the input pipe.
		close(pipefd_in[0]);
		// Write the input to the write end of the input pipe.
		if (write(pipefd_in[1], input, strlen(input)) == -1) {
			perror("write");
			close(pipefd_in[1]);
			close(pipefd_out[0]);
			return NULL;
		}
		close(pipefd_in[1]);
		rurima_log("{base}Input written to {green}%s{base}\n", argv[0]);
		// Get the output from the read end of the pipe.
		size_t buffer_size = 1024;
		size_t total_read = 0;
		char *output = malloc(buffer_size);
		ssize_t bytes_read;
		while ((bytes_read = read(pipefd_out[0], output + total_read, buffer_size - total_read - 1)) > 0) {
			total_read += (size_t)bytes_read;
			if (total_read >= buffer_size - 1) {
				buffer_size *= 2;
				char *new_output = realloc(output, buffer_size);
				output = new_output;
			}
		}
		if (bytes_read == -1) {
			perror("read");
			free(output);
			close(pipefd_out[0]);
			return NULL;
		}
		output[total_read] = '\0';
		close(pipefd_out[0]);
		int status = 0;
		waitpid(pid, &status, 0);
		if (WEXITSTATUS(status) != 0) {
			free(output);
			return NULL;
		}
		rurima_log("{base}Exec {green}%s{base} result: {purple}\n%s\n", argv[0], output);
		return output;
	}
	return NULL;
}
char *rurima_call_jq(const char *_Nonnull argv[], const char *_Nonnull input)
{
	/*
	 * Call jq with input.
	 * Warning: free() after use.
	 */
	rurima_log("{base}Calling jq with argv: \n");
	for (int i = 0; argv[i] != NULL; i++) {
		rurima_log("{cyan}argv[%d]: %s\n", i, argv[i]);
	}
	char *output = rurima_fork_execvp_get_stdout_with_input(argv, input);
	if (output == NULL || strcmp(output, "null") == 0) {
		free(output);
		return NULL;
	}
	return output;
}