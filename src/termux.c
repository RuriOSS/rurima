#include "include/rurima.h"
void rurima_setup_termux_tmp(void)
{
	if (getuid() != 0) {
		fprintf(stderr, "This function must be run as root.\n");
		exit(EXIT_FAILURE);
	}
	if (access("/data/data/com.termux/files/usr/bin/bash", F_OK) == -1) {
		fprintf(stderr, "Termux not found.\n");
		exit(EXIT_FAILURE);
	}
	// get owner id of /data/data/com.termux/files/usr.
	struct stat st;
	if (stat("/data/data/com.termux/files/usr", &st) == -1) {
		perror("stat");
		exit(EXIT_FAILURE);
	}
	int termux_uid = st.st_uid;
	if (unshare(CLONE_NEWNS) == -1) {
		perror("unshare");
		exit(EXIT_FAILURE);
	}
	pid_t pid = fork();
	if (pid == -1) {
		perror("fork");
		exit(EXIT_FAILURE);
	}
	if (pid == 0) {
		if (mount("tmpfs", "/tmp", "tmpfs", 0, "") == -1) {
			perror("mount");
			exit(EXIT_FAILURE);
		}
		setuid(termux_uid);
		setenv("PATH", "/data/data/com.termux/files/usr/bin", 1);
		setenv("HOME", "/data/data/com.termux/files/home", 1);
		setenv("TMPDIR", "/data/data/com.termux/files/tmp", 1);
		execl("/data/data/com.termux/files/usr/bin/bash", "bash", NULL);
		perror("execl");
		exit(EXIT_FAILURE);
	} else {
		// Exit as the same state of child process.
		int status;
		waitpid(pid, &status, 0);
		if (WIFEXITED(status)) {
			exit(WEXITSTATUS(status));
		} else {
			exit(EXIT_FAILURE);
		}
	}
}