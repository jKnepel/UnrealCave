// Copyright 2016-2018 Rodrigo Villani Pereira

#include "NeoKinectFace.h"

#include "NeoKinectManager.h"

void FIntBoundingBox::FromRectI(const RectI& Rect)
{
	Left = Rect.Left;
	Top = Rect.Top;
	Right = Rect.Right;
	Bottom = Rect.Bottom;
}

void FFacePoints2D::FromKinectFacePoints(const KinectFacePoints& Points)
{
	LeftEye.X = Points.leftEye.x;
	LeftEye.Y = Points.leftEye.y;

	RightEye.X = Points.rightEye.x;
	RightEye.Y = Points.rightEye.y;

	Nose.X = Points.nose.x;
	Nose.Y = Points.nose.y;

	LeftMouthCorner.X = Points.leftMouthCorner.x;
	LeftMouthCorner.Y = Points.leftMouthCorner.y;

	RightMouthCorner.X = Points.rightMouthCorner.x;
	RightMouthCorner.Y = Points.rightMouthCorner.y;
}

void FFacePoints3D::FromKinectFacePoints(const KinectFacePoints& ColorPoints)
{
	Vector3 Location;

	Location = UNeoKinectManager::ColorWithDepthFromColorOnly(ColorPoints.leftEye) * 100.f;
	LeftEye.Set(-Location.z, Location.x, Location.y);

	Location = UNeoKinectManager::ColorWithDepthFromColorOnly(ColorPoints.rightEye) * 100.f;
	RightEye.Set(-Location.z, Location.x, Location.y);

	Location = UNeoKinectManager::ColorWithDepthFromColorOnly(ColorPoints.nose) * 100.f;
	Nose.Set(-Location.z, Location.x, Location.y);

	Location = UNeoKinectManager::ColorWithDepthFromColorOnly(ColorPoints.leftMouthCorner) * 100.f;
	LeftMouthCorner.Set(-Location.z, Location.x, Location.y);

	Location = UNeoKinectManager::ColorWithDepthFromColorOnly(ColorPoints.rightMouthCorner) * 100.f;
	RightMouthCorner.Set(-Location.z, Location.x, Location.y);
}

UNeoKinectFace::UNeoKinectFace() :
	bIsTracked(false)
{
}

UNeoKinectFace::~UNeoKinectFace()
{
}

void UNeoKinectFace::GetEyeClosedAsExec(EKinectBodySide Eye, EDetectionResult& Result)
{
	if (Eye == EKinectBodySide::Left)
	{
		Result = IsLeftEyeClosed;
	}
	else
	{
		Result = IsRightEyeClosed;
	}
}

EDetectionResult UNeoKinectFace::GetEyeClosed(EKinectBodySide Eye)
{
	if (Eye == EKinectBodySide::Left)
		return IsLeftEyeClosed;
	else
		return IsRightEyeClosed;
}

void UNeoKinectFace::GetIsEngagedAsExec(EDetectionResult& Result)
{
	Result = IsEngaged;
}

void UNeoKinectFace::GetIsHappyAsExec(EDetectionResult& Result)
{
	Result = IsHappy;
}

void UNeoKinectFace::GetIsLookingAwayAsExec(EDetectionResult& Result)
{
	Result = IsLookingAway;
}

void UNeoKinectFace::GetIsMouthMovedAsExec(EDetectionResult& Result)
{
	Result = IsMouthMoved;
}

void UNeoKinectFace::GetIsMouthOpenAsExec(EDetectionResult& Result)
{
	Result = IsMouthOpen;
}

void UNeoKinectFace::GetIsWearingGlassesAsExec(EDetectionResult& Result)
{
	Result = IsWearingGlasses;
}

bool UNeoKinectFace::IsFromBody(const UNeoKinectBody* Body)
{
	return Body->TrackingId == TrackingId;
}

void UNeoKinectFace::UpdateFromKinectFace(const KinectFace& kFace)
{
	bool bWasTracked = bIsTracked;

	bIsTracked = kFace.bIsTracked;
	TrackingId = kFace.trackingId;

	ColorBoundingBox.FromRectI(kFace.boxColor);
	InfraredBoundingBox.FromRectI(kFace.boxInfrared);

	ColorAlignmentPoints.FromKinectFacePoints(kFace.pointsColor);
	InfraredAlignmentPoints.FromKinectFacePoints(kFace.pointsInfrared);
	ColorWithDepthAlignmentPoints.FromKinectFacePoints(kFace.pointsColor);

	/* Camera space face transform */
	Location.Set(
		kFace.pivot.Z * 100.f,
		-kFace.pivot.X * 100.f,
		kFace.pivot.Y * 100.f);

	FQuat Quat = FQuat(
		-kFace.rotation.z,
		kFace.rotation.x,
		-kFace.rotation.y,
		kFace.rotation.w
		);
	Orientation = FRotationMatrix::MakeFromXZ(
		Quat.GetAxisZ(),
		-Quat.GetAxisY()).Rotator();

	/* Color space face location */
	if (UNeoKinectManager::bUsingJointsColorSpaceTransforms)
	{
		// Convert camera space location to color space with depth
		Vector3 ColorDepthLocation = UNeoKinectManager::CameraLocationToColorWithDepth(Vector3(kFace.pivot));
		if (ColorDepthLocation.IsZero())
		{
			ColorLocation = Location;
		}
		else
		{
			ColorLocation.Set(
				ColorDepthLocation.z * 100.0f,
				-ColorDepthLocation.x * 100.0f,
				ColorDepthLocation.y * 100.0f);
		}
	}

	IsEngaged = static_cast<EDetectionResult>(kFace.isEngaged);
	IsHappy = static_cast<EDetectionResult>(kFace.isHappy);
	IsLookingAway = static_cast<EDetectionResult>(kFace.isLookingAway);
	IsMouthMoved = static_cast<EDetectionResult>(kFace.isMouthMoved);
	IsMouthOpen = static_cast<EDetectionResult>(kFace.isMouthOpen);
	IsLeftEyeClosed = static_cast<EDetectionResult>(kFace.isLeftEyeClosed);
	IsRightEyeClosed = static_cast<EDetectionResult>(kFace.isRightEyeClosed);
	IsWearingGlasses = static_cast<EDetectionResult>(kFace.isWearingGlasses);

	if (bWasTracked != bIsTracked)
	{
		if (bIsTracked)
			OnFaceBeginTrack.Broadcast(Index);
		else
			OnFaceLost.Broadcast(Index);
	}
}

void UNeoKinectFace::Reset()
{
	bIsTracked = false;
	TrackingId = 0ULL;

	ColorBoundingBox = FIntBoundingBox();
	InfraredBoundingBox = FIntBoundingBox();

	ColorAlignmentPoints = FFacePoints2D();
	InfraredAlignmentPoints = FFacePoints2D();

	Location.Set(0.f, 0.f, 0.f);
	ColorLocation = Location;

	Orientation.Yaw =
		Orientation.Pitch =
		Orientation.Roll = 0.f;

	IsEngaged =
		IsHappy =
		IsLookingAway =
		IsMouthMoved =
		IsMouthOpen =
		IsLeftEyeClosed =
		IsRightEyeClosed =
		IsWearingGlasses = EDetectionResult::Unknown;
}