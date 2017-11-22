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

#define WAFFLE_API_VERSION 0x0103

#include <assert.h>
#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#include <waffle.h>

#include "gl_defs.h"

GLenum (APIENTRY *glGetError)(void);
void (APIENTRY *glGetIntegerv)(GLenum pname, GLint *params);
const GLubyte * (APIENTRY *glGetString)(GLenum name);
const GLubyte * (APIENTRY *glGetStringi)(GLenum name, GLint i);

void (APIENTRY *glAttachShader) (GLuint program, GLuint shader);
void (APIENTRY *glCompileShader) (GLuint shader);
GLuint (APIENTRY *glCreateProgram) (void);
GLuint (APIENTRY* glCreateShader) (GLenum type);
void (APIENTRY *glDeleteProgram) (GLuint program);
void (APIENTRY *glDeleteShader) (GLuint shader);
void (APIENTRY *glLinkProgram) (GLuint program);
void (APIENTRY *glGetProgramiv) (GLuint program, GLenum pname, GLint *params);
void (APIENTRY *glShaderSource) (GLuint shader, GLsizei count, const GLchar *const*string, const GLint *length);

void (APIENTRY *glGetProgramBinary) (GLuint program, GLsizei bufSize, GLsizei *length, GLenum *binaryFormat, void *binary);
void (APIENTRY *glProgramBinary) (GLuint program, GLenum binaryFormat, const void *binary, GLsizei length);

int gl_version;

#define NORETURN __attribute__((noreturn))

static void NORETURN
error_printf(const char *module, const char *fmt, ...)
{
    va_list ap;

    va_start(ap, fmt);
    fprintf(stderr, "%s error: ", module);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    va_end(ap);

    exit(EXIT_FAILURE);
}

static void
error_waffle(void)
{
    const struct waffle_error_info *info = waffle_error_get_info();
    const char *code = waffle_error_to_string(info->code);

    if (info->message_length > 0)
        error_printf("Waffle", "0x%x %s: %s", info->code, code, info->message);
    else
        error_printf("Waffle", "0x%x %s", info->code, code);
}

static void
error_get_gl_symbol(const char *name)
{
    error_printf("gbm_ctx", "failed to get function pointer for %s", name);
}

#define GET_REQUIRED_FUNCTION(f) get_required_func((void**)&(f), #f)
static void
get_required_func(void **ptr, const char *name)
{
    *ptr = waffle_dl_sym(WAFFLE_DL_OPENGL, name);
    if (!*ptr)
        *ptr = waffle_get_proc_address(name);
    if (!*ptr)
        error_get_gl_symbol("glGetStringi");
}

static void
require_gl_extension(const char *name)
{
    const size_t max_ext_len = 128;
    int num_exts = 0;

    glGetIntegerv(GL_NUM_EXTENSIONS, &num_exts);
    if (glGetError()) {
        error_printf("context", "glGetIntegerv(GL_NUM_EXTENSIONS) failed");
    }

    for (uint32_t i = 0; i < num_exts; i++) {
        const uint8_t *ext = glGetStringi(GL_EXTENSIONS, i);
        if (!ext || glGetError()) {
            error_printf("context", "glGetStringi(GL_EXTENSIONS) failed");
        } else if (strncmp((const char*) ext, name, max_ext_len) == 0) {
            return;
        }
    }

    error_printf("context", "required extension is not supported: %s", name);
}

#define WINDOW_WIDTH  320
#define WINDOW_HEIGHT 240

static bool
try_create_context(struct waffle_display *dpy,
                   int version,
                   struct waffle_context **out_ctx,
                   struct waffle_config **out_config,
                   bool exit_on_fail)
{
    int i;
    int32_t config_attrib_list[64];
    struct waffle_context *ctx = NULL;
    struct waffle_config *config = NULL;

    i = 0;
    config_attrib_list[i++] = WAFFLE_CONTEXT_API;
    config_attrib_list[i++] = WAFFLE_CONTEXT_OPENGL;

    config_attrib_list[i++] = WAFFLE_CONTEXT_PROFILE;
    config_attrib_list[i++] = WAFFLE_CONTEXT_CORE_PROFILE;

    config_attrib_list[i++] = WAFFLE_CONTEXT_MAJOR_VERSION;
    config_attrib_list[i++] = version / 10;
    config_attrib_list[i++] = WAFFLE_CONTEXT_MINOR_VERSION;
    config_attrib_list[i++] = version % 10;

    static int32_t dont_care_attribs[] = {
        WAFFLE_RED_SIZE,
        WAFFLE_GREEN_SIZE,
        WAFFLE_BLUE_SIZE,
        WAFFLE_ALPHA_SIZE,
        WAFFLE_DEPTH_SIZE,
        WAFFLE_STENCIL_SIZE,
        WAFFLE_DOUBLE_BUFFERED,
    };
    int dont_care_attribs_count =
        sizeof(dont_care_attribs) / sizeof(dont_care_attribs[0]);

    for (int j = 0; j < dont_care_attribs_count; j++) {
        config_attrib_list[i++] = dont_care_attribs[j];
        config_attrib_list[i++] = WAFFLE_DONT_CARE;
    }

    config_attrib_list[i++] = 0;

    config = waffle_config_choose(dpy, config_attrib_list);
    if (!config) {
        goto fail;
    }

    ctx = waffle_context_create(config, NULL);
    if (!ctx) {
        goto fail;
    }

    *out_ctx = ctx;
    *out_config = config;
    return true;

fail:
    if (exit_on_fail) {
        error_waffle();
    }
    if (ctx) {
        waffle_context_destroy(ctx);
    }
    if (config) {
        waffle_config_destroy(config);
    }

    return false;
}

#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))

static void
create_context(struct waffle_display *dpy,
               struct waffle_context **out_ctx,
               struct waffle_config **out_config)
{
    bool ok = false;

    static int known_gl_profile_versions[] =
      { 32, 33, 40, 41, 42, 43, 44, 45, 46 };

    for (int i = ARRAY_SIZE(known_gl_profile_versions) - 1; i >= 0; i--) {
        ok = try_create_context(dpy, known_gl_profile_versions[i], out_ctx,
                                out_config, false);
        if (ok) {
            return;
        }
    }

    error_printf("context", "failed to create\n");
}

static int
parse_version(const char *version)
{
    int count, major, minor;

    if (version == NULL)
        return 0;

    while (*version != '\0' && !isdigit(*version))
        version++;

    count = sscanf(version, "%d.%d", &major, &minor);
    if (count != 2)
        return 0;

    if (minor > 9)
        return 0;

    return (major * 10) + minor;
}

static struct waffle_display *dpy;
static struct waffle_config *config;
static struct waffle_context *ctx;
static struct waffle_window *window;

bool
create_gbm_context(void)
{
    bool ok;
    int i;

    int32_t init_attrib_list[3];

    i = 0;
    init_attrib_list[i++] = WAFFLE_PLATFORM;
    init_attrib_list[i++] = WAFFLE_PLATFORM_GBM;
    init_attrib_list[i++] = WAFFLE_NONE;

    ok = waffle_init(init_attrib_list);
    if (!ok)
        error_waffle();

    dpy = waffle_display_connect(NULL);
    if (!dpy)
        error_waffle();

    if (!waffle_display_supports_context_api(dpy, WAFFLE_CONTEXT_OPENGL)) {
        error_printf("gbm_ctx", "Display does not support %s",
                     waffle_enum_to_string(WAFFLE_CONTEXT_OPENGL));
    }

    glGetError = waffle_dl_sym(WAFFLE_DL_OPENGL, "glGetError");
    if (!glGetError)
        error_get_gl_symbol("glGetError");

    glGetIntegerv = waffle_dl_sym(WAFFLE_DL_OPENGL, "glGetIntegerv");
    if (!glGetIntegerv)
        error_get_gl_symbol("glGetIntegerv");

    glGetString = waffle_dl_sym(WAFFLE_DL_OPENGL, "glGetString");
    if (!glGetString) {
        error_get_gl_symbol("glGetString");
    }

    create_context(dpy, &ctx, &config);

    window = waffle_window_create(config, WINDOW_WIDTH, WINDOW_HEIGHT);
    if (!window)
        error_waffle();

    ok = waffle_make_current(dpy, window, ctx);
    if (!ok)
        error_waffle();

    const char *version_str = (const char *) glGetString(GL_VERSION);
    if (glGetError() != GL_NO_ERROR || version_str == NULL) {
        version_str = "GL_ERROR";
    }

    gl_version = parse_version(version_str);

    GET_REQUIRED_FUNCTION(glGetStringi);

    GET_REQUIRED_FUNCTION(glAttachShader);
    GET_REQUIRED_FUNCTION(glCompileShader);
    GET_REQUIRED_FUNCTION(glCreateProgram);
    GET_REQUIRED_FUNCTION(glCreateShader);
    GET_REQUIRED_FUNCTION(glDeleteProgram);
    GET_REQUIRED_FUNCTION(glDeleteShader);
    GET_REQUIRED_FUNCTION(glLinkProgram);
    GET_REQUIRED_FUNCTION(glGetProgramiv);
    GET_REQUIRED_FUNCTION(glShaderSource);

    return true;
}

bool
destroy_gbm_context(void)
{
    bool ok;

    ok = waffle_make_current(dpy, NULL, NULL);
    if (!ok)
        error_waffle();

    ok = waffle_window_destroy(window);
    if (!ok)
        error_waffle();

    ok = waffle_context_destroy(ctx);
    if (!ok)
        error_waffle();

    ok = waffle_config_destroy(config);
    if (!ok)
        error_waffle();

    ok = waffle_display_disconnect(dpy);
    if (!ok)
        error_waffle();

    return true;
}

void
require_get_program_binary(void)
{
    require_gl_extension("GL_ARB_get_program_binary");

    int num_formats = 0;
    glGetIntegerv(GL_NUM_PROGRAM_BINARY_FORMATS, &num_formats);

    if (num_formats <= 0)
        error_printf("context", "no binary formats are supported");

    GET_REQUIRED_FUNCTION(glGetProgramBinary);
    GET_REQUIRED_FUNCTION(glProgramBinary);
}
