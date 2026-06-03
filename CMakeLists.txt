cmake_minimum_required(VERSION 3.10)
project(rurima C)

set(CMAKE_C_STANDARD 11)

set(RURIMA_CORE_SOURCES
    src/net.c
    src/checkdep.c
    src/dockerhub.c
    src/exec.c
    src/termux.c
    src/info.c
    src/lxcmirror.c
    src/shared.c
    src/signal.c
    src/archive.c
    src/subcommand.c
    src/main.c
    src/ssfx/ssfx.c
    src/easteregg/daijin.c
    src/catsh/src/catsh.c
    # ruri core
    src/ruri/src/caplist.c
    src/ruri/src/chroot.c
    src/ruri/src/cprintf.c
    src/ruri/src/info.c
    src/ruri/src/rurienv.c
    src/ruri/src/rurifetch.c
    src/ruri/src/seccomp.c
    src/ruri/src/signal.c
    src/ruri/src/umount.c
    src/ruri/src/unshare.c
    src/ruri/src/rootless.c
    src/ruri/src/mount.c
    src/ruri/src/k2v.c
    src/ruri/src/k2v3.c
    src/ruri/src/watchdog.c
    src/ruri/src/elf-magic.c
    src/ruri/src/config.c
    src/ruri/src/cgroup.c
    src/ruri/src/passwd.c
    src/ruri/src/ps.c
    src/ruri/src/ruri.c
    # ruri easteregg
    src/ruri/src/easteregg/action.c
    src/ruri/src/easteregg/nekofeng.c
    src/ruri/src/easteregg/layer.c
    src/ruri/src/easteregg/typewriter.c
)

add_executable(rurima ${RURIMA_CORE_SOURCES})

# find del libcap
find_library(CAP_LIB NAMES cap)
if(NOT CAP_LIB)
    message(FATAL_ERROR "libcap not found! Please install libcap or libcap-dev")
endif()

# find dep libseccomp
find_library(SECCOMP_LIB NAMES seccomp)
if(NOT SECCOMP_LIB)
    message(FATAL_ERROR "libseccomp not found! Please install libseccomp or libseccomp-dev")
endif()

target_link_libraries(rurima 
    pthread 
    m 
    ${CAP_LIB} 
    ${SECCOMP_LIB}
)

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
    add_custom_command(TARGET rurima POST_BUILD
        COMMAND ${CMAKE_STRIP} rurima
        COMMENT "Stripping binary..."
    )
endif()

add_custom_target(up
    COMMAND cd ${CMAKE_SOURCE_DIR}/src/catsh && git pull origin main
    COMMAND cd ${CMAKE_SOURCE_DIR}/src/ssfx && git pull origin main
    COMMAND cd ${CMAKE_SOURCE_DIR}/src/ruri && git pull origin main
    COMMENT "Updating submodules..."
)
