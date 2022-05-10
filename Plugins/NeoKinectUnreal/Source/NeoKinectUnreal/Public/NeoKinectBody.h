// Copyright 2016-2018 Rodrigo Villani Pereira

#pragma once

#include "CoreMinimal.h"
#include "NeoKinectEnums.h"

#include "NeoKinectBody.generated.h"

using namespace NeoKinect;

/**
 * Joint properties.
 * 
 * The Location and Orientation of joints, by default, is given in camera space,
 * which center is a point in the Kinect sensor not matching the color camera.
 * So, to use joints over the color frame image, these coordinates must be reprojected.
 * To get these reprojected values, use ColorLocation and ColorOrientation.
 */
USTRUCT(BlueprintType, Category = "NeoKinect|Structs")
struct FKinectJoint
{
	GENERATED_BODY()

	/** Joint type. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Joints")
	EKinectJointType		Type;
	/** Joint tracking confidence. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Joints")
	EKinectTrackingState	Confidence;

	/** Joint location relative to Kinect sensor in camera space. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "position translation transform camera"))
	FVector		Location;
	/** Joint orientation relative to Kinect sensor in camera space. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "transform rotation angles camera"))
	FRotator	Orientation;

	/** Joint location relative to Kinect sensor in color space with depth. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "position translation transform color"))
	FVector		ColorLocation;
	/** Joint orientation relative to Kinect sensor in color space. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "transform rotation angles color"))
	FRotator	ColorOrientation;

	FKinectJoint() :
		Type(EKinectJointType::AnkleLeft),
		Confidence(EKinectTrackingState::NotTracked),
		Location(FVector::ZeroVector),
		Orientation(FRotator::ZeroRotator),
		ColorLocation(FVector::ZeroVector),
		ColorOrientation(FRotator::ZeroRotator)
	{}
};

/**
* Which edges are clipping a body on the view frustum.
*/
USTRUCT(BlueprintType, Category = "NeoKinect|Structs")
struct FKinectClippedEdges
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	bool Left;
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	bool Right;
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	bool Top;
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	bool Bottom;

	FKinectClippedEdges() :
		Left(false), Right(false), Top(false), Bottom(false)
	{}
};

/**
* Represents a trackable body.
*/
UCLASS(BlueprintType, Category = "NeoKinect|Body")
class NEOKINECTUNREAL_API UNeoKinectBody : public UObject
{
	GENERATED_BODY()

public:
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBodyTrackDelegate, int32, BodyIndex);

	UNeoKinectBody();
	~UNeoKinectBody();

	/** Retrieves this body index (0~5). */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	int32	BodyIndex;
	/** Body's unique Tracking ID. */
	UPROPERTY()
	int64	TrackingId;

	/** Body clipped edges from the sensor's view. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body", Meta = (Keywords = "cut borders"))
	FKinectClippedEdges	ClippedEdges;

	/** If this body represents a tracked user. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	bool	bIsTracked;
	/** If body's movement is restricted somehow. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	bool	bIsRestricted;

	/** Left hand pose */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Hands")
	EKinectHandState		HandLeftState;
	/** Left hand tracking state */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Hands")
	EKinectTrackingState	HandLeftTrackingState;

	/** Right hand pose */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Hands")
	EKinectHandState		HandRightState;
	/** Right hand tracking state */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body|Hands")
	EKinectTrackingState	HandRightTrackingState;

	/** Body lean tracking confidence. */
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body")
	EKinectTrackingState	LeanTrackingState;
	/**
	* Body lean.
	* Ranges from -1 to 1 being left to right in X and back to	front in Y.
	* Values of -1 and 1 represent roughly 45 degrees of inclination.
	*/
	UPROPERTY(BlueprintReadOnly, Category = "NeoKinect|Body", Meta = (Keywords = "inclination"))
	FVector2D				Lean;

	UPROPERTY()
	TArray<FKinectJoint> Joints;

	/** Called when this Body object starts tracking an user */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect|Body")
	FBodyTrackDelegate	OnBodyBeginTrack;

	/** Called when this Body object loses track of its user */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect|Body")
	FBodyTrackDelegate	OnBodyLost;

	/**
	* Get Left/Right hand state as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Hand Which hand?
	* @param States Current Hand state.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Body|Hands", Meta = (ExpandEnumAsExecs = "States"))
	void GetHandStateAsExec(EKinectBodySide Hand, EKinectHandState& States);

	/**
	* Get Left/Right hand state.
	*
	* @param Hand Which hand?
	* @return Current Hand state.
	*/
	UFUNCTION(BlueprintPure, Category = "SamrtKinect|Body|Hands")
	EKinectHandState GetHandState(EKinectBodySide Hand);

	/**
	* Get Left/Right hand tracking state as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param Hand Which hand?
	* @param States Current Hand tracking state.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Body|Hands", Meta = (ExpandEnumAsExecs = "States"))
	void GetHandConfidenceAsExec(EKinectBodySide Hand, EKinectTrackingState& States);

	/**
	* Get Left/Right hand tracking state.
	*
	* @param Hand Which hand?
	* @return Current Hand tracking state.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Hands")
	EKinectTrackingState GetHandConfidence(EKinectBodySide Hand);

	/**
	* Get body lean tracking confidence as execution pins.
	* Useful for redirecting code path, like the Branch node.
	*
	* @param States Lean tracking state.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Body", Meta = (ExpandEnumAsExecs = "States", Keywords = "inclination"))
	void GetLeanConfidenceAsExec(EKinectTrackingState& States);

	/**
	* Contains this body's each joint tracking state and transform.
	*
	* @return This body's each joint tracking state and transform.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints")
	TArray<FKinectJoint> GetJoints();

	/**
	* Get a Joint's location relative to Kinect sensor.
	*
	* @param Joint The joint type.
	* @return Joint's location relative to Kinect sensor.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "position translation transform"))
	FVector GetJointLocation(EKinectJointType Joint);

	/**
	* Get a Joint's orientation relative to Kinect sensor.
	*
	* @param Joint The joint type.
	* @return Joint's orientation relative to Kinect sensor.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "rotation angles transform"))
	FRotator GetJointOrientation(EKinectJointType Joint);

	/**
	* Get a Joint's location relative to Kinect sensor aligned with the Color frame.
	* So, if a camera is created with the same FOV as the Kinect Color camera's and
	* the Color frame texture is used as background, this location will align perfectly
	* with the corresponding joint in the Color image.
	*
	* @param Joint The joint type.
	* @return Joint's location relative to Kinect sensor.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "position translation transform"))
	FVector GetJointLocationColor(EKinectJointType Joint);

	/**
	* Get a Joint's orientation relative to Kinect sensor aligned with the Color
	* aligned joint location.
	*
	* @param Joint The joint type.
	* @return Joint's orientation relative to Kinect sensor.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "rotation angles transform"))
	FRotator GetJointOrientationColor(EKinectJointType Joint);

	/**
	* Get a Joint's tracking confidence as exec pins.
	* Useful for redirecting the code path, like the Branch node.
	*
	* @param Joint The joint type.
	* @param States Joint's tracking confidence.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Body|Joints", Meta = (ExpandEnumAsExecs = "States"))
	void GetJointConfidenceAsExec(EKinectJointType Joint, EKinectTrackingState& States);

	/**
	* Get a Joint's tracking confidence.
	*
	* @param Joint The joint type.
	* @return Joint's tracking confidence.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints")
	EKinectTrackingState GetJointConfidence(EKinectJointType Joint);

	/**
	* Calculate the distance between two joints in the Kinect sensor space.
	*
	* @param JointA First joint.
	* @param JointB Second joint.
	* @return Distance between the two joints.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "size length"))
	float GetJointsDistance(EKinectJointType JointA, EKinectJointType JointB);

	/**
	* Calculate the distance from a Joint's location to it's parent's location.
	* Useful for getting bone sizes. Eg.: for Left Elbow, the result will be the
	* distance from Left Elbow to Left Shoulder. Spine Base is the skeleton's root,
	* so it will return 0.
	*
	* @param Joint The joint type.
	* @return Distance from Joint to it's parent.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "size length"))
	float GetBoneLength(EKinectJointType Joint);

	/**
	* The Dot product between a Joint direction and a Vector.
	*
	* @return -1 ~ 1: 1 = same direction as vector. -1 = opposite from vector.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (Keywords = "pointing direction lookat"))
	float JointDotVector(EKinectJointType Joint, FVector Vector);

	void UpdateFromKinectBody(const KinectBody& kBody, const KinectSensor& Sensor);

	void Reset();
};