/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Thomas Voss <thomas.voss@canonical.com>
 */

#include <ubuntu/application/ui/ubuntu_application_ui.h>

#include <assert.h>
#include <dlfcn.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void *android_dlopen(const char *filename, int flag);
extern void *android_dlsym(void *handle, const char *symbol);

#ifdef __cplusplus
}
#endif

namespace
{

struct Bridge
{
    static const char* path_to_library()
    {
        return "/system/lib/libubuntu_application_ui.so";
    }
    
    Bridge() : lib_handle(android_dlopen(path_to_library(), RTLD_LAZY))
    {
        assert(lib_handle && "Error loading ubuntu_application_api");
    }

    ~CameraBridge()
    {
        // TODO android_dlclose(libcamera_handle);
    }

    void* resolve_symbol(const char* symbol) const
    {
        return android_dlsym(lib_handle, symbol);
    }
    
    void* lib_handle;
};

static Bridge bridge;

}

#ifdef __cplusplus
extern "C" {
#endif

/**********************************************************/
/*********** Implementation starts here *******************/
/**********************************************************/

#define DLSYM(fptr, sym) if (*(fptr) == NULL) { *(fptr) = (void *) bridge.resolve_symbol(sym); }
    
#define IMPLEMENT_FUNCTION0(return_type, symbol)  \
    return_type symbol()                          \
    {                                             \
        static return_type (*f)() = NULL;         \
        DLSYM(&f, #symbol);                \
        return f();}
    
#define IMPLEMENT_FUNCTION1(return_type, symbol, arg1) \
    return_type symbol(arg1 _1)                        \
    {                                                  \
        static return_type (*f)(arg1) = NULL;          \
        DLSYM(&f, #symbol);                     \
        return f(_1); }

#define IMPLEMENT_VOID_FUNCTION1(symbol, arg1)               \
    void symbol(arg1 _1)                                     \
    {                                                        \
        static void (*f)(arg1) = NULL;                       \
        DLSYM(&f, #symbol);                           \
        f(_1); }

#define IMPLEMENT_FUNCTION2(return_type, symbol, arg1, arg2)    \
    return_type symbol(arg1 _1, arg2 _2)                        \
    {                                                           \
        static return_type (*f)(arg1, arg2) = NULL;             \
        DLSYM(&f, #symbol);                              \
        return f(_1, _2); }

#define IMPLEMENT_VOID_FUNCTION2(symbol, arg1, arg2)            \
    void symbol(arg1 _1, arg2 _2)                               \
    {                                                           \
        static void (*f)(arg1, arg2) = NULL;                    \
        DLSYM(&f, #symbol);                              \
        f(_1, _2); }

#define IMPLEMENT_VOID_FUNCTION3(symbol, arg1, arg2, arg3)      \
    void symbol(arg1 _1, arg2 _2, arg3 _3)                      \
    {                                                           \
        static void (*f)(arg1, arg2, arg3) = NULL;              \
        DLSYM(&f, #symbol);                                     \
        f(_1, _2, _3); }

#define IMPLEMENT_VOID_FUNCTION7(symbol, arg1, arg2, arg3, arg4, arg5, arg6, arg7) \
    void symbol(arg1 _1, arg2 _2, arg3 _3, arg4 _4, arg5 _5, arg6 _6, arg7 _7) \
    {                                                                   \
        static void (*f)(arg1, arg2, arg3, arg4, arg5, arg6, arg7) = NULL; \
        DLSYM(&f, #symbol);                                             \
        f(_1, _2, _3, _4, _5, _6, _7); }


    IMPLEMENT_VOID_FUNCTION2(ubuntu_application_ui_init, int, char**);
    IMPLEMENT_FUNCTION0(StageHint, ubuntu_application_ui_setup_get_stage_hint);
    IMPLEMENT_FUNCTION0(FormFactorHint, ubuntu_application_ui_setup_get_form_factor_hint);
    IMPLEMENT_VOID_FUNCTION1(ubuntu_application_ui_start_a_new_session, const char*);
    IMPLEMENT_VOID_FUNCTION7(ubuntu_application_ui_create_surface, ubuntu_application_ui_surface*, const char*, int, int, SurfaceRole, input_event_cb, void*);

#ifdef __cplusplus
}
#endif
