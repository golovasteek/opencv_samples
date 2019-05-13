#pragma once
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdexcept>

namespace egl {

typedef EGLStreamKHR (*eglCreateStreamKHR_type)(
    EGLDisplay    dpy,
    const EGLint *attrib_list);

typedef EGLBoolean (*eglDestroyStreamKHR_type)(
    EGLDisplay dpy,
    EGLStreamKHR);

class Error : public std::runtime_error
{
public:
    explicit Error(const char* what)
        : std::runtime_error(what)
    {}
};

class Framework {
public:
    Framework();
    
    const eglCreateStreamKHR_type eglCreateStreamKHR;
    const eglDestroyStreamKHR_type eglDestroyStreamKHR;
};

class Display {
public:
    Display();
    ~Display();
    EGLDisplay get() const { return display_; }

private:
    const EGLDisplay display_;
};

class Stream {
public:
    Stream(const Framework& f, const Display& d);
    ~Stream();
    EGLStreamKHR get() { return stream_; };

private:
    const Framework& framework_;
    const Display& display_;
    EGLStreamKHR stream_;
};
}