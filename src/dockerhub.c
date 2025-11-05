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
 * NOTE: skip_layer is actually "start at layer",
 * but to keep compatible with old version,
 * we still use skip_layer variable name.
 */
void rurima_free_docker_config(struct RURIMA_DOCKER *_Nonnull config)
{
	/*
	 * free() the docker config.
	 */
	free(config->workdir);
	for (int i = 0; config->env[i] != NULL; i++) {
		free(config->env[i]);
	}
	for (int i = 0; config->command[i] != NULL; i++) {
		free(config->command[i]);
	}
	for (int i = 0; config->entrypoint[i] != NULL; i++) {
		free(config->entrypoint[i]);
	}
	free(config->architecture);
	free(config);
}
static void print_export_env(struct RURIMA_DOCKER *_Nonnull config)
{
	/*
	 * Print export env command.
	 */
	for (int i = 0; config->env[i] != NULL && config->env[i + 1] != NULL; i += 2) {
		printf("export %s=\"%s\"\n", config->env[i], config->env[i + 1]);
	}
}
static void print_chroot_command(struct RURIMA_DOCKER *_Nonnull config, char *_Nullable savedir)
{
	/*
	 * Print command to use chroot as runtime.
	 */
	print_export_env(config);
	printf("mount --bind %s %s\n", savedir == NULL ? "/path/to/container" : savedir, savedir == NULL ? "/path/to/container" : savedir);
	printf("mount --bind /dev ");
	printf("%s/dev\n", savedir == NULL ? "/path/to/container" : savedir);
	printf("mount --bind /proc ");
	printf("%s/proc\n", savedir == NULL ? "/path/to/container" : savedir);
	printf("mount --bind /sys ");
	printf("%s/sys\n", savedir == NULL ? "/path/to/container" : savedir);
	printf("chroot ");
	printf("%s ", savedir == NULL ? "/path/to/container" : savedir);
	if (config->command[0] != NULL) {
		for (int i = 0; config->command[i] != NULL; i++) {
			printf("%s ", config->command[i]);
		}
	} else if (config->entrypoint[0] != NULL) {
		if (config->workdir != NULL) {
			printf("%s/", config->workdir);
		}
		for (int i = 0; config->entrypoint[i] != NULL; i++) {
			printf("%s ", config->entrypoint[i]);
		}
	}
	printf("\n");
}
static void print_proot_command(struct RURIMA_DOCKER *_Nonnull config, char *_Nullable savedir)
{
	/*
	 * Print command to use proot as runtime.
	 */
	print_export_env(config);
	printf("OTHER_ARGS=\"\"\n");
	printf("QEMU_PATH=\"\"\n");
	printf("proot $OTHER_ARGS -0 ");
	if (config->workdir != NULL) {
		printf("-w %s ", config->workdir);
	}
	if (strcmp(config->architecture, rurima_docker_get_host_arch()) != 0) {
		printf("-q $QEMU_PATH ");
	}
	printf("-r %s ", savedir == NULL ? "/path/to/container" : savedir);
	if (config->command[0] != NULL) {
		for (int i = 0; config->command[i] != NULL; i++) {
			printf("%s ", config->command[i]);
		}
	} else if (config->entrypoint[0] != NULL) {
		for (int i = 0; config->entrypoint[i] != NULL; i++) {
			printf("%s ", config->entrypoint[i]);
		}
	}
	printf("\n");
}
static void print_ruri_command(struct RURIMA_DOCKER *_Nonnull config, char *_Nullable savedir)
{
	/*
	 * Print command to use ruri as runtime.
	 */
	printf("rurima r ");
	printf("-w ");
	if (config->workdir != NULL) {
		printf("-W %s ", config->workdir);
	}
	if (strcmp(config->architecture, rurima_docker_get_host_arch()) != 0) {
		printf("-a %s ", config->architecture);
		printf("-q /path/to/qemu-%s-static ", config->architecture);
	}
	for (int i = 0; config->env[i] != NULL && config->env[i + 1] != NULL; i += 2) {
		printf("-e \"%s\" ", config->env[i]);
		if (strcmp(config->env[i + 1], "") != 0) {
			printf("\"%s\" ", config->env[i + 1]);
		} else {
			printf("\" \" ");
		}
	}
	printf("%s ", savedir == NULL ? "/path/to/container" : savedir);
	if (config->command[0] != NULL) {
		for (int i = 0; config->command[i] != NULL; i++) {
			printf("%s ", config->command[i]);
		}
	} else if (config->entrypoint[0] != NULL) {
		for (int i = 0; config->entrypoint[i] != NULL; i++) {
			printf("%s ", config->entrypoint[i]);
		}
	}
	printf("\n");
}
void rurima_show_docker_config(struct RURIMA_DOCKER *_Nonnull config, char *_Nullable savedir, char *_Nullable runtime, bool quiet)
{
	/*
	 * Show docker config.
	 */
	if (!quiet) {
		cprintf("{base}\nConfig:\n");
		cprintf("{base}  Workdir:\n    {cyan}%s\n", config->workdir == NULL ? "NULL" : config->workdir);
		cprintf("{base}  Env:\n");
		if (config->env[0] == NULL) {
			cprintf("{cyan}NULL\n");
		} else {
			for (int i = 0; config->env[i] != NULL; i += 2) {
				cprintf("{cyan}    %s = ", config->env[i]);
				cprintf("{cyan}%s\n", config->env[i + 1]);
			}
		}
		cprintf("{base}  Command:\n    ");
		if (config->command[0] == NULL) {
			cprintf("{cyan}NULL\n");
		} else {
			for (int i = 0; config->command[i] != NULL; i++) {
				cprintf("{cyan}%s ", config->command[i]);
			}
			cprintf("{clear}\n");
		}
		cprintf("{base}  Entrypoint:\n    ");
		if (config->entrypoint[0] == NULL) {
			cprintf("{cyan}NULL\n");
		} else {
			for (int i = 0; config->entrypoint[i] != NULL; i++) {
				cprintf("{cyan}%s ", config->entrypoint[i]);
			}
			cprintf("{clear}\n");
		}
	}
	if (runtime == NULL) {
		runtime = "ruri";
	}
	if (strcmp(runtime, "ruri") == 0) {
		if (!quiet) {
			cprintf("{base}Run with ruri:\n");
			printf("\033[38;2;219;240;240m\n");
		}
		print_ruri_command(config, savedir);
	} else if (strcmp(runtime, "proot") == 0) {
		if (!quiet) {
			cprintf("{base}Run with proot:\n");
			printf("\033[38;2;219;240;240m\n");
		}
		print_proot_command(config, savedir);
		if (!quiet) {
			rurima_warning("\n{yellow}Please replace [OTHER_ARGS] with your proot args!");
		}
	} else if (strcmp(runtime, "chroot") == 0) {
		if (!quiet) {
			cprintf("{base}Run with chroot:\n");
			printf("\033[38;2;219;240;240m\n");
		}
		print_chroot_command(config, savedir);
		if (!quiet) {
			if (strcmp(config->architecture, rurima_docker_get_host_arch()) != 0) {
				rurima_warning("{yellow}For chroot, please configure binfimt_misc manually!\n");
			}
		}
	} else {
		rurima_error("Unknown container runtime!");
	}
	if (!quiet) {
		printf("\n\033[0m");
		if (savedir == NULL) {
			rurima_warning("{yellow}Please replace /path/to/container with your container path!\n");
		}
		if (strcmp(config->architecture, rurima_docker_get_host_arch()) != 0) {
			rurima_warning("{yellow}Please replace /path/to/qemu-%s-static with your qemu binary path!\n", config->architecture);
		}
	}
}
static char *get_auth_server_from_header(const char *_Nonnull header, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get auth server from header.
	 * Example:
	 * www-authenticate: Bearer realm="https://auth.docker.io/token",service="registry.docker.io"
	 *
	 */
	// Just to show you how ugly if we don't force lowercase the header.
	const char *p = rurima_strstr_ignore_case(header, "wWw-aUthEntIcAtE: ");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No auth server found!\n");
	}
	p = strstr(p, "realm=");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No auth server found!\n");
	}
	p = strstr(p, "\"");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No auth server found!\n");
	}
	p++;
	const char *q = strstr(p, "\"");
	if (q == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No auth server found!\n");
	}
	char *ret = malloc((size_t)(q - p + 1));
	strncpy(ret, p, (size_t)(q - p));
	ret[q - p] = '\0';
	return ret;
}
static char *get_service_from_header(const char *_Nonnull header, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get service from header.
	 * Example:
	 * www-authenticate: Bearer realm="https://auth.docker.io/token",service="registry.docker.io"
	 */
	const char *p = rurima_strstr_ignore_case(header, "wWw-aUthEntIcAtE: ");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No service found!\n");
	}
	p = strstr(p, "service=");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No service found!\n");
	}
	p = strstr(p, "\"");
	if (p == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No service found!\n");
	}
	p++;
	const char *q = strstr(p, "\"");
	if (q == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}No service found!\n");
	}
	char *ret = malloc((size_t)(q - p + 1));
	strncpy(ret, p, (size_t)(q - p));
	ret[q - p] = '\0';
	return ret;
}
static char *get_auth_server_url(const char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get auth server url.
	 * Example:
	 * https://auth.docker.io/token?service=registry.docker.io
	 *
	 */
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/", mirror);
	char *curl_command[] = { "curl", "--max-time", "5", "-s", "-L", "-I", url, NULL };
	char *response = rurima_fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		if (fallback) {
			return NULL;
		}
		rurima_error("{red}Failed to get auth server!\n");
	}
	char *server = get_auth_server_from_header(response, fallback);
	if (server == NULL) {
		if (fallback) {
			free(response);
			return NULL;
		}
		rurima_error("{red}Failed to get auth server!\n");
	}
	char *service = get_service_from_header(response, fallback);
	if (service == NULL) {
		if (fallback) {
			free(response);
			free(server);
			return NULL;
		}
		rurima_error("{red}Failed to get service!\n");
	}
	free(response);
	char *ret = malloc(strlen(server) + strlen(service) + 22);
	sprintf(ret, "%s?service=%s", server, service);
	free(server);
	free(service);
	rurima_log("{base}Auth server url: {cyan}%s{clear}\n", ret);
	return ret;
}
static char *get_token(char *_Nonnull image, char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get token from Docker mirror.
	 * This token is used to pull other files of image.
	 *
	 */
	char url[4096] = { '\0' };
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	char *auth_server_url = get_auth_server_url(mirror, fallback);
	if (auth_server_url == NULL) {
		if (fallback) {
			// For some OCI mirrors, they may not use www-authenticate header.
			char default_url[4096] = { '\0' };
			strcat(default_url, "https://");
			strcat(default_url, mirror);
			strcat(default_url, "/token?service=");
			strcat(default_url, mirror);
			auth_server_url = strdup(default_url);
			rurima_log("{red}Can not get auth server, using default %s\n", auth_server_url);
		} else {
			rurima_error("{red}Failed to get auth server!\n");
		}
	}
	strcat(url, auth_server_url);
	free(auth_server_url);
	strcat(url, "&scope=repository%3A");
	strcat(url, image);
	strcat(url, "%3Apull");
	rurima_log("{base}url: {cyan}%s{clear}\n", url);
	char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *token_json = rurima_fork_execvp_get_stdout(curl_command);
	if (token_json == NULL) {
		rurima_error("{red}Failed to get token!\n");
	}
	char *jq_cmd_0[] = { "jq", "-r", "-j", ".token", NULL };
	char *ret = rurima_call_jq(jq_cmd_0, token_json);
	if (ret == NULL) {
		if (fallback) {
			free(token_json);
			rurima_log("{red}Can not get token, using homo magic token 1145141919810\n");
			// We hope the server administrator is homo.
			return strdup("1145141919810");
		} else {
			rurima_error("{red}Failed to get token!");
		}
	}
	free(token_json);
	rurima_log("{base}Token: {cyan}%s{clear}\n", ret);
	return ret;
}
static char *get_tag_manifests(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get manifests of image.
	 * This is used to get digest of image.
	 *
	 */
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/%s/manifests/%s", mirror, image, tag);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.docker.distribution.manifest.list.v1+json", "-H", auth, url, NULL };
	char *ret = rurima_fork_execvp_get_stdout(curl_command);
	if (ret == NULL) {
		rurima_error("{red}Failed to get manifests!\n");
	}
	rurima_log("{base}Manifests: \n{cyan}%s{clear}\n", ret);
	// Check if there is an error message.
	char *jq_cmd_0[] = { "jq", "-r", ".errors[] | .message", NULL };
	char *error_msg = rurima_call_jq(jq_cmd_0, ret);
	if (error_msg != NULL && strlen(error_msg) > 0) {
		rurima_log("{red}Error message: %s\n", error_msg);
		free(ret);
		ret = NULL;
		char *curl_command_err[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", "Accept: application/vnd.oci.image.index.v1+json", "-H", auth, url, NULL };
		ret = rurima_fork_execvp_get_stdout(curl_command_err);
		if (ret == NULL) {
			rurima_error("{red}Failed to get error message!\n");
		}
	}
	free(error_msg);
	free(auth);
	return ret;
}
static char *get_tag_digest(char *_Nonnull manifests, char *_Nullable architecture)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get digest of image.
	 * This is used to pull image.
	 *
	 */
	if (architecture == NULL) {
		architecture = rurima_docker_get_host_arch();
	}
	char *jq_cmd_0[] = { "jq", "-r", ".manifests", NULL };
	char *tmp = rurima_call_jq(jq_cmd_0, manifests);
	if (tmp == NULL) {
		rurima_error("{red}Failed to get manifests!\n");
	}
	char *jq_arg_0 = malloc(strlen(architecture) + 128);
	sprintf(jq_arg_0, ".[] | select(.platform.architecture == \"%s\")|select(.platform.os == \"linux\")|.digest", architecture);
	char *jq_cmd_1[] = { "jq", "-r", "-j", jq_arg_0, NULL };
	char *digest = rurima_call_jq(jq_cmd_1, tmp);
	free(jq_arg_0);
	if (digest == NULL) {
		free(tmp);
		return NULL;
	}
	rurima_log("{base}Digest: %s{clear}\n", digest);
	free(tmp);
	return digest;
}
struct BLOBS {
	char **_Nonnull blobs;
	size_t *_Nonnull size; // size of each blob
};
static struct BLOBS *get_blobs(const char *_Nonnull image, const char *_Nonnull digest, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get blobs of image.
	 * This is used to pull image.
	 * The layers to get will be stored in the returned array.
	 *
	 */
	char url[4096] = { '\0' };
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	sprintf(url, "https://%s/v2/%s/manifests/%s", mirror, image, digest);
	rurima_log("{base}url: {cyan}%s{clear}\n", url);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	rurima_log("{base}Command:\n{cyan}curl -L -s -H \"%s\" %s\n", auth, url);
	char *response = rurima_fork_execvp_get_stdout(curl_command);
	rurima_log("{base}Response: \n{cyan}%s{clear}\n", response);
	if (response == NULL) {
		rurima_error("{red}Failed to get blobs!\n");
	}
	char *jq_cmd_0[] = { "jq", "-r", ".layers", NULL };
	char *layers = rurima_call_jq(jq_cmd_0, response);
	if (layers == NULL) {
		rurima_error("{red}Failed to get layers!\n");
	}
	struct BLOBS *ret = malloc(sizeof(struct BLOBS));
	ret->blobs = NULL;
	ret->size = NULL;
	char *jq_cmd_1[] = { "jq", "-r", ".[] | .digest", NULL };
	char *layers_orig = rurima_call_jq(jq_cmd_1, layers);
	if (layers_orig == NULL) {
		rurima_error("{red}Failed to get digest!\n");
	}
	size_t len = rurima_split_lines(layers_orig, &ret->blobs);
	char **size_char = NULL;
	char *jq_cmd_2[] = { "jq", "-r", ".[] | .size", NULL };
	char *layers_size = rurima_call_jq(jq_cmd_2, layers);
	if (layers_size == NULL) {
		rurima_error("{red}Failed to get layers size!\n");
	}
	len = rurima_split_lines(layers_size, &size_char);
	ret->size = malloc(sizeof(size_t) * (len + 1));
	for (size_t i = 0; i < len; i++) {
		ret->size[i] = (size_t)strtoull(size_char[i], NULL, 10);
	}
	for (size_t i = 0; i < len; i++) {
		free(size_char[i]);
	}
	free(size_char);
	free(layers_size);
	free(layers_orig);
	if (len == 0) {
		rurima_error("{red}Failed to get layers!\n");
	}
	free(layers);
	free(response);
	free(auth);
	return ret;
}
static char *get_short_sha(const char *_Nonnull sha)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Just for displaying layer information.
	 *
	 */
	const char *p = &sha[7];
	char *ret = malloc(32);
	strncpy(ret, p, 16);
	ret[16] = '\0';
	return ret;
}
static void pull_images(char *_Nonnull image, struct BLOBS *_Nonnull blobs, char *_Nonnull token, char *_Nonnull savedir, char *_Nullable mirror, bool fallback, int skip_layer)
{
	/*
	 * Pull images.
	 *
	 * This function will pull all layers of image,
	 * and extract them to savedir.
	 * Fallback mode will get token every time pull a layer.
	 *
	 */
	rurima_check_dir_deny_list(savedir);
	char *token_tmp = NULL;
	char url[4096] = { '\0' };
	char filename[4096] = { '\0' };
	if (rurima_mkdirs(savedir, 0755) != 0) {
		rurima_error("{red}Failed to create directory!\n");
	}
	chdir(savedir);
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	int count = 0;
	for (int i = 0; blobs->blobs[i] != NULL; i++) {
		count++;
	}
	if (count <= skip_layer) {
		rurima_error("{red}Skip layer is larger than or equal to the number of layers!\n");
	}
	for (int i = skip_layer;; i++) {
		if (blobs->blobs[i] == NULL) {
			break;
		}
		char *sha = get_short_sha(blobs->blobs[i]);
		cprintf("{base}Pulling{cyan} %s {base}as{cyan} layer-%d\n", sha, i);
		free(sha);
		sprintf(url, "https://%s/v2/%s/blobs/%s", mirror, image, blobs->blobs[i]);
		sprintf(filename, "layer-%d", i);
		if (fallback) {
			token_tmp = get_token(image, mirror, fallback);
		} else {
			token_tmp = strdup(token);
		}
		char *auth = malloc(strlen(token_tmp) + 114);
		auth[0] = '\0';
		sprintf(auth, "Authorization: Bearer %s", token_tmp);
		free(token_tmp);
		rurima_log("{base}Command:\n{cyan}curl -L -s -H \"%s\" %s -o %s\n", auth, url, filename);
		int ret = 0;
		if (!fallback) {
			ret = rurima_download_file(url, filename, auth, (ssize_t)blobs->size[i]);
		} else {
			char *curl_command[] = { "curl", "-L", "-s", "-H", auth, url, "-o", filename, NULL };
			ret = rurima_fork_execvp(curl_command);
		}
		if (ret != 0) {
			rurima_error("{red}Failed to pull image!\n");
		}
		free(auth);
		if (!fallback && rurima_sha256sum_exists()) {
			char *sha256_command[] = { "sha256sum", filename, NULL };
			char *sha256 = rurima_fork_execvp_get_stdout(sha256_command);
			if (sha256 == NULL) {
				rurima_error("{red}Failed to get sha256!\n");
			}
			if (strchr(sha256, ' ') == NULL) {
				rurima_error("{red}Failed to get sha256!\n");
			}
			*strchr(sha256, ' ') = '\0';
			rurima_log("{base}SHA256: %s\n", sha256);
			if (strcmp(sha256, &(blobs->blobs[i][7])) != 0) {
				rurima_error("{red}SHA256 mismatch!\n");
			}
			rurima_log("{base}SHA256 match!\n");
			free(sha256);
		}
		ret = rurima_extract_archive(filename, ".");
		if (ret != 0) {
			rurima_error("{red}Failed to extract archive!\n");
		}
		remove(filename);
	}
}
static char *get_config_digest_fallback(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Fallback to get config.
	 *
	 */
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char *jq_command_0[] = { "jq", "-r", "-j", ".config.digest", NULL };
	char *ret = rurima_call_jq(jq_command_0, manifests);
	if (ret == NULL) {
		rurima_error("{red}Failed to get config!\n");
	}
	free(manifests);
	return ret;
}
static char *get_config_digest(const char *_Nonnull image, const char *_Nonnull tag, const char *_Nullable digest, const char *_Nonnull token, const char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get the digest of config of image.
	 *
	 */
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	if (digest == NULL) {
		if (!fallback) {
			rurima_error("{red}No digest found!\n");
		}
		return get_config_digest_fallback(image, tag, token, mirror);
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/%s/manifests/%s", mirror, image, digest);
	char *auth = malloc(strlen(token) + 114);
	sprintf(auth, "Authorization: Bearer %s", token);
	char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = rurima_fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		rurima_error("{red}Failed to get config!\n");
	}
	char *jq_command_0[] = { "jq", "-r", "-j", ".config.digest", NULL };
	char *config = rurima_call_jq(jq_command_0, response);
	if (config == NULL) {
		free(response);
		free(auth);
		if (!fallback) {
			rurima_error("{red}Failed to get config!\n");
		}
		return get_config_digest_fallback(image, tag, token, mirror);
	}
	rurima_log("{base}Config: %s{clear}\n", config);
	free(response);
	free(auth);
	return config;
}
static char *env_get_right(const char *_Nonnull env)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * This is used to parse env to key and value.
	 *
	 */
	const char *p = env;
	for (size_t i = 0; i < strlen(env); i++) {
		if (env[i] == '\\') {
			i++;
			continue;
		}
		if (env[i] == '=') {
			p = &env[i + 1];
			break;
		}
	}
	return strdup(p);
}
static char *env_get_left(const char *_Nonnull env)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Same like above.
	 *
	 */
	char *ret = malloc(strlen(env) + 1);
	strcpy(ret, env);
	for (size_t i = 0; i < strlen(env); i++) {
		if (env[i] == '\\') {
			i++;
			continue;
		}
		if (env[i] == '=') {
			ret[i] = '\0';
			break;
		}
	}
	char *tmp = ret;
	ret = strdup(tmp);
	free(tmp);
	return ret;
}
static void parse_env(char *const *_Nonnull env, char **_Nonnull buf, int len)
{
	/*
	 * Parse env.
	 *
	 * Env is like KEY=VALUE,
	 * so we need to split them.
	 * This is because ruri use `-e ENV VALUE` to define env.
	 *
	 */
	if (len == 0) {
		return;
	}
	int j = 0;
	for (int i = 0; i < len; i++) {
		buf[j] = env_get_left(env[i]);
		buf[j + 1] = env_get_right(env[i]);
		buf[j + 2] = NULL;
		buf[j + 3] = NULL;
		j += 2;
	}
}
static struct RURIMA_DOCKER *get_image_config(const char *_Nonnull image, const char *_Nonnull config, const char *_Nonnull token, const char *_Nullable mirror)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Get the config of image.
	 * return a struct RURIMA_DOCKER.
	 *
	 */
	struct RURIMA_DOCKER *ret = malloc(sizeof(struct RURIMA_DOCKER));
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	char url[4096] = { '\0' };
	sprintf(url, "https://%s/v2/%s/blobs/%s", mirror, image, config);
	char *auth = malloc(strlen(token) + 114);
	auth[0] = '\0';
	sprintf(auth, "Authorization: Bearer %s", token);
	char *curl_command[] = { "curl", "-L", "-s", "-H", "Accept: application/vnd.oci.image.manifest.v1+json", "-H", auth, url, NULL };
	char *response = rurima_fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		rurima_error("{red}Failed to get config!\n");
	}
	rurima_log("{base}Config:\n{cyan} %s\n", response);
	{
		char *jq_command_0[] = { "jq", "-r", "-j", ".architecture", NULL };
		char *architecture = rurima_call_jq(jq_command_0, response);
		rurima_log("{base}Arch: {cyan}%s{clear}\n", architecture == NULL ? "NULL" : architecture);
		if (architecture == NULL) {
			ret->architecture = NULL;
		} else {
			ret->architecture = architecture;
		}
	}
	{
		char *jq_command_1[] = { "jq", "-r", "-j", ".config.WorkingDir", NULL };
		char *workdir = rurima_call_jq(jq_command_1, response);
		rurima_log("{base}Workdir: {cyan}%s{clear}\n", workdir == NULL ? "NULL" : workdir);
		if (workdir == NULL) {
			ret->workdir = NULL;
		} else {
			ret->workdir = workdir;
		}
	}
	{
		char *jq_command_2[] = { "jq", "-r", "-j", "-c", ".config.Env", NULL };
		char *env_from_json = rurima_call_jq(jq_command_2, response);
		rurima_log("{base}Env: {cyan}%s{clear}\n", env_from_json == NULL ? "NULL" : env_from_json);
		if (env_from_json != NULL) {
			char *tmp = malloc(strlen(env_from_json) + 114);
			sprintf(tmp, "env=%s\n", env_from_json);
			char *env[RURI_MAX_ENVS / 2];
			env[0] = NULL;
			int len = k2v_get_key(char_array, "env", tmp, env, RURI_MAX_ENVS / 2);
			parse_env(env, ret->env, len);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Env[%d]: {cyan}%s{clear}\n", i, env[i]);
				free(env[i]);
			}
			ret->env[len * 2] = NULL;
			free(tmp);
			free(env_from_json);
		} else {
			ret->env[0] = NULL;
		}
	}
	{
		char *jq_command_3[] = { "jq", "-r", "-j", "-c", ".config.Entrypoint", NULL };
		char *entrypoint = rurima_call_jq(jq_command_3, response);
		rurima_log("{base}Entrypoint: {cyan}%s{clear}\n", entrypoint == NULL ? "NULL" : entrypoint);
		if (entrypoint != NULL) {
			char *tmp = malloc(strlen(entrypoint) + 114);
			sprintf(tmp, "entrypoint=%s\n", entrypoint);
			int len = k2v_get_key(char_array, "entrypoint", tmp, ret->entrypoint, RURI_MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Entrypoint[%d]: {cyan}%s{clear}\n", i, ret->entrypoint[i]);
			}
			ret->entrypoint[len] = NULL;
			free(tmp);
			free(entrypoint);
		} else {
			ret->entrypoint[0] = NULL;
		}
	}
	{
		char *jq_command_4[] = { "jq", "-r", "-j", "-c", ".config.Cmd", NULL };
		char *cmdline = rurima_call_jq(jq_command_4, response);
		rurima_log("{base}Cmdline: {cyan}%s{clear}\n", cmdline == NULL ? "NULL" : cmdline);
		if (cmdline != NULL) {
			char *tmp = malloc(strlen(cmdline) + 114);
			sprintf(tmp, "cmdline=%s\n", cmdline);
			int len = k2v_get_key(char_array, "cmdline", tmp, ret->command, RURI_MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Cmdline[%d]: {cyan}%s{clear}\n", i, ret->command[i]);
			}
			ret->command[len] = NULL;
			free(tmp);
			free(cmdline);
		} else {
			ret->command[0] = NULL;
		}
	}
	free(response);
	free(auth);
	return ret;
}
struct RURIMA_DOCKER *rurima_get_docker_config(char *_Nonnull image, char *_Nonnull tag, char *_Nullable architecture, char *_Nullable mirror, bool fallback)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Return the config of image.
	 * This function is called by subcommand.c
	 *
	 */
	char *token = get_token(image, mirror, fallback);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char *digest = get_tag_digest(manifests, architecture);
	// digest can be NULL, then we go to fallback.
	char *config = get_config_digest(image, tag, digest, token, mirror, fallback);
	if (config == NULL) {
		rurima_error("{red}Failed to get config!\n");
	}
	struct RURIMA_DOCKER *ret = get_image_config(image, config, token, mirror);
	free(manifests);
	free(token);
	free(digest);
	free(config);
	return ret;
}
static struct RURIMA_DOCKER *docker_pull_fallback(char *_Nonnull image, char *_Nonnull tag, char *_Nonnull savedir, char *_Nullable mirror, int skip_layer)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * Fallback to pull image.
	 *
	 */
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	char *token = get_token(image, mirror, true);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	struct BLOBS blobs;
	char *jq_cmd_0[] = { "jq", "-r", ".layers", NULL };
	char *layers = rurima_call_jq(jq_cmd_0, manifests);
	if (layers == NULL) {
		rurima_error("{red}Failed to get layers!\n");
	}
	char *jq_cmd_1[] = { "jq", "-r", ".[] | .digest", NULL };
	char *layers_orig = rurima_call_jq(jq_cmd_1, layers);
	if (layers_orig == NULL) {
		rurima_error("{red}Failed to get digest!\n");
	}
	size_t len = rurima_split_lines(layers_orig, &blobs.blobs);
	blobs.size = malloc(sizeof(size_t) * (len + 1));
	for (size_t i = 0; i < len; i++) {
		blobs.size[i] = (size_t)0; // Fallback, we don't know the size.
	}
	free(layers_orig);
	if (len == 0) {
		rurima_error("{red}Failed to get digest!\n");
	}
	end_loading_animation();
	pull_images(image, &blobs, token, savedir, mirror, true, skip_layer);
	free(token);
	token = get_token(image, mirror, true);
	char *config = get_config_digest_fallback(image, tag, token, mirror);
	struct RURIMA_DOCKER *ret = get_image_config(image, config, token, mirror);
	free(manifests);
	free(token);
	free(config);
	free(layers);
	return ret;
}
struct RURIMA_DOCKER *rurima_docker_pull(struct RURIMA_DOCKER_PULL *_Nonnull action)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * We pull all the layers of image,
	 * and extract them to savedir.
	 * And we return the config of image.
	 *
	 */
	char *image = action->image;
	char *tag = action->tag;
	char *architecture = action->architecture;
	char *savedir = action->savedir;
	char *mirror = action->mirror;
	int skip_layers = action->skip_layer;
	bool fallback = action->fallback;
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	start_loading_animation("Fetching metadata...");
	char *token = get_token(image, mirror, fallback);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char *digest = get_tag_digest(manifests, architecture);
	if (digest == NULL) {
		free(manifests);
		free(token);
		if (!fallback) {
			rurima_error("{red}Failed to get digest!\n");
		}
		return docker_pull_fallback(image, tag, savedir, mirror, skip_layers);
	}
	struct BLOBS *blobs = get_blobs(image, digest, token, mirror);
	if (blobs == NULL) {
		rurima_error("{red}Failed to get blobs!\n");
	}
	end_loading_animation();
	pull_images(image, blobs, token, savedir, mirror, fallback, skip_layers);
	start_loading_animation("Fetching config metadata...");
	if (fallback) {
		free(token);
		token = get_token(image, mirror, true);
	}
	char *config = get_config_digest(image, tag, digest, token, mirror, fallback);
	if (fallback) {
		free(token);
		token = get_token(image, mirror, true);
	}
	struct RURIMA_DOCKER *ret = get_image_config(image, config, token, mirror);
	free(manifests);
	free(token);
	free(digest);
	free(blobs->size);
	for (size_t i = 0; blobs->blobs[i] != NULL; i++) {
		free(blobs->blobs[i]);
	}
	free(blobs->blobs);
	free(blobs);
	free(config);
	end_loading_animation();
	return ret;
}
static char *docker_search__(char *_Nonnull url)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * This is the core parse function of rurima_docker_search().
	 * It will read info from url, and return next url.
	 *
	 */
	char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *response = rurima_fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		rurima_error("{red}Failed to get response from dockerhub!\n");
	}
	rurima_log("{base}Response from dockerhub:\n{cyan}%s{clear}\n", response);
	char *jq_cmd_1[] = { "jq", "-r", ".next", NULL };
	char *next_url = rurima_call_jq(jq_cmd_1, response);
	if (next_url == NULL) {
		rurima_error("{red}Failed to get next url!\n");
	}
	char *jq_cmd_2[] = { "jq", "-r", ".results", NULL };
	char *results = rurima_call_jq(jq_cmd_2, response);
	if (results == NULL) {
		rurima_error("{red}No results found!\n");
	}
	rurima_log("{base}Results:\n{cyan}%s{clear}\n", results);
	char **name = NULL;
	char *jq_cmd_3[] = { "jq", "-r", ".[].repo_name", NULL };
	char *name_ori = rurima_call_jq(jq_cmd_3, results);
	if (name_ori == NULL) {
		rurima_error("{red}No results found!\n");
	}
	size_t len = rurima_split_lines_allow_null(name_ori, &name);
	if (len == 0) {
		rurima_error("{red}No results found!\n");
	}
	free(name_ori);
	char **description = NULL;
	char *jq_cmd_4[] = { "jq", "-r", ".[].short_description", NULL };
	char *description_ori = rurima_call_jq(jq_cmd_4, results);
	if (description_ori == NULL) {
		rurima_error("{red}No results found!\n");
	}
	size_t len2 = rurima_split_lines_allow_null(description_ori, &description);
	free(description_ori);
	if (len2 == 0) {
		rurima_error("{red}No results found!\n");
	}
	if (len2 != len) {
		rurima_error("{red}Incorrect json!\n");
	}
	char **is_offical = NULL;
	char *jq_cmd_5[] = { "jq", "-r", ".[].is_official", NULL };
	char *is_offical_ori = rurima_call_jq(jq_cmd_5, results);
	if (is_offical_ori == NULL) {
		rurima_error("{red}No results found!\n");
	}
	size_t len3 = rurima_split_lines_allow_null(is_offical_ori, &is_offical);
	free(is_offical_ori);
	if (len3 == 0) {
		rurima_error("{red}No results found!\n");
	}
	if (len3 != len) {
		rurima_error("{red}Incorrect json!\n");
	}
	for (size_t i = 0; i < len - 1; i++) {
		if (strcmp(is_offical[i], "true") == 0) {
			if (!rurima_global_config.quiet) {
				cprintf("{yellow}%s {green}[official]\n", name[i]);
				if (description[i] != NULL && strlen(description[i]) > 0) {
					cprintf("  {cyan}Description: %s\n", description[i]);
				} else {
					cprintf("  {cyan}Description: No description\n");
				}
			} else {
				printf("%s [official]\n", name[i]);
			}
		} else {
			if (!rurima_global_config.quiet) {
				cprintf("{yellow}%s\n", name[i]);
				if (description[i] != NULL) {
					cprintf("  {cyan}Description: %s\n", description[i]);
				} else {
					cprintf("  {cyan}Description: No description\n");
				}
			} else {
				printf("%s\n", name[i]);
			}
		}
	}
	free(response);
	for (size_t i = 0; i < len; i++) {
		free(name[i]);
		free(description[i]);
		free(is_offical[i]);
	}
	free(results);
	free(name);
	free(description);
	free(is_offical);
	return next_url;
}
int rurima_docker_search(const char *_Nonnull image, const char *_Nonnull page_size, bool quiet, const char *_Nullable mirror)
{
	/*
	 *
	 * An implementation of docker search.
	 * Return value is not important here,
	 * because we will error() directly if failed.
	 *
	 */
	char *url = malloc(4096);
	url[0] = '\0';
	if (mirror == NULL) {
		mirror = "hub.docker.com";
	}
	strcat(url, "https://");
	strcat(url, mirror);
	strcat(url, "/v2/search/repositories/?page_size=");
	strcat(url, page_size);
	strcat(url, "&query=");
	strcat(url, image);
	while (true) {
		rurima_log("{base}url: {cyan}%s{clear}\n", url);
		char *next_url = docker_search__(url);
		if (next_url == NULL) {
			exit(EXIT_SUCCESS);
		}
		rurima_log("{base}nexturl: {cyan}%s{clear}\n", next_url);
		if (quiet) {
			free(url);
			free(next_url);
			break;
		}
		char goto_next[114] = { '\0' };
		rurima_get_input("\n{purple}Continue see more results? (y/n): ", goto_next);
		rurima_log("{base}goto_next: {cyan}%s{clear}\n", goto_next);
		if (strcmp(goto_next, "y") == 0) {
			free(url);
			url = next_url;
			rurima_log("{base}next_url: {cyan}%s{clear}\n", next_url);
		} else {
			free(url);
			free(next_url);
			break;
		}
	}
	return 0;
}
static char *docker_search_tag__(char *_Nonnull image, char *_Nonnull url, char *_Nullable architecture)
{
	/*
	 * Warning: free() the return value after use.
	 *
	 * This is the core parse function of rurima_docker_search_tag().
	 * It will read info from url, and return next url.
	 *
	 */
	rurima_log("{base}url: {cyan}%s{clear}\n", url);
	char *curl_command[] = { "curl", "-L", "-s", url, NULL };
	char *response = rurima_fork_execvp_get_stdout(curl_command);
	if (response == NULL) {
		rurima_error("{red}Failed to get response from dockerhub!\n");
	}
	rurima_log("{base}Response from dockerhub:\n{cyan}%s{clear}\n", response);
	if (architecture == NULL) {
		architecture = rurima_docker_get_host_arch();
	}
	char *jq_cmd_0[] = { "jq", "-r", ".next", NULL };
	char *next_url = rurima_call_jq(jq_cmd_0, response);
	rurima_log("{base}next_url: {cyan}%s{clear}\n", next_url);
	char *jq_cmd_1[] = { "jq", "-r", "-j", ".results", NULL };
	char *results = rurima_call_jq(jq_cmd_1, response);
	char *jq_arg_0 = malloc(strlen(architecture) + 1024);
	sprintf(jq_arg_0, ".[]|select(.images.[].architecture==\"%s\")|.name", architecture);
	if (results == NULL) {
		free(jq_arg_0);
		rurima_error("{red}No results found!\n");
	}
	char **name = NULL;
	char *jq_cmd_2[] = { "jq", "-r", jq_arg_0, NULL };
	char *name_ori = rurima_call_jq(jq_cmd_2, results);
	if (name_ori == NULL) {
		free(jq_arg_0);
		rurima_error("{red}No results found!\n");
	}
	size_t len = rurima_split_lines_allow_null(name_ori, &name);
	if (len == 0) {
		free(jq_arg_0);
		rurima_error("{red}No results found!\n");
	}
	rurima_log("{base}Results:\n{cyan}%s{clear}\n", name_ori);
	for (size_t i = 0; i < len - 1; i++) {
		if (!rurima_global_config.quiet) {
			cprintf("{yellow}[%s]: {cyan}%s{clear}\n", image, name[i]);
		} else {
			printf("[%s]: %s\n", image, name[i]);
		}
		free(name[i]);
	}
	free(jq_arg_0);
	free(response);
	free(results);
	return next_url;
}
int rurima_docker_search_tag(char *_Nonnull image, char *_Nonnull page_size, char *_Nullable architecture, bool quiet, char *_Nullable mirror)
{
	/*
	 * An implementation of docker tag search.
	 *
	 * The return value is not important here,
	 * because we will rurima_error() directly if failed.
	 *
	 */
	char *url = malloc(4096);
	url[0] = '\0';
	if (mirror == NULL) {
		mirror = "hub.docker.com";
	}
	strcat(url, "https://");
	strcat(url, mirror);
	strcat(url, "/v2/repositories/");
	strcat(url, image);
	strcat(url, "/tags/?page_size=");
	strcat(url, page_size);
	while (true) {
		rurima_log("{base}url: {cyan}%s{clear}\n", url);
		char *next_url = docker_search_tag__(image, url, architecture);
		if (next_url == NULL) {
			exit(EXIT_SUCCESS);
		}
		rurima_log("{base}nexturl: {cyan}%s{clear}\n", next_url);
		if (quiet) {
			free(url);
			free(next_url);
			return 0;
			break;
		}
		char goto_next[114] = { '\0' };
		rurima_get_input("\n{purple}Continue see more results? (y/n): ", goto_next);
		rurima_log("{base}goto_next: {cyan}%s{clear}\n", goto_next);
		if (strcmp(goto_next, "y") == 0) {
			free(url);
			url = next_url;
			rurima_log("{base}next_url: {cyan}%s{clear}\n", next_url);
		} else {
			free(url);
			free(next_url);
			break;
		}
	}
	return 0;
}
static void docker_add_archlist__(char *_Nonnull arch, char ***_Nullable archlist)
{
	/*
	 * Add arch to archlist.
	 *
	 * We will not add duplicate arch.
	 *
	 */
	if (strcmp(arch, "unknown") == 0) {
		return;
	}
	if (*archlist == NULL) {
		*archlist = malloc(sizeof(char *) * 3);
		(*archlist)[0] = strdup(arch);
		(*archlist)[1] = NULL;
		return;
	}
	for (int i = 0; (*archlist)[i] != NULL; i++) {
		if (strcmp((*archlist)[i], arch) == 0) {
			return;
		}
	}
	size_t j = 0;
	for (; (*archlist)[j] != NULL; j++)
		;
	*archlist = realloc(*archlist, sizeof(char *) * (j + 3));
	for (int i = 0;; i++) {
		if ((*archlist)[i] == NULL) {
			(*archlist)[i] = strdup(arch);
			(*archlist)[i + 1] = NULL;
			break;
		}
	}
}
static void docker_print_arch(const char *_Nonnull image, char *const *_Nonnull arch, size_t len)
{
	/*
	 * Print architecture of image.
	 *
	 * We will not print unknown architecture or duplicate architecture.
	 *
	 */
	char **archlist = NULL;
	for (size_t i = 0; i < len; i++) {
		docker_add_archlist__(arch[i], &archlist);
	}
	if (archlist == NULL) {
		rurima_error("{red}No results found!\n");
	}
	for (int i = 0; archlist[i] != NULL; i++) {
		if (!rurima_global_config.quiet) {
			cprintf("{yellow}[%s]: {cyan}%s{clear}\n", image, archlist[i]);
		} else {
			printf("[%s]: %s\n", image, archlist[i]);
		}
	}
	for (int i = 0; archlist[i] != NULL; i++) {
		free(archlist[i]);
	}
	free(archlist);
}
int rurima_docker_search_arch(char *_Nonnull image, char *_Nonnull tag, char *_Nullable mirror, bool fallback)
{
	/*
	 * Get architecture of image:tag.
	 *
	 * The return value is not important here,
	 * because we will rurima_error() directly if failed.
	 *
	 */
	if (mirror == NULL) {
		mirror = rurima_global_config.docker_mirror;
	}
	char *token = get_token(image, mirror, fallback);
	char *manifests = get_tag_manifests(image, tag, token, mirror);
	char **arch = NULL;
	char *jq_cmd_0[] = { "jq", "-r", ".manifests", NULL };
	char *tmp = rurima_call_jq(jq_cmd_0, manifests);
	if (tmp == NULL) {
		rurima_error("{red}Failed to get manifests!\n");
	}
	char *jq_cmd_1[] = { "jq", "-r", ".[] | .platform.architecture", NULL };
	char *arch_ori = rurima_call_jq(jq_cmd_1, tmp);
	if (arch_ori == NULL) {
		free(tmp);
		rurima_error("{red}Failed to get architecture!\n");
	}
	size_t len = rurima_split_lines(arch_ori, &arch);
	free(arch_ori);
	if (len == 0) {
		rurima_error("{red}No results found!\n");
	}
	docker_print_arch(image, arch, len);
	free(manifests);
	free(token);
	for (size_t i = 0; i < len; i++) {
		free(arch[i]);
	}
	free(arch);
	free(tmp);
	return 0;
}
void rurima_docker_print_config_from_json(const char *_Nonnull config, const char *_Nonnull savedir)
{
	/*
	 * For load subcommand.
	 * We just print the config from json string.
	 */
	struct RURIMA_DOCKER *ret = malloc(sizeof(struct RURIMA_DOCKER));
	char *response = config;
	{
		char *jq_command_0[] = { "jq", "-r", "-j", ".architecture", NULL };
		char *architecture = rurima_call_jq(jq_command_0, response);
		rurima_log("{base}Arch: {cyan}%s{clear}\n", architecture == NULL ? "NULL" : architecture);
		if (architecture == NULL) {
			ret->architecture = NULL;
		} else {
			ret->architecture = architecture;
		}
	}
	{
		char *jq_command_1[] = { "jq", "-r", "-j", ".config.WorkingDir", NULL };
		char *workdir = rurima_call_jq(jq_command_1, response);
		rurima_log("{base}Workdir: {cyan}%s{clear}\n", workdir == NULL ? "NULL" : workdir);
		if (workdir == NULL) {
			ret->workdir = NULL;
		} else {
			ret->workdir = workdir;
		}
	}
	{
		char *jq_command_2[] = { "jq", "-r", "-j", "-c", ".config.Env", NULL };
		char *env_from_json = rurima_call_jq(jq_command_2, response);
		rurima_log("{base}Env: {cyan}%s{clear}\n", env_from_json == NULL ? "NULL" : env_from_json);
		if (env_from_json != NULL) {
			char *tmp = malloc(strlen(env_from_json) + 114);
			sprintf(tmp, "env=%s\n", env_from_json);
			char *env[RURI_MAX_ENVS / 2];
			env[0] = NULL;
			int len = k2v_get_key(char_array, "env", tmp, env, RURI_MAX_ENVS / 2);
			parse_env(env, ret->env, len);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Env[%d]: {cyan}%s{clear}\n", i, env[i]);
				free(env[i]);
			}
			ret->env[len * 2] = NULL;
			free(tmp);
			free(env_from_json);
		} else {
			ret->env[0] = NULL;
		}
	}
	{
		char *jq_command_3[] = { "jq", "-r", "-j", "-c", ".config.Entrypoint", NULL };
		char *entrypoint = rurima_call_jq(jq_command_3, response);
		rurima_log("{base}Entrypoint: {cyan}%s{clear}\n", entrypoint == NULL ? "NULL" : entrypoint);
		if (entrypoint != NULL) {
			char *tmp = malloc(strlen(entrypoint) + 114);
			sprintf(tmp, "entrypoint=%s\n", entrypoint);
			int len = k2v_get_key(char_array, "entrypoint", tmp, ret->entrypoint, RURI_MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Entrypoint[%d]: {cyan}%s{clear}\n", i, ret->entrypoint[i]);
			}
			ret->entrypoint[len] = NULL;
			free(tmp);
			free(entrypoint);
		} else {
			ret->entrypoint[0] = NULL;
		}
	}
	{
		char *jq_command_4[] = { "jq", "-r", "-j", "-c", ".config.Cmd", NULL };
		char *cmdline = rurima_call_jq(jq_command_4, response);
		rurima_log("{base}Cmdline: {cyan}%s{clear}\n", cmdline == NULL ? "NULL" : cmdline);
		if (cmdline != NULL) {
			char *tmp = malloc(strlen(cmdline) + 114);
			sprintf(tmp, "cmdline=%s\n", cmdline);
			int len = k2v_get_key(char_array, "cmdline", tmp, ret->command, RURI_MAX_COMMANDS);
			for (int i = 0; i < len; i++) {
				rurima_log("{base}Cmdline[%d]: {cyan}%s{clear}\n", i, ret->command[i]);
			}
			ret->command[len] = NULL;
			free(tmp);
			free(cmdline);
		} else {
			ret->command[0] = NULL;
		}
	}
	rurima_show_docker_config(ret, savedir, "ruri", false);
	rurima_free_docker_config(ret);
	return;
}