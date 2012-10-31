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
#include <AwesomePlayer.h>
#include <AudioTrack.h>

#include <media/Metadata.h>
#include <media/stagefright/MediaExtractor.h>

#include <binder/ProcessState.h>
#include <gui/SurfaceTexture.h>

#include <utils/KeyedVector.h>
#include <utils/Log.h>

#undef LOG_TAG
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

// ----- MediaPlayer Interface Implementation Wrapper ----- //

namespace android {
    struct AwesomePlayer;
}

class Player : public android::MediaPlayerInterface
{
    public:

    class PlayerAudioOutput: public MediaPlayerBase::AudioSink
    {
    public:

        PlayerAudioOutput();
        virtual ~PlayerAudioOutput();

        virtual bool ready() const { return mTrack != NULL; }
        virtual bool realtime() const { return true; }
        virtual ssize_t bufferSize() const;
        virtual ssize_t frameCount() const;
        virtual ssize_t channelCount() const;
        virtual ssize_t frameSize() const;
        virtual uint32_t latency() const;
        virtual float msecsPerFrame() const;
        virtual android::status_t getPosition(uint32_t *position);
        virtual int getSessionId();

        virtual android::status_t open(
                uint32_t sampleRate, int channelCount,
                int format, int bufferCount,
                AudioCallback cb, void *cookie);

        virtual void start();
        virtual ssize_t write(const void* buffer, size_t size);
        virtual void stop();
        virtual void flush();
        virtual void pause();
        virtual void close();
        void setAudioStreamType(int streamType) { mStreamType = streamType; }
        void setVolume(float left, float right);
        virtual android::status_t dump(int fd, const android::Vector<android::String16>& args) const;

        static bool isOnEmulator();
        static int getMinBufferCount();

    private:
        static void setMinBufferCount();
        static void CallbackWrapper(int event, void *me, void *info);

        android::AudioTrack *mTrack;
        AudioCallback mCallback;
        void *mCallbackCookie;
        int mStreamType;
        float mLeftVolume;
        float mRightVolume;
        float mMsecsPerFrame;
        uint32_t mLatency;
        int mSessionId;
        static bool mIsOnEmulator;
        // 12 for emulator; otherwise 4
        static int mMinBufferCount;

    public:
        uint32_t mNumFramesWritten;
        void snoopWrite(const void*, size_t);
    };

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

        virtual void get_transformation_matrix_for_surface_texture(GLfloat* matrix)
        {
            mTexture->getTransformMatrix(matrix);
        }

        virtual void setVideoSizeCb(on_msg_set_video_size cb)
        {
            mSetVideoSizeCb = cb;
        }

    protected:
        // Is the AudioSink ready to be used, set some presets for it
        virtual android::status_t prepareAudioSink();

        static void notifyCbStatic(void* cookie, int msg, int ext1, int ext2, const android::Parcel *obj);
        void notifyCb(int msg, int ext1, int ext2, const android::Parcel *obj);

    private:
        android::AwesomePlayer *mPlayer;
        FrameAvailableListener *mListener;
        PlayerAudioOutput *mAudioSink;
        android::sp<android::SurfaceTexture> mTexture;

        on_msg_set_video_size mSetVideoSizeCb;
}; // Player

using namespace android;

Player::Player()
    : mPlayer(new android::AwesomePlayer),
      mListener(new FrameAvailableListener),
      mAudioSink(new PlayerAudioOutput),
      mTexture(NULL)
{
    REPORT_FUNCTION();

    mPlayer->setListener(this);
    setNotifyCallback(static_cast<void *>(this), notifyCbStatic);
}

Player::~Player()
{
    REPORT_FUNCTION();

    stop();

    delete mListener;
    mListener = NULL;

    delete mPlayer;
    mPlayer = NULL;

    // FIXME: Figure out how to not have it segfault here
    //delete mAudioSink;
    //mAudioSink = NULL;
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

status_t Player::prepareAudioSink()
{
    assert(mAudioSink != NULL);

    mAudioSink->setVolume(0.1, 0.1);

    return OK;
}

status_t Player::prepare()
{
    REPORT_FUNCTION();
    return prepareAudioSink() && mPlayer->prepare();
}

status_t Player::prepareAsync()
{
    REPORT_FUNCTION();
    return prepareAudioSink() && mPlayer->prepareAsync();
}

status_t Player::start()
{
    REPORT_FUNCTION();
    setAudioSink(mAudioSink);
    mAudioSink->start();
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

    assert(mAudioSink != NULL);
    assert(mPlayer != NULL);

    mAudioSink->stop();
    mAudioSink->flush();
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

void android_media_set_video_size_cb(on_msg_set_video_size cb)
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return;

    player_instance->setVideoSizeCb(cb);
}

Player *android_media_new_player()
{
    REPORT_FUNCTION()

    Player* player = new Player();

    if (player == NULL)
        return NULL;

    player_instance = player;

    player_instance->setUID(0);

    // Required for internal player state processing. Without this, prepare() and start() hang.
    android::ProcessState::self()->startThreadPool();

    return player;
}

status_t android_media_set_data_source(const char* url)
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return BAD_VALUE;

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

void android_media_surface_texture_get_transformation_matrix(GLfloat* matrix)
{
    player_instance->get_transformation_matrix_for_surface_texture(matrix);
}

android::status_t android_media_play()
{
    REPORT_FUNCTION()

    if (player_instance == NULL)
        return BAD_VALUE;

    player_instance->prepare();
    player_instance->start();
    const char *tmp = player_instance->isPlaying() ? "yes" : "no";
    printf("Is playing?: %s\n", tmp);

#if 0
    if (player_instance->isPlaying())
    {
        sleep(2);
        // Seek to 130 seconds in (for ironman3 trailer)
        player_instance->seekTo(130000);
    }
#endif

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

// From android::MediaPlayerListener
void Player::notifyCbStatic(void* cookie, int msg, int ext1, int ext2, const Parcel *obj)
{
    REPORT_FUNCTION();

    assert(cookie != NULL);
    Player *playerClass = static_cast<Player *>(cookie);
    playerClass->notifyCb(msg, ext1, ext2, obj);
}

void Player::notifyCb(int msg, int ext1, int ext2, const Parcel *obj)
{
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
            assert(mAudioSink);
            mAudioSink->flush();
            break;
        case android::MEDIA_SET_VIDEO_SIZE:
            ALOGV("MEDIA_SET_VIDEO_SIZE msg\n");
            if (mSetVideoSizeCb != NULL)
                mSetVideoSizeCb(ext1, ext2, NULL);
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

/* Implementation of AudioSink interface */
#undef LOG_TAG
#define LOG_TAG "PlayerAudioSink"

int Player::PlayerAudioOutput::mMinBufferCount = 4;
bool Player::PlayerAudioOutput::mIsOnEmulator = false;

Player::PlayerAudioOutput::PlayerAudioOutput()
    : mCallback(NULL),
      mCallbackCookie(NULL)
{
    mTrack = 0;
    mStreamType = AUDIO_STREAM_MUSIC;
    mLeftVolume = 1.0;
    mRightVolume = 1.0;
    mLatency = 0;
    mMsecsPerFrame = 0;
    mNumFramesWritten = 0;
    setMinBufferCount();
}

Player::PlayerAudioOutput::~PlayerAudioOutput()
{
    close();
}

void Player::PlayerAudioOutput::setMinBufferCount()
{
    mIsOnEmulator = false;
    mMinBufferCount = 4;
}

bool Player::PlayerAudioOutput::isOnEmulator()
{
    setMinBufferCount();
    return mIsOnEmulator;
}

int Player::PlayerAudioOutput::getMinBufferCount()
{
    setMinBufferCount();
    return mMinBufferCount;
}

ssize_t Player::PlayerAudioOutput::bufferSize() const
{
    if (mTrack == 0) return NO_INIT;
    return mTrack->frameCount() * frameSize();
}

ssize_t Player::PlayerAudioOutput::frameCount() const
{
    if (mTrack == 0) return NO_INIT;
    return mTrack->frameCount();
}

ssize_t Player::PlayerAudioOutput::channelCount() const
{
    if (mTrack == 0) return NO_INIT;
    return mTrack->channelCount();
}

ssize_t Player::PlayerAudioOutput::frameSize() const
{
    if (mTrack == 0) return NO_INIT;
    return mTrack->frameSize();
}

uint32_t Player::PlayerAudioOutput::latency () const
{
    return mLatency;
}

float Player::PlayerAudioOutput::msecsPerFrame() const
{
    return mMsecsPerFrame;
}

status_t Player::PlayerAudioOutput::getPosition(uint32_t *position)
{
    if (mTrack == 0)
        return NO_INIT;
    return mTrack->getPosition(position);
}

status_t Player::PlayerAudioOutput::open(
        uint32_t sampleRate, int channelCount, int format, int bufferCount,
        AudioCallback cb, void *cookie)
{
    mCallback = cb;
    mCallbackCookie = cookie;

    // Check argument "bufferCount" against the mininum buffer count
    if (bufferCount < mMinBufferCount) {
        LOGV("bufferCount (%d) is too small and increased to %d",
            bufferCount, mMinBufferCount);
        bufferCount = mMinBufferCount;

    }
    LOGV("open(%u, %d, %d, %d)", sampleRate, channelCount, format, bufferCount);
    if (mTrack) close();
    int afSampleRate;
    int afFrameCount;
    int frameCount;

    if (AudioSystem::getOutputFrameCount(&afFrameCount, mStreamType) !=
     NO_ERROR) {
        return NO_INIT;
    }
    if (AudioSystem::getOutputSamplingRate(&afSampleRate, mStreamType) !=
     NO_ERROR) {
        return NO_INIT;
    }

    frameCount = (sampleRate*afFrameCount*bufferCount)/afSampleRate;

    AudioTrack *t;
    if (mCallback != NULL) {
        t = new AudioTrack(
                mStreamType,
                sampleRate,
                format,
                (channelCount == 2) ?
                 AUDIO_CHANNEL_OUT_STEREO : AUDIO_CHANNEL_OUT_MONO,
                frameCount,
                0 /* flags */,
                CallbackWrapper,
                this);
    } else {
        t = new AudioTrack(
                mStreamType,
                sampleRate,
                format,
                (channelCount == 2) ?
                 AUDIO_CHANNEL_OUT_STEREO : AUDIO_CHANNEL_OUT_MONO,
                frameCount);
    }

    if ((t == 0) || (t->initCheck() != NO_ERROR)) {
        LOGE("Unable to create audio track");
        delete t;
        return NO_INIT;
    }

    LOGV("setVolume");
    t->setVolume(mLeftVolume, mRightVolume);
    mMsecsPerFrame = 1.e3 / (float) sampleRate;
    mLatency = t->latency();
    mTrack = t;
    return NO_ERROR;
}

void Player::PlayerAudioOutput::start()
{
    LOGV("start");
    if (mTrack) {
        mTrack->setVolume(mLeftVolume, mRightVolume);
        mTrack->start();
        mTrack->getPosition(&mNumFramesWritten);
    }
}

void Player::PlayerAudioOutput::snoopWrite(const void* buffer, size_t size)
{
    // Visualization buffers not supported
    return;
}

ssize_t Player::PlayerAudioOutput::write(const void* buffer, size_t size)
{
    LOG_FATAL_IF(mCallback != NULL, "Don't call write if supplying a callback.");

    LOGV("write(%p, %u)", buffer, size);
    if (mTrack) {
        snoopWrite(buffer, size);
        ssize_t ret = mTrack->write(buffer, size);
        mNumFramesWritten += ret / 4; // assume 16 bit stereo
        return ret;
    }
    return NO_INIT;
}

void Player::PlayerAudioOutput::stop()
{
    LOGV("stop");
    if (mTrack) mTrack->stop();
}

void Player::PlayerAudioOutput::flush()
{
    LOGV("flush");
    if (mTrack) mTrack->flush();
}

void Player::PlayerAudioOutput::pause()
{
    LOGV("PlayerAudioOutput::pause");
    if (mTrack) mTrack->pause();
}

void Player::PlayerAudioOutput::close()
{
    LOGV("close");
    delete mTrack;
    mTrack = 0;
}

void Player::PlayerAudioOutput::setVolume(float left, float right)
{
    LOGV("setVolume(%f, %f)", left, right);
    mLeftVolume = left;
    mRightVolume = right;
    if (mTrack) {
        mTrack->setVolume(left, right);
    }
}

// static
void Player::PlayerAudioOutput::CallbackWrapper(int event, void *cookie, void *info)
{
    //LOGV("PlayerAudioOutput::callbackwrapper");
    if (event != AudioTrack::EVENT_MORE_DATA) {
        return;
    }

    PlayerAudioOutput *me = (PlayerAudioOutput *)cookie;
    AudioTrack::Buffer *buffer = (AudioTrack::Buffer *)info;

    size_t actualSize = (*me->mCallback)(
            me, buffer->raw, buffer->size, me->mCallbackCookie);

    buffer->size = actualSize;

    if (actualSize > 0) {
        me->snoopWrite(buffer->raw, actualSize);
    }
}

status_t Player::PlayerAudioOutput::dump(int fd, const Vector<String16>& args) const
{
    const size_t SIZE = 256;
    char buffer[SIZE];
    String8 result;

    result.append(" PlayerAudioOutput\n");
    snprintf(buffer, SIZE-1, "  stream type(%d), left - right volume(%f, %f)\n",
            mStreamType, mLeftVolume, mRightVolume);
    result.append(buffer);
    snprintf(buffer, SIZE-1, "  msec per frame(%f), latency (%d)\n",
            mMsecsPerFrame, mLatency);
    result.append(buffer);
    ::write(fd, result.string(), result.size());
    if (mTrack != 0) {
        mTrack->dump(fd, args);
    }
    return NO_ERROR;
}

int Player::PlayerAudioOutput::getSessionId()
{
    return mSessionId;
}
