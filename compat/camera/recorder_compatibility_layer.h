/*
 * Copyright (C) 2013 Canonical Ltd
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
 */

#ifndef RECORDER_COMPATIBILITY_LAYER_H_
#define RECORDER_COMPATIBILITY_LAYER_H_

#include <stdint.h>
#include <unistd.h>

#include <camera_compatibility_layer.h>

#ifdef __cplusplus
extern "C" {
#endif

    struct MediaRecorderWrapper;

    MediaRecorderWrapper *android_media_new_recorder();
    int android_recorder_initCheck(MediaRecorderWrapper *mr);
    int android_recorder_setCamera(MediaRecorderWrapper *mr, CameraControl* control);
    int android_recorder_setVideoSource(MediaRecorderWrapper *mr, int vs);
    int android_recorder_setAudioSource(MediaRecorderWrapper *mr, int as);
    int android_recorder_setOutputFormat(MediaRecorderWrapper *mr, int of);
    int android_recorder_setVideoEncoder(MediaRecorderWrapper *mr, int ve);
    int android_recorder_setAudioEncoder(MediaRecorderWrapper *mr, int ae);
    int android_recorder_setOutputFile(MediaRecorderWrapper *mr, int fd);
    int android_recorder_setVideoSize(MediaRecorderWrapper *mr, int width, int height);
    int android_recorder_setVideoFrameRate(MediaRecorderWrapper *mr, int frames_per_second);
    int android_recorder_start(MediaRecorderWrapper *mr);
    int android_recorder_stop(MediaRecorderWrapper *mr);
    int android_recorder_prepare(MediaRecorderWrapper *mr);
    int android_recorder_reset(MediaRecorderWrapper *mr);

#ifdef __cplusplus
}
#endif

#endif
