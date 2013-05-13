/*
 * Copyright © 2013 Canonical Ltd.
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

#include "recorder_compatibility_layer.h"
#include <camera_control.h>

#include <camera/Camera.h>
#include <camera/ICamera.h>
#include <media/mediarecorder.h>

#undef LOG_TAG
#define LOG_TAG "MediaRecorderCompatibilityLayer"
#include <utils/KeyedVector.h>
#include <utils/Log.h>

#define REPORT_FUNCTION() ALOGV("%s \n", __PRETTY_FUNCTION__);

/*!
 * \brief The MediaRecorderListenerWrapper class is used to listen to events
 * from the MediaRecorder
 */
class MediaRecorderListenerWrapper : public android::MediaRecorderListener
{
public:
    MediaRecorderListenerWrapper()
    {
    }

    void notify(int msg, int ext1, int ext2, const android::Parcel *obj)
    {
        ALOGV("\tmsg: %d, ext1: %d, ext2: %d \n", msg, ext1, ext2);

        switch(msg)
        {
            default:
                ALOGV("\tUnknown notification\n");
        }
    }

private:
};

/*!
 * \brief The MediaRecorderWrapper struct wraps the MediaRecorder class
 */
struct MediaRecorderWrapper : public android::MediaRecorder
{
public:
    MediaRecorderWrapper()
        : MediaRecorder()
    {
    }

    ~MediaRecorderWrapper()
    {
        reset();
    }

private:
    android::sp<MediaRecorderListenerWrapper> media_recorder_listener;
};

/*!
 * \brief android_media_new_recorder creates a new MediaRecorder
 * \return New MediaRecorder object
 */
MediaRecorderWrapper *android_media_new_recorder()
{
    REPORT_FUNCTION()

    MediaRecorderWrapper *mr = new MediaRecorderWrapper;
    if (mr == NULL)
    {
        ALOGE("Failed to create new MediaRecorderWrapper instance.");
        return NULL;
    }

    return mr;
}

/*!
 * \brief android_recorder_initCheck
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \return -1 if an error occured
 */
int android_recorder_initCheck(MediaRecorderWrapper *mr)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->initCheck();
}

/*!
 * \brief android_recorder_setCamera sets the camera object for recording videos
 * from the camera
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param control Wrapper for the camera (see camera in hybris)
 * \return -1 if an error occured
 */
int android_recorder_setCamera(MediaRecorderWrapper *mr, CameraControl* control)
{
    REPORT_FUNCTION()
    assert(mr);
    assert(control);

    return mr->setCamera(control->camera->remote(), control->camera->getRecordingProxy());
}

/*!
 * \brief android_recorder_setVideoSource sets the video source.
 * If no video source is set, only audio is recorded.
 * values defined in /frameworks/av/include/media/mediarecorder.h
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param vs
 * \return -1 if an error occured
 */
int android_recorder_setVideoSource(MediaRecorderWrapper *mr, int vs)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setVideoSource(vs);
}

/*!
 * \brief android_recorder_setAudioSource
 * values are defined in /system/core/include/system/audio.h
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param as
 * \return -1 if an error occured
 */
int android_recorder_setAudioSource(MediaRecorderWrapper *mr, int as)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setAudioSource(as);
}

/*!
 * \brief android_recorder_setOutputFormat
 * values defined in /frameworks/av/include/media/mediarecorder.h
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param of
 * \return -1 if an error occured
 */
int android_recorder_setOutputFormat(MediaRecorderWrapper *mr, int of)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setOutputFormat(of);
}

/*!
 * \brief android_recorder_setVideoEncoder
 * values defined in /frameworks/av/include/media/mediarecorder.h
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param ve
 * \return -1 if an error occured
 */
int android_recorder_setVideoEncoder(MediaRecorderWrapper *mr, int ve)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setVideoEncoder(ve);
}

/*!
 * \brief android_recorder_setAudioEncoder
 * values defined in /frameworks/av/include/media/mediarecorder.h
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param ae
 * \return -1 if an error occured
 */
int android_recorder_setAudioEncoder(MediaRecorderWrapper *mr, int ae)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setAudioEncoder(ae);
}

/*!
 * \brief android_recorder_setOutputFile sets the output file to the given file descriptor
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param fd File descriptor of an open file, that the stream can be written to
 * \return -1 if an error occured
 */
int android_recorder_setOutputFile(MediaRecorderWrapper *mr, int fd)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setOutputFile(fd, 0, 0);
}

/*!
 * \brief android_recorder_setVideoSize
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param width
 * \param height
 * \return -1 if an error occured
 */
int android_recorder_setVideoSize(MediaRecorderWrapper *mr, int width, int height)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setVideoSize(width, height);
}

/*!
 * \brief android_recorder_setVideoFrameRate
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param frames_per_second
 * \return
 */
int android_recorder_setVideoFrameRate(MediaRecorderWrapper *mr, int frames_per_second)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->setVideoFrameRate(frames_per_second);
}

/*!
 * \brief android_recorder_setParameters sets a parameter. Even those without
 * explicit function.
 * For possible parameters look for example in StagefrightRecorder::setParameter()
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \param parameters list of parameters. format is "parameter1=value;parameter2=value"
 * \return -1 if an error occured
 */
int android_recorder_setParameters(MediaRecorderWrapper *mr, const char* parameters)
{
    REPORT_FUNCTION()
    assert(mr);

    android::String8 params(parameters);
    return mr->setParameters(params);
}

/*!
 * \brief android_recorder_start starts the recording.
 * The MediaRecorder has to be in state "prepared"
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \return -1 if an error occured
 */
int android_recorder_start(MediaRecorderWrapper *mr)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->start();
}

/*!
 * \brief android_recorder_stop Stops a running recording.
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \return -1 if an error occured
 */
int android_recorder_stop(MediaRecorderWrapper *mr)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->stop();
}

/*!
 * \brief android_recorder_prepare put the MediaRecorder into state "prepare"
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \return -1 if an error occured
 */
int android_recorder_prepare(MediaRecorderWrapper *mr)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->prepare();
}

/*!
 * \brief android_recorder_reset resets the MediaRecorder
 * \param mr MediaRecorderWrapper that is the MediaRecorder object
 * \return -1 if an error occured
 */
int android_recorder_reset(MediaRecorderWrapper *mr)
{
    REPORT_FUNCTION()
    assert(mr);

    return mr->reset();
}
