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
// #include "media_compatibility_layer_capabilities.h"
// #include "media_compatibility_layer_configuration_translator.h"

#include <fcntl.h>
#include <sys/stat.h>

#include <media/mediaplayer.h>
#include <AwesomePlayer.h>

#include <media/Metadata.h>
#include <media/stagefright/MediaExtractor.h>

#include <binder/ProcessState.h>
#include <gui/SurfaceTexture.h>

#include <utils/KeyedVector.h>
#include <utils/Log.h>

#define LOG_TAG "MediaCompatibilityLayer"

#define ALOGV(...) fprintf(stderr, __VA_ARGS__)
#define REPORT_FUNCTION() ALOGV("%s \n", __PRETTY_FUNCTION__);

struct FrameAvailableListener : public android::SurfaceTexture::FrameAvailableListener
{
    // From android::SurfaceTexture::FrameAvailableListener
    void onFrameAvailable()
    {
        REPORT_FUNCTION();
#if 0
        if (listener)
            listener->on_preview_texture_needs_update_cb(listener->context);
#endif
    }

};

struct MediaListener : public android::MediaPlayerListener
{
    // From android::MediaPlayerListener
    void notify(int msg, int ext1, int ext2, const android::Parcel *obj)
    {
        REPORT_FUNCTION();

        printf("\text1: %d, ext2: %d \n", ext1, ext2);

        switch(msg)
        {
            case android::MEDIA_PREPARED:
                ALOGV("MEDIA_PREPARED msg\n");
                break;
            case android::MEDIA_PLAYBACK_COMPLETE:
                ALOGV("MEDIA_PLAYBACK_COMPLETE msg\n");
                break;
            case android::MEDIA_BUFFERING_UPDATE:
                ALOGV("MEDIA_BUFFERING_UPDATE msg\n");
                break;
            case android::MEDIA_SEEK_COMPLETE:
                ALOGV("MEDIA_SEEK_COMPLETE msg\n");
                break;
            case android::MEDIA_SET_VIDEO_SIZE:
                ALOGV("MEDIA_SET_VIDEO_SIZE msg\n");
                break;
            case android::MEDIA_TIMED_TEXT:
                ALOGV("MEDIA_TIMED_TEXT msg\n");
                break;
            case android::MEDIA_ERROR:
                ALOGV("MEDIA_ERROR msg\n");
                break;
            case android::MEDIA_INFO:
                ALOGV("MEDIA_INFO msg\n");
                break;
            default:
                ALOGV("Unknown media msg\n");
                break;
        }
    }

    typedef void (*on_msg_error)(void* context);

    // Called whenever an error occurs while the camera HAL executes a command
    on_msg_error on_msg_error_cb;

    void* context;
};

// ----- MediaPlayer Interface Implementation Wrapper ----- //

namespace android {
    struct AwesomePlayer;
}

class Player : public android::MediaPlayerInterface
{
    public:
        Player();
        virtual ~Player();

        virtual android::status_t initCheck();

        virtual android::status_t setUID(uid_t uid);

        virtual android::status_t setDataSource(
                const char *url,
                const android::KeyedVector<android::String8, android::String8> *headers = NULL);

        virtual android::status_t setDataSource(int fd, int64_t offset, int64_t length);

        virtual android::status_t setDataSource(const android::sp<android::IStreamSource> &source);

        virtual android::status_t setVideoSurfaceTexture(
                const android::sp<android::SurfaceTexture> &surfaceTexture);
        virtual android::status_t setVideoSurfaceTexture(
                const android::sp<android::ISurfaceTexture> &surfaceTexture) {return android::OK;}
        virtual android::status_t prepare();
        virtual android::status_t prepareAsync();

        virtual android::status_t start();
        virtual android::status_t stop();
        virtual android::status_t pause();

        virtual bool isPlaying();

        virtual android::status_t seekTo(int msec);
        virtual android::status_t getCurrentPosition(int *msec);
        virtual android::status_t getDuration(int *msec);
        virtual android::status_t reset();
        virtual android::status_t setLooping(int loop);
        virtual android::player_type playerType();
        virtual android::status_t invoke(const android::Parcel &request, android::Parcel *reply);
        virtual void setAudioSink(const android::sp<AudioSink> &audioSink);
        virtual android::status_t setParameter(int key, const android::Parcel &request);
        virtual android::status_t getParameter(int key, android::Parcel *reply);

        virtual android::status_t getMetadata(
                const android::media::Metadata::Filter& ids, android::Parcel *records);

        virtual android::status_t dump(int fd, const android::Vector<android::String16> &args) const;

        virtual void updateSurfaceTexture()
        {
            mTexture->updateTexImage();
        }

    private:
        android::AwesomePlayer *mPlayer;
        FrameAvailableListener *mListener;
        android::sp<MediaListener> mMListener;
        android::sp<android::SurfaceTexture> mTexture;
}; // Player


using namespace android;

// From android::MediaPlayerListener
static void notifyCb(void* cookie, int msg, int ext1, int ext2, const Parcel *obj)
{
    REPORT_FUNCTION();
    printf("\text1: %d, ext2: %d \n", ext1, ext2);
#if 0
    if (!listener)
        return;
#endif

    switch(msg)
    {
        case android::MEDIA_PREPARED:
            ALOGV("MEDIA_PREPARED msg\n");
            break;
        case android::MEDIA_PLAYBACK_COMPLETE:
            ALOGV("MEDIA_PLAYBACK_COMPLETE msg\n");
            break;
        case android::MEDIA_BUFFERING_UPDATE:
            ALOGV("MEDIA_BUFFERING_UPDATE msg\n");
            break;
        case android::MEDIA_SEEK_COMPLETE:
            ALOGV("MEDIA_SEEK_COMPLETE msg\n");
            break;
        case android::MEDIA_SET_VIDEO_SIZE:
            ALOGV("MEDIA_SET_VIDEO_SIZE msg\n");
            break;
        case android::MEDIA_TIMED_TEXT:
            ALOGV("MEDIA_TIMED_TEXT msg\n");
            break;
        case android::MEDIA_ERROR:
            ALOGV("MEDIA_ERROR msg\n");
            break;
        case android::MEDIA_INFO:
            ALOGV("MEDIA_INFO msg\n");
            break;
        default:
            ALOGV("Unknown media msg\n");
    }
}


Player::Player()
    : mPlayer(new android::AwesomePlayer),
      mListener(new FrameAvailableListener),
      mMListener(new MediaListener)
{
    REPORT_FUNCTION();

    mPlayer->setListener(this);
    setNotifyCallback(NULL, notifyCb);
}

Player::~Player()
{
    REPORT_FUNCTION();

    // Put the player into an initialized state (idle)
    reset();

    delete mListener;
    mListener = NULL;

    delete mPlayer;
    mPlayer = NULL;
}

status_t Player::initCheck()
{
    return OK;
}

status_t Player::setUID(uid_t uid)
{
    REPORT_FUNCTION();
    mPlayer->setUID(uid);
    return OK;
}

status_t Player::setDataSource(
        const char *url,
        const KeyedVector<String8, String8> *headers)
{
    REPORT_FUNCTION();
    return mPlayer->setDataSource(url, headers);
}

status_t Player::setDataSource(int fd, int64_t offset, int64_t length)
{
    REPORT_FUNCTION();
    return mPlayer->setDataSource(dup(fd), offset, length);
}

status_t Player::setDataSource(const sp<IStreamSource> &source)
{
    return mPlayer->setDataSource(source);
}

status_t Player::setVideoSurfaceTexture(const sp<SurfaceTexture> &surfaceTexture)
{
    REPORT_FUNCTION();
    // FIXME: This causes a stack crash, figure out if I need this and what the problem is.
#if 0
    mpreview_texture->setFrameAvailableListener(
        android::sp<android::SurfaceTexture::FrameAvailableListener>(mListener));
#endif

    surfaceTexture->setBufferCount(5);
    mTexture = surfaceTexture;
    return mPlayer->setSurfaceTexture(surfaceTexture);
}

status_t Player::prepare()
{
    REPORT_FUNCTION();
    return mPlayer->prepare();
}

status_t Player::prepareAsync()
{
    REPORT_FUNCTION();
    return mPlayer->prepareAsync();
}

status_t Player::start()
{
    REPORT_FUNCTION();
    return mPlayer->play();
}

status_t Player::stop()
{
    REPORT_FUNCTION();
    pause();
    return OK;
}

status_t Player::pause()
{
    REPORT_FUNCTION();
    return mPlayer->pause();
}

bool Player::isPlaying()
{
    return mPlayer->isPlaying();
}

status_t Player::seekTo(int msec)
{
    LOGV("seekTo %.2f secs", msec / 1E3);
    status_t err = mPlayer->seekTo((int64_t)msec * 1000);

    return err;
}

status_t Player::getCurrentPosition(int *msec)
{
    REPORT_FUNCTION();
    int64_t positionUs;
    status_t err = mPlayer->getPosition(&positionUs);

    if (err != OK) {
        return err;
    }

    *msec = (positionUs + 500) / 1000;

    return OK;
}

status_t Player::getDuration(int *msec)
{
    REPORT_FUNCTION();
    int64_t durationUs;
    status_t err = mPlayer->getDuration(&durationUs);

    if (err != OK) {
        *msec = 0;
        return OK;
    }

    *msec = (durationUs + 500) / 1000;

    return OK;
}

status_t Player::reset()
{
    REPORT_FUNCTION();
    mPlayer->reset();
    return OK;
}

status_t Player::setLooping(int loop)
{
    REPORT_FUNCTION();
    return mPlayer->setLooping(loop);
}

player_type Player::playerType()
{
    REPORT_FUNCTION();
    return STAGEFRIGHT_PLAYER;
}

status_t Player::invoke(const Parcel &request, Parcel *reply)
{
    REPORT_FUNCTION();
    return INVALID_OPERATION;
}

void Player::setAudioSink(const sp<AudioSink> &audioSink)
{
    REPORT_FUNCTION();
    MediaPlayerInterface::setAudioSink(audioSink);

    mPlayer->setAudioSink(audioSink);
}

status_t Player::setParameter(int key, const Parcel &request)
{
    REPORT_FUNCTION();
    return mPlayer->setParameter(key, request);
}

status_t Player::getParameter(int key, Parcel *reply)
{
    REPORT_FUNCTION();
    return mPlayer->getParameter(key, reply);
}

status_t Player::getMetadata(const media::Metadata::Filter& ids, Parcel *records)
{
    REPORT_FUNCTION();
    using media::Metadata;

    uint32_t flags = mPlayer->flags();

    Metadata metadata(records);

    metadata.appendBool(
            Metadata::kPauseAvailable,
            flags & MediaExtractor::CAN_PAUSE);

    metadata.appendBool(
            Metadata::kSeekBackwardAvailable,
            flags & MediaExtractor::CAN_SEEK_BACKWARD);

    metadata.appendBool(
            Metadata::kSeekForwardAvailable,
            flags & MediaExtractor::CAN_SEEK_FORWARD);

    metadata.appendBool(
            Metadata::kSeekAvailable,
            flags & MediaExtractor::CAN_SEEK);

    return OK;
}

status_t Player::dump(int fd, const Vector<String16> &args) const
{
    REPORT_FUNCTION();
    return mPlayer->dump(fd, args);
}

namespace
{
    sp<Player> player_instance;
    static int fd = -1;
} // namespace

Player *android_media_new_player()
{
    REPORT_FUNCTION()

    Player* player = new Player();

    if (player == NULL)
        return NULL;

    player_instance = player;

    player_instance->setUID(0);
    player_instance->reset();

    // Required for internal player state processing. Without this, prepare() and start() hang.
    android::ProcessState::self()->startThreadPool();

    return player;
}

status_t android_media_set_data_source(const char* url)
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return BAD_VALUE;

    // TODO: close the fd later
    fd = open(url, O_RDONLY);
    if (fd < 0)
    {
        ALOGV("Failed to open source data at: %s\n", url);
        return BAD_VALUE;
    }

    struct stat st;
    stat(url, &st);

    ALOGV("source file length: %lld\n", st.st_size);

    player_instance->setDataSource(fd, 0, st.st_size);

    return OK;
}

status_t android_media_set_preview_texture(int texture_id)
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return BAD_VALUE;

    static const bool allow_synchronous_mode = true;
    // Create a new SurfaceTexture from the texture_id in asynchronous mode (don't wait on all data in the buffer)
    printf("Calling setVideoSurfaceTexture()\n");
    player_instance->setVideoSurfaceTexture(android::sp<android::SurfaceTexture>(
        new android::SurfaceTexture(
            texture_id,
            allow_synchronous_mode)));

    printf("Success setVideoSurfaceTexture()\n");

    return OK;
}

void android_media_update_surface_texture()
{
    player_instance->updateSurfaceTexture();
}

android::status_t android_media_play()
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return BAD_VALUE;

    printf("Calling prepare()\n");
    player_instance->prepare();
    printf("Calling start()\n");
    player_instance->start();
    const char *tmp = player_instance->isPlaying() ? "yes" : "no";
    printf("Is playing?: %s\n", tmp);

    return OK;
}

android::status_t android_media_pause()
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return BAD_VALUE;

    player_instance->pause();

    return OK;
}

android::status_t android_media_stop()
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return BAD_VALUE;

    player_instance->stop();

    if (fd > -1)
        close(fd);
    fd = -1;

    return OK;
}

bool android_media_is_playing()
{
    if (player_instance != NULL)
        if (player_instance->isPlaying())
            return true;

    return false;
}
