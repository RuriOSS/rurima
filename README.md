[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.14099730.svg)](https://doi.org/10.5281/zenodo.14099730)

[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/RuriOSS/rurima)

[![Build release](https://github.com/RuriOSS/rurima/actions/workflows/build.yml/badge.svg)](https://github.com/RuriOSS/rurima/actions/workflows/build.yml)

![](https://img.shields.io/badge/Powered%20By-GNU%20C-00d000?style=flat&labelColor=gray&logo=C)

# About:
So, what is rurima?       
The enhanced version of ruri.          
ruri only focus on running container, but rurima can also provide the function of getting rootfs image and backup/restore.          
And it will be a more powerful container manager in the fulture.            
With the `docker` and `lxc` subcommand of rurima, you can search & get & unpack images from dockerhub or LXC mirror easily.       
# Not Only Ruri Container Manager:
Rurima was planned to be the ruri container manager, but as rurima has a full integration of ruri now, you can just use it as a more powerful version of ruri, although the container manager function is still WIP.               
For more info about ruri, see [ruri](https://github.com/Moe-hacker/ruri)      
# Note & WIP:
This project does not follow OCI and can only be a `PARTIAL` replacement of docker, this project is still under development.       
# Terms of Use:
See [TERMS_OF_USE.md](TERMS_OF_USE.md)
# For Android user:
You might need to root your phone before using rurima, some container might not work properly with proot.  

# What's new:
You can use `rurima ota` to update rurima automatically now.      
And, we are working for the migration from daijin, so rurima will fully replace it in the future.      
# WARNING:      
```
* Your warranty is void.
* I am not responsible for anything that may happen to your device by using this program.
* You do it at your own risk and take the responsibility upon yourself.
* This project is open source, you can make your own fork/rewrite but not to blame the author.
* Docker is a registered trademark of Docker, Inc. This program has no relationship with it.
* This program has no Super Cow Powers.
```
# Download:
You can get rurima binary (staticly linked) for arm64, armv7, armhf, riscv64, i386, loong64, s390x, ppc64le and x86_64 from the release page.     
Or run the follwing command to get rurima to ./rurima and ./rurima-dbg(debug version):     
```sh
. <(curl -sL https://get.ruri.zip/rurima)
```   
# The new pull subcommand:
It's a wrap of docker/lxc pull subcommand.      
For example:      
```sh
rurima pull alpine:edge ./test
```
```sh
rurima pull whyour/qinglong ./test
```
```sh
rurima pull ubuntu ./test
```
It will search lxc image first, if not found, it will auto try to pull rootfs from dockerhub.      
# Example usage of docker subcommand:
Get `alpine` image, use tag `edge`, save to `./test`     
```sh
rurima docker pull -i alpine -t edge -s ./test
```     
Get `whyour/qinglong` docker image.      
```sh
rurima docker pull -i whyour/qinglong -s ./test
```
Get `ubuntu` docker image.      
```sh
rurima docker pull -i ubuntu -s ./test
```
Search for image `ubuntu`:      
```sh
rurima docker search -i ubuntu
```
Search tag for `ubuntu`:      
```sh
rurima docker tag -i ubuntu
```
Use docker mirror `dockerpull.org`
```sh
rurima docker pull -m dockerpull.org -i ubuntu -s ./test
```
Try every mirrorlist:      
```sh
rurima docker pull -T -i ubuntu -s ./test
```
Try fallback mode:      
```sh
rurima docker pull -T -f -i ubuntu -s ./test
```
# Example usage of lxc subcommand:
Pull `alpine` version `edge`.      
```sh
rurima lxc pull -o alpine -v edge -s ./test
```
Use `mirrors.bfsu.edu.cn/lxc-images` as mirror:   
```sh
rurima lxc pull -m mirrors.bfsu.edu.cn/lxc-images -o alpine -v edge -s ./test
```    
# About suid or caps:
Rurima does not allow to set any suid/sgid (with root) or capability on it, it will check it in main() and error() if detected these unsafe settings.      
So, please always use sudo instead.     
# Reporting bugs:
Please use the debug version(rurima-dbg) in release to get debug logs, and please tell me the command you run to cause the unexpected behavior you think!                     
# NOTICE:
This program is not official tool of docker or dockerhub, you can report bugs here, but this program has no relation with docker.      
Docker is a registered trademark of Docker, Inc.      
# Dependent:   
rurima needs tar, xz, gzip, file, you can find these static binary for aarch64, armv7, x86_64, i386 or riscv64 in：      
[tar-static](https://github.com/Moe-sushi/tar-static)      
[xz-static](https://github.com/Moe-sushi/xz-static)     
[gzip-static](https://github.com/Moe-sushi/gzip-static)     
[file-static](https://github.com/Moe-sushi/file-static)      
rurima need `curl` for downloading images, you can find a third party build here (Thanks stunnel) [stunnel/static-curl](https://github.com/stunnel/static-curl).      
rurima need `sha256sum` for checking downloaded file from dockerhub, use `--fallback` option can disable this.      
rurima need `jq` for parsing json, you can find it in [jq](https://github.com/jqlang).      
rurima also need proot if you are unpacking rootfs without root privileges, you can find it by the following way:      
First, you know your cpu arch, then, for example for aarch64:      
```sh
curl -sL https://mirrors.tuna.tsinghua.edu.cn/alpine/edge/testing/aarch64/ | grep proot-static
```
You got:      
```
<tr><td class="link"><a href="proot-static-5.4.0-r1.apk" title="proot-static-5.4.0-r1.apk">proot-static-5.4.0-r1.apk</a></td><td class="size">118.8 KiB</td><td class="date">25 Oct 2024 19:10:30 +0000</td></tr>
```
So that you download:      
```
https://mirrors.tuna.tsinghua.edu.cn/alpine/edge/testing/aarch64/proot-static-5.4.0-r1.apk
```
and finally, tar -xvf *.apk to unpack it. So you got proot.static, rename it to proot and put it in your $PATH.           
# TODO:
Manage ruri containers and configs.   
Config support, a bit like Dockerfile.   
....
# Usage:
```
Usage: rurima [subcommand] [options]
Subcommands:
  docker: DockerHub support.
  lxc: LXC mirror support.
  pull: Pull rootfs.
  unpack: Unpack rootfs.
  backup: Backup rootfs.
  ruri: Built-in ruri command.
  help: Show help message.
Options:
  -h, --help: Show help message.
```
---------

<p align="center">「僕らタイムフライヤー</p>
<p align="center">時を駆け上がるクライマー</p>
<p align="center">時のかくれんぼ</p>
<p align="center">はぐれっこはもういやなんだ」</p>
