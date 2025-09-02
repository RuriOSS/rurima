#!/bin/sh
# This is a wrapper script for setting up a network namespace with veth pairs and a bridge
# Default settings
NETNS_NAME="ruri_netns"
VETH_NS="ruri_veth"
VETH_HOST="ruri_hostveth"
BRIDGE_NAME="ruri_br0"
BRIDGE_IP="192.168.100.1/24"
BRIDGE_NET="192.168.100.0/24"
NS_IP="192.168.100.2/24"
NS_GATEWAY="192.168.100.1"
OUT_IFACE="wlan0"
FORCE="false"
# Parse args
i=0
for arg in "$@"; do
    i=$((i + 1))
    case $arg in
        --ns-ip=*) NS_IP="${arg#*=}" ;;
        --ns-gateway=*) NS_GATEWAY="${arg#*=}" ;;
        --netns-name=*) NETNS_NAME="${arg#*=}" ;;
        --veth-ns=*) VETH_NS="${arg#*=}" ;;
        --veth-host=*) VETH_HOST="${arg#*=}" ;;
        --bridge-name=*) BRIDGE_NAME="${arg#*=}" ;;
        --bridge-ip=*)
            BRIDGE_IP="${arg#*=}"
            BRIDGE_NET="${BRIDGE_IP%/*}/24"
            ;;
        --out-iface=*) OUT_IFACE="${arg#*=}" ;;
        --force) FORCE="true" ;;
        --help)
            echo "Usage: $0 [--netns-name=NAME] [--veth-ns=NAME] [--veth-host=NAME] [--bridge-name=NAME] [--bridge-ip=IP/CIDR] [--out-iface=IFACE]"
            exit 0
            ;;
        *) shift $((i-1)); break;;
    esac
done

# Check if devices exist
ip netns list | grep -qw "$NETNS_NAME" && ip link show "$BRIDGE_NAME" > /dev/null 2>&1 && ip link show "$VETH_HOST" > /dev/null 2>&1

if [ $? -eq 0 ] && [ "$FORCE" != "true" ]; then
    # Maybe show warning here?
    if [ $# -eq 0 ]; then
        echo "No command specified for namespace execution."
        exit 0
    fi
    ip netns exec "$NETNS_NAME" "$@"
    exit 0
fi

# Cleanup
ip netns del "$NETNS_NAME" 2>/dev/null
ip link del "$BRIDGE_NAME" 2>/dev/null
ip link del "$VETH_HOST" 2>/dev/null
sleep 0.3

# Enable IP forwarding
echo 1 > /proc/sys/net/ipv4/ip_forward
sysctl -w net.ipv6.conf.all.forwarding=1 > /dev/null 2>&1

# Create network namespace
ip netns add "$NETNS_NAME" || exit 1

# enable loopback
ip netns exec "$NETNS_NAME" ip link set up dev lo

# Create veth pair
ip link add "$VETH_HOST" type veth peer name "$VETH_NS" || exit 1

# Move veth to namespace
ip link set "$VETH_NS" netns "$NETNS_NAME" || exit 1

# Enable interfaces
ip link set "$VETH_HOST" up
ip netns exec "$NETNS_NAME" ip link set up dev "$VETH_NS"

# Create bridge device
ip link add "$BRIDGE_NAME" type bridge || exit 1
ip link set "$BRIDGE_NAME" up
ip link set "$VETH_HOST" master "$BRIDGE_NAME"
ip addr add "$BRIDGE_IP" dev "$BRIDGE_NAME"


# Configure IP and routing for namespace
ip netns exec "$NETNS_NAME" ip addr add "$NS_IP" dev "$VETH_NS"
ip netns exec "$NETNS_NAME" ip route add default via "$NS_GATEWAY" dev "$VETH_NS"
# Enable NAT
iptables -t nat -D POSTROUTING -s "$BRIDGE_NET" -o "$OUT_IFACE" -j MASQUERADE 2>/dev/null
iptables -D FORWARD -i "$BRIDGE_NAME" -o "$OUT_IFACE" -j ACCEPT 2>/dev/null
iptables -D FORWARD -i "$OUT_IFACE" -o "$BRIDGE_NAME" -m state --state ESTABLISHED,RELATED -j ACCEPT 2>/dev/null
iptables -t nat -I POSTROUTING -s "$BRIDGE_NET" -o "$OUT_IFACE" -j MASQUERADE

# Enable forwarding
iptables -I FORWARD -i "$BRIDGE_NAME" -o "$OUT_IFACE" -j ACCEPT
iptables -I FORWARD -i "$OUT_IFACE" -o "$BRIDGE_NAME" -m state --state ESTABLISHED,RELATED -j ACCEPT

# Policy routing
ip rule add from all fwmark 0/0x1ffff iif "$OUT_IFACE" lookup "$OUT_IFACE" 2>/dev/null
ip rule add iif "$BRIDGE_NAME" lookup "$OUT_IFACE" 2>/dev/null

# Add route to table
ip route add table "$OUT_IFACE" "$BRIDGE_NET" via "${BRIDGE_IP%/*}" dev "$BRIDGE_NAME" 2>/dev/null

# Allow input/output on bridge interface
iptables -I INPUT -i "$BRIDGE_NAME" -j ACCEPT
iptables -I OUTPUT -o "$BRIDGE_NAME" -j ACCEPT
if [ $# -eq 0 ]; then
    echo "No command specified for namespace execution."
    exit 0
fi

ip netns exec "$NETNS_NAME" "$@"