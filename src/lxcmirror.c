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
/*
 * /meta/1.0/index-system format:
 *
 * os;version;arch;type;time;dir
 * os;version;arch;type;time;dir
 * ........
 */
static char *line_get_value(const char *_Nonnull line, int index)
{
	/*
	 * Get the value of the index in the line.
	 * Warning: free() the return value after use.
	 *
	 * Example:
	 * line = "a;b;c;d;e;f"
	 * line_get_value(line, 2) will return "c"
	 *
	 */
	char *tmp = strdup(line);
	char *p = tmp;
	// Skip index number of ';'.
	for (int i = 0; i < index; i++) {
		p = strchr(p, ';');
		if (p == NULL) {
			free(tmp);
			return NULL;
		}
		p++;
	}
	if (strchr(p, ';') == NULL) {
		if (strchr(p, '\n') != NULL) {
			*strchr(p, '\n') = '\0';
		}
	} else {
		*strchr(p, ';') = '\0';
	}
	char *ret = strdup(p);
	free(tmp);
	return ret;
}
static const char *goto_next_line(const char *_Nonnull buf)
{
	/*
	 * Return the pointer to the next line.
	 * If no next line, return NULL.
	 */
	const char *p = strchr(buf, '\n');
	if (p == NULL) {
		return NULL;
	}
	return p + 1;
}
static char *get_current_line(const char *_Nonnull buf)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get the current line.
	 * If no line, return NULL.
	 *
	 */
	char *tmp = strdup(buf);
	char *p = strchr(tmp, '\n');
	if (p == NULL) {
		free(tmp);
		return NULL;
	}
	*p = '\0';
	char *ret = strdup(tmp);
	free(tmp);
	return ret;
}
static char *get_lxc_index(const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get lxc index info from mirror.
	 *
	 * Return: The index info.
	 *
	 */
	char url[4096];
	if (mirror == NULL) {
		sprintf(url, "https://%s/meta/1.0/index-system", rurima_global_config.lxc_mirror);
	} else {
		sprintf(url, "https://%s/meta/1.0/index-system", mirror);
	}
	char *command[] = { "curl", "-L", "-s", url, NULL };
	return rurima_fork_execvp_get_stdout(command);
}
static char *lxc_get_image_dir(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nonnull version, const char *_Nullable architecture, const char *_Nullable type)
{
	/*
	 * Get the image dir from mirror.
	 *
	 * Return: The image dir.
	 * If not found, return NULL.
	 */
	char *buf = get_lxc_index(mirror);
	const char *p = buf;
	if (p == NULL) {
		free(buf);
		return NULL;
	}
	if (architecture == NULL) {
		architecture = rurima_lxc_get_host_arch();
	}
	rurima_log("arch: %s\n", architecture);
	if (type == NULL) {
		type = "default";
	}
	char *str_to_find = malloc(strlen(os) + strlen(version) + strlen(architecture) + strlen(type) + 10);
	sprintf(str_to_find, "%s;%s;%s;%s", os, version, architecture, type);
	rurima_log("str to find: %s\n", str_to_find);
	if (strstr(p, str_to_find) == NULL) {
		free(str_to_find);
		free(buf);
		return NULL;
	}
	p = strstr(p, str_to_find);
	p = strchr(p, '/');
	char *tmp = strdup(p);
	for (size_t i = 0; i < strlen(tmp); i++) {
		if (tmp[i] == '\n') {
			tmp[i] = '\0';
			break;
		}
	}
	char *ret = strdup(tmp);
	free(str_to_find);
	free(tmp);
	free(buf);
	return ret;
}
void rurima_lxc_get_image_list(const char *_Nullable mirror, const char *_Nullable architecture)
{
	/*
	 * Get the image list from mirror,
	 * and show them.
	 */
	if (architecture == NULL) {
		architecture = rurima_lxc_get_host_arch();
	}
	if (mirror == NULL) {
		mirror = rurima_global_config.lxc_mirror;
	}
	char *buf = get_lxc_index(mirror);
	const char *p = buf;
	char *line = NULL;
	if (p == NULL) {
		free(buf);
		rurima_error("{red}Failed to get index.\n");
	}
	bool found = false;
	while ((line = get_current_line(p)) != NULL) {
		char *os = line_get_value(line, 0);
		char *version = line_get_value(line, 1);
		char *arch = line_get_value(line, 2);
		char *type = line_get_value(line, 3);
		if (strcmp(arch, architecture) != 0) {
			free(os);
			free(version);
			free(arch);
			free(type);
			free(line);
			p = goto_next_line(p);
			continue;
		}
		found = true;
		if (!rurima_global_config.quiet) {
			cprintf("{yellow}%-13s {green}: {purple}%-10s {green}: {cyan}%-8s\n", os, version, type);
		} else {
			printf("%-13s : %-10s : %-8s\n", os, version, type);
		}
		free(os);
		free(version);
		free(arch);
		free(type);
		free(line);
		p = goto_next_line(p);
	}
	if (!found) {
		rurima_error("{red}No image found.\n");
	}
	free(buf);
}
void rurima_lxc_search_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nullable architecture)
{
	/*
	 * Search the image from mirror,
	 * and show them.
	 */
	if (architecture == NULL) {
		architecture = rurima_lxc_get_host_arch();
	}
	if (mirror == NULL) {
		mirror = rurima_global_config.lxc_mirror;
	}
	char *buf = get_lxc_index(mirror);
	const char *p = buf;
	char *line = NULL;
	if (p == NULL) {
		free(buf);
		rurima_error("{red}Failed to get index.\n");
	}
	bool found = false;
	while ((line = get_current_line(p)) != NULL) {
		char *os_cur = line_get_value(line, 0);
		char *version = line_get_value(line, 1);
		char *arch = line_get_value(line, 2);
		char *type = line_get_value(line, 3);
		if (strcmp(arch, architecture) != 0 || strcmp(os_cur, os) != 0) {
			free(os_cur);
			free(version);
			free(arch);
			free(type);
			free(line);
			p = goto_next_line(p);
			continue;
		}
		found = true;
		if (!rurima_global_config.quiet) {
			cprintf("{yellow}%-13s {green}: {purple}%-10s {green}: {cyan}%-8s\n", os, version, type);
		} else {
			printf("%-13s : %-10s : %-8s\n", os, version, type);
		}
		free(os_cur);
		free(version);
		free(arch);
		free(type);
		free(line);
		p = goto_next_line(p);
	}
	free(buf);
	if (!found) {
		rurima_error("{red}No image found.\n");
	}
}
bool rurima_lxc_have_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nonnull version, const char *_Nullable architecture, const char *_Nullable type)
{
	/*
	 * Check if the image is available in mirror.
	 *
	 * Return: true if available, false if not.
	 */
	start_loading_animation("Fetching metadata...");
	if (architecture == NULL) {
		architecture = rurima_lxc_get_host_arch();
	}
	if (mirror == NULL) {
		mirror = rurima_global_config.lxc_mirror;
	}
	if (type == NULL) {
		type = "default";
	}
	if (architecture == NULL) {
		architecture = rurima_lxc_get_host_arch();
	}
	char *dir = lxc_get_image_dir(mirror, os, version, architecture, type);
	end_loading_animation();
	if (dir == NULL) {
		return false;
	}
	free(dir);
	return true;
}
void rurima_lxc_pull_image(const char *_Nullable mirror, const char *_Nonnull os, const char *_Nonnull version, const char *_Nullable architecture, const char *_Nullable type, const char *_Nonnull savedir)
{
	/*
	 * Pull the rootfs.tar.xz from mirror,
	 * and extract it to savedir.
	 */
	rurima_check_dir_deny_list(savedir);
	if (architecture == NULL) {
		architecture = rurima_lxc_get_host_arch();
	}
	start_loading_animation("Fetching metadata...");
	char *dir = lxc_get_image_dir(mirror, os, version, architecture, type);
	end_loading_animation();
	if (dir == NULL) {
		rurima_error("{red}Image not found.\n");
	}
	if (rurima_mkdirs(savedir, 0755) == -1) {
		rurima_error("{red}Failed to create directory.\n");
	}
	chdir(savedir);
	if (mirror == NULL) {
		mirror = rurima_global_config.lxc_mirror;
	}
	char *url = malloc(strlen(mirror) + strlen(dir) + 114);
	sprintf(url, "https://%s/%srootfs.tar.xz", mirror, dir);
	cprintf("{base}Pulling {cyan}rootfs.tar.xz\n");
	rurima_download_file(url, "rootfs.tar.xz", NULL, -1);
	rurima_extract_archive("rootfs.tar.xz", ".");
	remove("rootfs.tar.xz");
	free(url);
	free(dir);
}
void rurima_lxc_search_arch(const char *_Nullable mirror, const char *_Nonnull os)
{
	/*
	 * Search the architecture of the image from mirror,
	 * and show them.
	 */
	if (mirror == NULL) {
		mirror = rurima_global_config.lxc_mirror;
	}
	char *buf = get_lxc_index(mirror);
	const char *p = buf;
	char *line = NULL;
	if (p == NULL) {
		free(buf);
		rurima_error("{red}Failed to get index.\n");
	}
	bool found = false;
	while ((line = get_current_line(p)) != NULL) {
		char *os_cur = line_get_value(line, 0);
		char *version = line_get_value(line, 1);
		char *arch = line_get_value(line, 2);
		char *type = line_get_value(line, 3);
		if (strcmp(os_cur, os) != 0) {
			free(os_cur);
			free(version);
			free(arch);
			free(type);
			free(line);
			p = goto_next_line(p);
			continue;
		}
		found = true;
		if (!rurima_global_config.quiet) {
			cprintf("{yellow}%-13s {green}: {purple}%-10s {green}: {cyan}%-8s\n", os, version, arch);
		} else {
			printf("%-13s : %-10s : %-8s\n", os, version, arch);
		}
		free(os_cur);
		free(version);
		free(arch);
		free(type);
		free(line);
		p = goto_next_line(p);
	}
	free(buf);
	if (!found) {
		rurima_error("{red}No image found.\n");
	}
}
