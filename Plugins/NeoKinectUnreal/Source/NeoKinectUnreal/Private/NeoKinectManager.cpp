// Copyright 2016-2022 Rodrigo Villani Pereira

#include "NeoKinectManager.h"
#include "KinectThread.h"
#include "NeoKinectBody.h"
#include "NeoKinectFace.h"

#include "RenderingThread.h"
#include "Containers/Ticker.h"
#include "Engine/TextureDefines.h"
#include "Engine/TextureRenderTarget2D.h"

#define CUR_CLASS_FUNC (FString(__FUNCTION__))

/*
* STRUCTS METHODS
*/
#pragma region Structs

FFrameTextureData::~FFrameTextureData()
{
	Release();
}

bool FFrameTextureData::IsTextureValid() const
{
	if (!IsValid(Texture) || UFrameType == EKinectFrame::Count)
		return false;

	const FThreadFrameBuffer &ThreadBuffer = FKinectThread::FrameBuffers[static_cast<uint64>(UFrameType)];
	return ThreadBuffer.IsAllocated();
}

void FFrameTextureData::Release()
{
	if (UFrameType == EKinectFrame::Count)
		return;

	FThreadFrameBuffer &ThreadBuffer = FKinectThread::FrameBuffers[static_cast<uint64>(UFrameType)];
	ThreadBuffer.DeallocateBuffers();

	if (IsValid(Texture))
	{
		Texture->ReleaseResource();
		Texture->RemoveFromRoot();
		Texture->ConditionalBeginDestroy();
	}
	Texture = nullptr;
}

FrameDescription FFrameTextureData::GetFrameDescription(const KinectSensor *Kinect) const
{
	return Kinect->GetFrameDescription(KFrameType);
}

#pragma endregion Structs

/*
* STATICS
*/
#pragma region Statics

KinectSensor* UNeoKinectManager::Kinect = nullptr;
FKinectThread* UNeoKinectManager::KinectThread = nullptr;
//bool UNeoKinectManager::bAutoShutdown = false;

bool UNeoKinectManager::bUsingBodyFrame = false;
bool UNeoKinectManager::bUsingJointsColorSpaceTransforms = false;
bool UNeoKinectManager::bUsingFaceFrame = false;

TArray<UNeoKinectBody*> UNeoKinectManager::Bodies;
TArray<UNeoKinectFace*> UNeoKinectManager::Faces;
FFrameTextureData UNeoKinectManager::FramesData[static_cast<int32>(EKinectFrame::Count)];

UNeoKinectManager* UNeoKinectManager::EventsDispatcher = nullptr;
FTSTicker::FDelegateHandle UNeoKinectManager::TickerHandle;


#pragma endregion Statics

/*
* INITIALIZATION / UNINITIALIZATION
*/
#pragma region Initialization_Uninitialization

UNeoKinectManager::UNeoKinectManager()
{
	if (!Kinect) return;

	// Bind to the bodies and faces tracking events
	for (int32 i = 0; i < KinectBodyCount; ++i)
	{
		Bodies[i]->OnBodyBeginTrack.AddUniqueDynamic(this, &UNeoKinectManager::BodyBeginTrackBroadcast);
		Bodies[i]->OnBodyLost.AddUniqueDynamic(this, &UNeoKinectManager::BodyLostBroadcast);
		Faces[i]->OnFaceBeginTrack.AddUniqueDynamic(this, &UNeoKinectManager::FaceBeginTrackBroadcast);
		Faces[i]->OnFaceLost.AddUniqueDynamic(this, &UNeoKinectManager::FaceLostBroadcast);
	}
}

UNeoKinectManager::~UNeoKinectManager()
{
	OnBodyBeginTrack.Clear();
	OnBodyLost.Clear();
	OnFaceBeginTrack.Clear();
	OnFaceLost.Clear();

	UninitSensor();
	this->RemoveFromRoot();
	EventsDispatcher = nullptr;
}

void UNeoKinectManager::initStatics()
{
	// do we actually need to init?
	if (!(Bodies.Num() != KinectBodyCount || Faces.Num() != KinectBodyCount
		|| Bodies[2]->BodyIndex != 2 || Faces[2]->Index != 2
		|| FramesData[2].UFrameType != EKinectFrame::NormalizedDepth))
		return; // if none of the above were true

	// reset any allocated memory
	Bodies.Empty(KinectBodyCount);
	Faces.Empty(KinectBodyCount);
	for (FFrameTextureData& FrameData : FramesData)
	{
		FrameData.Release();
	}

	Bodies.SetNum(KinectBodyCount, true);
	Faces.SetNum(KinectBodyCount, true);

	for (int32 i = 0; i < KinectBodyCount; ++i)
	{
		Bodies[i] = NewObject<UNeoKinectBody>();
		Bodies[i]->AddToRoot();
		Bodies[i]->BodyIndex = i;
		Faces[i] = NewObject<UNeoKinectFace>();
		Faces[i]->AddToRoot();
		Faces[i]->Index = i;
	}

	FFrameTextureData* FrameData = &FramesData[static_cast<int32>(EKinectFrame::Color)];
	FrameData->UPixelFormat = EPixelFormat::PF_B8G8R8A8;
	FrameData->KPixelFormat = KinectPixelFormat::B8G8R8A8;
	FrameData->Width = KinectColorWidth;
	FrameData->Height = KinectColorHeight;
	FrameData->PixelSize = 4;
	FrameData->KFrameType = KinectFrameType::Color;
	FrameData->bRemapped = false;
	FrameData->bIsSRGB = true;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::Depth)];
	FrameData->UPixelFormat = EPixelFormat::PF_G16;
	FrameData->KPixelFormat = KinectPixelFormat::G16;
	FrameData->Width = KinectDepthWidth;
	FrameData->Height = KinectDepthHeight;
	FrameData->PixelSize = 2;
	FrameData->KFrameType = KinectFrameType::Depth;
	FrameData->bRemapped = false;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::NormalizedDepth)];
	FrameData->UPixelFormat = EPixelFormat::PF_B8G8R8A8;
	FrameData->KPixelFormat = KinectPixelFormat::B8G8R8A8;
	FrameData->Width = KinectDepthWidth;
	FrameData->Height = KinectDepthHeight;
	FrameData->PixelSize = 4;
	FrameData->KFrameType = KinectFrameType::Depth;
	FrameData->bRemapped = false;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::BodyIndex)];
	FrameData->UPixelFormat = EPixelFormat::PF_G8;
	FrameData->KPixelFormat = KinectPixelFormat::G8;
	FrameData->Width = KinectDepthWidth;
	FrameData->Height = KinectDepthHeight;
	FrameData->PixelSize = 1;
	FrameData->KFrameType = KinectFrameType::BodyIndex;
	FrameData->bRemapped = false;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::BodyIndexColor)];
	FrameData->UPixelFormat = EPixelFormat::PF_B8G8R8A8;
	FrameData->KPixelFormat = KinectPixelFormat::B8G8R8A8;
	FrameData->Width = KinectDepthWidth;
	FrameData->Height = KinectDepthHeight;
	FrameData->PixelSize = 4;
	FrameData->KFrameType = KinectFrameType::BodyIndex;
	FrameData->bRemapped = false;
	FrameData->bIsSRGB = true;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::Color_DepthSpace)];
	FrameData->UPixelFormat = EPixelFormat::PF_B8G8R8A8;
	FrameData->KPixelFormat = KinectPixelFormat::B8G8R8A8;
	FrameData->Width = KinectDepthWidth;
	FrameData->Height = KinectDepthHeight;
	FrameData->PixelSize = 4;
	FrameData->KFrameType = KinectFrameType::Color;
	FrameData->bRemapped = true;
	FrameData->TargetSpace = KinectCoordinateSpace::Depth;
	FrameData->bIsSRGB = true;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::Depth_ColorSpace)];
	FrameData->UPixelFormat = EPixelFormat::PF_G16;
	FrameData->KPixelFormat = KinectPixelFormat::G16;
	FrameData->Width = KinectColorWidth;
	FrameData->Height = KinectColorHeight;
	FrameData->PixelSize = 2;
	FrameData->KFrameType = KinectFrameType::Depth;
	FrameData->bRemapped = true;
	FrameData->TargetSpace = KinectCoordinateSpace::Color;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::BodyIndex_ColorSpace)];
	FrameData->UPixelFormat = EPixelFormat::PF_G8;
	FrameData->KPixelFormat = KinectPixelFormat::G8;
	FrameData->Width = KinectColorWidth;
	FrameData->Height = KinectColorHeight;
	FrameData->PixelSize = 1;
	FrameData->KFrameType = KinectFrameType::BodyIndex;
	FrameData->bRemapped = true;
	FrameData->TargetSpace = KinectCoordinateSpace::Color;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::Infrared)];
	FrameData->UPixelFormat = EPixelFormat::PF_G16;
	FrameData->KPixelFormat = KinectPixelFormat::G16;
	FrameData->Width = KinectDepthWidth;
	FrameData->Height = KinectDepthHeight;
	FrameData->PixelSize = 2;
	FrameData->KFrameType = KinectFrameType::Infrared;
	FrameData->bRemapped = false;

	FrameData = &FramesData[static_cast<int32>(EKinectFrame::LongExposureInfrared)];
	FrameData->UPixelFormat = EPixelFormat::PF_G16;
	FrameData->KPixelFormat = KinectPixelFormat::G16;
	FrameData->Width = KinectDepthWidth;
	FrameData->Height = KinectDepthHeight;
	FrameData->PixelSize = 2;
	FrameData->KFrameType = KinectFrameType::LongExposureInfrared;
	FrameData->bRemapped = false;

	for (int32 i = 0; i < static_cast<int32>(EKinectFrame::Count); ++i)
	{
		FFrameTextureData& fd = FramesData[i];

		fd.UFrameType = static_cast<EKinectFrame>(i);
		fd.BufferSize = fd.Width * fd.Height * fd.PixelSize;
	}
}
#pragma endregion Initialization_Uninitialization

/*
* BLUEPRINT FUNCTIONS
*/
#pragma region Blueprint_Functions

UNeoKinectManager* UNeoKinectManager::GetKinectEventsDispatcher()
{
	return EventsDispatcher;
}

/*
*	BASICS
*/
#pragma region Basics

void UNeoKinectManager::InitSensor()
{
	if (!Kinect)
	{
		Kinect = new KinectSensor();
		if (Kinect->InitSensor())
		{
			UE_LOG(NeoKinectLog, Warning, TEXT("Kinect sensor initialized."));
			// start Kinect polling thread
			KinectThread = FKinectThread::InitKinectPolling(Kinect);
			initStatics();

			// set bodies, faces and textures update timer
			TickerHandle = FTSTicker::GetCoreTicker().AddTicker(FTickerDelegate::CreateStatic(&OnTick), 0.016666666f);

			// prepare the events handler
			if (!EventsDispatcher)
			{
				EventsDispatcher = NewObject<UNeoKinectManager>();
				EventsDispatcher->AddToRoot();
			}
		}
		else
		{
			UE_LOG(NeoKinectLog, Error, TEXT("Kinect sensor failed to initialize."));
			Kinect = nullptr;
		}
	}
}

bool UNeoKinectManager::IsKinectAvailable()
{
	return Kinect != nullptr && Kinect->IsSensorAvailable();
}

void UNeoKinectManager::UninitSensor()
{
	// end Kinect polling thread
	if (KinectThread)
	{
		KinectThread->Shutdown();
		KinectThread = nullptr;
	}

	// release frame textures
	for (FFrameTextureData& FrameData : FramesData)
	{
		FrameData.Release();
	}

	// zero bodies and faces values
	ResetBodies();
	ResetFaces();

	// get global timer manager (World independent)
	FTSTicker::GetCoreTicker().RemoveTicker(TickerHandle);

	if (Kinect) // release sensor
	{
		SetUseBodyFrame(false);
		SetUseFaceFrame(false);

		Kinect->UninitSensor();
		UE_LOG(NeoKinectLog, Warning, TEXT("Kinect sensor uninitialized."));
		delete Kinect;
		Kinect = nullptr;
	}
}

KinectSensor* UNeoKinectManager::GetSensor()
{
	return Kinect;
}

/*void UNeoKinectManager::SetAutoShutdown(const bool bValue)
{
	bAutoShutdown = bValue;
}

bool UNeoKinectManager::GetAutoShutdown()
{
	return bAutoShutdown;
}*/

float UNeoKinectManager::GetSensorTilt()
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return 0.0f;
	}
	return Kinect->GetSensorTilt();
}

FVector UNeoKinectManager::GetGroundPlane(float &Height)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		Height = 0.0f;
		return FVector(0.0f);
	}

	const Vector4 FloorClipPlane = Kinect->GetFloorClipPlane();
	Height = FloorClipPlane.w * 100.0f;
	return FVector(
		-FloorClipPlane.z,
		FloorClipPlane.x,
		FloorClipPlane.y
		);
}

#pragma endregion Basics

#pragma region Exposure

void UNeoKinectManager::SetAutoExposure(float exposureCompensation /* = 0.0f */)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return;
	}

	Kinect->SetAutoExposure(exposureCompensation);
}

void UNeoKinectManager::SetSemiAutoExposure(float pseudoExposureTime)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return;
	}

	Kinect->SetSemiAutoExposure(pseudoExposureTime);
}

void UNeoKinectManager::SetManualExposure(float frameExposureTime, float analogGain)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return;
	}

	Kinect->SetManualExposure(frameExposureTime, analogGain);
}

#pragma endregion Exposure

#pragma region Remapping

FVector UNeoKinectManager::CameraLocationToColorWithDepth(FVector CameraLocation)
{
	Vector3 KinectLocation(
		-CameraLocation.Y * 0.01f,
		CameraLocation.Z * 0.01f,
		CameraLocation.X * 0.01f);

	KinectLocation = CameraLocationToColorWithDepth(KinectLocation);
	
	return FVector(
		KinectLocation.z,
		-KinectLocation.x,
		KinectLocation.y) * 100.0f;
}

Vector3 UNeoKinectManager::CameraLocationToColorWithDepth(Vector3 CameraLocation)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return Vector3(0.0f);
	}

	Vector2 ColorLocation;

	if (Kinect->GetRemappedPoint(KinectCoordinateSpace::Camera, KinectCoordinateSpace::Color,
		CameraLocation, ColorLocation
		))
	{
		//  when the joint can not be seen in color camera
		if (ColorLocation.x == -std::numeric_limits<float>::infinity()
			|| ColorLocation.y == -std::numeric_limits<float>::infinity())
		{
			return Vector3(0.0f);
		}

		static const FrameDescription ColorDescription = Kinect->GetFrameDescription(KinectFrameType::Color);
		const float HalfWidth = static_cast<float>(ColorDescription.width >> 1); // half frame width
		const float HalfHeight = static_cast<float>(ColorDescription.height >> 1); // half frame height
		const float HalfFovH = FMath::DegreesToRadians(ColorDescription.horizontalFieldOfView / 2.f); // half horizontal FOV in radians
		const float HalfFovV = FMath::DegreesToRadians(ColorDescription.verticalFieldOfView / 2.f); // half vertical FOV in radians
		const float HoriCornerTan = FMath::Tan(HalfFovH);
		const float VertCornerTan = FMath::Tan(HalfFovV);

		const float xPct = (ColorLocation.x - HalfWidth) / HalfWidth;
		const float yPct = (ColorLocation.y - HalfHeight) / HalfHeight;

		// position at z = 1m from the camera
		const float x = xPct * HoriCornerTan;
		const float y = -yPct * VertCornerTan;
		const Vector3 Direction(x, y, 1.0f);

		return Direction * CameraLocation.z;
	}

	return Vector3(0.0f);
}

FVector2D UNeoKinectManager::CameraLocationToColorPoint(FVector CameraLocation)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FVector2D(0.0f, 0.0f);
	}

	const Vector3 KinectLocation(
		-CameraLocation.Y * 0.01f,
		CameraLocation.Z * 0.01f,
		CameraLocation.X * 0.01f);
	Vector2 ColorPoint;

	if (Kinect->GetRemappedPoint(KinectCoordinateSpace::Camera, KinectCoordinateSpace::Color,
		KinectLocation, ColorPoint))
	{
		return FVector2D(ColorPoint.x, ColorPoint.y);
	}

	return FVector2D(0.0f, 0.0f);
}

FVector2D UNeoKinectManager::CameraLocationToDepthPoint(FVector CameraLocation)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FVector2D(0.0f, 0.0f);
	}

	const Vector3 KinectLocation(
		-CameraLocation.Y * 0.01f,
		CameraLocation.Z * 0.01f,
		CameraLocation.X * 0.01f);
	Vector2 DepthPoint;

	if (Kinect->GetRemappedPoint(KinectCoordinateSpace::Camera, KinectCoordinateSpace::Depth,
		KinectLocation, DepthPoint))
	{
		return FVector2D(DepthPoint.x, DepthPoint.y);
	}

	return FVector2D(0.0f, 0.0f);
}

Vector3 UNeoKinectManager::ColorWithDepthFromColorOnly(const Vector2& ColorLocation)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return Vector3(0.0f);
	}

	const uint16 Depth = Kinect->SampleDepthFromColorCoords(ColorLocation);

	if (!Kinect || Depth == 0 /* invalid depth */)
		return Vector3(0.0f);

	const float DepthMeters = static_cast<float>(Depth) * 0.001f;
	
	static const FrameDescription ColorDescription = Kinect->GetFrameDescription(KinectFrameType::Color);
	const float HalfWidth = static_cast<float>(ColorDescription.width >> 1); // half frame width
	const float HalfHeight = static_cast<float>(ColorDescription.height >> 1); // half frame height
	const float HalfFovH = FMath::DegreesToRadians(ColorDescription.horizontalFieldOfView / 2.f); // half horizontal FOV in radians
	const float HalfFovV = FMath::DegreesToRadians(ColorDescription.verticalFieldOfView / 2.f); // half vertical FOV in radians
	const float HoriCornerTan = FMath::Tan(HalfFovH);
	const float VertCornerTan = FMath::Tan(HalfFovV);

	const float xPct = (ColorLocation.x - HalfWidth) / HalfWidth;
	const float yPct = (ColorLocation.y - HalfHeight) / HalfHeight;

	// normal axis
	const float xNormal = FMath::Sin(FMath::Atan(xPct * HoriCornerTan));
	const float yNormal = FMath::Sin(FMath::Atan(-yPct * VertCornerTan));
	const Vector3 Direction(
		xNormal,
		yNormal,
		FMath::Sqrt(1 - (yNormal * yNormal + xNormal * xNormal))
		);

	return Direction * DepthMeters;
}

FVector2D UNeoKinectManager::DepthPointToColorPoint(FVector2D DepthPoint)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FVector2D(0.0f, 0.0f);
	}

	const Vector2 Depth(DepthPoint.X, DepthPoint.Y);
	Vector2 ColorPoint;

	if (Kinect->GetRemappedPoint(KinectCoordinateSpace::Depth, KinectCoordinateSpace::Color,
		Depth, ColorPoint))
	{
		return FVector2D(ColorPoint.x, ColorPoint.y);
	}

	return FVector2D(0.0f, 0.0f);
}

FVector UNeoKinectManager::DepthPointToCameraLocation(FVector2D DepthPoint)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FVector(0.0f);
	}

	const Vector2 Depth(DepthPoint.X, DepthPoint.Y);
	Vector3 Result;

	if (Kinect->GetRemappedPoint(KinectCoordinateSpace::Depth, KinectCoordinateSpace::Camera,
		Depth, Result))
	{
		return FVector(Result.z, -Result.x, Result.y) * 100.0f;
	}

	return FVector(0.0f);
}

FVector2D UNeoKinectManager::ColorPointToDepthPoint(FVector2D ColorPoint)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FVector2D(0.0f, 0.0f);
	}

	const Vector2 Color(ColorPoint.X, ColorPoint.Y);
	Vector2 DepthPoint;

	if (!Kinect->GetRemappedPoint(KinectCoordinateSpace::Color, KinectCoordinateSpace::Depth,
		Color, DepthPoint))
		return FVector2D(0.0f, 0.0f);

	return FVector2D(DepthPoint.x, DepthPoint.y);
}

#pragma endregion Remapping

/*
*	TEXTURES
*/
#pragma region Textures

FVector2D UNeoKinectManager::GetFrameFieldOfView(EKinectFrame FrameType)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FVector2D(0.0f, 0.0f);
	}
	const FrameDescription fd = FramesData[static_cast<int32>(FrameType)].GetFrameDescription(Kinect);

	return FVector2D(fd.horizontalFieldOfView, fd.verticalFieldOfView);
}

FIntPoint UNeoKinectManager::GetFrameSize(EKinectFrame FrameType)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FIntPoint(0, 0);
	}

	const FFrameTextureData& Frame = FramesData[static_cast<int32>(FrameType)];
	if (Frame.bRemapped)
	{
		return FIntPoint(Frame.Width, Frame.Height);
	}
	else
	{
		const FrameDescription fd = Frame.GetFrameDescription(Kinect);

		return FIntPoint(fd.width, fd.height);
	}
}

UTextureRenderTarget2D* UNeoKinectManager::SetUseFrame(EKinectFrame FrameType, bool bEnable)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return nullptr;
	}

	FFrameTextureData& FrameData = FramesData[static_cast<uint32>(FrameType)];

	if (bEnable)
	{
		if (FrameData.IsTextureValid())
			return FrameData.Texture;

		// if it's a remapped type, we'll need the depth frame
		if (FramesData[static_cast<int32>(FrameType)].bRemapped)
		{
			if (!Kinect->SetUseFrame(KinectFrameType::Depth)) // if depth frame fails to be used
				return nullptr;
		}

		// configures and allocate the buffer memory
		FThreadFrameBuffer &ThreadBuffer = FKinectThread::FrameBuffers[static_cast<uint64>(FrameType)];
		ThreadBuffer.Configure(FrameData.KFrameType, FrameData.KPixelFormat,
			FrameData.TargetSpace, FrameData.bRemapped);
		// checks if allocation was successful
		if (!ThreadBuffer.AllocateBuffers(FrameData.BufferSize))
		{
			UE_LOG(NeoKinectLog, Error, TEXT("Failed to allocate memory for texture buffer!"));
			return nullptr;
		}

		// alert Kinect of frame usage
		if (!Kinect->SetUseFrame(FrameData.KFrameType))
			return nullptr;

		// create the GPU texture
		FrameData.Texture = CreateTexture(
			FrameData.Width, FrameData.Height,
			FrameData.UPixelFormat, FrameData.bIsSRGB);
		
		return FrameData.Texture;
	}
	else // bEnable == false
	{
		// Free unused resources
		ReleaseFrameResources(FrameData);
		return nullptr;
	}
}

bool UNeoKinectManager::GetIsUsingFrame(EKinectFrame FrameType)
{
	const FFrameTextureData& FrameData = FramesData[static_cast<uint32>(FrameType)];

	return FrameData.IsTextureValid();
}

UTextureRenderTarget2D* UNeoKinectManager::GetFrame(EKinectFrame FrameType)
{
	const FFrameTextureData& FrameData = FramesData[static_cast<uint32>(FrameType)];
	
	if (FrameData.IsTextureValid())
		return FrameData.Texture;

	return nullptr;
}

float UNeoKinectManager::GetDepthFramePixel(FVector2D Coordinate)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return 0.0f;
	}
	
	return static_cast<float>(Kinect->SampleDepthFrame(Vector2(Coordinate.X, Coordinate.Y))) * 0.1f;
}

float UNeoKinectManager::GetDepthFromColorFramePixel(FVector2D Coordinate)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return 0.0f;
	}

	return static_cast<float>(Kinect->SampleDepthFromColorCoords(Vector2(Coordinate.X, Coordinate.Y))) * 0.1f;
}

FLinearColor UNeoKinectManager::GetColorFramePixel(FVector2D Coordinate)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return FLinearColor();
	}
	if (!IsUsingColor())
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s but the Color frame is not in use"), *CUR_CLASS_FUNC);
		return FLinearColor();
	}

	FThreadFrameBuffer& ThreadBuffer(FKinectThread::FrameBuffers[static_cast<uint64>(EKinectFrame::Color)]);
	uint8 const *Buffer = ThreadBuffer.GetReadBuffer(false);
	if (!Buffer) return FLinearColor();

	const static uint32 ColorWidth = GetFrameSize(EKinectFrame::Color).X;
	const static uint32 ColorHeight = GetFrameSize(EKinectFrame::Color).Y;

	if (static_cast<uint32>(Coordinate.X) >= ColorWidth || static_cast<uint32>(Coordinate.Y) >= ColorHeight)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("%s: coordinates were beyond frame dimensions and were clamped."), *CUR_CLASS_FUNC);
		Coordinate.X = FMath::Min(ColorWidth - 1, static_cast<uint32>(Coordinate.X));
		Coordinate.Y = FMath::Min(ColorHeight - 1, static_cast<uint32>(Coordinate.Y));
	}

	ThreadBuffer.Lock();
	const FLinearColor Color = *(reinterpret_cast<const FColor*>(
		Buffer + 4 * (ColorWidth * static_cast<uint32>(Coordinate.Y) + static_cast<int32>(Coordinate.X))
		));
	ThreadBuffer.Unlock();

	return Color;
}
#pragma endregion Textures

/*
*	BODIES / JOINTS
*/
#pragma region Bodies_Joints

TArray<UNeoKinectBody*> UNeoKinectManager::GetBodies()
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return TArray<UNeoKinectBody*>();
	}
	SetUseBodyFrame();
	
	return Bodies;
}

TArray<UNeoKinectBody*> UNeoKinectManager::GetTrackedBodies()
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return TArray<UNeoKinectBody*>();
	}
	SetUseBodyFrame();

	TArray<UNeoKinectBody*> tBodies;
	tBodies.Reserve(KinectBodyCount);
	for (UNeoKinectBody*& Body : Bodies)
	{
		if (Body->bIsTracked)
			tBodies.Add(Body);
	}

	return tBodies;
}

UNeoKinectBody* UNeoKinectManager::GetNearestBody(bool& HasBody)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return nullptr;
	}

	SetUseBodyFrame();

	UNeoKinectBody* nBody = nullptr;
	for (UNeoKinectBody*& Body : Bodies)
	{
		if (Body->bIsTracked)
		{
			// if no nearest body
			if (nBody == nullptr) nBody = Body;
			// else, if current body's hip is nearest from the sensor than nBody's
			else if (nBody->Joints[static_cast<int32>(EKinectJointType::SpineBase)].Location.Size() >
				Body->Joints[static_cast<int32>(EKinectJointType::SpineBase)].Location.Size())
				nBody = Body;
		}
	}
	HasBody = nBody != nullptr;
	return nBody;
}

void UNeoKinectManager::SetUseJointsColorSpaceTransforms(const bool bUse)
{
	if (bUsingJointsColorSpaceTransforms == bUse) return;

	bUsingJointsColorSpaceTransforms = bUse;
	if (!bUse && Kinect)
	{
		for (UNeoKinectBody *& uBody : Bodies)
		{
			for (FKinectJoint &Joint : uBody->Joints)
			{
				Joint.ColorLocation = FVector::ZeroVector;
				Joint.ColorOrientation = FRotator::ZeroRotator;
			}
		}
	}
}

bool UNeoKinectManager::IsUsingJointsColorSpaceTransforms()
{
	return Kinect && bUsingJointsColorSpaceTransforms;
}

int32 UNeoKinectManager::BodyToIndex(const UNeoKinectBody* Body)
{
	return Body->BodyIndex;
}

UNeoKinectBody* UNeoKinectManager::IndexToBody(const int32 Index)
{
	if (Index < 0 || Index >= Bodies.Num())
		return Bodies[0];
	return Bodies[Index];
}

int32 UNeoKinectManager::JointToIndex(const EKinectJointType Joint)
{
	return static_cast<int32>(Joint);
}

EKinectJointType UNeoKinectManager::IndexToJoint(const int32 Index)
{
	return static_cast<EKinectJointType>(Index >= KinectJointCount || Index < 0 ? 0 : Index);
}
#pragma endregion Bodies_Joints

/*
*	FACE TRACKING
*/
#pragma region Face_Tracking

TArray<UNeoKinectFace*> UNeoKinectManager::GetFaces()
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return TArray<UNeoKinectFace*>();
	}

	SetUseFaceFrame();

	return Faces;
}

TArray<UNeoKinectFace*> UNeoKinectManager::GetTrackedFaces()
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return TArray<UNeoKinectFace*>();
	}

	SetUseFaceFrame();

	TArray<UNeoKinectFace*> tFaces;
	tFaces.Reserve(KinectBodyCount);
	for (UNeoKinectFace*& Face : Faces)
	{
		if (Face->bIsTracked)
			tFaces.Add(Face);
	}

	return tFaces;
}

UNeoKinectFace* UNeoKinectManager::GetNearestFace(bool& HasAnyFace)
{
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return nullptr;
	}

	SetUseFaceFrame();

	UNeoKinectFace* nFace = nullptr;
	for (UNeoKinectFace*& Face : Faces)
	{
		if (Face->bIsTracked)
		{
			// if no nearest face
			if (nFace == nullptr) nFace = Face;
			// else, if current face's pivot is nearest from the sensor than nFace's
			else if (nFace->Location.Size() > Face->Location.Size())
				nFace = Face;
		}
	}
	HasAnyFace = nFace != nullptr;
	return nFace;
}

int32 UNeoKinectManager::FaceToIndex(const UNeoKinectFace *Face)
{
	return Face->Index;
}

UNeoKinectFace* UNeoKinectManager::IndexToFace(const int32 Index)
{
	if (Index < 0 || Index >= Faces.Num())
		return Faces[0];
	return Faces[Index];
}
#pragma endregion Face_Tracking

/*
*	CONSTANTS
*/
#pragma region Constants 

int32 UNeoKinectManager::GetTrackableBodyCount()
{
	return KinectBodyCount;
}

int32 UNeoKinectManager::GetTrackableFaceCount()
{
	return KinectBodyCount;
}

int32 UNeoKinectManager::GetJointCount()
{
	return KinectJointCount;
}
#pragma endregion Constants

#pragma endregion Blueprint_Functions

#pragma region NonBlueprintable

#pragma region Utils

bool UNeoKinectManager::OnTick(float DeltaTime)
{
	if (Kinect && Kinect->IsSensorAvailable() && KinectThread)
	{
		/*
		Read the data from the Kinect polling thread and update stuff as necessary
		*/

		// try locking body and face data. If it doesn't work, leave to next frame.
		if (bUsingBodyFrame && KinectThread->bBodiesDirty
			&& FKinectThread::BodiesFacesCritSec.TryLock())
		{
			// process bodies before face tracking - face tracking depends on bodies data
			ProcessBodies(KinectThread->GetBodiesData());

			FKinectThread::BodiesFacesCritSec.Unlock();
		}

		if (bUsingFaceFrame && KinectThread->bFacesDirty
			&& FKinectThread::BodiesFacesCritSec.TryLock())
		{
			ProcessFaces(KinectThread->GetFacesData());

			FKinectThread::BodiesFacesCritSec.Unlock();
		}

		// Update textures on GPU from render thread
		ENQUEUE_RENDER_COMMAND(FUpdateFrames)([](FRHICommandListImmediate& RHICmdList){ UpdateFrames(); });
	}
	return true;
}

bool UNeoKinectManager::IsUsingDepth()
{
	for (const FFrameTextureData& fd : FramesData)
	{
		if ((fd.KFrameType == KinectFrameType::Depth || fd.bRemapped)
			&& fd.IsTextureValid())
			return true;
	}
	return false;
}

bool UNeoKinectManager::IsUsingBodyIndex()
{
	for (const FFrameTextureData& fd : FramesData)
	{
		if (fd.KFrameType == KinectFrameType::BodyIndex && fd.IsTextureValid())
			return true;
	}
	return false;
}

bool UNeoKinectManager::IsUsingColor()
{
	for (const FFrameTextureData& fd : FramesData)
	{
		if (fd.KFrameType == KinectFrameType::Color && fd.IsTextureValid())
			return true;
	}
	return false;
}
#pragma endregion Utils

/*
* TEXTURES
*/
#pragma region Textures

UTextureRenderTarget2D* UNeoKinectManager::CreateTexture(int32 Width, int32 Height, EPixelFormat PixelFormat, bool sRGB)
{
	// Create the texture
	UTextureRenderTarget2D* Texture = NewObject<UTextureRenderTarget2D>(GetTransientPackage(), NAME_None, EObjectFlags::RF_Transient);
	check(Texture);
#if WITH_EDITORONLY_DATA
	Texture->MipGenSettings = TMGS_NoMipmaps;
#endif // WITH_EDITORONLY_DATA
	Texture->OverrideFormat = PixelFormat;
	Texture->RenderTargetFormat = [&PixelFormat, &sRGB]()
	{
		switch (PixelFormat)
		{
		case EPixelFormat::PF_G8: return ETextureRenderTargetFormat::RTF_R8;
		case EPixelFormat::PF_G16: return ETextureRenderTargetFormat::RTF_R16f;
		case EPixelFormat::PF_B8G8R8A8:
			if (sRGB)
				return ETextureRenderTargetFormat::RTF_RGBA8_SRGB;
			return ETextureRenderTargetFormat::RTF_RGBA8;

		default:;
		}
		return ETextureRenderTargetFormat::RTF_RGBA8;
	}();
	Texture->bAutoGenerateMips = false;
	Texture->SRGB = sRGB;
	Texture->bForceLinearGamma = !sRGB;
	Texture->InitAutoFormat(Width, Height);
	Texture->UpdateResource();
	Texture->AddToRoot();

	return Texture;
}

void UNeoKinectManager::UpdateFrames()
{
	// if the game thread issued a shutdown, the textures may become invalid at any time
	// so, we check first
	if (Kinect)
	{
		// Update the textures whose buffer is new
		constexpr int32 n = static_cast<int32>(EKinectFrame::Count);
		for (int32 i = 0; i < n; ++i)
		{
			FFrameTextureData& FrameData(FramesData[static_cast<uint64>(i)]);
			FThreadFrameBuffer& ThreadBuffer(FKinectThread::FrameBuffers[static_cast<uint64>(i)]);
			const uint8* Buffer = ThreadBuffer.GetReadBuffer();

			if (Buffer && FrameData.IsTextureValid())
			{
				ThreadBuffer.Lock();
				{
					RHIUpdateTexture2D(
						FrameData.Texture->GetRenderTargetResource()->GetRenderTargetTexture(),
						0,
						FUpdateTextureRegion2D(0, 0, 0, 0, FrameData.Width, FrameData.Height),
						FrameData.Width * FrameData.PixelSize,
						Buffer
						);
				}
				ThreadBuffer.Unlock();
			}
		}
	}
}

void UNeoKinectManager::ReleaseFrameResources(FFrameTextureData& FrameData)
{
	FrameData.Release();

	switch (FrameData.KFrameType)
	{
	case KinectFrameType::Color:
		if (!IsUsingColor() && Kinect)
			Kinect->SetUseFrame(KinectFrameType::Color, false);
		break;

	case KinectFrameType::Depth:
		if (!IsUsingDepth() && Kinect)
			Kinect->SetUseFrame(KinectFrameType::Depth, false);
		break;

	case KinectFrameType::BodyIndex:
		if (!IsUsingBodyIndex() && Kinect)
			Kinect->SetUseFrame(KinectFrameType::BodyIndex, false);

	default:
		break;
	}
}

#pragma endregion Textures

/*
* BODY PROCESSING
*/
#pragma region Body_Process

void UNeoKinectManager::SetUseBodyFrame(bool bUse)
{
	if (bUse == bUsingBodyFrame) return;
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return;
	}

	Kinect->SetUseFrame(KinectFrameType::Body, bUse);

	FKinectThread::BodiesFacesCritSec.Lock();
	FKinectThread::bUseBodyTracking = bUse;
	FKinectThread::BodiesFacesCritSec.Unlock();

	bUsingBodyFrame = bUse;
}

void UNeoKinectManager::ProcessBodies(const KinectBody* KBodies)
{
	if (!KBodies) return;

	for (int32 i = 0; i < KinectBodyCount; ++i)
	{
		UNeoKinectManager::Bodies[i]->UpdateFromKinectBody(KBodies[i], *Kinect);
	}
}

void UNeoKinectManager::ResetBodies()
{
	for (int32 i = 0; i < KinectBodyCount; ++i)
	{
		if (!Bodies.IsValidIndex(i)) continue;
		Bodies[i]->Reset();
	}
}
#pragma endregion Body_Process

/*
* FACE TRACKING PROCESSING
*/
#pragma region Face_Process

void UNeoKinectManager::SetUseFaceFrame(bool bUse)
{
	if (bUse == bUsingFaceFrame) return;
	if (!Kinect)
	{
		UE_LOG(NeoKinectLog, Warning,
			TEXT("Tried to call %s without initializing the sensor"), *CUR_CLASS_FUNC);
		return;
	}
	
	// face pivot and tracking solving depends on body data
	if (bUse) SetUseBodyFrame();

	Kinect->SetUseFrame(KinectFrameType::Face, bUse);

	FKinectThread::BodiesFacesCritSec.Lock();
	FKinectThread::bUseFaceTracking = bUse;
	FKinectThread::BodiesFacesCritSec.Unlock();

	bUsingFaceFrame = bUse;
}

void UNeoKinectManager::ProcessFaces(const KinectFace* KFaces)
{
	if (!KFaces) return;
	for (int32 i = 0; i < KinectBodyCount; ++i)
	{
		UNeoKinectManager::Faces[i]->UpdateFromKinectFace(KFaces[i]);
	}
}

void UNeoKinectManager::ResetFaces()
{
	for (int32 i = 0; i < KinectBodyCount; ++i)
	{
		if (!Faces.IsValidIndex(i)) continue;
		Faces[i]->Reset();
	}
}
#pragma endregion Face_Process

void UNeoKinectManager::BodyBeginTrackBroadcast(int32 BodyIndex)
{
	OnBodyBeginTrack.Broadcast(Bodies[BodyIndex]);
}

void UNeoKinectManager::BodyLostBroadcast(int32 BodyIndex)
{
	OnBodyLost.Broadcast(Bodies[BodyIndex]);
}

void UNeoKinectManager::FaceBeginTrackBroadcast(int32 FaceIndex)
{
	OnFaceBeginTrack.Broadcast(Faces[FaceIndex]);
}

void UNeoKinectManager::FaceLostBroadcast(int32 FaceIndex)
{
	OnFaceLost.Broadcast(Faces[FaceIndex]);
}

#pragma endregion NonBlueprintable
