
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
int rurima_fork_execvp(char *_Nonnull argv[])
{
	/*
	 * fork(2) and then execvp(3).
	 * Return the exit status of the child process.
	 */
	struct cth_result *result = cth_exec(argv, NULL, true, false);
	rurima_log("{base}Exec {green}%s{base} result: {purple}\n%s\n", argv[0], result->stdout_ret);
	rurima_log("{base}Exit code: {green}%d{base}\n", result->exit_code);
	int exit_code = result->exit_code;
	cth_free_result(&result);
	return exit_code;
}
char *rurima_fork_execvp_get_stdout_ignore_err(char *_Nonnull argv[])
{
	/*
	 * Warning: free() after use.
	 * We will fork(2) and then execvp(3).
	 * And then we will get the stdout of the child process.
	 * Return the stdout of the child process.
	 * If failed, return NULL.
	 */
	struct cth_result *result = cth_exec(argv, NULL, true, true);
	rurima_log("{base}Exec {green}%s{base} result: {purple}\n%s\n", argv[0], result->stdout_ret);
	rurima_log("{base}Exit code: {green}%d{base}\n", result->exit_code);
	char *output = result->stdout_ret ? strdup(result->stdout_ret) : NULL;
	cth_free_result(&result);
	return output;
}
char *rurima_fork_execvp_get_stdout(char *_Nonnull argv[])
{
	/*
	 * Warning: free() after use.
	 * We will fork(2) and then execvp(3).
	 * And then we will get the stdout of the child process.
	 * Return the stdout of the child process.
	 * If failed, return NULL.
	 */
	struct cth_result *result = cth_exec(argv, NULL, true, true);
	rurima_log("{base}Exec {green}%s{base} result: {purple}\n%s\n", argv[0], result->stdout_ret);
	rurima_log("{base}Exit code: {green}%d{base}\n", result->exit_code);
	if (result->exit_code == 0) {
		char *output = result->stdout_ret ? strdup(result->stdout_ret) : NULL;
		cth_free_result(&result);
		return output;
	}
	cth_free_result(&result);
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
char *rurima_fork_execvp_get_stdout_with_input(char *_Nonnull argv[], char *_Nonnull input)
{
	/*
	 * Warning: free() after use.
	 * We will fork(2) and then execvp(3).
	 * And then we will get the stdout of the child process.
	 * Return the stdout of the child process.
	 * If failed, return NULL.
	 */
	struct cth_result *result = cth_exec(argv, input, true, true);
	rurima_log("{base}Exec {green}%s{base} result: {purple}\n%s\n", argv[0], result->stdout_ret);
	rurima_log("{base}Exit code: {green}%d{base}\n", result->exit_code);
	if (result->exit_code == 0) {
		char *output = result->stdout_ret ? strdup(result->stdout_ret) : NULL;
		cth_free_result(&result);
		return output;
	}
	cth_free_result(&result);
	return NULL;
}
char *rurima_call_jq(char *_Nonnull argv[], char *_Nonnull input)
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
	rurima_log("{base}jq output: {cyan}%s{clear}\n", output);
	return output;
}