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
void rurima_netns_wrapper(int argc, char **argv)
{
	char *wrapper = "#!/bin/sh \n\
# This is a wrapper script for setting up a network namespace with veth pairs and a bridge \n\
# Default settings \n\
NETNS_NAME=\"ruri_netns\" \n\
VETH_NS=\"ruri_veth\" \n\
VETH_HOST=\"ruri_hostveth\" \n\
BRIDGE_NAME=\"ruri_br0\" \n\
BRIDGE_IP=\"192.168.100.1/24\" \n\
BRIDGE_NET=\"192.168.100.0/24\" \n\
NS_IP=\"192.168.100.2/24\" \n\
NS_GATEWAY=\"192.168.100.1\" \n\
OUT_IFACE=\"wlan0\" \n\
FORCE=\"false\" \n\
# Parse args \n\
i=0 \n\
for arg in \"$@\"; do \n\
    i=$((i + 1)) \n\
    case $arg in \n\
        --ns-ip=*) NS_IP=\"${arg#*=}\" ;; \n\
        --ns-gateway=*) NS_GATEWAY=\"${arg#*=}\" ;; \n\
        --netns-name=*) NETNS_NAME=\"${arg#*=}\" ;; \n\
        --veth-ns=*) VETH_NS=\"${arg#*=}\" ;; \n\
        --veth-host=*) VETH_HOST=\"${arg#*=}\" ;; \n\
        --bridge-name=*) BRIDGE_NAME=\"${arg#*=}\" ;; \n\
        --bridge-ip=*) \n\
            BRIDGE_IP=\"${arg#*=}\" \n\
            BRIDGE_NET=\"${BRIDGE_IP%/*}/24\" \n\
            ;; \n\
        --out-iface=*) OUT_IFACE=\"${arg#*=}\" ;; \n\
        --force) FORCE=\"true\" ;; \n\
        --help) \n\
            echo \"Usage: $0 [--netns-name=NAME] [--veth-ns=NAME] [--veth-host=NAME] [--bridge-name=NAME] [--bridge-ip=IP/CIDR] [--out-iface=IFACE]\" \n\
            exit 0 \n\
            ;; \n\
        *) shift $((i-1)); break;; \n\
    esac \n\
done \n\
 \n\
# Check if devices exist \n\
ip netns list | grep -qw \"$NETNS_NAME\" && ip link show \"$BRIDGE_NAME\" > /dev/null 2>&1 && ip link show \"$VETH_HOST\" > /dev/null 2>&1 \n\
 \n\
if [ $? -eq 0 ] && [ \"$FORCE\" != \"true\" ]; then \n\
    # Maybe show warning here? \n\
    if [ $# -eq 0 ]; then \n\
        echo \"No command specified for namespace execution.\" \n\
        exit 0 \n\
    fi \n\
    ip netns exec \"$NETNS_NAME\" \"$@\" \n\
    exit 0 \n\
fi \n\
 \n\
# Cleanup \n\
ip netns del \"$NETNS_NAME\" 2>/dev/null \n\
ip link del \"$BRIDGE_NAME\" 2>/dev/null \n\
ip link del \"$VETH_HOST\" 2>/dev/null \n\
sleep 0.3 \n\
 \n\
# Enable IP forwarding \n\
echo 1 > /proc/sys/net/ipv4/ip_forward \n\
sysctl -w net.ipv6.conf.all.forwarding=1 > /dev/null 2>&1 \n\
 \n\
# Create network namespace \n\
ip netns add \"$NETNS_NAME\" || exit 1 \n\
 \n\
# enable loopback \n\
ip netns exec \"$NETNS_NAME\" ip link set up dev lo \n\
 \n\
# Create veth pair \n\
ip link add \"$VETH_HOST\" type veth peer name \"$VETH_NS\" || exit 1 \n\
 \n\
# Move veth to namespace \n\
ip link set \"$VETH_NS\" netns \"$NETNS_NAME\" || exit 1 \n\
 \n\
# Enable interfaces \n\
ip link set \"$VETH_HOST\" up \n\
ip netns exec \"$NETNS_NAME\" ip link set up dev \"$VETH_NS\" \n\
 \n\
# Create bridge device \n\
ip link add \"$BRIDGE_NAME\" type bridge || exit 1 \n\
ip link set \"$BRIDGE_NAME\" up \n\
ip link set \"$VETH_HOST\" master \"$BRIDGE_NAME\" \n\
ip addr add \"$BRIDGE_IP\" dev \"$BRIDGE_NAME\" \n\
 \n\
 \n\
# Configure IP and routing for namespace \n\
ip netns exec \"$NETNS_NAME\" ip addr add \"$NS_IP\" dev \"$VETH_NS\" \n\
ip netns exec \"$NETNS_NAME\" ip route add default via \"$NS_GATEWAY\" dev \"$VETH_NS\" \n\
# Enable NAT \n\
iptables -t nat -D POSTROUTING -s \"$BRIDGE_NET\" -o \"$OUT_IFACE\" -j MASQUERADE 2>/dev/null \n\
iptables -D FORWARD -i \"$BRIDGE_NAME\" -o \"$OUT_IFACE\" -j ACCEPT 2>/dev/null \n\
iptables -D FORWARD -i \"$OUT_IFACE\" -o \"$BRIDGE_NAME\" -m state --state ESTABLISHED,RELATED -j ACCEPT 2>/dev/null \n\
iptables -t nat -I POSTROUTING -s \"$BRIDGE_NET\" -o \"$OUT_IFACE\" -j MASQUERADE \n\
 \n\
# Enable forwarding \n\
iptables -I FORWARD -i \"$BRIDGE_NAME\" -o \"$OUT_IFACE\" -j ACCEPT \n\
iptables -I FORWARD -i \"$OUT_IFACE\" -o \"$BRIDGE_NAME\" -m state --state ESTABLISHED,RELATED -j ACCEPT \n\
 \n\
# Policy routing \n\
ip rule add from all fwmark 0/0x1ffff iif \"$OUT_IFACE\" lookup \"$OUT_IFACE\" 2>/dev/null \n\
ip rule add iif \"$BRIDGE_NAME\" lookup \"$OUT_IFACE\" 2>/dev/null \n\
 \n\
# Add route to table \n\
ip route add table \"$OUT_IFACE\" \"$BRIDGE_NET\" via \"${BRIDGE_IP%/*}\" dev \"$BRIDGE_NAME\" 2>/dev/null \n\
 \n\
# Allow input/output on bridge interface \n\
iptables -I INPUT -i \"$BRIDGE_NAME\" -j ACCEPT \n\
iptables -I OUTPUT -o \"$BRIDGE_NAME\" -j ACCEPT \n\
if [ $# -eq 0 ]; then \n\
    echo \"No command specified for namespace execution.\" \n\
    exit 0 \n\
fi \n\
 \n\
ip netns exec \"$NETNS_NAME\" \"$@\" \n";
	// Create a memfd.
	int memfd = memfd_create("rurima_script", 0);
	if (memfd == -1) {
		perror("memfd_create");
		exit(1);
	}
	// Write the script to the memfd.
	write(memfd, wrapper, strlen(wrapper));
	lseek(memfd, 0, SEEK_SET);
	char script_path[PATH_MAX];
	sprintf(script_path, "/proc/%d/fd/%d", getpid(), memfd);
	int cmd = 0;
	for (int i = 0; i < argc; i++) {
		if (strcmp(argv[i], "--") == 0) {
			cmd = i;
			break;
		}
	}
	char **wrapper_argv = malloc(sizeof(char *) * (argc + 8));
	wrapper_argv[0] = NULL;
	rurima_add_argv(&wrapper_argv, "sh");
	rurima_add_argv(&wrapper_argv, script_path);
	for (int i = 0; i < cmd; i++) {
		rurima_add_argv(&wrapper_argv, argv[i]);
	}
	rurima_add_argv(&wrapper_argv, "rurima");
	rurima_add_argv(&wrapper_argv, "ruri");
	for (int i = cmd + 1; i < argc; i++) {
		rurima_add_argv(&wrapper_argv, argv[i]);
	}
	execvp(wrapper_argv[0], wrapper_argv);
}