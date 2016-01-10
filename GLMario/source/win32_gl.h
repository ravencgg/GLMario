#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <gl\GL.h>
#include "..\Dependencies\gl\glext.h"

#if !defined(__GL_IMPLEMENTATION__) && !UNITY_BUILD
#define EXTERN extern
#else
#define EXTERN
#endif

EXTERN PFNGLCREATESHADERPROC glCreateShader;
EXTERN PFNGLSHADERSOURCEPROC glShaderSource;
EXTERN PFNGLCOMPILESHADERPROC glCompileShader;
EXTERN PFNGLCREATEPROGRAMPROC glCreateProgram;
EXTERN PFNGLATTACHSHADERPROC glAttachShader;
EXTERN PFNGLLINKPROGRAMPROC glLinkProgram;
EXTERN PFNGLGETPROGRAMIVPROC glGetProgramiv;
EXTERN PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog;
EXTERN PFNGLDETACHSHADERPROC glDetachShader;
EXTERN PFNGLDELETESHADERPROC glDeleteShader;
EXTERN PFNGLGENBUFFERSPROC glGenBuffers;
EXTERN PFNGLGENVERTEXARRAYSPROC glGenVertexArrays;
EXTERN PFNGLENABLEVERTEXARRAYATTRIBPROC glEnableVertexArrayAttrib;
EXTERN PFNGLBINDBUFFERPROC glBindBuffer;
EXTERN PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer;
EXTERN PFNGLBUFFERDATAPROC glBufferData;
EXTERN PFNGLBINDVERTEXARRAYPROC glBindVertexArray;
EXTERN PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
EXTERN PFNGLUSEPROGRAMPROC glUseProgram;
EXTERN PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation;
EXTERN PFNGLBUFFERSUBDATAPROC glBufferSubData;
EXTERN PFNGLUNIFORM1FPROC glUniform1f;
EXTERN PFNGLUNIFORM2FPROC glUniform2f;
EXTERN PFNGLUNIFORM3FPROC glUniform3f;
EXTERN PFNGLUNIFORM4FPROC glUniform4f;
EXTERN PFNGLDELETEBUFFERSPROC glDeleteBuffers;
EXTERN PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays;
EXTERN PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray;

#undef EXTERN

#if defined(__GL_IMPLEMENTATION__) || UNITY_BUILD
static void* LoadGLFunctionPointer(const char *name)
{
    void *result = nullptr;
    result = (void *)wglGetProcAddress(name);
    if (result == 0 ||
        (result == (void*)0x1) || (result == (void*)0x2) || (result == (void*)0x3) ||
        (result == (void*)-1))
    {
        static HMODULE module = 0;
        if (module == 0)
        {
            module = LoadLibraryA("opengl32.dll");
        }
        if (!module)
        {
            return nullptr;
        }
        result = (void *)GetProcAddress(module, name);
    }
    return result;
}

static bool LoadGLFunctionPointers()
{
#define LOAD_FUNCTION(type, name) name = (type) LoadGLFunctionPointer(#name); \
                                  if(name == nullptr) return false; // success = false;

    bool success = true;

    LOAD_FUNCTION(PFNGLCREATESHADERPROC, glCreateShader);
    LOAD_FUNCTION(PFNGLSHADERSOURCEPROC, glShaderSource);
    LOAD_FUNCTION(PFNGLCOMPILESHADERPROC, glCompileShader);
    LOAD_FUNCTION(PFNGLCREATEPROGRAMPROC, glCreateProgram);
    LOAD_FUNCTION(PFNGLATTACHSHADERPROC, glAttachShader);
    LOAD_FUNCTION(PFNGLLINKPROGRAMPROC, glLinkProgram);
    LOAD_FUNCTION(PFNGLGETPROGRAMIVPROC , glGetProgramiv);
    LOAD_FUNCTION(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog);
    LOAD_FUNCTION(PFNGLDETACHSHADERPROC, glDetachShader);
    LOAD_FUNCTION(PFNGLDELETESHADERPROC, glDeleteShader);
    LOAD_FUNCTION(PFNGLGENBUFFERSPROC, glGenBuffers);
    LOAD_FUNCTION(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays);
    LOAD_FUNCTION(PFNGLENABLEVERTEXARRAYATTRIBPROC, glEnableVertexArrayAttrib);
    LOAD_FUNCTION(PFNGLBINDBUFFERPROC, glBindBuffer);
    LOAD_FUNCTION(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer);
    LOAD_FUNCTION(PFNGLBUFFERDATAPROC, glBufferData);
    LOAD_FUNCTION(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray);
    LOAD_FUNCTION(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers);
    LOAD_FUNCTION(PFNGLUSEPROGRAMPROC, glUseProgram);
    LOAD_FUNCTION(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation);
    LOAD_FUNCTION(PFNGLBUFFERSUBDATAPROC, glBufferSubData);
    LOAD_FUNCTION(PFNGLUNIFORM1FPROC, glUniform1f);
    LOAD_FUNCTION(PFNGLUNIFORM2FPROC, glUniform2f);
    LOAD_FUNCTION(PFNGLUNIFORM3FPROC, glUniform3f);
    LOAD_FUNCTION(PFNGLUNIFORM4FPROC, glUniform4f);
    LOAD_FUNCTION(PFNGLDELETEBUFFERSPROC, glDeleteBuffers);
    LOAD_FUNCTION(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays);
    LOAD_FUNCTION(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray);

#if 0
    LOAD_FUNCTION(, );
#endif

    return success;
#undef LOAD_FUNCTION
}

#endif
