/*
 * Copyright (C) 2008 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cutils/ashmem.h>

/*
 * Implementation of the user-space ashmem API for devices, which have our
 * ashmem-enabled kernel. See ashmem-sim.c for the "fake" tmp-based version,
 * used by the simulator.
 */
#define LOG_TAG "ashmem"

#include <errno.h>
#include <fcntl.h>
#include <linux/ashmem.h>
#include <pthread.h>
#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/sysmacros.h>
#include <sys/types.h>
#include <unistd.h>
#include <log/log.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <utils/Compat.h>

#ifndef HAVE_MEMFD_CREATE
#if ((ULONG_MAX) == (UINT_MAX))
#define __NR_memfd_create  (385)
#else
#define __NR_memfd_create 279
#endif
#define SYS_memfd_create __NR_memfd_create
static inline int
memfd_create(const char *name, unsigned int flags)
{
   return syscall(SYS_memfd_create, name, flags);
}
#endif

int ashmem_create_region(const char* name, size_t size) {

    int fd = memfd_create(name, 0);
    if (fd < 0) {
      ALOGE("JDB: memfd_create failed! %i\n", fd);
      return fd;
    }
    if (TEMP_FAILURE_RETRY(ftruncate(fd, size)) == -1) {
      ALOGE("JDB: ftruncate failed!\n");
      close(fd);
      return -1;
    }

    return fd;
}

int ashmem_set_prot_region(int /*fd*/, int /*prot*/) {
    return 0;
}

int ashmem_pin_region(int /*fd*/, size_t /*offset*/, size_t /*len*/) {
    return 0 /*ASHMEM_NOT_PURGED*/;
}

int ashmem_unpin_region(int /*fd*/, size_t /*offset*/, size_t /*len*/) {
    return 0 /*ASHMEM_IS_UNPINNED*/;
}

int ashmem_get_size_region(int fd)
{
    struct stat buf;
    int result = fstat(fd, &buf);
    if (result == -1) {
        return -1;
    }

    /*
     * Check if this is an "ashmem" region.
     * TODO: This is very hacky, and can easily break.
     * We need some reliable indicator.
     */
    if (!(buf.st_nlink == 0 && S_ISREG(buf.st_mode))) {
        errno = ENOTTY;
        return -1;
    }

    return buf.st_size;
}

int ashmem_valid(int /*fd*/)
{
    return  1;
}

