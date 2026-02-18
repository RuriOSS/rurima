#!/data/data/com.termux/files/usr/bin/bash
check_command() {
  if [[ -e "$(command -v $1 2>&1)" ]]; then
    printf "\033[32mCommand $1 found\033[0m\n"
  else
    printf "\033[31mError: $1 not found\033[0m\n"
    exit 1
  fi
}
Ok() {
  if [[ $1 != 0 ]]; then
    printf "\033[31mError: $2\033[0m\n"
    exit 1
  fi
}
build_debian_rootfs() {
  rurima pull debian:trixie ./debian
  Ok $? "Failed to pull rootfs"
  cat <<EEOF >debian/init.sh
#!/bin/bash
rm /etc/resolv.conf
echo nameserver 1.1.1.1 > /etc/resolv.conf
apt update
apt install -y linux-image-arm64
apt install -y netplan.io
apt install -y chrony
systemctl enable chrony

cat <<EOF > /etc/netplan/90-default.yaml
network:
    version: 2
    ethernets:
all-en:
    match:
name: en*
    dhcp4: false

    addresses:
      - 192.168.10.2/24
    routes:
      - to: default
via: 192.168.10.1
    nameservers:
  addresses: [8.8.8.8]
    dhcp6: true
    dhcp6-overrides:
use-domains: true
all-eth:
    match:
name: eth*
    dhcp4: true
    dhcp4-overrides:
use-domains: true
    dhcp6: true
    dhcp6-overrides:
use-domains: true
EOF

cat <<EOF > /usr/bin/net.sh
sleep 2
ip route del default
ip addr add 192.168.10.2/24 dev enp0s2
ip link set enp0s2 up
gvforwarder & 2>&1 >/dev/null
sleep 2
ip route add default via 192.168.127.1
EOF

chmod 777 /usr/bin/net.sh

cat <<EOF >/etc/rc.local
#!/bin/bash
sleep 3
net.sh
mkdir -p /mnt/shared
mount -t virtiofs shared /mnt/shared
exit 0
EOF

chmod 777 /etc/rc.local
echo "root:xxxx" | sudo chpasswd

EEOF
  chmod 777 debian/init.sh

  rurima r -p ./debian bash /init.sh
  cp debian/vmlinuz .
  cp debian/initrd.img .
  cd debian
  rurima r -U .
  cd ..
  cd debian
  rm init.sh
  cp ${GVF} ./usr/bin/
  chmod 777 usr/bin/gvforwarder
  mkfs.erofs ../data.img .
  cd ..
}
migrate_to_ext4() {
  # Migrate erofs image $1 to ext4 image $2
  # $1 will be removed
  # As obbo broke ext4 loop mount in their f**king tampered kernel
  FROM="$1"
  TO="$2"
  rm kernel
  wget https://github.com/polygraphene/gunyah-on-sd-guide/releases/download/v0.0.3/kernel
  Ok $? "Failed to download kernel"
  rurima ruri -U ./alpine >/dev/null 2>&1
  rm -rf ./alpine
  rurima pull alpine:edge ./alpine
  Ok $? "Failed to pull rootfs"
  cat <<EOF >alpine/init.sh
#!/bin/sh
mount -t proc proc /proc
mount -t sysfs sysfs /sys
mount -t tmpfs tmpfs /dev
mdev -s
mount /dev/vdb /mnt/from
mount /dev/vdc /mnt/to
cd /mnt/from
cp -a -r ./* /mnt/to/
sync
sleep 2s
sync
printf "\n\n\033[1;32m///// <OK>\033[0m\n\n"
/sbin/poweroff -f
sleep 3
EOF
  chmod 777 alpine/init.sh
  mkdir -p alpine/mnt/from
  mkdir -p alpine/mnt/to
  rm alpine.img
  cd alpine
  mkfs.erofs ../alpine.img .
  cd ..
  unset LD_PRELOAD
  crosvm run \
    --disable-sandbox \
    --protected-vm-without-firmware \
    --swiotlb 512 \
    --params 'loglevel=4 root=/dev/vda init=/init.sh' \
    --mem 8192 --cpus 4 \
    --socket vm.sock \
    --vsock 3 \
    --block alpine.img,root \
    --block ${FROM} \
    --block ${TO} \
    kernel
  rurima ruri -U ./alpine
  rm -rf ./alpine
  rm -rf alpine.img kernel.img ${FROM}
}
start_vm() {
  ifname=crosvm_tap
  if [ ! -d /sys/class/net/$ifname ]; then
    # https://crosvm.dev/book/devices/net.html
    ip tuntap add mode tap user root vnet_hdr crosvm_tap
    ip addr add 192.168.10.1/24 dev crosvm_tap
    ip link set crosvm_tap up

    # routing
    sysctl net.ipv4.ip_forward=1
    HOST_DEV=$(ip route get 8.8.8.8 | awk -- '{printf $5}')
    iptables -t nat -A POSTROUTING -o "${HOST_DEV}" -j MASQUERADE
    iptables -A FORWARD -i "${HOST_DEV}" -o crosvm_tap -m state --state RELATED,ESTABLISHED -j ACCEPT
    iptables -A FORWARD -i crosvm_tap -o "${HOST_DEV}" -j ACCEPT

    # the main route table needs to be added
    ip rule add from all lookup main pref 1
  fi
  rm ../network.sock
  killall -9 gvproxy
  gvproxy -listen vsock://:1024 -listen unix://$(pwd)/../network.sock &
  sleep 2
  curl --unix-socket /data/data/com.termux/files/home/network.sock http:/unix/services/forwarder/expose -X POST -d '{"local":":22","remote":"192.168.127.2:22"}'
  ulimit -l unlimited
  rm vm.sock
  killall -9 crosvm
  unset LD_PRELOAD
  /apex/com.android.virt/bin/crosvm run \
    --disable-sandbox \
    --protected-vm-without-firmware \
    --swiotlb 512 \
    --params 'loglevel=4 root=/dev/vda rw' \
    --shared-dir /sdcard/shared:shared:type=fs \
    --mem 8192 --cpus 4 \
    --net tap-name=crosvm_tap \
    --socket vm.sock \
    --vsock 3 \
    --initrd initrd.img \
    --block debian.img,root \
    vmlinuz
}
if [ ! -n "$BASH" ]; then
  printf "\033[31mThis is a bash script\033[0m\n"
  exit 1
fi
if [[ ! -e /apex/com.android.virt/bin/crosvm ]]; then
  printf "\033[31mError: crosvm not found, make sure that your device supports virtualization\033[0m\n"
  exit 1
fi
if [[ ! -e /dev/kvm && ! -e /dev/gunyah && ! -e /dev/gzvm ]]; then
  printf "\033[31mError: KVM, GenieZone or Gunyah are not found, make sure that your device supports virtualization\033[0m\n"
  exit 1
fi
if [[ -e debian.img ]]; then
  start_vm
  exit 0
fi
check_command mkfs.erofs
check_command wget
check_command jq
check_command xz
check_command gzip
check_command file
check_command curl
check_command git
check_command make
check_command go
check_command rurima
git clone https://github.com/containers/gvisor-tap-vsock
cd gvisor-tap-vsock
GOARCH=arm64 make
if [[ -e bin/gvforwarder && -e bin/gvproxy ]]; then
  export GVF=$(realpath bin/gvforwarder)
  cp bin/gvproxy $PREFIX/bin
else
  printf "\033[31mError: failed to compile gvisor-tap-vsock"
fi
cd ..
build_debian_rootfs
dd if=/dev/zero of=debian.img bs=1G count=8
mkfs.ext4 debian.img
migrate_to_ext4 data.img debian.img
mkdir /sdcard/shared
printf "\n\n"
printf "\033[1;32mDefault root password is: xxxx\033[0m"
printf "\n\n"
echo 3
sleep 1
echo 2
sleep 1
echo 1
sleep 1
echo start......
start_vm
