/*
  Simple DirectMedia Layer
  Copyright (C) 1997-2017 Sam Lantinga <slouken@libsdl.org>

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
*/
#include "SDL_internal.h"

#if defined(SDL_VIDEO_DRIVER_VITA) && defined(SDL_VIDEO_VITA_VGL)
#include <stdlib.h>
#include <string.h>

#include "SDL3/SDL_error.h"
#include "SDL_vitavideo.h"
#include "SDL3/SDL_hints.h"

#define MEMORY_VITAGL_THRESHOLD 12 * 1024 * 1024

// only one instance of vitaGL can run at the same time
static int vgl_initialized = 0;

bool VITA_GLES_LoadLibrary(SDL_VideoDevice *_this, const char *path)
{
    if (!vgl_initialized) {
        // init vitaGL once and never deinit it again until the driver dies
        enum SceGxmMultisampleMode gxm_ms;

        switch (_this->gl_config.multisamplesamples) {
            case 2:  gxm_ms = SCE_GXM_MULTISAMPLE_2X; break;
            case 4:
            case 8:
            case 16: gxm_ms = SCE_GXM_MULTISAMPLE_4X; break;
            default: gxm_ms = SCE_GXM_MULTISAMPLE_NONE; break;
        }

        sceSysmoduleLoadModule(SCE_SYSMODULE_IME);
        vglInitExtended(0, 960, 544, MEMORY_VITAGL_THRESHOLD, gxm_ms);
        vgl_initialized = 1;
    }

    _this->gl_config.driver_loaded = 1;
    return true;
}

SDL_FunctionPointer VITA_GLES_GetProcAddress(SDL_VideoDevice *_this, const char *proc)
{
    if (!proc || !*proc)
    {
        return NULL;
    }

    return vglGetProcAddress(proc);
}

void VITA_GLES_UnloadLibrary(SDL_VideoDevice *_this)
{
    if (vgl_initialized)
    {
        vgl_initialized = 0;
    }
    _this->gl_config.driver_loaded = 0;
}

SDL_GLContext VITA_GLES_CreateContext(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_WindowData *wdata = (SDL_WindowData *) window->internal;

    _this->gl_config.red_size = 8;
    _this->gl_config.green_size = 8;
    _this->gl_config.blue_size = 8;
    _this->gl_config.alpha_size = 8;
    _this->gl_config.depth_size = 32;
    _this->gl_config.stencil_size = 8;

    // force context version to what we actually support
    _this->gl_config.major_version = 2;
    _this->gl_config.minor_version = 0;
    _this->gl_config.profile_mask = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
    _this->gl_config.accelerated = 1;

    wdata->uses_gles = true;
    window->flags |= SDL_WINDOW_FULLSCREEN;

    // return a dummy pointer and pretend that it's a GL context
    return (SDL_GLContext)&vgl_initialized;
}

bool VITA_GLES_MakeCurrent(SDL_VideoDevice *_this, SDL_Window *window, SDL_GLContext context)
{
    if (!vgl_initialized) {
        SDL_SetError("vitaGL is not initialized");
        return -1;
    }

    glFinish();
    glClearColor(0.f, 0.f, 0.f, 0.f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
    glFinish();

    return true;
}

bool VITA_GLES_SetSwapInterval(SDL_VideoDevice *_this, int interval)
{
    if (!vgl_initialized) {
        return SDL_SetError("vitaGL is not initialized");
    }
    _this->gl_data->swapinterval = interval;
    eglSwapInterval(0, interval);
    return true;
}

bool VITA_GLES_GetSwapInterval(SDL_VideoDevice *_this, int *interval)
{
    if (!vgl_initialized) {
        SDL_SetError("vitaGL is not initialized");
        return -1;
    }
    *interval = _this->gl_data->swapinterval;
    return true;
}

bool VITA_GLES_SwapWindow(SDL_VideoDevice *_this, SDL_Window *window)
{
    SDL_VideoData *videodata = _this->internal;

    if (!vgl_initialized) {
        SDL_SetError("vitaGL is not initialized");
        return -1;
    }

    if (videodata->ime_active) {
        sceImeUpdate();
    }

    vglSwapBuffers(GL_TRUE);

    return true;
}

bool VITA_GLES_DestroyContext(SDL_VideoDevice *_this, SDL_GLContext context)
{
    if (!vgl_initialized) {
        SDL_SetError("vitaGL is not initialized");
        return false;
    }

    glFinish();

    return true;
}

void VITA_GLES_DefaultProfileConfig(SDL_VideoDevice *_this, int *mask, int *major, int *minor)
{
    *mask = SDL_GL_CONTEXT_PROFILE_COMPATIBILITY;
    *major = 2;
    *minor = 0;
}

#endif /* SDL_VIDEO_DRIVER_VITA && SDL_VIDEO_VITA_VGL */

/* vi: set ts=4 sw=4 expandtab: */
