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

#include <fcntl.h>
#include <sys/stat.h>

#include <media/mediaplayer.h>

#include <binder/ProcessState.h>
#include <gui/SurfaceTexture.h>

#include <utils/Log.h>

#undef LOG_TAG
#define LOG_TAG "MediaCompatibilityLayer"

#define ALOGV(...) fprintf(stderr, __VA_ARGS__)
#define REPORT_FUNCTION() ALOGV("%s \n", __PRETTY_FUNCTION__);

struct FrameAvailableListener : public android::SurfaceTexture::FrameAvailableListener
{
public:
    // From android::SurfaceTexture::FrameAvailableListener
    void onFrameAvailable()
    {
        // REPORT_FUNCTION();
        if (mSetVideoTextureNeedsUpdateCb != NULL)
            mSetVideoTextureNeedsUpdateCb(mVideoTextureNeedsUpdateContext);
    }

    void setVideoTextureNeedsUpdateCb(on_video_texture_needs_update cb, void *context)
    {
        REPORT_FUNCTION();

        mSetVideoTextureNeedsUpdateCb = cb;
        mVideoTextureNeedsUpdateContext = context;
    }

private:
    on_video_texture_needs_update mSetVideoTextureNeedsUpdateCb;
    void *mVideoTextureNeedsUpdateContext;
};

class MediaPlayerListenerWrapper : public android::MediaPlayerListener
{
public:
    void notify(int msg, int ext1, int ext2, const android::Parcel *obj)
    {
        printf("\tmsg: %d, ext1: %d, ext2: %d \n", msg, ext1, ext2);

        switch(msg)
        {
            case android::MEDIA_PREPARED:
                ALOGV("\tMEDIA_PREPARED msg\n");
                break;
            case android::MEDIA_PLAYBACK_COMPLETE:
                ALOGV("\tMEDIA_PLAYBACK_COMPLETE msg\n");
                break;
            case android::MEDIA_BUFFERING_UPDATE:
                ALOGV("\tMEDIA_BUFFERING_UPDATE msg\n");
                break;
            case android::MEDIA_SEEK_COMPLETE:
                ALOGV("\tMEDIA_SEEK_COMPLETE msg\n");
                break;
            case android::MEDIA_SET_VIDEO_SIZE:
                ALOGV("\tMEDIA_SET_VIDEO_SIZE msg\n");
                if (mSetVideoSizeCb != NULL)
                    mSetVideoSizeCb(ext1, ext2, mVideoSizeContext);
                else
                    LOGE("Failed to set video size. mSetVideoSizeCb is NULL.");

                break;
            case android::MEDIA_TIMED_TEXT:
                ALOGV("\tMEDIA_TIMED_TEXT msg\n");
                break;
            case android::MEDIA_ERROR:
                ALOGV("\tMEDIA_ERROR msg\n");
                break;
            case android::MEDIA_INFO:
                ALOGV("\tMEDIA_INFO msg\n");
                break;
            default:
                ALOGV("\tUnknown media msg\n");
        }
    }

    void setVideoSizeCb(on_msg_set_video_size cb, void *context)
    {
        REPORT_FUNCTION();

        mSetVideoSizeCb = cb;
        mVideoSizeContext = context;
    }


private:
    on_msg_set_video_size mSetVideoSizeCb;
    void *mVideoSizeContext;
};

// ----- MediaPlayer Wrapper ----- //

struct MediaPlayerWrapper : public android::MediaPlayer
{
public:
    MediaPlayerWrapper()
        : MediaPlayer(),
          mTexture(NULL),
          mMPListener(new MediaPlayerListenerWrapper()),
          mFrameListener(new FrameAvailableListener)
    {
        setListener(mMPListener);
    }

    ~MediaPlayerWrapper()
    {
        reset();
    }

    android::status_t setVideoSurfaceTexture(const android::sp<android::SurfaceTexture> &surfaceTexture)
    {
        REPORT_FUNCTION();

        surfaceTexture->setBufferCount(5);
        mTexture = surfaceTexture;
        mTexture->setFrameAvailableListener(mFrameListener);

        return MediaPlayer::setVideoSurfaceTexture(surfaceTexture);
    }

    void updateSurfaceTexture()
    {
        assert(mTexture != NULL);
        mTexture->updateTexImage();
    }

    void get_transformation_matrix_for_surface_texture(GLfloat* matrix)
    {
        assert(mTexture != NULL);
        mTexture->getTransformMatrix(matrix);
    }

    void setVideoSizeCb(on_msg_set_video_size cb, void *context)
    {
        REPORT_FUNCTION();

        assert(mMPListener != NULL);
        mMPListener->setVideoSizeCb(cb, context);
    }

    void setVideoTextureNeedsUpdateCb(on_video_texture_needs_update cb, void *context)
    {
        REPORT_FUNCTION();

        assert(mFrameListener != NULL);
        mFrameListener->setVideoTextureNeedsUpdateCb(cb, context);
    }

public:
    android::Mutex mguard;

private:
    android::sp<android::SurfaceTexture> mTexture;
    android::sp<MediaPlayerListenerWrapper> mMPListener;
    android::sp<FrameAvailableListener> mFrameListener;
}; // MediaPlayerWrapper

using namespace android;

// ----- Media Player C API Implementation ----- //

namespace
{
    static int fd = -1;
} // namespace

void android_media_set_video_size_cb(MediaPlayerWrapper *mp, on_msg_set_video_size cb, void *context)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return;
    }

    Mutex::Autolock al(mp->mguard);
    mp->setVideoSizeCb(cb, context);
}

void android_media_set_video_texture_needs_update_cb(MediaPlayerWrapper *mp, on_video_texture_needs_update cb, void *context)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return;
    }

    Mutex::Autolock al(mp->mguard);
    mp->setVideoTextureNeedsUpdateCb(cb, context);
}

MediaPlayerWrapper *android_media_new_player()
{
    REPORT_FUNCTION()

    MediaPlayerWrapper *mp = new MediaPlayerWrapper();
    if (mp == NULL)
    {
        LOGE("Failed to create new MediaPlayerWrapper instance.");
        return NULL;
    }

    // Required for internal player state processing. Without this, prepare() and start() hang.
    ProcessState::self()->startThreadPool();

    return mp;
}

int android_media_set_data_source(MediaPlayerWrapper *mp, const char* url)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    if (url == NULL)
    {
        LOGE("url must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    fd = open(url, O_RDONLY);
    if (fd < 0)
    {
        LOGE("Failed to open source data at: %s\n", url);
        return BAD_VALUE;
    }

    struct stat st;
    stat(url, &st);

    LOGD("source file length: %lld\n", st.st_size);

    mp->setDataSource(fd, 0, st.st_size);

    return OK;
}

int android_media_set_preview_texture(MediaPlayerWrapper *mp, int texture_id)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    static const bool allow_synchronous_mode = true;
    // Create a new SurfaceTexture from the texture_id in synchronous mode (don't wait on all data in the buffer)
    mp->setVideoSurfaceTexture(android::sp<android::SurfaceTexture>(
        new android::SurfaceTexture(
            texture_id,
            allow_synchronous_mode)));

    return OK;
}

void android_media_update_surface_texture(MediaPlayerWrapper *mp)
{
    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return;
    }

    Mutex::Autolock al(mp->mguard);
    mp->updateSurfaceTexture();
}

void android_media_surface_texture_get_transformation_matrix(MediaPlayerWrapper *mp, GLfloat* matrix)
{
    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return;
    }

    Mutex::Autolock al(mp->mguard);
    mp->get_transformation_matrix_for_surface_texture(matrix);
}

int android_media_play(MediaPlayerWrapper *mp)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    mp->prepare();
    mp->start();
    const char *tmp = mp->isPlaying() ? "yes" : "no";
    printf("Is playing?: %s\n", tmp);

#if 0
    if (mp->isPlaying())
    {
        sleep(2);
        // Seek to 120 seconds in (for ironman3 trailer)
        mp->seekTo(120000);
    }
#endif

    return OK;
}

int android_media_pause(MediaPlayerWrapper *mp)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    mp->pause();

    return OK;
}

int android_media_stop(MediaPlayerWrapper *mp)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    mp->stop();

    if (fd > -1)
        close(fd);
    fd = -1;

    return OK;
}

bool android_media_is_playing(MediaPlayerWrapper *mp)
{
    if (mp != NULL)
    {
        Mutex::Autolock al(mp->mguard);
        if (mp->isPlaying())
            return true;
    }

    return false;
}

int android_media_seek_to(MediaPlayerWrapper *mp, int msec)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    return mp->seekTo(msec);
}

int android_media_get_current_position(MediaPlayerWrapper *mp, int *msec)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    return mp->getCurrentPosition(msec);
}

int android_media_get_duration(MediaPlayerWrapper *mp, int *msec)
{
    REPORT_FUNCTION()

    if (mp == NULL)
    {
        LOGE("mp must not be NULL");
        return BAD_VALUE;
    }

    Mutex::Autolock al(mp->mguard);
    return mp->getDuration(msec);
}
