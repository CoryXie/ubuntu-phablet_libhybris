/*
 * Copyright (C) 2012 Canonical Ltd
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#include "camera_compatibility_layer.h"
#include "camera_compatibility_layer_capabilities.h"
#include "recorder_compatibility_layer.h"

#include <input_stack_compatibility_layer.h>
#include <input_stack_compatibility_layer_codes_key.h>
#include <input_stack_compatibility_layer_flags_key.h>

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

int shot_counter = 1;
int32_t current_zoom_level = 1;
bool new_camera_frame_available = true;
MediaRecorderWrapper *mr = 0;
GLuint preview_texture_id = 0;

void error_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void shutter_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void zoom_msg_cb(void* context, int32_t new_zoom_level)
{
    printf("%s \n", __PRETTY_FUNCTION__);

    CameraControl* cc = static_cast<CameraControl*>(context);
    static int zoom;
    current_zoom_level = new_zoom_level;
}

void autofocus_msg_cb(void* context)
{
    printf("%s \n", __PRETTY_FUNCTION__);
}

void raw_data_cb(void* data, uint32_t data_size, void* context)
{
    printf("%s: %d \n", __PRETTY_FUNCTION__, data_size);
}

void jpeg_data_cb(void* data, uint32_t data_size, void* context)
{
    printf("%s: %d \n", __PRETTY_FUNCTION__, data_size);
    CameraControl* cc = static_cast<CameraControl*>(context);
    android_camera_start_preview(cc);
}

void size_cb(void* ctx, int width, int height)
{
    printf("Supported size: [%d,%d]\n", width, height);
}

void preview_texture_needs_update_cb(void* ctx)
{
    new_camera_frame_available = true;
}

void on_new_input_event(Event* event, void* context)
{
    assert(context);

    if (event->type == KEY_EVENT_TYPE && event->action == ISCL_KEY_EVENT_ACTION_UP)
    {
        printf("We have got a key event: %d \n", event->details.key.key_code);

        CameraControl* cc = static_cast<CameraControl*>(context);
        
        int ret;
        switch(event->details.key.key_code)
        {
            case ISCL_KEYCODE_VOLUME_UP:
                printf("Starting video recording\n");

                android_camera_unlock(cc);

                ret = android_recorder_setCamera(mr, cc);
                if (ret < 0) {
                    printf("android_recorder_setCamera() failed\n");
                    return;
                }
                //state initial / idle
                ret = android_recorder_setAudioSource(mr, ANDROID_AUDIO_SOURCE_CAMCORDER);
                if (ret < 0) {
                    printf("android_recorder_setAudioSource() failed\n");
                    return;
                }
                ret = android_recorder_setVideoSource(mr, ANDROID_VIDEO_SOURCE_CAMERA);
                if (ret < 0) {
                    printf("android_recorder_setVideoSource() failed\n");
                    return;
                }
                //state initialized
                ret = android_recorder_setOutputFormat(mr, ANDROID_OUTPUT_FORMAT_MPEG_4);
                if (ret < 0) {
                    printf("android_recorder_setOutputFormat() failed\n");
                    return;
                }
                //state DataSourceConfigured
                ret = android_recorder_setAudioEncoder(mr, ANDROID_AUDIO_ENCODER_AAC);
                if (ret < 0) {
                    printf("android_recorder_setAudioEncoder() failed\n");
                    return;
                }
                ret = android_recorder_setVideoEncoder(mr, ANDROID_VIDEO_ENCODER_H264);
                if (ret < 0) {
                    printf("android_recorder_setVideoEncoder() failed\n");
                    return;
                }

                int fd;
                fd = open("/root/test_video.avi", O_WRONLY | O_CREAT);
                if (fd < 0) {
                    printf("Could not open file for video recording\n");
                    printf("FD: %i\n", fd);
                    return;
                }
                ret = android_recorder_setOutputFile(mr, fd);
                if (ret < 0) {
                    printf("android_recorder_setOutputFile() failed\n");
                    return;
                }

                ret = android_recorder_setVideoSize(mr, 1280, 720);
                if (ret < 0) {
                    printf("android_recorder_setVideoSize() failed\n");
                    return;
                }
                ret = android_recorder_setVideoFrameRate(mr, 30);
                if (ret < 0) {
                    printf("android_recorder_setVideoFrameRate() failed\n");
                    return;
                }

                ret = android_recorder_prepare(mr);
                if (ret < 0) {
                    printf("android_recorder_prepare() failed\n");
                    return;
                }
                //state prepared
                ret = android_recorder_start(mr);
                if (ret < 0) {
                    printf("android_recorder_start() failed\n");
                    return;
                }
                break;
            case ISCL_KEYCODE_VOLUME_DOWN:
                printf("Stoping video recording\n");
                ret = android_recorder_stop(mr);
                printf("Stoping video recording returned\n");
                if (ret < 0) {
                    printf("android_recorder_stop() failed\n");
                    return;
                }
                printf("Stopped video recording\n");
                ret = android_recorder_reset(mr);
                if (ret < 0) {
                    printf("android_recorder_reset() failed\n");
                    return;
                }
                printf("Reset video recorder\n");
                break;
        }
    }
}

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
        "CameraCompatLayerTestSurface"
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
    static const char* vertex_shader()
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
   
    static const char* fragment_shader()
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
    
    static GLuint loadShader(GLenum shaderType, const char* pSource) {
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

    static GLuint create_program(const char* pVertexSource, const char* pFragmentSource) {
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

int main(int argc, char** argv)
{
    CameraControlListener listener;
    memset(&listener, 0, sizeof(listener));
    listener.on_msg_error_cb = error_msg_cb;
    listener.on_msg_shutter_cb = shutter_msg_cb;
    listener.on_msg_focus_cb = autofocus_msg_cb;
    listener.on_msg_zoom_cb = zoom_msg_cb;

    listener.on_data_raw_image_cb = raw_data_cb;
    listener.on_data_compressed_image_cb = jpeg_data_cb;
    listener.on_preview_texture_needs_update_cb = preview_texture_needs_update_cb;
    CameraControl* cc = android_camera_connect_to(BACK_FACING_CAMERA_TYPE,
                                                  &listener);
    if (cc == NULL)
    {
        printf("Problem connecting to camera");
        return 1;
    }

    listener.context = cc;

    mr = android_media_new_recorder();

    AndroidEventListener event_listener;
    event_listener.on_new_event = on_new_input_event;
    event_listener.context = cc;
    
    InputStackConfiguration input_configuration = { true, 25000 };
    
    android_input_stack_initialize(&event_listener, &input_configuration);
    android_input_stack_start();
        
    android_camera_dump_parameters(cc);

    printf("Supported video sizes:\n");
    android_camera_enumerate_supported_video_sizes(cc, size_cb, NULL);

    int min_fps, max_fps, current_fps;

    android_camera_set_preview_size(cc, 1280, 720);
    
    int width, height;
    android_camera_get_video_size(cc, &width, &height);
    printf("Current video size: [%d,%d]\n", width, height);

    ClientWithSurface cs = client_with_surface(true /* Associate surface with egl. */);

    if (!cs.surface)
    {
        printf("Problem acquiring surface for preview");
        return 1;
    }

    EGLDisplay disp = sf_client_get_egl_display(cs.client);
    EGLSurface surface = sf_surface_get_egl_surface(cs.surface);
    
    sf_surface_make_current(cs.surface);
    RenderData render_data;
    glGenTextures(1, &preview_texture_id);
    glClearColor(1.0, 0., 0.5, 1.);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(
        GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(
        GL_TEXTURE_EXTERNAL_OES, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    android_camera_set_preview_texture(cc, preview_texture_id);
    android_camera_start_preview(cc);

    GLfloat transformation_matrix[16];
    android_camera_get_preview_texture_transformation(cc, transformation_matrix);
    glUniformMatrix4fv(render_data.matrix_loc, 1, GL_FALSE, transformation_matrix);

    printf("Started camera preview.\n");
    
    while(true)
    {
                
        /*if (new_camera_frame_available)
        {
            printf("Updating texture");            
            new_camera_frame_available = false;
            }*/
        static GLfloat vVertices[] = { 0.0f, 0.0f, 0.0f, // Position 0
                0.0f, 0.0f, // TexCoord 0
                0.0f, 1.0f, 0.0f, // Position 1
                0.0f, 1.0f, // TexCoord 1
                1.0f, 1.0f, 0.0f, // Position 2
                1.0f, 1.0f, // TexCoord 2
                1.0f, 0.0f, 0.0f, // Position 3
                1.0f, 0.0f // TexCoord 3
        };
        
        GLushort indices[] = { 0, 1, 2, 0, 2, 3 };
        
        // Set the viewport
        // Clear the color buffer
        glClear(GL_COLOR_BUFFER_BIT);
        // Use the program object
        glUseProgram(render_data.program_object);
        // Enable attributes
        glEnableVertexAttribArray(render_data.position_loc);
        glEnableVertexAttribArray(render_data.tex_coord_loc);
        // Load the vertex position
        glVertexAttribPointer(render_data.position_loc,
                              3,
                              GL_FLOAT,
                              GL_FALSE,
                              5 * sizeof(GLfloat),
                              vVertices);
        // Load the texture coordinate
        glVertexAttribPointer(render_data.tex_coord_loc,
                              2,
                              GL_FLOAT,
                              GL_FALSE,
                              5 * sizeof(GLfloat),
                              vVertices+3);
        
        glActiveTexture(GL_TEXTURE0);
        // Set the sampler texture unit to 0
        glUniform1i(render_data.sampler_loc, 0);
        glUniform1i(render_data.matrix_loc, 0);
        android_camera_update_preview_texture(cc);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
        glDisableVertexAttribArray(render_data.position_loc);
        glDisableVertexAttribArray(render_data.tex_coord_loc);

        eglSwapBuffers(disp, surface);
    }
}
