#include <CDX_LogNDebug.h>
#define LOG_TAG "CedarXRecorder"
#include <utils/Log.h>

#include "CedarXRecorder.h"

#include <binder/IPCThreadState.h>
#include <binder/IServiceManager.h>
#include <media/stagefright/MediaDebug.h>
#include <media/stagefright/MetadataBufferType.h>
#include <media/stagefright/MetaData.h>
#include <media/MediaProfiles.h>
#include <camera/ICamera.h>
#include <camera/CameraParameters.h>
#include <surfaceflinger/Surface.h>

#include <type_camera.h>
#include <CDX_PlayerAPI.h>
#include <OMX_Core.h>
#include <OMX_Component.h>

#include <include_writer/recorde_writer.h>

#define F_LOG 	LOGV("%s, line: %d", __FUNCTION__, __LINE__);

extern "C" int CedarXRecorderCallbackWrapper(void *cookie, int event, void *info);

namespace android {

//----------------------------------------------------------------------------------
// CDXCameraListener
//----------------------------------------------------------------------------------
struct CDXCameraListener : public CameraListener 
{
	CDXCameraListener(CedarXRecorder * recorder);

    virtual void notify(int32_t msgType, int32_t ext1, int32_t ext2);
    virtual void postData(int32_t msgType, const sp<IMemory> &dataPtr, camera_frame_metadata_t *metadata);
    virtual void postDataTimestamp(nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr);

protected:
    virtual ~CDXCameraListener();

private:
	CedarXRecorder * mRecorder;

    CDXCameraListener(const CDXCameraListener &);
    CDXCameraListener &operator=(const CDXCameraListener &);
};


CDXCameraListener::CDXCameraListener(CedarXRecorder * recorder)
    : mRecorder(recorder) 
{
	LOGV("CDXCameraListener Construct\n");
}

CDXCameraListener::~CDXCameraListener() {
	LOGV("CDXCameraListener Destruct\n");
}

void CDXCameraListener::notify(int32_t msgType, int32_t ext1, int32_t ext2) 
{
    LOGV("notify(%d, %d, %d)", msgType, ext1, ext2);
}

void CDXCameraListener::postData(int32_t msgType, const sp<IMemory> &dataPtr, camera_frame_metadata_t *metadata) 
{
    LOGV("postData(%d, ptr:%p, size:%d)",
         msgType, dataPtr->pointer(), dataPtr->size());
}

void CDXCameraListener::postDataTimestamp(
        nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) 
{
	// LOGD("CDXCameraListener::postDataTimestamp\n");
	mRecorder->dataCallbackTimestamp(timestamp, msgType, dataPtr);
}

// To collect the encoder usage for the battery app
static void addBatteryData(uint32_t params) {
    sp<IBinder> binder =
        defaultServiceManager()->getService(String16("media.player"));
    sp<IMediaPlayerService> service = interface_cast<IMediaPlayerService>(binder);
    CHECK(service.get() != NULL);

    service->addBatteryData(params);
}

//----------------------------------------------------------------------------------
// CedarXRecorder
//----------------------------------------------------------------------------------

CedarXRecorder::CedarXRecorder()
    : mTimeBetweenTimeLapseVideoFramesUs(0)
    , mLastTimeLapseFrameRealTimestampUs(0)
    , mLastTimeLapseFrameTimestampUs(0)
	, mOutputFd(-1)
	, mStarted(false)
	, mRecModeFlag(0)
	, mRecord(NULL)
	, mLatencyStartUs(0)
{

    LOGV("Constructor");

    mInputBuffer = NULL;

	reset();

	mFrameHeap = new MemoryHeapBase(sizeof(int));
	if (mFrameHeap->getHeapID() < 0)
	{
		LOGE("ERR(%s): Record heap creation fail", __func__);
        mFrameHeap.clear();
	}
	mFrameBuffer = new MemoryBase(mFrameHeap, 0, sizeof(int));
}

CedarXRecorder::~CedarXRecorder() 
{
    LOGV("CedarXRecorder Destructor");

	if (mFrameHeap != NULL)
	{
		mFrameHeap.clear();
		mFrameHeap = NULL;
	}

	LOGV("CedarXRecorder Destructor OK");
}

status_t CedarXRecorder::setAudioSource(audio_source_t as) 
{
    LOGV("setAudioSource: %d", as);

	mAudioSource = as;

    return OK;
}

status_t CedarXRecorder::setVideoSource(video_source vs) 
{
    LOGV("setVideoSource: %d", vs);

	mVideoSource = vs;
	
    return OK;
}

status_t CedarXRecorder::setOutputFormat(output_format of) 
{
    LOGV("setOutputFormat: %d", of);

	mOutputFormat = of;

    return OK;
}

status_t CedarXRecorder::setAudioEncoder(audio_encoder ae) 
{
    LOGV("setAudioEncoder: %d", ae);
   
	mRecModeFlag |= RECORDER_MODE_AUDIO;
	
    return OK;
}

status_t CedarXRecorder::setVideoEncoder(video_encoder ve) 
{
    LOGV("setVideoEncoder: %d", ve);
    
	mRecModeFlag |= RECORDER_MODE_VIDEO;
	
    return OK;
}

status_t CedarXRecorder::setVideoSize(int width, int height) 
{
    LOGV("setVideoSize: %dx%d", width, height);
   
    // Additional check on the dimension will be performed later
    mVideoWidth = width;
    mVideoHeight = height;

    return OK;
}

status_t CedarXRecorder::setVideoFrameRate(int frames_per_second) 
{
    LOGV("setVideoFrameRate: %d", frames_per_second);
    
    // Additional check on the frame rate will be performed later
    mFrameRate = frames_per_second;

    return OK;
}

status_t CedarXRecorder::setCamera(const sp<ICamera>& camera, const sp<ICameraRecordingProxy>& proxy)
{
    LOGV("setCamera");

	int err = UNKNOWN_ERROR;

    int64_t token = IPCThreadState::self()->clearCallingIdentity();
	
	if ((err = isCameraAvailable(camera, proxy, mCameraId)) != OK) {
        LOGE("Camera connection could not be established.");
    	IPCThreadState::self()->restoreCallingIdentity(token);
        return err;
    }
	
    IPCThreadState::self()->restoreCallingIdentity(token);

    return OK;
}

status_t CedarXRecorder::setMediaSource(const sp<MediaSource>& mediasource, int type)
{
    LOGV("setMediaSource");

    if(CDX_RECORDER_MEDIATYPE_VIDEO == type) {
    	mMediaSourceVideo = mediasource;
    }
    else {
    	;
    }

    return OK;
}

status_t CedarXRecorder::isCameraAvailable(
    const sp<ICamera>& camera, const sp<ICameraRecordingProxy>& proxy,
    int32_t cameraId) 
{
    if (camera == 0) 
	{
        mCamera = Camera::connect(cameraId);
        if (mCamera == 0) 
		{
			return -EBUSY;
        }
        mCameraFlags &= ~FLAGS_HOT_CAMERA;
    } 
	else 
	{
        // We get the proxy from Camera, not ICamera. We need to get the proxy
        // to the remote Camera owned by the application. Here mCamera is a
        // local Camera object created by us. We cannot use the proxy from
        // mCamera here.
        mCamera = Camera::create(camera);
        if (mCamera == 0) 
		{
			return -EBUSY;
        }
        mCameraProxy = proxy;
        mCameraFlags |= FLAGS_HOT_CAMERA;
        mDeathNotifier = new DeathNotifier();
        // isBinderAlive needs linkToDeath to work.
        mCameraProxy->asBinder()->linkToDeath(mDeathNotifier);
    }

    // This CHECK is good, since we just passed the lock/unlock
    // check earlier by calling mCamera->setParameters().
    CHECK_EQ(OK, mCamera->setPreviewDisplay(mPreviewSurface));
/*
	// store metadata in video buffers
	if (OK != mCamera->storeMetaDataInBuffers(true)) 
	{
		LOGW("storeMetaDataInBuffers failed");
	}
*/
	mCamera->sendCommand(CAMERA_CMD_SET_CEDARX_RECORDER, 0, 0);

    mCamera->lock();

    return OK;
}


status_t CedarXRecorder::setPreviewSurface(const sp<Surface>& surface)
{
    LOGV("setPreviewSurface: %p", surface.get());
    mPreviewSurface = surface;

    return OK;
}

status_t CedarXRecorder::queueBuffer(int index, int addr_y, int addr_c, int64_t timestamp)
{
    LOGV("queueBuffer index:%d",index);
    V4L2BUF_t buf;
    int ret;

    buf.index = index;
    buf.timeStamp = timestamp;
    buf.addrPhyY = addr_y;
    buf.width = mVideoWidth;
    buf.height = mVideoHeight;
    buf.crop_rect.top = 0;
    buf.crop_rect.left = 0;
    buf.crop_rect.width = buf.width;
    buf.crop_rect.height = buf.height;

    // if encoder is stopped or paused ,release this frame
	if (mStarted == false)
	{
		CedarXReleaseFrame(buf.index);
		return OK;
	}

//	if ((readTimeUs - mLatencyStartUs) < VIDEO_LATENCY_TIME)
//	{
//		CedarXReleaseFrame(buf.index);
//		return OK;
//	}

	ret = CDXRecorder_Control(CDX_CMD_SEND_BUF, (unsigned int)&buf, 0);
	if (ret != 0)
	{
		CedarXReleaseFrame(buf.index);
	}

    return OK;
}

status_t CedarXRecorder::setParamGeoDataLongitude(int64_t longitudex10000) 
{
    LOGD("setParamGeoDataLongitude: %lld", longitudex10000);
    if (longitudex10000 > 1800000 || longitudex10000 < -1800000) {
        return BAD_VALUE;
    }
	mGeoAvailable = true;
    mLongitudex10000 = longitudex10000;
    return OK;
}

status_t CedarXRecorder::setParamGeoDataLatitude(int64_t latitudex10000) 
{
    LOGD("setParamGeoDataLatitude: %lld", latitudex10000);
    if (latitudex10000 > 900000 || latitudex10000 < -900000) {
        return BAD_VALUE;
    }
	mGeoAvailable = true;
    mLatitudex10000 = latitudex10000;
    return OK;
}

status_t CedarXRecorder::setOutputFile(int fd) 
{
    LOGV("setOutputFile: %d", fd);

    mOutputFd = fd;

    return OK;
}

status_t CedarXRecorder::setParamVideoCameraId(int32_t cameraId) 
{
    LOGV("setParamVideoCameraId: %d", cameraId);
	
    mCameraId = cameraId;
	
    return OK;
}

status_t CedarXRecorder::setParamAudioEncodingBitRate(int32_t bitRate) 
{
    LOGV("setParamAudioEncodingBitRate: %d", bitRate);
	
    mAudioBitRate = bitRate;
	
    return OK;
}

status_t CedarXRecorder::setParamAudioSamplingRate(int32_t sampleRate) 
{
	LOGV("setParamAudioSamplingRate: %d", sampleRate);

	// Additional check on the sample rate will be performed later.
	mSampleRate = sampleRate;
    return OK;
}

status_t CedarXRecorder::setParamAudioNumberOfChannels(int32_t channels) 
{
    LOGV("setParamAudioNumberOfChannels: %d", channels);

    // Additional check on the number of channels will be performed later.
    mAudioChannels = channels;
    return OK;
}

status_t CedarXRecorder::setParamMaxFileDurationUs(int64_t timeUs) 
{
    LOGV("setParamMaxFileDurationUs: %lld us", timeUs);

    mMaxFileDurationUs = timeUs;
    return OK;
}

status_t CedarXRecorder::setParamMaxFileSizeBytes(int64_t bytes) 
{
    LOGV("setParamMaxFileSizeBytes: %lld bytes", bytes);
	
	mMaxFileSizeBytes = bytes;

	if (mMaxFileSizeBytes > (int64_t)MAX_FILE_SIZE)
	{
		mMaxFileSizeBytes = (int64_t)MAX_FILE_SIZE;
    	LOGD("force maxFileSizeBytes to %lld bytes", mMaxFileSizeBytes);
	}
	
    return OK;
}

status_t CedarXRecorder::setParamVideoEncodingBitRate(int32_t bitRate) 
{
    LOGV("setParamVideoEncodingBitRate: %d", bitRate);
	
    mVideoBitRate = bitRate;
    return OK;
}

// Always rotate clockwise, and only support 0, 90, 180 and 270 for now.
status_t CedarXRecorder::setParamVideoRotation(int32_t degrees) 
{
    LOGV("setParamVideoRotation: %d", degrees);
	
    mRotationDegrees = degrees % 360;

    return OK;
}

status_t CedarXRecorder::setParamTimeLapseEnable(int32_t timeLapseEnable) {
    LOGV("setParamTimeLapseEnable: %d", timeLapseEnable);

	mCaptureTimeLapse = timeLapseEnable;
    return OK;
}

status_t CedarXRecorder::setParamTimeBetweenTimeLapseFrameCapture(int64_t timeUs) {
    LOGV("setParamTimeBetweenTimeLapseFrameCapture: %lld us", timeUs);

    mTimeBetweenTimeLapseFrameCaptureUs = timeUs;
    return OK;
}

status_t CedarXRecorder::setListener(const sp<IMediaRecorderClient> &listener) 
{
    mListener = listener;

    return OK;
}

static void AudioRecordCallbackFunction(int event, void *user, void *info) {
    switch (event) {
        case AudioRecord::EVENT_MORE_DATA: {
            LOGW("AudioRecord reported EVENT_MORE_DATA!");
            break;
        }
        case AudioRecord::EVENT_OVERRUN: {
            LOGW("AudioRecord reported EVENT_OVERRUN!");
            break;
        }
        default:
            // does nothing
            break;
    }
}

status_t CedarXRecorder::CreateAudioRecorder()
{
	CHECK(mAudioChannels == 1 || mAudioChannels == 2);
    uint32_t flags = AudioRecord::RECORD_AGC_ENABLE |
                     AudioRecord::RECORD_NS_ENABLE  |
                     AudioRecord::RECORD_IIR_ENABLE;
    mRecord = new AudioRecord(
                mAudioSource, mSampleRate, AUDIO_FORMAT_PCM_16_BIT,
                mAudioChannels > 1? AUDIO_CHANNEL_IN_STEREO: AUDIO_CHANNEL_IN_MONO,
                4 * kMaxBufferSize / sizeof(int16_t), /* Enable ping-pong buffers */
                flags,
                NULL,	// AudioRecordCallbackFunction,
                this);

	if (mRecord == NULL)
	{
		LOGE("create AudioRecord failed");
		return UNKNOWN_ERROR;
	}

	status_t err = mRecord->initCheck();
	if (err != OK)
	{
		LOGE("AudioRecord is not initialized");
		return UNKNOWN_ERROR;
	}

	LOGV("~~~~~~~~~~~~~~framesize: %d ", mRecord->frameSize());
	LOGV("~~~~~~~~~~~~~~frameCount: %d ", mRecord->frameCount());		
	LOGV("~~~~~~~~~~~~~~channelCount: %d ", mRecord->channelCount());
	LOGV("~~~~~~~~~~~~~~getSampleRate: %d ", mRecord->getSampleRate());

	return OK;
}

int convertMuxerMode(int inputFormat)
{
	int muxer_mode;

	switch(inputFormat) {
	case OUTPUT_FORMAT_AWTS:
		muxer_mode = MUXER_MODE_AWTS;
		break;
	default:
		muxer_mode = MUXER_MODE_MP4;
		break;
	}

	return muxer_mode;
}

status_t CedarXRecorder::prepare() 
{
	LOGV("prepare");
	int srcWidth = 0, srcHeight = 0;
	int ret = OK;
    int muxer_mode;

	// Create audio recorder
	if (mRecModeFlag & RECORDER_MODE_AUDIO)
	{
		ret = CreateAudioRecorder();
		if (ret != OK)
		{
			LOGE("CreateAudioRecorder failed");
			return ret;
		}
	}

	// CedarX init
	CDXRecorder_Init(this);
	
	// set recorder mode to CDX_Recorder
	CDXRecorder_Control(CDX_CMD_SET_REC_MODE, mRecModeFlag, 0);
	
	CDXRecorder_Control(CDX_CMD_SET_OUTPUT_FORMAT, convertMuxerMode(mOutputFormat), 0);

	// create CedarX component
	ret = CDXRecorder_Control(CDX_CMD_PREPARE, 0, 0);
	if( ret == -1)
	{
		printf("CEDARX REPARE ERROR!\n");
		return UNKNOWN_ERROR;
	}

	// register callback
	CDXRecorder_Control(CDX_CMD_REGISTER_CALLBACK, (unsigned int)&CedarXRecorderCallbackWrapper, (unsigned int)this);
	
	// set file handle to CDX_Recorder render component
	ret = CDXRecorder_Control(CDX_CMD_SET_SAVE_FILE, (unsigned int)mOutputFd, 0);
	if(ret != OK)
	{
		LOGE("CedarXRecorder::prepare, CDX_CMD_SET_SAVE_FILE failed\n");
		return ret;
	}

	if (mVideoSource <= VIDEO_SOURCE_CAMERA)
	{
		int64_t token = IPCThreadState::self()->clearCallingIdentity();
		// Set the actual video recording frame size
		CameraParameters params(mCamera->getParameters());
		params.setPreviewSize(mVideoWidth, mVideoHeight);
		String8 s = params.flatten();
		if (OK != mCamera->setParameters(s)) {
			LOGE("Could not change settings."
				 " Someone else is using camera %d?", mCameraId);
			IPCThreadState::self()->restoreCallingIdentity(token);
			return -EBUSY;
		}
		CameraParameters newCameraParams(mCamera->getParameters());

		// Check on video frame size
		newCameraParams.getPreviewSize(&srcWidth, &srcHeight);
		if (srcWidth  == 0 || srcHeight == 0) {
			LOGE("Failed to set the video frame size to %dx%d",
					mVideoWidth, mVideoHeight);
			IPCThreadState::self()->restoreCallingIdentity(token);
			return UNKNOWN_ERROR;
		}
		IPCThreadState::self()->restoreCallingIdentity(token);
	
		LOGV("src: %dx%d, video: %dx%d", srcWidth, srcHeight, mVideoWidth, mVideoHeight);
	}
	else
	{
		srcWidth = mVideoWidth;
		srcHeight = mVideoHeight;
	}

	// set video size and FrameRate to CDX_Recorder
	VIDEOINFO_t vInfo;
	memset((void *)&vInfo, 0, sizeof(VIDEOINFO_t));

#if 0
	vInfo.video_source 		= mVideoSource;
	vInfo.src_width			= 176;
	vInfo.src_height		= 144;
	vInfo.width				= 160;			// mVideoWidth;
	vInfo.height			= 120;			// mVideoHeight;
	vInfo.frameRate			= 30*1000;		// mFrameRate;
	vInfo.bitRate			= 200000;		// mVideoBitRate;
	vInfo.profileIdc		= 66;
	vInfo.levelIdc			= 31;
	vInfo.geo_available		= mGeoAvailable;
	vInfo.latitudex10000	= mLatitudex10000;
	vInfo.longitudex10000	= mLongitudex10000;
	vInfo.rotate_degree		= mRotationDegrees;
#else
	vInfo.video_source 		= mVideoSource;
	vInfo.src_width			= srcWidth;
	vInfo.src_height		= srcHeight;
	vInfo.width				= mVideoWidth;
	vInfo.height			= mVideoHeight;
	vInfo.frameRate			= mFrameRate*1000;
	vInfo.bitRate			= mVideoBitRate;
	vInfo.profileIdc		= 66;
	vInfo.levelIdc			= 31;
	vInfo.geo_available		= mGeoAvailable;
	vInfo.latitudex10000	= mLatitudex10000;
	vInfo.longitudex10000	= mLongitudex10000;
	vInfo.rotate_degree		= mRotationDegrees;
#endif

	if (mVideoWidth == 0
		|| mVideoHeight == 0
		|| mFrameRate == 0
		|| mVideoBitRate == 0)
	{
		LOGE("error video para");
		return -1;
	}
	
	ret = CDXRecorder_Control(CDX_CMD_SET_VIDEO_INFO, (unsigned int)&vInfo, 0);
	if(ret != OK)
	{
		LOGE("CedarXRecorder::prepare, CDX_CMD_SET_VIDEO_INFO failed\n");
		return ret;
	}

	if (mRecModeFlag & RECORDER_MODE_AUDIO)
	{
		// set audio parameters
		AUDIOINFO_t aInfo;
		memset((void*)&aInfo, 0, sizeof(AUDIOINFO_t));
		aInfo.bitRate = mAudioBitRate;
		aInfo.channels = mAudioChannels;
		aInfo.sampleRate = mSampleRate;
		aInfo.bitsPerSample = 16;
		if (mAudioBitRate == 0
			|| mAudioChannels == 0
			|| mSampleRate == 0)
		{
			LOGE("error audio para");
			return -1;
		}

		ret = CDXRecorder_Control(CDX_CMD_SET_AUDIO_INFO, (unsigned int)&aInfo, 0);
		if(ret != OK)
		{
			LOGE("CedarXRecorder::prepare, CDX_CMD_SET_AUDIO_INFO failed\n");
			return ret;
		}	
	}

	// time lapse mode
	if (mCaptureTimeLapse)
	{
		LOGD("time lapse mode*****************************");
		mTimeBetweenTimeLapseVideoFramesUs = 1E6/mFrameRate;
		CDXRecorder_Control(CDX_CMD_SET_TIME_LAPSE, 0, 0);
	}

    return OK;
}

status_t CedarXRecorder::start() 
{
	LOGV("start");
	Mutex::Autolock autoLock(mStateLock);
	
	CHECK(mOutputFd >= 0);

	if (mVideoSource <= VIDEO_SOURCE_CAMERA)
	{
		LOGV("startCameraRecording");
		// Reset the identity to the current thread because media server owns the
		// camera and recording is started by the applications. The applications
		// will connect to the camera in ICameraRecordingProxy::startRecording.
		int64_t token = IPCThreadState::self()->clearCallingIdentity();
		if (mCameraFlags & FLAGS_HOT_CAMERA)
		{
			mCamera->unlock();
			mCamera.clear();
			CHECK_EQ(OK, mCameraProxy->startRecording(new CameraProxyListener(this)));
		}
		else
		{
			mCamera->setListener(new CDXCameraListener(this));
			mCamera->startRecording();
			CHECK(mCamera->recordingEnabled());
		}
		IPCThreadState::self()->restoreCallingIdentity(token);
	}

	// audio start
	if ((mRecModeFlag & RECORDER_MODE_AUDIO)
		&& mRecord != NULL)
	{
		mRecord->start();
	}

	mLatencyStartUs = systemTime() / 1000;
	LOGV("mLatencyStartUs: %lldus", mLatencyStartUs);
	LOGV("VIDEO_LATENCY_TIME: %dus, AUDIO_LATENCY_TIME: %dus", VIDEO_LATENCY_TIME, AUDIO_LATENCY_TIME);

	mStarted = true;
	CDXRecorder_Control(CDX_CMD_START, 0, 0);

	LOGV("CedarXRecorder::start OK\n");
    return OK;
}

status_t CedarXRecorder::pause() 
{
    LOGV("pause");
	Mutex::Autolock autoLock(mStateLock);

	mStarted = false;

	if(CDXRecorder_Control(CDX_CMD_GETSTATE, 0, 0) == CDX_STATE_PAUSE)
	{
		CDXRecorder_Control(CDX_CMD_RESUME, 0, 0);
	}
	else
	{
		CDXRecorder_Control(CDX_CMD_PAUSE, 0, 0);
	}
		
    return OK;
}

status_t CedarXRecorder::stop() 
{
    LOGV("stop");
	
    status_t err = OK;
	
	{
		Mutex::Autolock autoLock(mStateLock);
		if (mStarted == true)
		{
			mStarted = false;
		}
		else
		{
			return err;
		}
	}	

	if (mVideoSource <= VIDEO_SOURCE_CAMERA)
	{
		int64_t token = IPCThreadState::self()->clearCallingIdentity();
		if (mCameraFlags & FLAGS_HOT_CAMERA) {
			mCameraProxy->stopRecording();
		} else {
			mCamera->setListener(NULL);
			mCamera->stopRecording();
		}
		IPCThreadState::self()->restoreCallingIdentity(token);
	}

	CDXRecorder_Control(CDX_CMD_STOP, 0, 0);
	
	CDXRecorder_Exit();
	
	releaseCamera();

	// audio stop
	if (mRecModeFlag & RECORDER_MODE_AUDIO
		&& mRecord != NULL)
	{
		mRecord->stop();
		delete mRecord;
		mRecord = NULL;
	}

	LOGV("stopped\n");

	return err;
}

void CedarXRecorder::releaseCamera()
{
    if (mVideoSource <= VIDEO_SOURCE_CAMERA)
    {
    	LOGV("releaseCamera");

		if (mCamera != 0)
		{
			int64_t token = IPCThreadState::self()->clearCallingIdentity();
			if ((mCameraFlags & FLAGS_HOT_CAMERA) == 0)
			{
				LOGV("Camera was cold when we started, stopping preview");
				mCamera->stopPreview();
				mCamera->disconnect();
			}
			mCamera->unlock();
			mCamera.clear();
			mCamera = 0;
			IPCThreadState::self()->restoreCallingIdentity(token);
		}

		if (mCameraProxy != 0)
		{
			mCameraProxy->asBinder()->unlinkToDeath(mDeathNotifier);
			mCameraProxy.clear();
		}
    }

    mCameraFlags = 0;
}

status_t CedarXRecorder::reset() 
{
    LOGV("reset");

    // No audio or video source by default
    mAudioSource = AUDIO_SOURCE_CNT;
    mVideoSource = VIDEO_SOURCE_LIST_END;

    // Default parameters
    mOutputFormat  = OUTPUT_FORMAT_THREE_GPP;
    mAudioEncoder  = AUDIO_ENCODER_AAC;
    mVideoEncoder  = VIDEO_ENCODER_H264;
    mVideoWidth    = 176;
    mVideoHeight   = 144;
    mFrameRate     = 20;
    mVideoBitRate  = 192000;
    mSampleRate    = 8000;
    mAudioChannels = 1;
    mAudioBitRate  = 12200;
    mCameraId      = 0;
	mCameraFlags   = 0;

    mMaxFileDurationUs = 0;
    mMaxFileSizeBytes = 0;

    mRotationDegrees = 0;

    mOutputFd = -1;

	mStarted = false;
	mRecModeFlag = 0;
	mRecord = NULL;
	mLatencyStartUs = 0;

	mGeoAvailable = 0;
	mLatitudex10000 = -3600000;
    mLongitudex10000 = -3600000;

    return OK;
}

status_t CedarXRecorder::getMaxAmplitude(int *max) 
{
    LOGV("getMaxAmplitude");

	// to do
	*max = 100;
	
    return OK;
}

void CedarXRecorder::releaseOneRecordingFrame(const sp<IMemory>& frame) 
{
    if (mCameraProxy != NULL) {
        mCameraProxy->releaseRecordingFrame(frame);
    } else if (mCamera != NULL) {
        int64_t token = IPCThreadState::self()->clearCallingIdentity();
        mCamera->releaseRecordingFrame(frame);
        IPCThreadState::self()->restoreCallingIdentity(token);
    }
}

void CedarXRecorder::dataCallbackTimestamp(int64_t timestampUs,
        int32_t msgType, const sp<IMemory> &data) 
{
	unsigned int duration = 0;
	int64_t fileSizeBytes = 0;
	V4L2BUF_t buf;
	int ret = -1;
	int64_t readTimeUs = systemTime() / 1000;
	
	if (data == NULL)
	{
		LOGE("error IMemory data\n");
		return;
	}
	
	memcpy((void *)&buf, data->pointer(), sizeof(V4L2BUF_t));
	
	// if encoder is stopped or paused ,release this frame
	if (mStarted == false)
	{
		CedarXReleaseFrame(buf.index);
		return ;
	}

	if ((readTimeUs - mLatencyStartUs) < VIDEO_LATENCY_TIME)
	{
		CedarXReleaseFrame(buf.index);
		return ;
	}

	// time lapse mode
	if (mCaptureTimeLapse)
	{
		// LOGV("readTimeUs : %lld, lapse: %lld", readTimeUs, mLastTimeLapseFrameRealTimestampUs + mTimeBetweenTimeLapseFrameCaptureUs);
		if (readTimeUs < mLastTimeLapseFrameRealTimestampUs + mTimeBetweenTimeLapseFrameCaptureUs)
		{
			CedarXReleaseFrame(buf.index);
			return ;
		}
		mLastTimeLapseFrameRealTimestampUs = readTimeUs;

		buf.timeStamp = mLastTimeLapseFrameTimestampUs + mTimeBetweenTimeLapseVideoFramesUs;
		mLastTimeLapseFrameTimestampUs = buf.timeStamp;
	}
	
	// LOGV("CedarXRecorder::dataCallbackTimestamp: addrPhyY %x, timestamp %lld us", buf.addrPhyY, timestampUs);

	ret = CDXRecorder_Control(CDX_CMD_SEND_BUF, (unsigned int)&buf, 0); 
	if (ret != 0)
	{
		CedarXReleaseFrame(buf.index);
	}

	CDXRecorder_Control(CDX_CMD_GET_DURATION, (unsigned int)&duration, 0); 
	// LOGV("duration : %d", duration);
	
	if (mMaxFileDurationUs != 0 
		&& duration >= mMaxFileDurationUs / 1000)
	{
		mListener->notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_DURATION_REACHED, 0);
	}

	CDXRecorder_Control(CDX_CMD_GET_FILE_SIZE, (int64_t)&fileSizeBytes, 0); 
	// LOGV("fileSizeBytes : %lld", fileSizeBytes);
	
	if (mMaxFileSizeBytes > 0 
		&& fileSizeBytes >= mMaxFileSizeBytes)
	{
		mListener->notify(MEDIA_RECORDER_EVENT_INFO, MEDIA_RECORDER_INFO_MAX_FILESIZE_REACHED, 0);
	}
}

void CedarXRecorder::CedarXReleaseFrame(int index)
{
	if (mVideoSource <= VIDEO_SOURCE_CAMERA)
	{
		int * p = (int *)(mFrameBuffer->pointer());

		*p = index;
	
		releaseOneRecordingFrame(mFrameBuffer);
	}
	else
	{
		LOGV("CedarXReleaseFrame index=%d", index);
		mListener->notify(MEDIA_RECORDER_VENDOR_EVENT_EMPTY_BUFFER_ID, index, 0);
	}
}

status_t CedarXRecorder::CedarXReadAudioBuffer(void *pbuf, int *size, int64_t *timeStamp)
{	
    int64_t readTimeUs = systemTime() / 1000;

	// LOGV("CedarXRecorder::CedarXReadAudioBuffer, readTimeUs: %lld", readTimeUs);

    *size = 0;
	*timeStamp = readTimeUs;
	
	ssize_t n = mRecord->read(pbuf, kMaxBufferSize);
	if (n < 0)
	{
		LOGE("mRecord read audio buffer failed");
		return UNKNOWN_ERROR;
	}

	//LOGV("readTimeUs:%lld,mLatencyStartUs:%lld diff:%lld",readTimeUs, mLatencyStartUs, readTimeUs - mLatencyStartUs);
	if ((readTimeUs - mLatencyStartUs) < AUDIO_LATENCY_TIME)
	{
		return INVALID_OPERATION;
	}
	
	*size = n;

	// LOGV("timestamp: %lld, len: %d", readTimeUs, n);

	return OK;
}

status_t CedarXRecorder::readMediaBufferCallback(void *buf_header)
{
	OMX_BUFFERHEADERTYPE *omx_buf_header = (OMX_BUFFERHEADERTYPE*)buf_header;

	if(omx_buf_header->nFlags == OMX_PortDomainVideo)
	{
		//LOGV("readMediaBufferCallback 1");
		if (mInputBuffer) {
			mInputBuffer->release();
			mInputBuffer = NULL;
		}
		mMediaSourceVideo->read(&mInputBuffer, NULL);

		if (mInputBuffer != NULL) {
			OMX_U32 type;
			buffer_handle_t buffer_handle;
			char *data = (char *)mInputBuffer->data();

			type = *((OMX_U32*)data);
			memcpy(&buffer_handle, data+4, 4);

			LOGV("mInputBuffer->size() = %d type = %d buffer_handle = %p",mInputBuffer->size(),type,buffer_handle);
			if (type == kMetadataBufferTypeGrallocSource) {

			}

			mInputBuffer->meta_data()->findInt64(kKeyTime, &omx_buf_header->nTimeStamp);
		}
		else {
			return NO_MEMORY;
		}
	}

	return OK;
}

CedarXRecorder::CameraProxyListener::CameraProxyListener(CedarXRecorder * recorder) 
	:mRecorder(recorder){
}

void CedarXRecorder::CameraProxyListener::dataCallbackTimestamp(
        nsecs_t timestamp, int32_t msgType, const sp<IMemory>& dataPtr) {
    mRecorder->dataCallbackTimestamp(timestamp / 1000, msgType, dataPtr);
}

#if 0

extern "C" int CedarXRecReadAudioBuffer(void *p, void *pbuf, int *size, int64_t *timeStamp)
{
	return ((android::CedarXRecorder*)p)->CedarXReadAudioBuffer(pbuf, size, timeStamp);
}

extern "C" void CedarXRecReleaseOneFrame(void *p, int index) 
{
	((android::CedarXRecorder*)p)->CedarXReleaseFrame(index);
}

#else

int CedarXRecorder::CedarXRecorderCallback(int event, void *info)
{
	int ret = 0;
	int *para = (int*)info;

	//LOGV("----------CedarXRecorderCallback event:%d info:%p\n", event, info);

	switch (event) {
	case CDX_EVENT_READ_AUDIO_BUFFER:
		CedarXReadAudioBuffer((void *)para[0], (int*)para[1], (int64_t*)para[2]);
		break;
	case CDX_EVENT_RELEASE_VIDEO_BUFFER:
		CedarXReleaseFrame(*para);
		break;
	case CDX_EVENT_MEDIASOURCE_READ:
		ret = readMediaBufferCallback(info);
		break;
	default:
		break;
	}

	return ret;
}

extern "C" int CedarXRecorderCallbackWrapper(void *cookie, int event, void *info)
{
	return ((android::CedarXRecorder *)cookie)->CedarXRecorderCallback(event, info);
}

#endif

}  // namespace android

