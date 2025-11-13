// SPDX-License-Identifier: MIT
/*
 *
 * This file is part of rurima, with ABSOLUTELY NO WARRANTY.
 *
 * MIT License
 *
 * Copyright (c) 2025 Moe-hacker
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
#include "daijin.h"
// NOLINTNEXTLINE
static short x_win, y_win;
static size_t strlen_char32_rma(const char32_t *_Nonnull str)
{
	size_t len = 0;
	// clang-format off
	while (str[len] != U'\0') {
		len++;
	}
	// clang-format on
	return len;
}
static void print_art(int x, bool clear)
{
	// clang-format off
	char32_t art[][128] = {
			  U"  ⣀⠤⠤⠤⠤⢀⡀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
			  U"⣰⠋⠁⠀⠀⠀⠀⠀⠈⠲⣄⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀",
			  U"⢯⣀⣀⣀⡀⠀⠀⠀⠀⠀⠈⢦⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢀⣀⣀⢀⠀⠀⠀⠀⠀⠀⠀",
			  U"⠀⠀⠠⣏⠀⡀⠀⢀⡀⠀⠀⠈⡆⠀⠀⠀⠀⠀⠀⢀⣀⠤⠔⠂⠁⠀⠀⠀⠀⠉⠰⢄⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠈⠉⠒⢌⢳⠀⠀⢹⠀⠀⠀⢞⠩⠃⣀⠖⠂⠀⠀⠀⠀⠀⡴⣧⡒⢤⡀⠱⡄⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠈⣦⡇⠀⢸⠀⠀⠀⢈⡶⠚⠁⣀⠀⠀⠀⠀⠀⠀⡇⠈⠿⣷⣿⣄⠙⣦⡄",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡇⠀⣼⣀⣠⠔⣁⣠⣴⣾⠀⠀⠀⠀⠀⠀⠀⠱⡀⠤⠘⠿⡿⠁⠈⣴",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⡆⠀⣿⣿⣿⢿⣿⣿⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀⠉⠒⠒⠉⠀⠀⠀⢺",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⢸⠀⢰⣏⠊⢕⡒⠾⡯⣭⣽⡇⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⢠⠏",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⡿⠀⣼⠈⠀⠀⠈⠉⠙⠋⠋⡇⠔⠁⠀⠀⠀⠀⠀⡀⠀⢠⣀⣤⠾⠁⠀",
			  U"         ⢠⡇⠀⠏⠀⠀⠀⠀⠀⠀⠀⠀⢙⠦⠒⠒⢺⣁⠤⠊⠧⠔⢡⡟⠁⠀⠀ ",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⣸⠂⢸⡀⠀⠀⠀⠀⠀⣀⠤⠄⠛⠂⠀⠐⠊⠁⠀⠀⠀⠀⠈⣣⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⡟⠀⢸⠀⠀⠀⠀⢠⣞⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣾⡀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⡇⠀⠸⡄⠀⠀⠀⡼⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⣰⡏⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠸⠀⠀⢱⡀⠀⢰⡇⠀⠀⠀⠀⠀⢠⣀⠀⠀⠀⠀⠀⢠⠀⢹⠀⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠻⣕⣊⠹⢆⣼⡧⡀⠀⠀⠀⠀⠀⠳⢨⠀⠀⠀⠀⡆⠀⣿⠀⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠪⢗⢠⠀⠳⡌⠒⠀⠀⠀⠀⠀⠀⡆⠀⠀⢰⠁⠈⠇⠀⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠁⠓⢿⣤⣀⠀⠀⠀⠀⠀⣇⠀⠀⣸⠈⠉⢳⠀⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠈⠛⠶⢤⣄⡀⠀⢀⣸⡄⠀⣠⣱⡼⠟⠀⠀⠀⠀⠀",
			  U"⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠉⠈⠑⠋⠉⠁⠀⠀⠀⠀⠀⠀"};
	// clang-format on
	size_t lines = sizeof(art) / sizeof(art[0]);
	for (short i = 0; i < y_win - lines - 1; i++) {
		printf("\n");
	}
	fflush(stdout);
	for (size_t i = 0; i < lines; i++) {
		char32_t *line = art[i];
		size_t line_len = strlen_char32_rma(line);
		for (short k = 0; k < (x_win - line_len) / 2; k++) {
			printf(" ");
		}
		fflush(stdout);
		for (size_t j = 0; j < line_len; j++) {
			if ((i + x > (line_len - j) / 3 + 18) && clear) {
				printf(" ");
				continue;
			}
			if (!clear) {
				if ((i + 10 < x + j / 3) && x > 0) {
					printf("\033[0m");
				} else if ((i + 10 < x + j / 3 + 1) && x > 0) {
					printf("\033[37m");
				} else {
					printf("\033[38;2;254;228;208m");
				}
				fflush(stdout);
			}
			char character[64] = { '\0' };
			mbstate_t state = { 0 };
			size_t len = c32rtomb(character, line[j], &state);
			if (len == (size_t)-1) {
				perror("c32rtomb");
				return;
			}
			character[len] = '\0';
			printf("%s", character);
			fflush(stdout);
		}
		printf("\n");
	}
	printf("\033[0m");
	fflush(stdout);
}
void rurima_QwQ(void)
{
	struct winsize w;
	ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
	x_win = w.ws_col;
	y_win = w.ws_row;
	if (x_win < 60 || y_win < 30) {
		fprintf(stderr, "Please resize your terminal to at least 60x30 and try again.\n");
		return;
	}
	printf("\033[?25l");
	printf("\033[?1049h");
	setlocale(LC_CTYPE, "");
	printf("\033[H\033[2J");
	print_art(0, false);
	sleep(2);
	for (int i = 0; i < 30; i++) {
		printf("\033[H\033[2J");
		fflush(stdout);
		print_art(i, false);
		usleep(100000);
	}
	sleep(2);
	for (int i = 0; i < 30; i++) {
		printf("\033[H\033[2J");
		fflush(stdout);
		print_art(i, true);
		usleep(50000);
	}
	printf("\033[H\033[2J");
	printf("\033[?1049l");
	printf("\033[?25h");
}