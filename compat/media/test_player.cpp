/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * Authored by: Jim Hodapp <jim.hodapp@canonical.com>
 */

#include "media_compatibility_layer.h"

#include <utils/Errors.h>

#include <surface_flinger_compatibility_layer.h>

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <cstring>

using namespace android;

struct ClientWithSurface
{
    SfClient* client;
    SfSurface* surface;
};

ClientWithSurface client_with_surface(bool setup_surface_with_egl)
{
    ClientWithSurface cs = ClientWithSurface();

    cs.client = sf_client_create();

    if (!cs.client)
    {
        printf("Problem creating client ... aborting now.");
        return cs;
    }

    static const size_t primary_display = 0;

    SfSurfaceCreationParameters params =
    {
        0,
        0,
        sf_get_display_width(primary_display),
        sf_get_display_height(primary_display),
        -1, //PIXEL_FORMAT_RGBA_8888,
        15000,
        0.5f,
        setup_surface_with_egl, // Do not associate surface with egl, will be done by camera HAL
        "MediaCompatLayerTestSurface"
    };

    cs.surface = sf_surface_create(cs.client, &params);

    if (!cs.surface)
    {
        printf("Problem creating surface ... aborting now.");
        return cs;
    }

    sf_surface_make_current(cs.surface);

    return cs;
}

#define PRINT_GLERROR() printf("GL error@%d: %x\n", __LINE__, glGetError());

struct RenderData
{
    static const char *vertex_shader()
    {
        return
                "#extension GL_OES_EGL_image_external : require              \n"
                "attribute vec4 a_position;                                  \n"
                "attribute vec2 a_texCoord;                                  \n"
                "uniform mat4 m_texMatrix;                                   \n"
                "varying vec2 v_texCoord;                                    \n"
                "varying float topDown;                                      \n"
                "void main()                                                 \n"
                "{                                                           \n"
                "   gl_Position = a_position;                                \n"
                "   v_texCoord = a_texCoord;                                 \n"
                //                "   v_texCoord = (m_texMatrix * vec4(a_texCoord, 0.0, 1.0)).xy;\n"
                //"   topDown = v_texCoord.y;                                  \n"
                "}                                                           \n";
    }

    static const char *fragment_shader()
    {
        return
                "#extension GL_OES_EGL_image_external : require      \n"
                "precision mediump float;                            \n"
                "varying vec2 v_texCoord;                            \n"
                "uniform samplerExternalOES s_texture;               \n"
                "void main()                                         \n"
                "{                                                   \n"
                "  gl_FragColor = texture2D( s_texture, v_texCoord );\n"
                "}                                                   \n";
    }

    static GLuint loadShader(GLenum shaderType, const char* pSource)
    {
        GLuint shader = glCreateShader(shaderType);

        if (shader) {
            glShaderSource(shader, 1, &pSource, NULL);
            glCompileShader(shader);
            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
            if (!compiled) {
                GLint infoLen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);
                if (infoLen) {
                    char* buf = (char*) malloc(infoLen);
                    if (buf) {
                        glGetShaderInfoLog(shader, infoLen, NULL, buf);
                        fprintf(stderr, "Could not compile shader %d:\n%s\n",
                                shaderType, buf);
                        free(buf);
                    }
                    glDeleteShader(shader);
                    shader = 0;
                }
            }
        }
        else
        {
            printf("Error, during shader creation: %i\n", glGetError());
        }
        return shader;
    }

    static GLuint create_program(const char* pVertexSource, const char* pFragmentSource)
    {
        GLuint vertexShader = loadShader(GL_VERTEX_SHADER, pVertexSource);
        if (!vertexShader) {
            printf("vertex shader not compiled\n");
            return 0;
        }

        GLuint pixelShader = loadShader(GL_FRAGMENT_SHADER, pFragmentSource);
        if (!pixelShader) {
            printf("frag shader not compiled\n");
            return 0;
        }

        GLuint program = glCreateProgram();
        if (program) {
            glAttachShader(program, vertexShader);
            glAttachShader(program, pixelShader);
            glLinkProgram(program);
            GLint linkStatus = GL_FALSE;
            glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
            if (linkStatus != GL_TRUE) {
                GLint bufLength = 0;
                glGetProgramiv(program, GL_INFO_LOG_LENGTH, &bufLength);
                if (bufLength) {
                    char* buf = (char*) malloc(bufLength);
                    if (buf) {
                        glGetProgramInfoLog(program, bufLength, NULL, buf);
                        fprintf(stderr, "Could not link program:\n%s\n", buf);
                        free(buf);
                    }
                }
                glDeleteProgram(program);
                program = 0;
            }
        }
        return program;
    }

    RenderData() : program_object(create_program(vertex_shader(), fragment_shader()))
    {
        position_loc = glGetAttribLocation(program_object, "a_position");
        tex_coord_loc = glGetAttribLocation(program_object, "a_texCoord");
        sampler_loc = glGetUniformLocation(program_object, "s_texture");
        matrix_loc = glGetUniformLocation(program_object, "m_texMatrix");
    }

    // Handle to a program object
    GLuint program_object;
    // Attribute locations
    GLint  position_loc;
    GLint  tex_coord_loc;
    // Sampler location
    GLint sampler_loc;
    // Matrix location
    GLint matrix_loc;
};

static status_t setup_video_texture(ClientWithSurface *cs, GLuint *preview_texture_id)
{
    assert(cs != NULL);
    assert(preview_texture_id != NULL);

    sf_surface_make_current(cs->surface);

    glGenTextures(1, preview_texture_id);
    glClearColor(1.0, 0., 0.5, 1.);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    android_media_set_preview_texture(*preview_texture_id);

    return OK;
}

static status_t update_gl_buffer(RenderData *render_data, EGLDisplay *disp, EGLSurface *surface)
{
    assert(disp != NULL);
    assert(surface != NULL);

    static GLfloat vVertices[] = { 1.0f, -1.0f, 0.0f, // Position 0
        0.0f, 0.0f,         // TexCoord 0
        1.0f, 1.0f, 0.0f,   // Position 1
        0.0f, 1.0f,         // TexCoord 1
        -1.0f, 1.0f, 0.0f,  // Position 2
        1.0f, 1.0f,         // TexCoord 2
        -1.0f, -1.0f, 0.0f, // Position 3
        1.0f, 0.0f          // TexCoord 3
    };
    GLushort indices[] = { 0, 1, 2, 0, 2, 3 };

    android_media_update_surface_texture();

    glClear(GL_COLOR_BUFFER_BIT);
    // Use the program object
    glUseProgram(render_data->program_object);
    // Enable attributes
    glEnableVertexAttribArray(render_data->position_loc);
    glEnableVertexAttribArray(render_data->tex_coord_loc);
    // Load the vertex position
    glVertexAttribPointer(render_data->position_loc,
            3,
            GL_FLOAT,
            GL_FALSE,
            5 * sizeof(GLfloat),
            vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(render_data->tex_coord_loc,
            2,
            GL_FLOAT,
            GL_FALSE,
            5 * sizeof(GLfloat),
            vVertices+3);

    glActiveTexture(GL_TEXTURE0);
    // Set the sampler texture unit to 0
    glUniform1i(render_data->sampler_loc, 0);
    glUniform1i(render_data->matrix_loc, 0);
    // android_camera_update_preview_texture(cc);
    android_media_update_surface_texture();
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    glDisableVertexAttribArray(render_data->position_loc);
    glDisableVertexAttribArray(render_data->tex_coord_loc);

    eglSwapBuffers(*disp, *surface);

    return OK;
}

int main(int argc, char **argv)
{
    Player *player = android_media_new_player();
    if (player == NULL)
    {
        printf("Problem creating new media player.\n");
        return EXIT_FAILURE;
    }

    if (argc < 2)
    {
        printf("Usage: test_player <video_to_play>");
        return EXIT_FAILURE;
    }
    printf("Setting data source to: %s.\n", argv[1]);

    if (android_media_set_data_source(argv[1]) != OK)
    {
        printf("Failed to set data source: %s\n", argv[1]);
        return EXIT_FAILURE;
    }

    printf("Creating EGL surface.\n");
    ClientWithSurface cs = client_with_surface(true /* Associate surface with egl. */);
    if (!cs.surface)
    {
        printf("Problem acquiring surface for preview");
        return EXIT_FAILURE;
    }

    printf("Creating GL texture.\n");
    GLuint preview_texture_id;
    EGLDisplay disp = sf_client_get_egl_display(cs.client);
    EGLSurface surface = sf_surface_get_egl_surface(cs.surface);

    sf_surface_make_current(cs.surface);
    if (setup_video_texture(&cs, &preview_texture_id) != OK)
    {
        printf("Problem setting up GL texture for video surface.\n");
        return EXIT_FAILURE;
    }

    RenderData render_data;

    printf("Starting video playback.\n");
    android_media_play();

    while (android_media_is_playing())
    {
        update_gl_buffer(&render_data, &disp, &surface);
    }

    return EXIT_SUCCESS;
}
