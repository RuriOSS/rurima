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
void rurima_load_rootfs(int argc, char **argv)
{
	if (argc < 2 || strcmp(argv[0], "help") == 0 || strcmp(argv[0], "-h") == 0 || strcmp(argv[0], "--help") == 0) {
		cprintf("{base}Usage: rurima load <docker_image.tar> <rootfs_path>\n");
		cprintf("{base}Load rootfs from docker image tar to specified path.\n");
		return;
	}
	char *pack_path = argv[0];
	char *rootfs_path = argv[1];
	rurima_check_dir_deny_list(rootfs_path);
	if (access(pack_path, F_OK) != 0) {
		rurima_error("{red}Docker image tar file %s not found!\n", pack_path);
	}
	char tmp_dir[PATH_MAX];
	sprintf(tmp_dir, "%s/.rurima", rootfs_path);
	if (rurima_mkdirs(tmp_dir, 0755) != 0) {
		rurima_error("{red}Failed to create rootfs directory!\n");
	}
	rurima_extract_archive(pack_path, tmp_dir);
	// Read manifest.json
	char manifest_path[PATH_MAX];
	sprintf(manifest_path, "%s/manifest.json", tmp_dir);
	struct stat st;
	if (stat(manifest_path, &st) != 0) {
		rurima_error("{red}Failed to stat manifest.json!\n");
	}
	char *manifest_content = malloc(st.st_size + 1);
	FILE *manifest_file = fopen(manifest_path, "r");
	if (!manifest_file) {
		rurima_error("{red}Failed to open manifest.json!\n");
	}
	fread(manifest_content, 1, st.st_size, manifest_file);
	manifest_content[st.st_size] = 0;
	fclose(manifest_file);
	char *config_file = rurima_call_jq((char *[]){ "jq", "-r", ".[].Config", NULL }, manifest_content);
	if (!config_file) {
		rurima_error("{red}Failed to parse manifest.json!\n");
	}
	char **configs = NULL;
	size_t config_len = rurima_split_lines(config_file, &configs);
	if (config_len != 1) {
		rurima_error("{red}No config file or multiple config files found in manifest.json!\n");
	}
	// Extract layers
	char *layers_file_orig = rurima_call_jq((char *[]){ "jq", "-r", ".[] | .Layers | .[]", NULL }, manifest_content);
	if (!layers_file_orig) {
		rurima_error("{red}Failed to parse manifest.json!\n");
	}
	char **layers = NULL;
	size_t layers_len = rurima_split_lines(layers_file_orig, &layers);
	if (layers_len == 0) {
		rurima_error("{red}No layers found in manifest.json!\n");
	}
	for (size_t i = 0; i < layers_len; i++) {
		char layer_path[PATH_MAX];
		sprintf(layer_path, "%s/%s", tmp_dir, layers[i]);
		rurima_extract_archive(layer_path, rootfs_path);
	}
	// Parse configs.
	char config_file_path[PATH_MAX];
	char *config_content = NULL;
	sprintf(config_file_path, "%s/%s", tmp_dir, configs[0]);
	FILE *config_file_fp = fopen(config_file_path, "r");
	if (!config_file_fp) {
		rurima_error("{red}Failed to open config file %s!\n", config_file_path);
	}
	struct stat config_st;
	if (stat(config_file_path, &config_st) != 0) {
		rurima_error("{red}Failed to stat config file %s!\n", config_file_path);
	}
	config_content = malloc(config_st.st_size + 1);
	fread(config_content, 1, config_st.st_size, config_file_fp);
	config_content[config_st.st_size] = 0;
	fclose(config_file_fp);
	// Print config.
	rurima_docker_print_config_from_json(config_content, rootfs_path);
	free(manifest_content);
	free(config_file);
	free(layers_file_orig);
	for (size_t i = 0; i < layers_len; i++) {
		free(layers[i]);
	}
	free(layers);
	for (size_t i = 0; i < config_len; i++) {
		free(configs[i]);
	}
	free(configs);
	free(config_content);
	rurima_check_dir_deny_list(tmp_dir);
	cth_exec_command((char *[]){ "rm", "-rf", tmp_dir, NULL });
	exit(0);
}
