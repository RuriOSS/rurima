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
void rurima_show_version_info(void)
{
	/*
	 * Just show version info and license.
	 * Version info is defined in macro RURIMA_VERSION.
	 * RURIMA_COMMIT_ID is defined as -D option of compiler.
	 */
	cprintf("\n");
	cprintf("{base}      ●●●●  ●   ● ●●●●   ●●●  ●   ●   ●  \n");
	cprintf("{base}      ●   ● ●   ● ●   ●   ●   ●● ●●  ● ● \n");
	cprintf("{base}      ●●●●  ●   ● ●●●●    ●   ● ● ● ●●●●●\n");
	cprintf("{base}      ●  ●  ●   ● ●  ●    ●   ●   ● ●   ●\n");
	cprintf("{base}      ●   ●  ●●●  ●   ●  ●●●  ●   ● ●   ●\n");
	cprintf("{base}        Licensed under the MIT License\n");
	cprintf("{base}          <https://mit-license.org>\n");
	cprintf("{base}        Copyright (C) 2024 Moe-hacker\n\n");
	cprintf("{base}%s%s%s", "rurima version ..................:  ", RURIMA_VERSION, "\n");
#if defined(RURIMA_COMMIT_ID)
	cprintf("{base}%s%s%s", "rurima commit id ................:  ", RURIMA_COMMIT_ID, "\n");
#endif
	cprintf("{base}%s%s%s", "built-in ruri version ...........:  ", RURI_VERSION, "\n");
#if defined(RURI_COMMIT_ID)
	cprintf("{base}%s%s%s", "built-in ruri commit id .........:  ", RURI_COMMIT_ID, "\n");
#endif
#if defined(LIBCAP_MAJOR) && defined(LIBCAP_MINOR)
	cprintf("{base}%s%d%s%d%s", "libcap ..........................:  ", LIBCAP_MAJOR, ".", LIBCAP_MINOR, "\n");
#endif
	cprintf("{base}%s%s\n", "Compiler version ................:  ", __VERSION__);
	cprintf("{base}%s%s\n", "Build date ......................:  ", __TIMESTAMP__);
	cprintf("{base}\nThere is NO WARRANTY, to the extent permitted by law\n");
	cprintf("{base}Docker is a registered trademark of Docker, Inc\nThis program has no relationship with it\n");
	cprintf("{base}\nThis program has Super Neko Powers!!!\n");
	cprintf("{clear}\n");
}
// For `rurima -V`.
void rurima_show_version_code(void)
{
/*
 * The version code is not standard now,
 * so in fact it's very useless.
 * Maybe it can be useful one day...
 */
#ifdef RURIMA_OFFICIAL_BUILD
	cprintf("%s-official\n", RURIMA_VERSION);
#else
	cprintf("%s\n", RURIMA_VERSION);
#endif
}