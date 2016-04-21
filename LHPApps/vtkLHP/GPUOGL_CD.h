#ifndef GPUOGL_CD_H
#define GPUOGL_CD_H

#pragma once

#include <Windows.h>

#ifndef _WIN32
    #pragma message ("GPU_OGL_CD is supported for WIN32 platform only");
#else

class GPUOGL_CD {
protected:
	int width, height; // window width and height
private:
    HWND renderingWindow;         //<rendering window
    HDC windowsDeviceContext;       //<Windows device context
    HGLRC openglDeviceContext;      //<OpenGL rendering device context
    int openglContextReferences;

    HDC previousWindowsDeviceContext;    //<previous Windows device context associated with the current thread
    HGLRC previousOpenglDeviceContext;  //<previous OpenGL rendering device context associated with the current thread
    bool hasRegisteredWindowClass;		//<true, if this object has registered window class
    bool openglIsInitialized;
    bool openglIsSupported;
private:
    void CreateRenderingWindow();
    //void GetLastErrorText();
public:
    GPUOGL_CD(int width, int height);
    ~GPUOGL_CD();

    void enableOpenGL();
    void disableOpenGL();
    void setOpenGLContext();
    void releaseOpenGLContext();
	void swapBuffers();
};

#endif // _WIN32
#endif // GPUOGL_CD_H
