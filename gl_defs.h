/*
 * Copyright (c) 2017 Intel Corporation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice (including the next
 * paragraph) shall be included in all copies or substantial portions of the
 * Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef glfuncs_h_included
#define glfuncs_h_included

typedef char GLchar;
typedef float GLclampf;
typedef unsigned int GLbitfield;
typedef unsigned int GLuint;
typedef int GLint;
typedef int GLsizei;
typedef unsigned int GLenum;
typedef void GLvoid;
typedef unsigned char GLubyte;

enum {
    /* Copied from <GL/gl*.h>. */
    GL_NO_ERROR = 0,

    GL_CONTEXT_FLAGS = 0x821e,

    GL_VENDOR                              = 0x1F00,
    GL_RENDERER                            = 0x1F01,
    GL_VERSION                             = 0x1F02,
    GL_EXTENSIONS                          = 0x1F03,
    GL_NUM_EXTENSIONS                      = 0x821D,

    GL_FRAGMENT_SHADER                     = 0x8B30,
    GL_VERTEX_SHADER                       = 0x8B31,

    GL_PROGRAM_BINARY_LENGTH               = 0x8741,
    GL_NUM_PROGRAM_BINARY_FORMATS          = 0x87FE,
};

#define GL_MAJOR_VERSION                  0x821B
#define GL_MINOR_VERSION                  0x821C
#define GL_CONTEXT_PROFILE_MASK           0x9126
#define GL_CONTEXT_CORE_PROFILE_BIT       0x00000001

#ifndef APIENTRY
#define APIENTRY
#endif

extern GLenum (APIENTRY *glGetError)(void);
extern void (APIENTRY *glGetIntegerv)(GLenum pname, GLint *params);
extern const GLubyte * (APIENTRY *glGetString)(GLenum name);
extern const GLubyte * (APIENTRY *glGetStringi)(GLenum name, GLint i);

extern void (APIENTRY *glAttachShader) (GLuint program, GLuint shader);
extern void (APIENTRY *glCompileShader) (GLuint shader);
extern GLuint (APIENTRY *glCreateProgram) (void);
extern GLuint (APIENTRY* glCreateShader) (GLenum type);
extern void (APIENTRY *glDeleteProgram) (GLuint program);
extern void (APIENTRY *glDeleteShader) (GLuint shader);
extern void (APIENTRY *glLinkProgram) (GLuint program);
extern void (APIENTRY *glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
extern void (APIENTRY *glShaderSource) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);

extern void (APIENTRY *glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
extern void (APIENTRY *glProgramBinary) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);

#endif
