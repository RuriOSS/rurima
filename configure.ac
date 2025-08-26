AC_INIT([ruri],[3.8])
AC_PROG_CC
AC_CHECK_TOOL([STRIP],[strip])
AC_CHECK_LIB([pthread],[pthread_create],[PTHREAD_LIB="-lpthread"],[PTHREAD_LIB=""])
AC_CHECK_LIB([cap],[cap_get_flag],[LIBCAP_LIB="-lcap"],[LIBCAP_LIB=""])
AC_CHECK_LIB([seccomp],[seccomp_init],[LIBSECCOMP_LIB="-lseccomp"],[LIBSECCOMP_LIB=""])
CFLAGS=""
AC_ARG_ENABLE([static],[AS_HELP_STRING([--enable-static],[Enable static build])],[enable_static=yes],[enable_static=no])
if test "$enable_static" != "no";then
    AC_MSG_NOTICE([static build enabled])
    STATIC_CFLAGS="-static"
else
    STATIC_CFLAGS=""
fi
AC_DEFUN([CHECK_AND_ADD_CFLAGS],[AC_MSG_CHECKING([whether $CC supports $1])
        CFLAGS_SAVE="$CFLAGS"
        if test "$enable_static" != "no";then
            CFLAGS="$CFLAGS $1 -static"
        else
            CFLAGS="$CFLAGS $1"
        fi
        AC_LINK_IFELSE([AC_LANG_PROGRAM([],[])],
            [AC_MSG_RESULT([yes])
            CFLAGS="$CFLAGS_SAVE $1"],
            [AC_MSG_RESULT([no])
            CFLAGS="$CFLAGS_SAVE"])])
AC_ARG_ENABLE([debug],[AS_HELP_STRING([--enable-debug],[Enable debug log])],[enable_debug=yes],[enable_debug=no])
if test "$enable_debug" != "no";then
    AC_MSG_NOTICE([debug log enabled])
    DEBUG_CFLAGS="-DRURI_DEBUG -DRURIMA_DEBUG"
else
    DEBUG_CFLAGS=""
fi
AC_ARG_ENABLE([dev],[AS_HELP_STRING([--enable-dev],[Enable dev build])],[enable_dev=yes],[enable_dev=no])
if test "$enable_dev" != "no";then
    DEV_CFLAGS="-DRURI_DEV -DRURIMA_DEV"
    STRIP="true"
    AC_MSG_NOTICE([dev build enabled])
    CHECK_AND_ADD_CFLAGS([-g])
    CHECK_AND_ADD_CFLAGS([-O0])
    CHECK_AND_ADD_CFLAGS([-fno-omit-frame-pointer])
    CHECK_AND_ADD_CFLAGS([-Wl,-z,norelro])
    CHECK_AND_ADD_CFLAGS([-Wl,-z,execstack])
    CHECK_AND_ADD_CFLAGS([-fno-stack-protector])
    CHECK_AND_ADD_CFLAGS([-Wall])
    CHECK_AND_ADD_CFLAGS([-Wextra])
    CHECK_AND_ADD_CFLAGS([-pedantic])
    CHECK_AND_ADD_CFLAGS([-Wconversion])
    CHECK_AND_ADD_CFLAGS([-Wno-newline-eof])
    CHECK_AND_ADD_CFLAGS([-Wno-unused-command-line-argument])
    CHECK_AND_ADD_CFLAGS([-Wno-gnu-zero-variadic-macro-arguments])
    CHECK_AND_ADD_CFLAGS([-fsanitize=address,undefined])
    CHECK_AND_ADD_CFLAGS([-Wl,--build-id=sha1])
    CHECK_AND_ADD_CFLAGS([-ffunction-sections])
    CHECK_AND_ADD_CFLAGS([-fdata-sections])
    CHECK_AND_ADD_CFLAGS([-Wl,--gc-sections])
else
    DEV_CFLAGS=""
    CHECK_AND_ADD_CFLAGS([-ftrivial-auto-var-init=pattern])
    CHECK_AND_ADD_CFLAGS([-fcf-protection=full])
    CHECK_AND_ADD_CFLAGS([-flto=auto])
    CHECK_AND_ADD_CFLAGS([-fPIE])
    CHECK_AND_ADD_CFLAGS([-pie])
    CHECK_AND_ADD_CFLAGS([-Wl,-z,relro])
    CHECK_AND_ADD_CFLAGS([-Wl,-z,noexecstack])
    CHECK_AND_ADD_CFLAGS([-Wl,-z,now])
    CHECK_AND_ADD_CFLAGS([-fstack-protector-all])
    CHECK_AND_ADD_CFLAGS([-fstack-clash-protection])
    CHECK_AND_ADD_CFLAGS([-mshstk])
    CHECK_AND_ADD_CFLAGS([-Wno-unused-result])
    CHECK_AND_ADD_CFLAGS([-O2])
    CHECK_AND_ADD_CFLAGS([-Wno-format-security])
    CHECK_AND_ADD_CFLAGS([-Wl,--build-id=sha1])
    CHECK_AND_ADD_CFLAGS([-ffunction-sections])
    CHECK_AND_ADD_CFLAGS([-fdata-sections])
    CHECK_AND_ADD_CFLAGS([-Wl,--gc-sections])
    CHECK_AND_ADD_CFLAGS([-Wl,--strip-all])
    CHECK_AND_ADD_CFLAGS([-U_FORTIFY_SOURCE])
    CHECK_AND_ADD_CFLAGS([-D_FORTIFY_SOURCE=3])
    CHECK_AND_ADD_CFLAGS([-D_FILE_OFFSET_BITS=64])
    CHECK_AND_ADD_CFLAGS([-Wno-unused-command-line-argument])
fi
AC_MSG_CHECKING([for commit id])
RURI_COMMIT_ID=`git rev-parse --short HEAD 2>/dev/null`
if test "$RURI_COMMIT_ID" == "";then
    AC_MSG_RESULT([no])
    RURI_COMMIT_ID="-DRURI_COMMIT_ID=\\\"Unknown\\\""
else
    AC_MSG_RESULT([$RURI_COMMIT_ID])
    RURI_COMMIT_ID="-DRURI_COMMIT_ID=\\\"$RURI_COMMIT_ID\\\""
fi
CFLAGS="$CFLAGS $RURI_COMMIT_ID $COREONLY_CFLAGS $DEBUG_CFLAGS $DEV_CFLAGS $STATIC_CFLAGS $DISABLE_LIBCAP_CFLAGS $DISABLE_LIBSECCOMP_CFLAGS $DISABLE_RURIENV_CFLAGS"
LD_FLAGS="$PTHREAD_LIB $LIBCAP_LIB $LIBSECCOMP_LIB"
AC_SUBST([STRIP],[$STRIP])
AC_SUBST([CC],[$CC])
AC_SUBST([CFLAGS],[$CFLAGS])
AC_SUBST([LD_FLAGS],[$LD_FLAGS])
AC_CONFIG_FILES([config.mk])
AC_OUTPUT
AC_CONFIG_FILES([Makefile])
AC_MSG_NOTICE([CFLAGS: $CFLAGS])
AC_MSG_NOTICE([LD_FLAGS: $LD_FLAGS])
AC_MSG_NOTICE([configuration complete])
