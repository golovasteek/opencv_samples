#pragma once
#include <EGL/egl.h>
#include <EGL/eglext.h>

#include <stdexcept>

namespace egl {

typedef EGLStreamKHR (*eglCreateStreamKHR_type)(
    EGLDisplay, const EGLint*);

typedef EGLBoolean (*eglDestroyStreamKHR_type)(
    EGLDisplay, EGLStreamKHR);

typedef EGLBoolean (*eglQueryStreamKHR_type) (
    EGLDisplay, EGLStreamKHR,
    EGLenum, EGLint*);

class Error : public std::runtime_error
{
public:
    explicit Error(const std::string what)
        : std::runtime_error(what)
    {}
};

class Framework {
public:
    Framework();
    
    const eglCreateStreamKHR_type eglCreateStreamKHR;
    const eglDestroyStreamKHR_type eglDestroyStreamKHR;
    const eglQueryStreamKHR_type eglQueryStreamKHR;
};

class Display {
public:
    Display();
    ~Display();
    EGLDisplay get() const { return display_; }

private:
    const EGLDisplay display_;
};

class Socket {
public:
    Socket(const std::string& socketPath);
    ~Socket();

    int get() const { return fd_; };

private:
    int fd_;
};

class Stream {
public:
    enum class Endpoint {
        consumer = EGL_STREAM_CONSUMER_NV,
        producer = EGL_STREAM_PRODUCER_NV
    };
    Stream(
            const std::string& socketPath, Endpoint endpoint,
            const Framework& f, const Display& d);
    ~Stream();
    EGLStreamKHR get() { return stream_; };

    EGLint queryState();

private:
    const Framework& framework_;
    const Display& display_;
    EGLStreamKHR stream_;
    Socket socket_;
};


}