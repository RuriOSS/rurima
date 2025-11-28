# Undocumented part:
The `load`, `run`, `sfx` and `netns` subcommand is WIP, it should not be used by users.    
# Image source:
rurima currently supports both lxc-mirror and dockerhub as container image source.      
# Unified image puller:
rurima has a unified `pull` subcommand that can pull images easily.    
## Basic usage:
```sh
# pull alpine:edge to ./test:
rurima pull alpine:edge ./test
# pull hello-world to ./hello:
rurima pull hello-world ./hello
```
it's a bit like cmdline of docker, but, it will try lxc-mirror first, and auto switch to dockerhub if the image is not in lxc-mirror.       
## Full options:
```
Usage: rurima pull <options> [image]:[version] [savedir]
Options:
  -h, --help: Show help message.
  -m, --mirror: Mirror URL.
  -a, --arch: Architecture.
  -d, --docker: Only search dockerhub for image.
  -f, --fallback: Fallback mode, only for docker image.
```
All options should be used when you really need.     
## Examples:
```sh
# pull alpine:edge to ./test, but using dockerhub as source:
rurima pull --docker alpine:edge ./test

# pull hello-world to ./hello, but pull amd64 with fallback mode:
rurima pull --arch amd64 --fallback hello-world ./hello

# pull alpine:edge to ./test from bfsu lxc mirror:
rurima pull --mirror mirrors.bfsu.edu.cn/lxc-images alpine:edge ./test
```
# Utility commands:
## Backup and restore
```sh
# backup container at ./alpine to ./alpine.tar:
rurima backup -d ./alpine -f ./alpine.tar

# restore alpine.tar to ./alpine:
rurima restore -f ./alpine.tar -d ./alpine
```
## Other utility commands:
```sh
# update rurima:
rurima ota

# check for dependencies:
rurima dep
```
# Call built-in ruri:
ruri is a lightweight container runtime, it can be called directly by:
```sh
# Show ruri version info
rurima r -v

# Run container at ./alpine:
rurima r ./alpine
```
# docker subcommand:
The docker subcommand is an unofficial tool to get docker image from dockerhub.      
## Full usage:
```
Usage: rurima docker [subcommand] [options]
Subcommands:
  search: Search images from DockerHub.
  tag:    Search tags from DockerHub.
  pull:   Pull image from DockerHub.
  config: Get config of image from DockerHub.
  arch:   Search architecture of image from DockerHub.
  help:   Show help message.
Options:
  -i, --image: Image name.
  -t, --tag: Tag of image.
  -a, --arch: Architecture of image.
  -s, --savedir: Save directory of image.
  -p, --page_size: Page size of search.
  -m, --mirror: Mirror of DockerHub.
  -r, --runtime: runtime of container, support [ruri/proot/chroot].
  -q, --quiet: Quiet mode.
  -f, --fallback: Fallback mode.
  -T, --try-mirrors <mirror>: Try mirrors.
  -S, --start-at [num]: Start pulling layer at [num] when pulling image.
  -n, --no-progress: Do not show progress.

Note: please remove `https://` prefix from mirror url.
For example: `-m registry-1.docker.io`
You can add your perfered mirrors for `-T` option to try them first, for example: `-T hub.xdark.top -T dockerpull.org`
```
## Note:
`-i` and `-t` is required options for `config` and `pull` subcommand.    
`-i` is also used for `search` subcommand for image name.    
# lxc subcommand:
## Full usage:
```
Usage: rurima lxc [subcommand] [options]
Subcommands:
  pull: Pull image from LXC image server.
  list: List images from LXC image server.
  search: Search images from LXC image server.
  arch: Search architecture of images from LXC image server.
  help: Show help message.
Options:
  -m, --mirror: Mirror of LXC image server.
  -o, --os: OS of image.
  -v, --version: Version of image.
  -a, --arch: Architecture of image.
  -t, --type: Type of image.
  -s, --savedir: Save directory of image.
  -n, --no-progress: Do not show progress.

Note: please remove `https://` prefix from mirror url.
For example: `-m images.linuxcontainers.org`
```
## Note:
`-o`, `-s` and `-v` is required options for `pull` subcommand.