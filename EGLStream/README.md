##EGLStreams (efficient passing of images between processes)
#EGL intro
[EGL][1] is an interface between rendering APIs (e.g. OpenGL, OpenVG) and native platform windowing system (e.g. X11). EGL defines functions and types for buffer management and rendering.
The EGL API is managed by a non-profit organization [Khoros Group][2]. Important is that EGL is not a library, it is simply a definition of an API. And implementation is done mainly by device
driver vendors.

#EGL API conventions
_Binary portability first_

In order to define portable binary API, EGL defines all types used in its calls. So there are EGLint, EGLBoolean and so on.
All functionality is split to core part and extensions. Only the core part is mandatory for all implementation and therefore is defined in EGL headers.
The big part of the functionality is implemented in the form of extensions. Types provided by extensions are defined in the  extensions are defined in `eglext.h` header.
But functions provided by extensions are not part of the header. The signature, as well as the name of a function, is defined only in the documentation. And
in order to use it, client first has to retrieve the function address, cast it to proper function-pointer type and then use it.


```
typedef EGLStreamKHR (*eglCreateStreamKHR_type)(EGLDisplay, const EGLint*);
auto eglCreateStreamKHR = eglCreateStreamKHR_type eglGetProcAddress("eglCreateStreamKHR");
eglCreateStreamKHR(...);

```

Some extensions are vendor specific and they have to have vendor name in all names defined by the extension.
If the extension is adopted by more than one vendor, they can agree on the usage of "EXT" instead of the vendor name.
An extension "approved" by Khoros Group have "KHR" suffix instead of vendor name [3].

#EGLStreams and related extensions
The core part of EGL defines only primitives for rendering and management of buffers. But effective transmission of frame sequences between APIs is not defined. To cover the gap there is an [EGL_KHR_stream][4] extension, which defines `EGLStream` object, which can be connected from different APIs as consumer and producer, and used to pass frames.
"Connecting" procedure is not defined by EGL as well, and should be provided by client API ([example of such functions in CUDA][5]).

The downside of EGLStream extension is that both producer and consumer should be connected to the same EGLStream object, and therefore can not
belong to the same process. To allow this functionality, there is another extension [EGL_KHOR_stream_cross_process_fd][6]. Using the functions of this extension, it is possible to
query file descriptor of the stream and create another stream from this file descriptor. And later use it as two ends of the same stream.

There are downsides in these approaches as well: when different parts are created, it is not specified which end of the stream is being created, and it is defined later by client API.
And passing such file descriptors between processes, though is possible, but is quite a low-level procedure.
To address these issues, Nvidia defines set of folowing extensions:
  * [EGL_NV_stream_remote EGL_NV_stream_cross_process EGL_NV_stream_cross_partition EGL_NV_stream_cross_system][7]
  * [EGL_NV_stream_socket, EGL_NV_stream_socket_unix, EGL_NV_stream_socket_inet][8]

Which define means to connect EGLStreams placed in different processes, virtual systems on the same hardware, or on different hardware.
For cross-process communication, EGL_NV_stream_socket_unix extension is useful, which allows usage of Unix socket for inter-process EGLStream connection.

#Examples
This project provides example C++ wrapper around EGL and EGLStream, which maps C-style interface to C++: leverages RAII and hides some boilerplate code (e.g. retrieval of function pointers).


[1][https://en.wikipedia.org/wiki/EGL_(API)]
[2][https://www.khronos.org]
[3][https://www.khronos.org/registry/EGL/specs/eglspec.1.5.pdf]
[4][https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_stream.txt]
[5][https://docs.nvidia.com/cuda/cuda-driver-api/group__CUDA__EGL.html]
[6][https://www.khronos.org/registry/EGL/extensions/KHR/EGL_KHR_stream_cross_process_fd.txt]
[7][https://www.khronos.org/registry/EGL/extensions/NV/EGL_NV_stream_remote.txt]
[8][https://www.khronos.org/registry/EGL/extensions/NV/EGL_NV_stream_socket.txt]
