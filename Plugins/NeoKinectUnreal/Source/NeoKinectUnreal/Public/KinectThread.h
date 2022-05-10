// Copyright 2016-2018 Rodrigo Villani Pereira

#pragma once

#include <atomic>

#include "Windows/AllowWindowsPlatformTypes.h"
#include "NeoKinect.h"
#include "Windows/HideWindowsPlatformTypes.h"

#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "HAL/ThreadSafeCounter.h"
#include "CoreMinimal.h"
#include "NeoKinectEnums.h"

using namespace NeoKinect;

DECLARE_LOG_CATEGORY_CLASS(KinectThreadLog, Log, All);

struct FThreadFrameBuffer
{
	KinectFrameType FrameType;
	KinectPixelFormat PixelFormat;
	KinectCoordinateSpace TargetSpace;
	bool bRemapped;

	FThreadFrameBuffer();
	~FThreadFrameBuffer();

	void Configure(KinectFrameType Frame, KinectPixelFormat PixelFormat, KinectCoordinateSpace Space, bool IsRemapped);
	bool AllocateBuffers(uint32 Size);
	void DeallocateBuffers();
	bool IsAllocated();

	// Returns an updated buffer. If it was already consumed, return nullptr
	const uint8* GetReadBuffer(bool bConsume = true);
	uint8* GetWriteBuffer();

	void SetDirty();

	bool TryLock();
	void Lock();
	void Unlock();

	void LockWrite();
	void UnlockWrite();

private:
	std::atomic_bool bIsNew;
	uint8 *Buffer;
	uint8 *BackBuffer;
	FCriticalSection ReadMutex;
	FCriticalSection WriteMutex;

	void SwapBuffers();
};

// Kinect polling thread
class FKinectThread : public FRunnable
{
private:
	/*
	Kinect updates its frames, at most, at 30fps.
	But, as some of them will be available at slightly different times, I'll poll
	it at double that rate to be sure.
	*/
	static constexpr float cDesiredFPS = 60.f;

	// Singleton instance, can access the thread any time via static accessor, if it is active
	static FKinectThread *Runnable;
	// Thread to run FRunnable on
	FRunnableThread *Thread;
	// Stop this thread? Uses Thread Safe Counter
	FThreadSafeCounter StopTaskCounter;

	// Kinect sensor
	KinectSensor *pKinect;

	// Bodies data
	KinectBody Bodies[KinectBodyCount];

	// Face tracking data
	KinectFace Faces[KinectBodyCount];

public:
	// Constructor
	FKinectThread(KinectSensor *pSensor);
	// Destructor
	~FKinectThread();

	// Frames data. These control which frame buffers should or not be updated.
	static FThreadFrameBuffer FrameBuffers[static_cast<uint64>(EKinectFrame::Count)];
	// Controls whether body data should be read
	static bool bUseBodyTracking;
	// True if body data was updated since last reading
	bool bBodiesDirty;
	// Controls whether face tracking data should be read
	static bool bUseFaceTracking;
	// True if faces data was updated since last reading
	bool bFacesDirty;

	// Mutex for body/face data thread safety
	static FCriticalSection BodiesFacesCritSec;

	/*
	Start the thread and the worker from static (easy access)
	This code ensures only 1 KinectThread will be able to run at a time.
	This function returns a handle to the newly started instance.
	*/
	static FKinectThread *InitKinectPolling(KinectSensor *pSensor);

	// Makes sure this thread has stopped properly
	void EnsureCompletion();

	// Shuts down the thread. Static so it can easily be called from outside the thread context
	static void Shutdown();

	// Gets last polled bodies data. Returns nullptr if no new data
	const KinectBody *GetBodiesData();
	// Gets last polled faces data. Returns nullptr if no new data
	const KinectFace *GetFacesData();

	// Begin FRunnable interface
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	virtual void Exit();
	// End FRunnable interface
};