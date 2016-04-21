#include "GPUOGL_CD.h"
#include <Windows.h>

GPUOGL_CD::GPUOGL_CD(int width, int height) {
    this->width = width;
    this->height = height;
}

GPUOGL_CD::~GPUOGL_CD() {

}

void GPUOGL_CD::CreateRenderingWindow() {
    WNDCLASS wc;
    // register window class
    wc.style = CS_OWNDC;
    wc.lpfnWndProc = DefWindowProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = GetModuleHandle(NULL);
    wc.hIcon = LoadIcon( NULL, IDI_APPLICATION );
    wc.hCursor = LoadCursor( NULL, IDC_ARROW );
    wc.hbrBackground = (HBRUSH)GetStockObject( BLACK_BRUSH );
    wc.lpszMenuName = NULL;
    wc.lpszClassName = LPCSTR("GLSample");
    RegisterClass( &wc );

    // create main window
    renderingWindow = CreateWindow(
        LPCSTR("GLSample"), LPCSTR("OpenGL Sample"),
        //WS_OVERLAPPED,
        WS_CAPTION | WS_POPUPWINDOW | WS_VISIBLE,
        0, 0, width, height,
        NULL, NULL, wc.hInstance, NULL );
}

void GPUOGL_CD::enableOpenGL() {

    CreateRenderingWindow();

    PIXELFORMATDESCRIPTOR pfd;
    int format;

    // get the device context (DC)
    windowsDeviceContext = GetDC( renderingWindow );

    // set the pixel format for the DC
    ZeroMemory( &pfd, sizeof( pfd ) );
    pfd.nSize = sizeof( pfd );
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    //pfd.dwFlags = PFD_SUPPORT_OPENGL;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;
    format = ChoosePixelFormat( windowsDeviceContext, &pfd );
    SetPixelFormat( windowsDeviceContext, format, &pfd );

    // create and enable the render context (RC)
    openglDeviceContext = wglCreateContext( windowsDeviceContext );
    setOpenGLContext();
}

void GPUOGL_CD::disableOpenGL() {
    wglMakeCurrent( previousWindowsDeviceContext, previousOpenglDeviceContext );
    wglDeleteContext( openglDeviceContext );
    ReleaseDC( renderingWindow, windowsDeviceContext );
    DestroyWindow(renderingWindow);
}

void GPUOGL_CD::setOpenGLContext() {
    previousOpenglDeviceContext = wglGetCurrentContext();
    previousWindowsDeviceContext = wglGetCurrentDC();
    wglMakeCurrent( windowsDeviceContext, openglDeviceContext );
}

void GPUOGL_CD::releaseOpenGLContext() {
    wglMakeCurrent( previousWindowsDeviceContext, previousOpenglDeviceContext );
}

void GPUOGL_CD::swapBuffers() {
	SwapBuffers(windowsDeviceContext);
}
