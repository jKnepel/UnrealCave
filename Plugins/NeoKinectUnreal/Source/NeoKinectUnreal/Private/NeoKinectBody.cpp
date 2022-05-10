// Copyright 2016-2018 Rodrigo Villani Pereira

#include "NeoKinectBody.h"

#include "NeoKinectManager.h"

UNeoKinectBody::UNeoKinectBody() :
	TrackingId(0),
	bIsTracked(false),
	bIsRestricted(false),
	HandLeftState(EKinectHandState::NotTracked),
	HandLeftTrackingState(EKinectTrackingState::NotTracked),
	HandRightState(EKinectHandState::NotTracked),
	HandRightTrackingState(EKinectTrackingState::NotTracked),
	LeanTrackingState(EKinectTrackingState::NotTracked),
	Lean(FVector2D(0.f, 0.f))
{
	Joints.SetNum(KinectJointCount);

	int32 n = KinectJointCount;
	for (int32 i = 0; i < n; ++i)
	{
		Joints[i].Type = (EKinectJointType)i;
	}
}

UNeoKinectBody::~UNeoKinectBody()
{
	Joints.Empty();
}

void UNeoKinectBody::GetHandStateAsExec(EKinectBodySide Hand, EKinectHandState& States)
{
	if (Hand == EKinectBodySide::Left)
	{
		States = HandLeftState;
	}
	else
	{
		States = HandRightState;
	}
}

EKinectHandState UNeoKinectBody::GetHandState(EKinectBodySide Hand)
{
	if (Hand == EKinectBodySide::Left)
		return HandLeftState;
	else
		return HandRightState;
}

void UNeoKinectBody::GetHandConfidenceAsExec(EKinectBodySide Hand, EKinectTrackingState& States)
{
	if (Hand == EKinectBodySide::Left)
	{
		States = HandLeftTrackingState;
	}
	else
	{
		States = HandRightTrackingState;
	}
}

EKinectTrackingState UNeoKinectBody::GetHandConfidence(EKinectBodySide Hand)
{
	if (Hand == EKinectBodySide::Left)
		return HandLeftTrackingState;
	else
		return HandRightTrackingState;
}

void UNeoKinectBody::GetLeanConfidenceAsExec(EKinectTrackingState& States)
{
	States = LeanTrackingState;
}

TArray<FKinectJoint> UNeoKinectBody::GetJoints()
{
	return Joints;
}

FVector UNeoKinectBody::GetJointLocation(EKinectJointType Joint)
{
	return Joints[(int32)Joint].Location;
}

FRotator UNeoKinectBody::GetJointOrientation(EKinectJointType Joint)
{
	return Joints[(int32)Joint].Orientation;
}

FVector UNeoKinectBody::GetJointLocationColor(EKinectJointType Joint)
{
	return Joints[(int32)Joint].ColorLocation;
}

FRotator UNeoKinectBody::GetJointOrientationColor(EKinectJointType Joint)
{
	return Joints[(int32)Joint].ColorOrientation;
}

void UNeoKinectBody::GetJointConfidenceAsExec(EKinectJointType Joint, EKinectTrackingState& States)
{
	States = Joints[(int32)Joint].Confidence;
}

EKinectTrackingState UNeoKinectBody::GetJointConfidence(EKinectJointType Joint)
{
	return Joints[(int32)Joint].Confidence;
}

float UNeoKinectBody::GetJointsDistance(EKinectJointType JointA, EKinectJointType JointB)
{
	return FVector::Dist(Joints[(int32)JointB].Location, Joints[(int32)JointA].Location);
}

float UNeoKinectBody::GetBoneLength(EKinectJointType Joint)
{
	FVector p1 = Joints[(int32)Joint].Location;
	switch (Joint)
	{
	case EKinectJointType::SpineBase:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineMid].Location);
	case EKinectJointType::SpineMid:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineBase].Location);
	case EKinectJointType::Neck:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineShoulder].Location);
	case EKinectJointType::Head:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::Neck].Location);
	case EKinectJointType::ShoulderLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineShoulder].Location);
	case EKinectJointType::ElbowLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::ShoulderLeft].Location);
	case EKinectJointType::WristLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::ElbowLeft].Location);
	case EKinectJointType::HandLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::WristLeft].Location);
	case EKinectJointType::ShoulderRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineShoulder].Location);
	case EKinectJointType::ElbowRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::ShoulderRight].Location);
	case EKinectJointType::WristRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::ElbowRight].Location);
	case EKinectJointType::HandRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::WristRight].Location);
	case EKinectJointType::HipLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineBase].Location);
	case EKinectJointType::KneeLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::HipLeft].Location);
	case EKinectJointType::AnkleLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::KneeLeft].Location);
	case EKinectJointType::FootLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::AnkleLeft].Location);
	case EKinectJointType::HipRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineBase].Location);
	case EKinectJointType::KneeRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::HipRight].Location);
	case EKinectJointType::AnkleRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::KneeRight].Location);
	case EKinectJointType::FootRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::AnkleRight].Location);
	case EKinectJointType::SpineShoulder:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::SpineMid].Location);
	case EKinectJointType::HandTipLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::WristLeft].Location);
	case EKinectJointType::ThumbLeft:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::HandLeft].Location);
	case EKinectJointType::HandTipRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::WristRight].Location);
	case EKinectJointType::ThumbRight:
		return FVector::Dist(p1, Joints[(int32)EKinectJointType::HandRight].Location);
	default:
		return 0.f;
	}
}

float UNeoKinectBody::JointDotVector(EKinectJointType Joint, FVector Vector)
{
	Vector.Normalize();
	return FVector::DotProduct(
		Vector,
		FRotationMatrix(Joints[(int32)Joint].Orientation).GetScaledAxis(EAxis::Z));
}

void UNeoKinectBody::UpdateFromKinectBody(const KinectBody& kBody, const KinectSensor& Sensor)
{
	// For throwing BeginTrack/Lost events after update
	bool bWasTracked = bIsTracked;

	ClippedEdges.Left = kBody.clippedEdges.left;
	ClippedEdges.Right = kBody.clippedEdges.right;
	ClippedEdges.Top = kBody.clippedEdges.top;
	ClippedEdges.Bottom = kBody.clippedEdges.bottom;

	bIsTracked = kBody.isTracked;
	bIsRestricted = kBody.isRestricted;
	TrackingId = kBody.trackingId;

	HandLeftState = (EKinectHandState)kBody.handLeftState;
	HandLeftTrackingState = (EKinectTrackingState)kBody.handLeftTrackingState;

	HandRightState = (EKinectHandState)kBody.handRightState;
	HandRightTrackingState = (EKinectTrackingState)kBody.handRightTrackingState;

	LeanTrackingState = (EKinectTrackingState)kBody.leanTrackingState;
	Lean.Set(kBody.lean.x, kBody.lean.y);

	// update joints only if being tracked
	if (bIsTracked)
	{
		// camera space joints transforms
		for (int32 j = 0; j < KinectJointCount; ++j)
		{
			FKinectJoint& uJoint = Joints[j];
			KinectJoint joint = kBody.joints[j];

			uJoint.Confidence = (EKinectTrackingState)joint.trackingState;
			uJoint.Location.Set(
				joint.position.Z * 100.f,
				-joint.position.X * 100.f,
				joint.position.Y * 100.f);

			// Quaternion conversion from Kinect camera's coord system to Unreal's
			FQuat Quat = FQuat(
				-joint.orientation.z,
				joint.orientation.x,
				-joint.orientation.y,
				joint.orientation.w
				);
			// In order to match the joints orientations to Unreal Mannequin, we
			// still have some problems:
			//	  - Kinect's bones extend along its UP axis, which is Z in Unreal.
			//		But Unreal's Mannequin skeleton's bones extend along X (as it is
			//		in Maya). So we re-create the orientation swapping the axes.
			//
			//	  - Some bones extend towards X, some against it. That's good for doing
			//		mirrored rotations in the animation software, so we'll keep it here
			//		by flipping some axes.
			//
			//	  - The pelvis and spine bones front axis is the same as the limbs side axis.
			//		In the Mannequin these match up, so we'll have to invert some here.
#define JNT_ID(Name) static_cast<uint8>(EKinectJointType::Name)

			uint8 JointIndex = static_cast<uint8>(uJoint.Type);

			if (JointIndex <= JNT_ID(Head) || JointIndex == JNT_ID(SpineShoulder))
			{ // for spine bones
				uJoint.Orientation = FRotationMatrix::MakeFromXZ(
					Quat.GetAxisZ(),
					Quat.GetAxisY()).Rotator();
			}
			else if (JointIndex <= JNT_ID(WristLeft)
				|| JointIndex >= JNT_ID(HipRight) && JointIndex <= JNT_ID(FootRight))
			{ // for left arm and right leg
				uJoint.Orientation = FRotationMatrix::MakeFromXZ(
					Quat.GetAxisZ(),
					Quat.GetAxisX()).Rotator();
			}
			else if ( (JointIndex >= JNT_ID(ShoulderRight) && JointIndex <= JNT_ID(WristRight))
				|| (JointIndex >= JNT_ID(HipLeft) && JointIndex <= JNT_ID(FootLeft))
				|| JointIndex == JNT_ID(HandTipRight) || JointIndex == JNT_ID(ThumbRight)
				)
			{ // for right arm and left leg
				uJoint.Orientation = FRotationMatrix::MakeFromXZ(
					-Quat.GetAxisZ(),
					-Quat.GetAxisX()).Rotator();
			}
			else if (JointIndex == JNT_ID(HandTipLeft) || JointIndex == JNT_ID(ThumbLeft))
			{ // Left hand
				uJoint.Orientation = FRotationMatrix::MakeFromXZ(
					Quat.GetAxisZ(),
					Quat.GetAxisX()).Rotator();
			}
			else if (JointIndex == JNT_ID(HandLeft))
			{ // left hand
				uJoint.Orientation = FRotationMatrix::MakeFromXZ(
					Quat.GetAxisZ(),
					-Quat.GetAxisX()).Rotator();
			}
			else if (JointIndex == JNT_ID(HandRight))
			{ // right hand
				uJoint.Orientation = FRotationMatrix::MakeFromXZ(
					-Quat.GetAxisZ(),
					Quat.GetAxisX()).Rotator();
			}
#undef JNT_ID

		} // camera space joints transforms

		if (UNeoKinectManager::bUsingJointsColorSpaceTransforms)
		{
			// color space with depth joints transforms
			for (int32 j = 0; j < KinectJointCount; ++j)
			{
				FKinectJoint& uJoint = Joints[j];
				KinectJoint joint = kBody.joints[j];

				// Convert camera space location to color space with depth
				Vector3 ColorDepthLocation = UNeoKinectManager::CameraLocationToColorWithDepth(Vector3(joint.position));
				if (ColorDepthLocation.IsZero())
				{
					// fallback for when the joint can not be seen in color camera
					if (uJoint.Type == EKinectJointType::SpineBase) // oh shit! Our reference is gone!
					{
						uJoint.ColorLocation = uJoint.Location;
						continue;
					}

					FKinectJoint &SpineBase = Joints[static_cast<int32>(EKinectJointType::SpineBase)];
					uJoint.ColorLocation = SpineBase.ColorLocation + uJoint.Location - SpineBase.Location;
					continue;
				}

				uJoint.ColorLocation.Set(
					ColorDepthLocation.z * 100.0f,
					-ColorDepthLocation.x * 100.0f,
					ColorDepthLocation.y * 100.0f);
			} // color space with depth joints transforms

			// orientations in color space with depth
#define JOINT(Name) Joints[static_cast<int32>(EKinectJointType::Name)]
#define J_DIR(FromJoint, ToJoint) JOINT(ToJoint).ColorLocation - JOINT(FromJoint).ColorLocation
#define J_ROT(JointName, Twist) JOINT(JointName).ColorOrientation = FRotationMatrix::MakeFromX##Twist(XDir, Twist##Dir).Rotator()

			FVector XDir, YDir, ZDir, Dir1, Dir2;
			FQuat Rot1, Rot2, RLerp;
			float JointsDot;

			// Spine Base
			XDir = J_DIR(SpineBase, SpineMid);
			ZDir = J_DIR(HipLeft, HipRight);
			J_ROT(SpineBase, Z);

			// Spine Middle
			// same XDir
			ZDir = (ZDir + J_DIR(ShoulderLeft, ShoulderRight)) / 2.0f; // intermediary between hip and shoulders twist
			J_ROT(SpineMid, Z);

			// Spine Shoulder
			XDir = J_DIR(SpineMid, SpineShoulder);
			ZDir = J_DIR(ShoulderLeft, ShoulderRight);
			J_ROT(SpineShoulder, Z);

			// Neck
			XDir = J_DIR(SpineShoulder, Neck);
			// same ZDir
			J_ROT(Neck, Z);

			// Head
			XDir = J_DIR(Neck, Head);
			// same ZDir
			J_ROT(Head, Z);

			// shoulder left
			XDir = J_DIR(SpineShoulder, ShoulderLeft);
			ZDir = J_DIR(SpineMid, SpineShoulder);
			J_ROT(ShoulderLeft, Z);

			// elbow left
			XDir = J_DIR(ShoulderLeft, ElbowLeft);
			// interpolate between a rot for fully stretched arm and one for bent
			Dir1 = FVector::CrossProduct(J_DIR(ElbowLeft, WristLeft), J_DIR(ElbowLeft, ShoulderLeft)); // bent
			Rot1 = FRotationMatrix::MakeFromXZ(XDir, Dir1).ToQuat(); // bent
			Dir2 = FRotationMatrix(JOINT(ShoulderLeft).ColorOrientation).GetScaledAxis(EAxis::Y); // streched
			Rot2 = FRotationMatrix::MakeFromXY(XDir, Dir2).ToQuat(); // stretched
			Dir1 = J_DIR(ShoulderLeft, ElbowLeft); Dir2 = J_DIR(ElbowLeft, WristLeft);
			Dir1.Normalize(); Dir2.Normalize();
			JointsDot = FVector::DotProduct(Dir1, Dir2);
			RLerp = FQuat::Slerp(Rot2, Rot1,
				FMath::Clamp(FMath::Acos(JointsDot) / 0.349f /*~20°*/, 0.f, 1.f)
			);
			RLerp.Normalize();
			JOINT(ElbowLeft).ColorOrientation = RLerp.Rotator();

			// wrist left
			XDir = J_DIR(ElbowLeft, WristLeft);
			YDir = J_DIR(HandLeft, ThumbLeft);
			J_ROT(WristLeft, Y);

			// hand left
			XDir = J_DIR(WristLeft, HandTipLeft);
			ZDir = J_DIR(ThumbLeft, HandLeft);
			J_ROT(HandLeft, Z);

			// hand tip Left
			JOINT(HandTipLeft).ColorOrientation = JOINT(HandLeft).ColorOrientation;

			// thumb left
			XDir = J_DIR(HandLeft, ThumbLeft);
			YDir = FRotationMatrix(JOINT(HandLeft).ColorOrientation).GetScaledAxis(EAxis::Y);
			J_ROT(ThumbLeft, Y);

			// shoulder right
			XDir = J_DIR(ShoulderRight, SpineShoulder);
			ZDir = J_DIR(SpineShoulder, SpineMid);
			J_ROT(ShoulderRight, Z);

			// elbow right
			XDir = J_DIR(ElbowRight, ShoulderRight);
			// interpolate between a rot for fully stretched arm and one for bent
			Dir1 = FVector::CrossProduct(J_DIR(ElbowRight, WristRight), J_DIR(ElbowRight, ShoulderRight)); // bent
			Rot1 = FRotationMatrix::MakeFromXZ(XDir, Dir1).ToQuat(); // bent
			Dir2 = FRotationMatrix(JOINT(ShoulderRight).ColorOrientation).GetScaledAxis(EAxis::Y); // streched
			Rot2 = FRotationMatrix::MakeFromXY(XDir, Dir2).ToQuat(); // stretched
			Dir1 = J_DIR(ShoulderRight, ElbowRight); Dir2 = J_DIR(ElbowRight, WristRight);
			Dir1.Normalize(); Dir2.Normalize();
			JointsDot = FVector::DotProduct(Dir1, Dir2);
			RLerp = FQuat::Slerp(Rot2, Rot1,
				FMath::Clamp(FMath::Acos(JointsDot) / 0.349f /*~20°*/, 0.f, 1.f)
			);
			RLerp.Normalize();
			JOINT(ElbowRight).ColorOrientation = RLerp.Rotator();

			// wrist right
			XDir = J_DIR(WristRight, ElbowRight);
			YDir = J_DIR(ThumbRight, HandRight);
			J_ROT(WristRight, Y);

			// hand right
			XDir = J_DIR(HandTipRight, WristRight);
			ZDir = -YDir;
			J_ROT(HandRight, Z);

			// hand tip right
			JOINT(HandTipRight).ColorOrientation = JOINT(HandRight).ColorOrientation;

			// thumb right
			XDir = J_DIR(ThumbRight, HandRight);
			YDir = FRotationMatrix(JOINT(HandRight).ColorOrientation).GetScaledAxis(EAxis::Y);
			J_ROT(ThumbRight, Y);

			// Hip Left
			XDir = J_DIR(HipLeft, SpineBase);
			ZDir = J_DIR(SpineMid, SpineBase);
			J_ROT(HipLeft, Z);

			// Knee left
			XDir = J_DIR(KneeLeft, HipLeft);
			Dir1 = FVector::CrossProduct(J_DIR(KneeLeft, AnkleLeft), J_DIR(KneeLeft, HipLeft)); // bent
			Rot1 = FRotationMatrix::MakeFromXZ(XDir, Dir1).ToQuat(); // bent
			Dir2 = FRotationMatrix(JOINT(SpineMid).ColorOrientation).GetScaledAxis(EAxis::Y); // stretched
			Rot2 = FRotationMatrix::MakeFromXY(XDir, Dir2).ToQuat(); // stretched
			Dir1 = J_DIR(HipLeft, KneeLeft); Dir2 = J_DIR(KneeLeft, AnkleLeft);
			Dir1.Normalize(); Dir2.Normalize();
			JointsDot = FVector::DotProduct(Dir1, Dir2);
			RLerp = FQuat::Slerp(Rot2, Rot1, FMath::Clamp((FMath::Acos(JointsDot) - 0.2618f) / 0.5236f, 0.f, 1.f));
			RLerp.Normalize();
			JOINT(KneeLeft).ColorOrientation = RLerp.Rotator();

			// ankle left
			XDir = J_DIR(AnkleLeft, KneeLeft);
			ZDir = FRotationMatrix(JOINT(KneeLeft).ColorOrientation).GetScaledAxis(EAxis::Z);
			J_ROT(AnkleLeft, Z);

			// foot left
			// same X and Z dirs
			J_ROT(FootLeft, Z);

			// hip right
			XDir = J_DIR(SpineBase, HipRight);
			ZDir = J_DIR(SpineBase, SpineMid);
			J_ROT(HipRight, Z);

			// Knee Right
			XDir = J_DIR(HipRight, KneeRight);
			Dir1 = FVector::CrossProduct(J_DIR(KneeRight, AnkleRight), J_DIR(KneeRight, HipRight)); // bent
			Rot1 = FRotationMatrix::MakeFromXZ(XDir, Dir1).ToQuat(); // bent
			Dir2 = -FRotationMatrix(JOINT(SpineMid).ColorOrientation).GetScaledAxis(EAxis::Y); // stretched
			Rot2 = FRotationMatrix::MakeFromXY(XDir, Dir2).ToQuat(); // stretched
			Dir1 = J_DIR(HipRight, KneeRight); Dir2 = J_DIR(KneeRight, AnkleRight);
			Dir1.Normalize(); Dir2.Normalize();
			JointsDot = FVector::DotProduct(Dir1, Dir2);
			RLerp = FQuat::Slerp(Rot2, Rot1, FMath::Clamp((FMath::Acos(JointsDot) - 0.2618f) / 0.5236f, 0.f, 1.f));
			RLerp.Normalize();
			JOINT(KneeRight).ColorOrientation = RLerp.Rotator();

			// Ankle right
			XDir = J_DIR(KneeRight, AnkleRight);
			ZDir = FRotationMatrix(JOINT(KneeRight).ColorOrientation).GetScaledAxis(EAxis::Z);
			J_ROT(AnkleRight, Z);

			// foot right
			// same X and Z dirs
			J_ROT(FootRight, Z);
#undef JOINT
#undef J_DIR
#undef J_ROT
		} // if (bUsingJointsColorSpaceTransforms)
	} // if (bIsTracked)

	if (bWasTracked != bIsTracked)
	{
		if (bIsTracked)
			OnBodyBeginTrack.Broadcast(BodyIndex);
		else
			OnBodyLost.Broadcast(BodyIndex);
	}
}

void UNeoKinectBody::Reset()
{
	ClippedEdges.Left =
		ClippedEdges.Right =
		ClippedEdges.Top =
		ClippedEdges.Bottom = false;

	bIsTracked = false;
	bIsRestricted = false;
	TrackingId = 0Ui64;

	HandLeftState = EKinectHandState::NotTracked;
	HandLeftTrackingState = EKinectTrackingState::NotTracked;

	HandRightState = EKinectHandState::NotTracked;
	HandRightTrackingState = EKinectTrackingState::NotTracked;

	LeanTrackingState = EKinectTrackingState::NotTracked;
	Lean.Set(0.f, 0.f);

	if (Joints.Num() != KinectJointCount) return;

	for (int32 j = 0; j < KinectJointCount; ++j)
	{
		if (!Joints.IsValidIndex(j)) continue;
		FKinectJoint& Joint = Joints[j];

		Joint.Confidence = EKinectTrackingState::NotTracked;
		Joint.Location.Set(0.f, 0.f, 0.f);
		Joint.ColorLocation.Set(0.f, 0.f, 0.f);
		Joint.Orientation.Roll =
			Joint.Orientation.Pitch =
			Joint.Orientation.Yaw = 0.f;
		Joint.ColorOrientation.Roll =
			Joint.ColorOrientation.Pitch =
			Joint.ColorOrientation.Yaw = 0.f;
	}
}