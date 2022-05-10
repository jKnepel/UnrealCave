// Copyright 2016-2018 Rodrigo Villani Pereira

#include "KinectThread.h"

/*-----------------------------------------------------------------------------
	ThreadFrameBuffer
-----------------------------------------------------------------------------*/

FThreadFrameBuffer::FThreadFrameBuffer() :
	bRemapped(false),
	bIsNew(false),
	Buffer(nullptr)
{}

FThreadFrameBuffer::~FThreadFrameBuffer()
{
	DeallocateBuffers();
}

void FThreadFrameBuffer::Configure(KinectFrameType Frame, KinectPixelFormat Format, KinectCoordinateSpace Space, bool IsRemapped)
{
	this->FrameType = Frame;
	this->PixelFormat = Format;
	this->TargetSpace = Space;
	this->bRemapped = IsRemapped;
}

bool FThreadFrameBuffer::AllocateBuffers(uint32 Size)
{
	Buffer = new uint8[Size];
	BackBuffer = new uint8[Size];
	return Buffer && BackBuffer;
}

void FThreadFrameBuffer::DeallocateBuffers()
{
	bIsNew = false;

	Lock();
	if (Buffer) delete[] Buffer;
	Buffer = nullptr;
	Unlock();

	LockWrite();
	if (BackBuffer) delete[] BackBuffer;
	BackBuffer = nullptr;
	UnlockWrite();
}

bool FThreadFrameBuffer::IsAllocated()
{
	return Buffer && BackBuffer;
}

const uint8* FThreadFrameBuffer::GetReadBuffer(bool bConsume /*= true*/)
{
	if (bConsume)
	{
		if (Buffer && bIsNew)
		{
			bIsNew = false;
			return Buffer;
		}
		return nullptr;
	}
	else
	{
		return Buffer;
	}
}

uint8* FThreadFrameBuffer::GetWriteBuffer()
{
	return BackBuffer;
}

void FThreadFrameBuffer::SetDirty()
{
	SwapBuffers();
	bIsNew = true;
}

void FThreadFrameBuffer::SwapBuffers()
{
	Lock();

	uint8* Temp = Buffer;
	Buffer = BackBuffer;
	BackBuffer = Temp;

	Unlock();
}

bool FThreadFrameBuffer::TryLock()
{
	return ReadMutex.TryLock();
}

void FThreadFrameBuffer::Lock()
{
	ReadMutex.Lock();
}

void FThreadFrameBuffer::Unlock()
{
	ReadMutex.Unlock();
}

void FThreadFrameBuffer::LockWrite()
{
	WriteMutex.Lock();
}

void FThreadFrameBuffer::UnlockWrite()
{
	WriteMutex.Unlock();
}


/*-----------------------------------------------------------------------------
	KinectThread
-----------------------------------------------------------------------------*/

FKinectThread* FKinectThread::Runnable = nullptr;
FCriticalSection FKinectThread::BodiesFacesCritSec;

FThreadFrameBuffer FKinectThread::FrameBuffers[static_cast<uint64>(EKinectFrame::Count)];
bool FKinectThread::bUseBodyTracking = false;
bool FKinectThread::bUseFaceTracking = false;

FKinectThread::FKinectThread(KinectSensor *pSensor) :
	pKinect(pSensor),
	bBodiesDirty(false),
	bFacesDirty(false)
{
	Thread = FRunnableThread::Create(this, TEXT("FKinectThread")); // Windows give 8MB memory by default
	if (!Thread)
		UE_LOG(KinectThreadLog, Error, TEXT("Failed to create Kinect thread!"));
}

FKinectThread::~FKinectThread()
{
	if (Thread) delete Thread;
	Thread = nullptr;
}

FKinectThread* FKinectThread::InitKinectPolling(KinectSensor *pSensor)
{
	// Create new instance of thread if it does not exist and the platform
	// supports multi threading.
	if (!Runnable && FPlatformProcess::SupportsMultithreading())
	{
		Runnable = new FKinectThread(pSensor);
	}
	return Runnable;
}

void FKinectThread::EnsureCompletion()
{
	Stop();
	if (Thread)
		Thread->WaitForCompletion();
}

void FKinectThread::Shutdown()
{
	if (Runnable)
	{
		Runnable->EnsureCompletion();
		delete Runnable;
		Runnable = nullptr;
	}
}

const KinectBody* FKinectThread::GetBodiesData()
{
	if (bBodiesDirty)
	{
		bBodiesDirty = false;
		return Bodies;
	}
	return nullptr;
}

const KinectFace* FKinectThread::GetFacesData()
{
	if (bFacesDirty)
	{
		bFacesDirty = false;
		return Faces;
	}
	return nullptr;
}

bool FKinectThread::Init()
{
	UE_LOG(KinectThreadLog, Log, TEXT("Kinect thread inited."));
	return true;
}

uint32 FKinectThread::Run()
{
	static constexpr float UpdateInterval = 1.f / cDesiredFPS;

	UE_LOG(KinectThreadLog, Log, TEXT("Kinect thread running."));

	// while not told to stop this thread
	while (StopTaskCounter.GetValue() == 0)
	{
		// try locking any frames necessary
		static constexpr int FrameTypesCount = static_cast<int>(KinectFrameType::Count);
		
		for (int i = 0; i < FrameTypesCount; ++i)
		{
			const KinectFrameType FrameType = static_cast<KinectFrameType>(i);
			if (pKinect->GetIsUsingFrame(FrameType))
				pKinect->LockLatestFrame(FrameType);
		}

		if (bUseBodyTracking)
		{
			// process bodies before face tracking - face tracking depends on bodies data
			BodiesFacesCritSec.Lock();
			if (pKinect->GetLatestFrameData(Bodies))
				bBodiesDirty = true;
			BodiesFacesCritSec.Unlock();
		}

		if (bUseFaceTracking)
		{
			// process face tracking
			BodiesFacesCritSec.Lock();
			if (pKinect->GetLatestFrameData(Faces))
				bFacesDirty = true;
			BodiesFacesCritSec.Unlock();
		}

		// update any necessary buffers
		for (FThreadFrameBuffer& FrameBuffer : FrameBuffers)
		{
			if (FrameBuffer.IsAllocated() && pKinect->GetIsFrameLocked(FrameBuffer.FrameType))
			{
				FrameBuffer.LockWrite();
				// If this is a remapped type, use the correct function.
				// If the update works, marks buffer as new
				if (FrameBuffer.bRemapped)
				{
					if (pKinect->GetRemappedFrameData(
						FrameBuffer.FrameType, FrameBuffer.TargetSpace,
						FrameBuffer.GetWriteBuffer(), FrameBuffer.PixelFormat))
						FrameBuffer.SetDirty();
				}
				else
				{
					uint8* Buffer = FrameBuffer.GetWriteBuffer();
					if (pKinect->GetLatestFrameData(
						FrameBuffer.FrameType, Buffer,
						FrameBuffer.PixelFormat))
						FrameBuffer.SetDirty();
				}
				FrameBuffer.UnlockWrite();
			}
		}

		// unlock frames
		for (int i = 0; i < FrameTypesCount; ++i)
		{
			pKinect->UnlockLatestFrame(static_cast<KinectFrameType>(i));
		}

		// give time for Kinect to update its frames.
		FPlatformProcess::Sleep(UpdateInterval);
	}

	// ended successfully!
	return 0;
}

void FKinectThread::Stop()
{
	StopTaskCounter.Increment();
}

void FKinectThread::Exit()
{
	UE_LOG(KinectThreadLog, Log, TEXT("Kinect thread ended."));
}