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
void rurima_lxc(int argc, char **_Nonnull argv)
{
	char *mirror = NULL;
	char *os = NULL;
	char *version = NULL;
	char *architecture = NULL;
	char *type = NULL;
	char *savedir = NULL;
	if (argc == 0) {
		rurima_error("{red}No subcommand specified!\n");
	}
	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-m") == 0 || strcmp(argv[i], "--mirror") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No mirror specified!\n");
			}
			mirror = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-n") == 0 || strcmp(argv[i], "--no-process") == 0 || strcmp(argv[i], "--no-progress") == 0) {
			rurima_global_config.no_progress = true;
		} else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--os") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No os specified!\n");
			}
			os = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No version specified!\n");
			}
			version = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--arch") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No architecture specified!\n");
			}
			architecture = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-t") == 0 || strcmp(argv[i], "--type") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No type specified!\n");
			}
			type = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--savedir") == 0) {
			if (i + 1 >= argc) {
				rurima_error("{red}No save directory specified!\n");
			}
			rurima_check_dir_deny_list(argv[i + 1]);
			savedir = argv[i + 1];
			i++;
		} else if (strcmp(argv[i], "-q") == 0 || strcmp(argv[i], "--quiet") == 0) {
			rurima_global_config.quiet = true;
		} else {
			rurima_error("{red}Unknown argument!\n");
		}
	}
	if (strcmp(argv[0], "pull") == 0) {
		if (os == NULL) {
			rurima_error("{red}No os specified!\n");
		}
		if (version == NULL) {
			rurima_error("{red}No version specified!\n");
		}
		if (savedir == NULL) {
			rurima_error("{red}No save directory specified!\n");
		}
		if (!rurima_run_with_root()) {
			if (!proot_exist()) {
				rurima_warning("{yellow}You are not running as root, but proot not found, might cause bug unpacking rootfs!\n");
			}
		}
		rurima_lxc_pull_image(mirror, os, version, architecture, type, savedir);
	} else if (strcmp(argv[0], "list") == 0) {
		rurima_lxc_get_image_list(mirror, architecture);
	} else if (strcmp(argv[0], "search") == 0) {
		if (os == NULL) {
			rurima_error("{red}No os specified!\n");
		}
		rurima_lxc_search_image(mirror, os, architecture);
	} else if (strcmp(argv[0], "arch") == 0) {
		if (os == NULL) {
			rurima_error("{red}No os specified!\n");
		}
		rurima_lxc_search_arch(mirror, os);
	} else if (strcmp(argv[0], "help") == 0 || strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0) {
		cprintf("{base}Usage: rurima lxc [subcommand] [options]\n");
		cprintf("{base}Subcommands:\n");
		cprintf("{base}  pull: Pull image from LXC image server.\n");
		cprintf("{base}  list: List images from LXC image server.\n");
		cprintf("{base}  search: Search images from LXC image server.\n");
		cprintf("{base}  arch: Search architecture of images from LXC image server.\n");
		cprintf("{base}  help: Show help message.\n");
		cprintf("{base}Options:\n");
		cprintf("{base}  -m, --mirror: Mirror of LXC image server.\n");
		cprintf("{base}  -o, --os: OS of image.\n");
		cprintf("{base}  -v, --version: Version of image.\n");
		cprintf("{base}  -a, --arch: Architecture of image.\n");
		cprintf("{base}  -t, --type: Type of image.\n");
		cprintf("{base}  -s, --savedir: Save directory of image.\n");
		cprintf("{base}  -n, --no-progress: Do not show progress.\n");
		cprintf("\n{base}Note: please remove `https://` prefix from mirror url.\n");
		cprintf("{base}For example: `-m images.linuxcontainers.org`\n");
	} else {
		rurima_error("{red}Invalid subcommand!\n");
	}
}
