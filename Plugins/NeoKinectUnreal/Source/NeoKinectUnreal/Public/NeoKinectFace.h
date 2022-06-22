// Copyright 2016-2022 Rodrigo Villani Pereira

#pragma once

#include "NeoKinectEnums.h"

#include "Windows/AllowWindowsPlatformTypes.h"
#include "Kinect.Face.h"
#include "Windows/HideWindowsPlatformTypes.h"

#include "NeoKinectFace.generated.h"

class UNeoKinectBody;

namespace NeoKinect
{
	struct KinectFace;
	struct KinectFacePoints;
}

/**
* Describes the edges of a rectangle in a texture (int values)
*/
USTRUCT(BlueprintType, Category = "NeoKinect|Structs")
struct FIntBoundingBox
{
	GENERATED_BODY()

	// Left rectangle edge
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	int32 Left;
	// Top rectangle edge
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	int32 Top;
	// Right rectangle edge
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	int32 Right;
	// Bottom rectangle edge
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	int32 Bottom;

	void FromRectI(const RectI& Rect);
};

/**
* Lists the 2D of each face alignment point.
*/
USTRUCT(BlueprintType, Category = "NeoKinect|Structs")
struct FFacePoints2D
{
	GENERATED_BODY()
	
	// Left eye location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector2D LeftEye;
	// Right eye location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector2D RightEye;
	// Nose location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector2D Nose;
	// Left mouth corner location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector2D LeftMouthCorner;
	// Right mouth corner location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector2D RightMouthCorner;

	void FromKinectFacePoints(const NeoKinect::KinectFacePoints& Points);
};

/**
* Lists the 3D locations of each face alignment point.
*/
USTRUCT(BlueprintType, Category = "NeoKinect|Structs")
struct FFacePoints3D
{
	GENERATED_BODY()

	// Left eye location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector LeftEye;
	// Right eye location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector RightEye;
	// Nose location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector Nose;
	// Left mouth corner location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector LeftMouthCorner;
	// Right mouth corner location
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector RightMouthCorner;

	void FromKinectFacePoints(const NeoKinect::KinectFacePoints& ColorPoints);
};

/**
* Represents a trackable body's face.
*/
UCLASS(BlueprintType, Category = "NeoKinect|Face")
class NEOKINECTUNREAL_API UNeoKinectFace : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFaceTrackDelegate, int32, FaceIndex);

	UNeoKinectFace();
	~UNeoKinectFace();

	// This face's index in the faces array
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	int32				Index;

	// If this face is being tracked.
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	bool				bIsTracked;

	// Face tracking Id. Matches it's body TrackingId.
	UPROPERTY()
	int64				TrackingId;

	// The face bounding box in Color space.
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FIntBoundingBox		ColorBoundingBox;

	// The face bounding box in Depth space.
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FIntBoundingBox		InfraredBoundingBox;

	// The face alignment points in Color space.
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FFacePoints2D		ColorAlignmentPoints;

	// The face alignment points in Depth space.
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FFacePoints2D		InfraredAlignmentPoints;

	/**
	* The face alignment points aligned with the Color frame from the camera's
	* point of view, at a distance taken from Depth frame.
	* For this to have valid values the remapped Depth in Color Space frame must be activated.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FFacePoints3D		ColorWithDepthAlignmentPoints;

	// The face rotation.
	UPROPERTY(BlueprintReadOnly, Meta = (Keywords = "rotation quaternion angles transform"), Category = "NeoKinect|Face")
	FRotator			Orientation;

	// The head pivot location.
	UPROPERTY(BlueprintReadOnly, Meta = (Keywords = "location position center head translation transform"), Category = "NeoKinect|Face")
	FVector				Location;
	
	/** Head location aligned with the Color frame and with depth.
	* Only valid if color transforms were activated using SetUseJointsColorSpaceTransforms */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	FVector				ColorLocation;

	// Is the user engaged?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsEngaged;

	// Does the user looks happy?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsHappy;

	// Is the user looking away?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsLookingAway;

	// Did the user mouth move?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsMouthMoved;

	// Is the user mouth opened?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsMouthOpen;

	// Is the user left eye closed?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsLeftEyeClosed;

	// Is the user right eye closed?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsRightEyeClosed;

	// Is the user wearing glasses?
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Face")
	EDetectionResult	IsWearingGlasses;

	/** Called when this face object starts tracking a user's face */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect|Face")
	FFaceTrackDelegate	OnFaceBeginTrack;

	/** Called when this face object loses its user's face */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect|Face")
	FFaceTrackDelegate	OnFaceLost;

	/**
	* Checks if left/right eye is closed and use the result as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Eye Which eye?
	* @param Result Is selected eye closed? Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Face", Meta = (ExpandEnumAsExecs = "Result"))
	void GetEyeClosedAsExec(EKinectBodySide Eye, EDetectionResult& Result);

	/**
	* Checks if left/right eye is closed.
	* 
	* @param Eye Which eye?
	* @return Is selected eye closed? Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Face")
	EDetectionResult GetEyeClosed(EKinectBodySide Eye) const;

	/**
	* Get IsEngaged as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Result Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Face", Meta = (ExpandEnumAsExecs = "Result"))
	void GetIsEngagedAsExec(EDetectionResult& Result);

	/**
	* Gets IsHappy as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Result Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Face", Meta = (ExpandEnumAsExecs = "Result"))
	void GetIsHappyAsExec(EDetectionResult& Result);

	/**
	* Gets IsLookingAway as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Result Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Face", Meta = (ExpandEnumAsExecs = "Result"))
	void GetIsLookingAwayAsExec(EDetectionResult& Result);

	/**
	* Gets IsMouthMoved as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Result Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Face", Meta = (ExpandEnumAsExecs = "Result"))
	void GetIsMouthMovedAsExec(EDetectionResult& Result);

	/**
	* Gets IsMouthOpen as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Result Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Face", Meta = (ExpandEnumAsExecs = "Result"))
	void GetIsMouthOpenAsExec(EDetectionResult& Result);

	/**
	* Gets IsWearingGlasses as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Result Consider 'Maybe' more as a 'No'.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Face", Meta = (ExpandEnumAsExecs = "Result"))
	void GetIsWearingGlassesAsExec(EDetectionResult& Result);

	/**
	* Checks if this face belongs to the passed Body instance.
	*
	* @param Body The body to test from.
	* @return True if this face belongs to the passed body.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Face")
	bool IsFromBody(const UNeoKinectBody* Body) const;

	void UpdateFromKinectFace(const NeoKinect::KinectFace& kFace);

	void Reset();
};
