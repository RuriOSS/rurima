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
 * OUT of OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 *
 */
#include "../include/rurima.h"
void rurima_pull(int argc, char **_Nonnull argv)
{
	if (argc == 0) {
		rurima_error("{red}Unknown argument!\n");
	}
	char *mirror = NULL;
	char *image = NULL;
	char *version = NULL;
	char *architecture = NULL;
	char *savedir = NULL;
	char *start_at = NULL;
	bool docker_only = false;
	bool fallback = false;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
			cprintf("{base}Usage: rurima pull <options> [image]:[version] [savedir]\n");
			cprintf("{base}Options:\n");
			cprintf("{base}  -h, --help: Show help message.\n");
			cprintf("{base}  -m, --mirror: Mirror URL.\n");
			cprintf("{base}  -a, --arch: Architecture.\n");
			cprintf("{base}  -d, --docker: Only search dockerhub for image.\n");
			cprintf("{base}  -f, --fallback: Fallback mode, only for docker image.\n");
			cprintf("{base}  -S, --start-at: Start at specific layer.\n");
			cprintf("{base}Note: please remove `https://` prefix from mirror url.\n");
			cprintf("{base}This is just a wrap of docker and lxc subcommand.\n");
			cprintf("{base}It will re-exec itself to call the subcommand.\n");
			cprintf("{base}If -d option is not set, it will find lxc mirror first.\n");
			return;
		} else if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mirror") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No mirror specified!\n");
			}
			mirror = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--arch") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No architecture specified!\n");
			}
			architecture = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-S") == 0 || strcmp(argv[i], "--start-at") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No start at layer specified!\n");
			}
			start_at = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-d") == 0 || strcmp(argv[i], "--docker") == 0) {
			docker_only = true;
		} else if (strcmp(argv[i], "-f") == 0 || strcmp(argv[i], "--fallback") == 0) {
			fallback = true;
		} else {
			if (strstr(argv[i], ":") != NULL) {
				image = strtok(argv[i], ":");
				version = strtok(NULL, ":");
			} else {
				image = argv[i];
				version = "latest";
			}
			if (i + 1 < argc) {
				savedir = argv[i + 1];
			} else {
				rurima_error("{red}No save directory specified!\n");
			}
			char **rexec_argv = malloc(sizeof(char *) * 114);
			rexec_argv[0] = NULL;
			if (!docker_only && rurima_lxc_have_image(mirror, image, version, architecture, NULL)) {
				if (mirror == NULL) {
					mirror = rurima_global_config.lxc_mirror;
				}
				if (architecture == NULL) {
					architecture = rurima_lxc_get_host_arch();
				}
				rurima_add_argv(&rexec_argv, "lxc");
				rurima_add_argv(&rexec_argv, "pull");
				rurima_add_argv(&rexec_argv, "-m");
				rurima_add_argv(&rexec_argv, (char *)mirror);
				rurima_add_argv(&rexec_argv, "-o");
				rurima_add_argv(&rexec_argv, (char *)image);
				rurima_add_argv(&rexec_argv, "-v");
				rurima_add_argv(&rexec_argv, (char *)version);
				rurima_add_argv(&rexec_argv, "-a");
				rurima_add_argv(&rexec_argv, (char *)architecture);
				rurima_add_argv(&rexec_argv, "-s");
				rurima_add_argv(&rexec_argv, (char *)savedir);
				int exit_status = rurima_fork_rexec(rexec_argv);
				exit(exit_status);
			} else {
				if (mirror == NULL) {
					mirror = rurima_global_config.docker_mirror;
				}
				if (architecture == NULL) {
					architecture = rurima_docker_get_host_arch();
				}
				rurima_add_argv(&rexec_argv, "docker");
				rurima_add_argv(&rexec_argv, "pull");
				if (fallback) {
					rurima_add_argv(&rexec_argv, "-f");
				}
				rurima_add_argv(&rexec_argv, "-i");
				rurima_add_argv(&rexec_argv, (char *)image);
				rurima_add_argv(&rexec_argv, "-t");
				rurima_add_argv(&rexec_argv, (char *)version);
				rurima_add_argv(&rexec_argv, "-a");
				rurima_add_argv(&rexec_argv, (char *)architecture);
				rurima_add_argv(&rexec_argv, "-s");
				rurima_add_argv(&rexec_argv, (char *)savedir);
				rurima_add_argv(&rexec_argv, "-m");
				rurima_add_argv(&rexec_argv, (char *)mirror);
				if (start_at != NULL) {
					rurima_add_argv(&rexec_argv, "-S");
					rurima_add_argv(&rexec_argv, (char *)start_at);
				}
				int exit_status = rurima_fork_rexec(rexec_argv);
				exit(exit_status);
			}
		}
	}
	rurima_error("Emmmm, I think it will never reach here.\n");
}
