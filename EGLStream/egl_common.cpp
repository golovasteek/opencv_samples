#include "egl_common.h"

#include <stdexcept>
#include <iostream>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <unordered_map>

namespace egl {

template<class FunctionType>
FunctionType getExtFunction(const std::string& name)
{
    auto result = FunctionType(eglGetProcAddress(name.c_str()));
    if (result == nullptr) {
        throw std::runtime_error(name + " function is not found");
    }
    return result;
}

const std::unordered_map<EGLint, std::string> EGL_ERROR_STRINGS = {
    { EGL_SUCCESS, "The last function succeeded without error." },
    { EGL_NOT_INITIALIZED, "EGL is not initialized, or could not be initialized, for the specified EGL display connection." },
    { EGL_BAD_ACCESS, "EGL cannot access a requested resource (for example a context is bound in another thread)." },
    { EGL_BAD_ALLOC, "EGL failed to allocate resources for the requested operation." },
    { EGL_BAD_ATTRIBUTE,  "An unrecognized attribute or attribute value was passed in the attribute list." },
    { EGL_BAD_CONTEXT, "An EGLContext argument does not name a valid EGL rendering context." },
    { EGL_BAD_CONFIG, "An EGLConfig argument does not name a valid EGL frame buffer configuration." },
    { EGL_BAD_CURRENT_SURFACE, "The current surface of the calling thread is a window, pixel buffer or pixmap that is no longer valid." },
    { EGL_BAD_DISPLAY, "An EGLDisplay argument does not name a valid EGL display connection." },
    { EGL_BAD_SURFACE, "An EGLSurface argument does not name a valid surface (window, pixel buffer or pixmap) configured for GL rendering." },
    { EGL_BAD_MATCH, "Arguments are inconsistent (for example, a valid context requires buffers not supplied by a valid surface)." },
    { EGL_BAD_PARAMETER, "One or more argument values are invalid." },
    { EGL_BAD_NATIVE_PIXMAP, "A NativePixmapType argument does not refer to a valid native pixmap." },
    { EGL_BAD_NATIVE_WINDOW, "A NativeWindowType argument does not refer to a valid native window." },
    { EGL_CONTEXT_LOST, "A power management event has occurred. The application must destroy all contexts and reinitialise OpenGL ES state and objects to continue rendering." }
};

#define INIT_EXT_FUNCTION(name) \
    name(getExtFunction<name ## _type>(#name))


#define EGL_CHECK_CALL(expr) \
    [&]() { \
        auto result = (expr); \
        if (result == 0) { \
            EGLint error = eglGetError(); \
            throw Error(EGL_ERROR_STRINGS.at(error)); \
        } \
        return result;\
    }()


Framework::Framework()
    : INIT_EXT_FUNCTION(eglCreateStreamKHR)
    , INIT_EXT_FUNCTION(eglDestroyStreamKHR)
    , INIT_EXT_FUNCTION(eglQueryStreamKHR)
{
}

Display::Display()
    : display_(eglGetDisplay(EGL_DEFAULT_DISPLAY))
{
    EGLint maj;
    EGLint min;
    EGLBoolean initialized = eglInitialize(display_, &maj, &min);
    if (!initialized) {
        throw Error("Egl is not initialized");
    }
    std::cerr << "Hello EGL " << maj << "." << min << std::endl;
    
    std::cerr << "EGL_CLIENT_APIS: " << eglQueryString(display_, EGL_CLIENT_APIS) << std::endl;
    std::cerr << "EGL_EXTENSIONS: " << eglQueryString(display_, EGL_EXTENSIONS) << std::endl;
    std::cerr << "EGL_VENDOR: " << eglQueryString(display_, EGL_VENDOR) << std::endl;
    std::cerr << "EGL_VERSION: " << eglQueryString(display_, EGL_VERSION) << std::endl;
}

Display::~Display()
{
    auto status = eglTerminate(display_);
    if (!status) {
        std::cerr << "Termination failed" << std::endl;
    }

    std::cerr << "EGL Termindated" << std::endl;
}


Socket::Socket(const std::string& socketName, bool isServer)
{
    fd_ = socket(AF_UNIX, SOCK_SEQPACKET, 0);
    if (fd_ == -1) {
        throw Error("Can not create socket");
    }

    sockaddr_un addr;
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_UNIX;
    strcpy(addr.sun_path, socketName.data());

    if (isServer) {
        unlink(socketName.c_str());
        auto status = bind(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        if (status == -1) {
            throw Error(std::string("Can not bind: ") + sys_errlist[errno]);
        }
        
        status = listen(fd_, 5);
        if (status == -1) {
            throw Error("Can not listen");
        }

        sockaddr clientAddr;
        socklen_t clientAddrLen;
        std::cerr << "Waiting for connections..." << std::endl;
        auto msgSocket = accept(fd_, &clientAddr, &clientAddrLen);
        if (msgSocket == -1) {
            throw Error("Can not accept connection" + std::string(sys_errlist[errno]));
        }

        std::cerr << "Connected." << std::endl;
        char msg[16];
        auto readCount = read(msgSocket, msg, 16);
        if (readCount == -1) {
            throw Error(std::string("Can not establish connection") + sys_errlist[errno]);
        }
        std::cerr << "Got socket message" << std::endl;
        close(fd_);
        fd_ = msgSocket;
    } else {  // not server
        auto status = connect(fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
        if (status == -1) {
            throw Error(std::string("Can not connect: ") + sys_errlist[errno]);
        }
        char data[] = "Hello, egl stream!";
        status = write(fd_, data, sizeof(data));
        if (status == -1) {
            throw Error(std::string("Can not write to socket: ") + sys_errlist[errno]);
        } 
    }
    std::cerr << "Socket connected" << std::endl;
}

Socket::~Socket()
{
    close(fd_);
}

Stream::Stream(const std::string& socketPath, Endpoint endpoint, const Framework& framework, const Display& d)
    : framework_(framework)
    , display_(d)
    , socket_(socketPath, endpoint == Endpoint::consumer)
{
    EGLint streamAttributeList[] = {
        EGL_SUPPORT_REUSE_NV, EGL_FALSE,
        EGL_CONSUMER_LATENCY_USEC_KHR, 16000,
        EGL_CONSUMER_ACQUIRE_TIMEOUT_USEC_KHR, 64000,
        EGL_STREAM_TYPE_NV, EGL_STREAM_CROSS_PROCESS_NV,
        EGL_STREAM_ENDPOINT_NV, static_cast<EGLint>(endpoint),
        EGL_STREAM_PROTOCOL_NV, EGL_STREAM_PROTOCOL_SOCKET_NV,
        EGL_SOCKET_TYPE_NV, EGL_SOCKET_TYPE_UNIX_NV,
        EGL_SOCKET_HANDLE_NV, socket_.get(),
        EGL_NONE };
    stream_ = EGL_CHECK_CALL(framework_.eglCreateStreamKHR(d.get(), streamAttributeList));
}

Stream::~Stream()
{
    auto status = framework_.eglDestroyStreamKHR(display_.get(), stream_);
    if (!status) {
        std::cerr << "Can not destroy stream" << std::endl;
    } else {
        std::cerr << "Stream is destroyed" << std::endl;
    }
}

EGLint Stream::queryState()
{
    EGLint result = 0;
    auto status = framework_.eglQueryStreamKHR(display_.get(), stream_, EGL_STREAM_STATE_KHR, &result);

    if (!status) {
        EGLint error = eglGetError();
        std::cerr << "Query stream failed: 0x" << std::hex << error << std::endl;
    }
    return result;
}

}