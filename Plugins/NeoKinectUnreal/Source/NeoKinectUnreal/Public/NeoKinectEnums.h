// Copyright 2016-2018 Rodrigo Villani Pereira

#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"
#include "NeoKinect.h"
#include "Windows/HideWindowsPlatformTypes.h"

#include "NeoKinectEnums.generated.h"

/**
* The types of frames that can be read from the Kinect sensor.
*/
UENUM(BlueprintType, Category = "NeoKinect|Enums")
enum class EKinectFrame : uint8
{
	/** Normal camera texture. Standard RGBA format (alpha = 1) */
	Color				= 0	UMETA(DisplayName = "Color"),
	/** Depth sensor texture. Red channel only. Values are in millimeters */
	Depth				= 1	UMETA(DisplayName = "Depth"),
	/** Depth sensor ranging from 0 to 1. Standard RGBA format (alpha = 1) */
	NormalizedDepth		= 2	UMETA(DisplayName = "Normalized Depth"),
	/** Body index value in Red channel. In material editor, multiply Red by 255
	* to get actual index value (0~5) */
	BodyIndex			= 3	UMETA(DisplayName = "Body Index"),
	/** Body indexes with a color for each index. Standard RGBA format.
	* Alpha = 1 where there's a body, 0 otherwise */
	BodyIndexColor		= 4	UMETA(DisplayName = "Colored Body Index"),
	/** Not implemented */
	Color_DepthSpace	= 5 UMETA(DisplayName = "Color in Depth Space"),
	/** Depth sensor image remapped to fit the color frame. Red channel only.
	* Values in millimeters, 0 where remapping is not possible. */
	Depth_ColorSpace	= 6 UMETA(DisplayName = "Depth in Color Space"),
	/** Body index values remapped to fit the bodies in the color frame.
	* Red channel only. In material editor, multiply Red by 255 to get actual
	* index value (0~5). */
	BodyIndex_ColorSpace= 7 UMETA(DisplayName = "Body Index in Color Space"),
	/** Infrared sensor image. Red channel only, usually very dark. */
	Infrared			= 8 UMETA(DisplayName = "Infrared"),
	/** Long exposure infrared sensor image. Red channel only. Can lower the
	* sensor's FPS to get brighter image */
	LongExposureInfrared= 9 UMETA(DisplayName = "Long Exposure Infrared"),
	/** Cast to int to get the number of frame types */
	Count				= 10 UMETA(DisplayName = "Count")
};

/**
* Body sides.
*/
UENUM(BlueprintType, Category = "NeoKinect|Enums")
enum class EKinectBodySide : uint8
{
	Left	= 0	UMETA(DisplayName = "Left"),
	Right	= 1	UMETA(DisplayName = "Right")
};

/**
* Possible hand states
*/
UENUM(BlueprintType, Category = "NeoKinect|Enums")
enum class EKinectHandState : uint8
{
	/** Pretty close to NotTracked. */
	Unknown		= 0	UMETA(DisplayName = "Unknown"),
	NotTracked	= 1	UMETA(DisplayName = "Not Tracked"),
	Open		= 2	UMETA(DisplayName = "Open"),
	Closed		= 3	UMETA(DisplayName = "Closed"),
	/** Closed hand with index and middle fingers pointing up, near each other */
	Lasso		= 4	UMETA(DisplayName = "Lasso")
};

/**
* Tracking confidence.
*/
UENUM(BlueprintType, Category = "NeoKinect|Enums")
enum class EKinectTrackingState : uint8
{
	NotTracked	= 0	UMETA(DisplayName = "Not Tracked"),
	/** Tracked by deduction. Sometimes good, many times terrible. */
	Inferred	= 1	UMETA(DisplayName = "Inferred"),
	Tracked		= 2	UMETA(DisplayName = "Tracked")
};

/**
* The joints that the Kinect sensor tracks on each body. 25 total.
*/
UENUM(BlueprintType, Category = "NeoKinect|Enums")
enum class EKinectJointType : uint8
{
	/** Near the pelvis */
	SpineBase		= 0		UMETA(DisplayName = "Spine Base"),
	/** Near the stomach */
	SpineMid		= 1		UMETA(DisplayName = "Spine Middle"),
	Neck			= 2		UMETA(DisplayName = "Neck"),
	Head			= 3		UMETA(DisplayName = "Head"),
	ShoulderLeft	= 4		UMETA(DisplayName = "Left Shoulder"),
	ElbowLeft		= 5		UMETA(DisplayName = "Left Elbow"),
	WristLeft		= 6		UMETA(DisplayName = "Left Wrist"),
	/** In the palm center */
	HandLeft		= 7		UMETA(DisplayName = "Left Hand"),
	ShoulderRight	= 8		UMETA(DisplayName = "Right Shoulder"),
	ElbowRight		= 9		UMETA(DisplayName = "Right Elbow"),
	WristRight		= 10	UMETA(DisplayName = "Right Wrist"),
	/** In the palm center */
	HandRight		= 11	UMETA(DisplayName = "Right Hand"),
	/** Femur/Hip joint */
	HipLeft			= 12	UMETA(DisplayName = "Left Hip"),
	KneeLeft		= 13	UMETA(DisplayName = "Left Knee"),
	AnkleLeft		= 14	UMETA(DisplayName = "Left Ankle"),
	/** Tip of the foot */
	FootLeft		= 15	UMETA(DisplayName = "Left Foot"),
	/** Femur/Hip joint */
	HipRight		= 16	UMETA(DisplayName = "Right Hip"),
	KneeRight		= 17	UMETA(DisplayName = "Right Knee"),
	AnkleRight		= 18	UMETA(DisplayName = "Right Ankle"),
	/** Tip of the foot */
	FootRight		= 19	UMETA(DisplayName = "Right Foot"),
	/** Between the collar bones */
	SpineShoulder	= 20	UMETA(DisplayName = "Spine Shoulder"),
	HandTipLeft		= 21	UMETA(DisplayName = "Left Hand Tip"),
	ThumbLeft		= 22	UMETA(DisplayName = "Left Thumb"),
	HandTipRight	= 23	UMETA(DisplayName = "Right Hand Tip"),
	ThumbRight		= 24	UMETA(DisplayName = "Right Thumb")
};

/**
* Face properties detection results
*/
UENUM(BlueprintType, Category = "NeoKinect|Enums")
enum class EDetectionResult : uint8
{
	Unknown	= 0,
	No		= 1,
	/** Consider it more as a "No" */
	Maybe	= 2,
	Yes		= 3
};