/*
 * Copyright (C) 2012 Canonical Ltd
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
 *
 * Authored by: Michael Frey <michael.frey@canonical.com>
 */

#include <dlfcn.h>
#include <stddef.h>

extern void *android_dlopen(const char *filename, int flag);

static void *_libc = NULL;

static void _init_androidc()
{
    _libc = (void *) android_dlopen("/system/lib/libc.so", RTLD_LAZY);
}

void init_hybris()
{
    if (_libc == NULL)
        _init_androidc();
}
