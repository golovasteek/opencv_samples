#pragma once
#include <EGL/egl.h>
#include <EGL/eglext.h>

typedef EGLStreamKHR (*eglCreateStreamKHR_type)(
    EGLDisplay    dpy,
    const EGLint *attrib_list);

typedef EGLBoolean (*eglDestroyStreamKHR_type)(
    EGLDisplay dpy,
    EGLStreamKHR);

extern eglCreateStreamKHR_type eglCreateStreamKHR;
extern eglDestroyStreamKHR_type eglDestroyStreamKHR;

void EGLStreamExtInit();