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

#include <assert.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

#include "gl_defs.h"

extern int gl_version;

bool
create_gbm_context(void);
bool
destroy_gbm_context(void);
void
require_get_program_binary(void);

static GLuint
compile_shader(GLenum type, const char *source)
{
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    return shader;
}

static GLuint
make_vs_fs_program(const char *vsrc, const char *fsrc)
{
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vsrc);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fsrc);
    GLuint p = glCreateProgram();
    glAttachShader(p, vs);
    glAttachShader(p, fs);
    glLinkProgram(p);
    return p;
}

static bool
print_ctx_info()
{
    while(glGetError() != GL_NO_ERROR) {
        /* Clear all errors */
    }

    const char *vendor = (const char *) glGetString(GL_VENDOR);
    if (glGetError() != GL_NO_ERROR || vendor == NULL) {
        vendor = "GL_ERROR";
    }

    const char *renderer = (const char *) glGetString(GL_RENDERER);
    if (glGetError() != GL_NO_ERROR || renderer == NULL) {
        renderer = "GL_ERROR";
    }

    const char *version_str = (const char *) glGetString(GL_VERSION);
    if (glGetError() != GL_NO_ERROR || version_str == NULL) {
        version_str = "GL_ERROR";
    }

    printf("OpenGL vendor string: %s\n", vendor);
    printf("OpenGL renderer string: %s\n", renderer);
    printf("OpenGL version string: %s\n", version_str);

    return true;
}

static void
get_program_binary_sample(void)
{
    static const char vsrc[] =
        "void main()\n"
        "{\n"
        "    gl_Position = gl_Vertex;\n"
        "}\n";
    static const char fsrc[] =
        "#version 120\n"
        "uniform vec4 color = vec4(0.0, 1.0, 0.0, 1.0);\n"
        "\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = color;\n"
        "}\n";

    GLuint p = make_vs_fs_program(vsrc, fsrc);

    GLsizei binary_length;
    glGetProgramiv(p, GL_PROGRAM_BINARY_LENGTH, &binary_length);
    printf("program binary expected length: %d\n", binary_length);

    GLenum binary_format;
    void *buf = malloc(binary_length);
    GLsizei length_returned;
    glGetProgramBinary(p, binary_length, &length_returned, &binary_format, buf);
    printf("program binary length returned: %d\n", length_returned);
}

int
main(int argc, char **argv)
{
    if (!create_gbm_context())
        return EXIT_FAILURE;

    print_ctx_info();

    require_get_program_binary();
    get_program_binary_sample();

    if (!destroy_gbm_context())
        return EXIT_FAILURE;

    return EXIT_SUCCESS;
}
