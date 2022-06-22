// Copyright 2016-2022 Rodrigo Villani Pereira

#pragma once

#include "Windows/AllowWindowsPlatformTypes.h"
#include <NeoKinect.h>
#include "Windows/HideWindowsPlatformTypes.h"

#include "NeoKinectEnums.h"
#include "PixelFormat.h"
#include "Containers/Ticker.h"

#include "NeoKinectManager.generated.h"

class FKinectThread;
class UNeoKinectBody;
class UNeoKinectFace;
class UTextureRenderTarget2D;

DECLARE_LOG_CATEGORY_CLASS(NeoKinectLog, Log, All);

/** Structure to manage Kinect frame types lifetime and updating */
struct FFrameTextureData
{
	UTextureRenderTarget2D *Texture = nullptr;
	uint32 Width = 0;
	uint32 Height = 0;
	uint32 PixelSize = 0;
	uint32 BufferSize = 0;
	EPixelFormat UPixelFormat = EPixelFormat::PF_Unknown;
	NeoKinect::KinectPixelFormat KPixelFormat = NeoKinect::KinectPixelFormat::None;
	EKinectFrame UFrameType = EKinectFrame::Count;
	NeoKinect::KinectFrameType KFrameType = NeoKinect::KinectFrameType::Count;
	NeoKinect::KinectCoordinateSpace TargetSpace = NeoKinect::KinectCoordinateSpace::Count;
	bool bIsSRGB = false;
	bool bRemapped = false;

	~FFrameTextureData();
	
	inline bool IsTextureValid() const;
	void Release();
	NeoKinect::FrameDescription GetFrameDescription(const NeoKinect::KinectSensor *Kinect) const;
};

/**
* Access the Kinect v2 sensor capabilities.
*
* When using any Remapped type of frame or any Coordinate Remapping methods,
* Depth frame use will automatically be switched on, since those functionalities
* depend on it. But don't worry about performance. The depth frame will only
* get written and updated to a texture on the GPU if the user explicitly
* asks for it using SetUseFrame().
*
* When using Face Tracking, Body Tracking will also be automatically switched on.
* Face Tracking depends on Body Tracking for the head pivot position and for
* fixing issues when looking for a Face.
*/
UCLASS(Category = "NeoKinect", BlueprintType)
class NEOKINECTUNREAL_API UNeoKinectManager : public UObject
{
	GENERATED_BODY()

public:

	/** Stores the bodies that Kinect can track. The array is created only once
	* and lasts until the end of the program. */
	static TArray<UNeoKinectBody*> Bodies;
	/** Stores the faces that Kinect can track. The array is created only once
	* and lasts until the end of the program. */
	static TArray<UNeoKinectFace*> Faces;
	/** Stores all the possible frame types Kinect or the program can create. 
	* The array is created only once and lasts until the end of the program. */
	static FFrameTextureData FramesData[static_cast<int32>(EKinectFrame::Count)];

	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FBodyTrackDelegate, UNeoKinectBody*, Body);
	DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FFaceTrackDelegate, UNeoKinectFace*, Face);

	/** Constructor: used only to bind the static dispatcher to the bodies/faces
	* tracking events. */
	UNeoKinectManager();
	/** Destructor: unbind the static dispatcher events and makes sure to
	* un-initialize the Kinect sensor before the program exits. */
	virtual ~UNeoKinectManager() override;

	/*-------------------------------------------------------------------------
		EVENTS
	-------------------------------------------------------------------------*/

private:
	/** Class singleton used to give Blueprints access to the tracking events. */
	static UNeoKinectManager* EventsDispatcher;

public:
	/** Called when the Kinect sensor has just detected a new user's body. */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect")
	FBodyTrackDelegate	OnBodyBeginTrack;
	
	/** Called when the Kinect sensor has just lost track of a user's body. */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect")
	FBodyTrackDelegate	OnBodyLost;

	/** Called when the Kinect sensor has just detected a new user's face. */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect")
	FFaceTrackDelegate	OnFaceBeginTrack;
	
	/** Called when the Kinect sensor has just lost track of a user's face. */
	UPROPERTY(BlueprintAssignable, Category = "NeoKinect")
	FFaceTrackDelegate	OnFaceLost;

	/**
	* Use the object result of this function to bind to the Kinect body/face
	* tracking events. You can also bind to individual bodies/faces using their
	* respective objects.
	*
	* @return Kinect events dispatcher object.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect")
	static UNeoKinectManager* GetKinectEventsDispatcher();

	/*////////////////////////////////////////////////////////////////////////////
	
								BLUEPRINT FUNCTIONS
	
	////////////////////////////////////////////////////////////////////////////*/

	/*
	*	BASICS
	*/

	/**
	* Initializes the Kinect sensor.
	* 
	* Even though this function always works without errors, check if the sensor
	* has really started by calling IsKinectAvailable. It can take a while, so
	* check in intervals until it does.
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Init Kinect Sensor", Keywords = "turn on initialize enable start begin"), Category = "NeoKinect")
	static void InitSensor();
	
	/**
	* Returns if sensor is already on.
	*
	* It may take a while for the sensor to initialize, so keep checking. If it
	* takes too long, either there's no connected sensor or it's malfunctioning.
	*
	* @return true if Sensor is available and operational.
	*/
	UFUNCTION(BlueprintPure, meta = (Keywords = "on working works usable"), Category = "NeoKinect")
	static bool IsKinectAvailable();

	/**
	* Use very carefully! Will shutdown the sensor and also invalidate any of its
	* frame textures (color, depth etc). Make sure you call this when none of them
	* are in use (preferably when quitting the game). Remember: with great power,
	* comes great responsibility!
	*/
	UFUNCTION(BlueprintCallable, meta = (DisplayName = "Uninit Kinect Sensor", Keywords = "turn off uninitialize disable end"), Category = "NeoKinect")
	static void UninitSensor();

	/**
	* Retrieves the Kinect sensor or NULL if not initialized.
	* 
	* @return The Kinect sensor or NULL if not initialized.
	*/
	static NeoKinect::KinectSensor* GetSensor();

	/**
	* Gets sensor tilt in degrees. It represents the pitch of the sensor in Unreal
	* coordinates (X pointing the same direction as the Kinect camera). Negative
	* values mean it's pointing down.
	*
	* @return Kinect sensor tilt in degrees. Negative values mean it's pointing down.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect", meta = (DisplayName = "Get Kinect Tilt"))
	static float GetSensorTilt();

	/**
	* Kinect height from the ground and ground normal. Only works if the sensor
	* can see the ground.
	* Bodies reading must be in use, as this data is read from the Body frames.
	*
	* @param Height Kinect distance from the ground
	* @return Ground normal
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect", meta = (DisplayName = "Get Kinect Ground Plane", Keywords = "height normal location floor"))
	static FVector GetGroundPlane(float &Height);

	/*
	*	EXPOSURE CONTROL
	*/

	/*
	* Fully automatic exposure setting with exposure compensation. Only affects the Color frame.
	* @param exposureCompensation	Negative value gives an underexposed image,
	*								positive gives an overexposed image.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|ColorExposure", meta = (DisplayName = "Set Kinect to Auto Exposure", Keywords = "image frame color brightness"))
	static void SetAutoExposure(float exposureCompensation = 0.0f);

	/*
	* Sets a pseudo-exposure time in ms. Only affects the Color frame.
	*
	* The actual frame integration time is set to a multiple of fluorescent light period
	* that is shorter than the requested time e.g. requesting 16 ms will set 10 ms
	* in Australia (100Hz), 8.33 ms in USA (120Hz).
	* The gain is automatically set to compensate for the reduced integration time.
	* 
	* Requesting less than a single fluorescent light period will set the integration time
	* to the requested value.
	*
	* To set the shortest non-flickering integration period for any country, simply set
	* a pseudo-exposure time of between (10.0, 16.667) ms, which will automatically drop
	* the integration time to 10 or 8.3 ms depending on country, while setting the analog
	* gain control to a brighter value.
	*
	* @param pseudoExposureTime	Pseudo-exposure time in ms. [0.0, 640]ms
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|ColorExposure", meta = (DisplayName = "Set Kinect to Semi Auto Exposure", Keywords = "image frame color brightness"))
	static void SetSemiAutoExposure(float pseudoExposureTime);

	/*
	* Manually set frame exposure time in ms and analog gain. Only affects the Color frame.
	* @param frameExposureTime	Time of exposure for each frame, in ms. [0.0, 640]ms
	* @param analogGain			Exposure analog gain.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|ColorExposure", meta = (DisplayName = "Set Kinect to Manual Exposure", Keywords = "image frame color brightness"))
	static void SetManualExposure(float frameExposureTime, float analogGain);

	/*
	*	COORDINATE REMAPPING
	*/

	/**
	* Converts a 3D location in Camera space to Color space with depth, so the new
	* location matches the Color frame position from camera's point of view
	* without turning into 2D.
	*
	* @param CameraLocation The 3D Camera space coordinate to be converted (in Unreal units)
	* @return Realigned 3D coordinate that matches the Color frame from the camera's
	*		  point of view (in Unreal units)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Remapping", Meta = (Keywords = "convert coordinate space remap"))
	static FVector CameraLocationToColorWithDepth(FVector CameraLocation);
	/** Native NeoKinect implementation of the above */
	static NeoKinect::Vector3 CameraLocationToColorWithDepth(NeoKinect::Vector3 CameraLocation);

	/**
	* Remaps a 3D camera location to a 2D point in the Color frame.
	*
	* @param CameraLocation The 3D Camera space coordinate to be remapped (in Unreal units)
	* @return The point location in the Color frame that matches the Camera
	*		  location input (in pixels)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Remapping", Meta = (Keywords = "convert coordinate space remap"))
	static FVector2D CameraLocationToColorPoint(FVector CameraLocation);

	/**
	* Remaps a 3D camera location to a point in the Depth frame.
	*
	* @param CameraLocation The 3D Camera space coordinate to be remapped (in Unreal units)
	* @return The point location in the Depth frame that matches the Camera
	*		  location input (in pixels)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Remapping", Meta = (Keywords = "convert coordinate space remap"))
	static FVector2D CameraLocationToDepthPoint(FVector CameraLocation);

	/**
	* Creates a Color with depth point from a Color point only. Viewed from the
	* camera perspective, this point aligns with the Color frame without losing
	* it's depth.
	*
	* This method uses the remapped Color buffer in Depth space so, for it to
	* work, the Depth in Color Space frame must be in use.
	*
	* @param ColorLocation A point in the Color frame to be remapped (in pixels)
	* @return The 3D version of the Color point (in Unreal units)
	*/
	static NeoKinect::Vector3 ColorWithDepthFromColorOnly(const NeoKinect::Vector2& ColorLocation);

	/**
	* Remaps a point location from the Depth frame to the Color frame.
	*
	* @param DepthPoint A point in the Depth frame (in pixels)
	* @return Remapped Depth frame point to a Color frame point (in pixels)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Remapping", Meta = (Keywords = "convert coordinate space remap"))
	static FVector2D DepthPointToColorPoint(FVector2D DepthPoint);

	/**
	* Remaps a point location from the Depth frame to the 3D camera space.
	* 
	* @param DepthPoint A point in the Depth frame (in pixels)
	* @return Remapped Depth frame point to a 3D Camera location (in Unreal units)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Remapping", Meta = (Keywords = "convert coordinate space remap"))
	static FVector DepthPointToCameraLocation(FVector2D DepthPoint);

	/**
	* Remaps a point from the Color frame to the Depth frame.
	* 
	* @param ColorPoint A point in the Color frame (in pixels)
	* @return Color point remapped to its related location in the Depth frame (in pixels)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Remapping", Meta = (Keywords = "convert coordinate space remap"))
	static FVector2D ColorPointToDepthPoint(FVector2D ColorPoint);


	/*
	*	TEXTURES (Color, Depth etc)
	*/

	/**
	* Get a frame horizontal and vertical field of view.
	*
	* @param FrameType The frame type to request the FOV from.
	* @return x = horizontal FOV, y = vertical FOV
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Frames", Meta = (Keywords = "width height FOV texture angle field of view", DisplayName = "Get Kinect FOV"))
	static FVector2D GetFrameFieldOfView(EKinectFrame FrameType);

	/**
	* Get a frame texture width and height in pixels.
	*
	* @param FrameType The frame type to request the size from.
	* @return x = width, y = height
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Frames", Meta = (Keywords = "width height texture resolution", DisplayName = "Get Kinect Frame Size"))
	static FIntPoint GetFrameSize(EKinectFrame FrameType);

	/**
	* Enables or disables updating of a Kinect frame type.
	*
	* @param FrameType The frame type to enable/disable
	* @param bEnable If the selected frame type should be enabled
	* @return The Frame texture. Null if Enable = false.
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Frames", Meta = (Keywords = "texture sensor camera enable disable toggle", DisplayName = "Set Use Kinect Frame"))
	static UTextureRenderTarget2D* SetUseFrame(EKinectFrame FrameType, bool bEnable);

	/**
	* Returns if a Kinect frame type is enabled. If it is, it means you can get
	* it's texture with GetFrame.
	*
	* @param FrameType The frame type to ask for. 
	* @return If FrameType is enabled or not.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Frames", Meta = (Keywords = "texture sensor camera enabled", DisplayName = "Get Is Using Kinect Frame"))
	static bool GetIsUsingFrame(EKinectFrame FrameType);

	/**
	* Get the texture that represents the requested frame type.
	*
	* @param FrameType The frame type to get the texture from.
	* @return The texture that represents the requested frame type. Null if it has
	*		  not been enabled.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Frames", Meta = (Keywords = "texture sensor camera", DisplayName = "Get Kinect Frame"))
	static UTextureRenderTarget2D* GetFrame(EKinectFrame FrameType);

	/**
	* Gets the depth value from a specific pixel on the Depth frame.
	* 
	* @param Coordinate A point in the Depth frame (in pixels)
	* @return The Depth value (distance from the Kinect sensor) in the input point
	*		  (in centimeters)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Frames", Meta = (Keywords = "sample texture sensor camera"))
	static float GetDepthFramePixel(FVector2D Coordinate);

	/**
	* Gets the depth value from a specific pixel on the Color frame
	*
	* @param Coordinate A point in the Color frame (in pixels)
	* @return The Depth value (distance from the Kinect sensor) in the input point
	*		  (in centimeters)
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Frames", Meta = (Keywords = "sample texture remap"))
	static float GetDepthFromColorFramePixel(FVector2D Coordinate);

	/**
	* Gets the color value from a specific pixel on the Color frame.
	* 
	* @param Coordinate A point in the Color frame (in pixels)
	* @return The Color value in the input point
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Frames", Meta = (Keywords = "sample texture sensor camera"))
	static FLinearColor GetColorFramePixel(FVector2D Coordinate);

	/*
	*	BODIES / JOINTS
	*/

	/**
	* Array of all bodies (tracked and not tracked).
	* The bodies in the array are updated automatically, so you don't need to
	* request the array more than once.
	*
	* @return Bodies array.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body", meta = (DisplayName = "Get Kinect Bodies"))
	static TArray<UNeoKinectBody*> GetBodies();

	/**
	* Array with only the currently tracked bodies.
	*
	* @return The currently tracked bodies.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body", meta = (DisplayName = "Get Kinect Tracked Bodies"))
	static TArray<UNeoKinectBody*> GetTrackedBodies();

	/**
	* Gets tracked body whose Spine Base joint is nearest to the Kinect sensor.
	*
	* @param HasBody If there's at least one tracked body.
	* @return The nearest body from the sensor. Null if no bodies are tracked.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body", meta = (DisplayName = "Get Kinect Nearest Body"))
	static UNeoKinectBody* GetNearestBody(bool& HasBody);

	/**
	* Toggle the use of Color texture aligned joint locations and orientations.
	*
	* Calculating these remapped transforms introduce a small overhead, so it's
	* disabled by default and reading the Color values from a Joint will return
	* Zero values until this is enabled.
	*
	* @param bUse Enable usage of Joints' Color space transforms?
	*/
	UFUNCTION(BlueprintCallable, Category = "NeoKinect|Body", meta = (Keywords = "joints remap"))
	static void SetUseJointsColorSpaceTransforms(const bool bUse);

	/**
	* Return if Joints' Color space transforms usage is enabled
	*
	* @return True if Joints' Color space transforms usage is enabled
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body")
	static bool IsUsingJointsColorSpaceTransforms();

	/**
	* Cast a Body in the bodies array to its index number.
	*
	* @param Body The body instance.
	* @return Body index.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body", Meta = (CompactNodeTitle = "->", Keywords = "convert cast"))
	static int32 BodyToIndex(const UNeoKinectBody* Body);

	/**
	* Cast an index to a body from the bodies array.
	*
	* @param Index Index.
	* @return Body instance. If index > 5 or index < 0, it's the body at index 0.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body", Meta = (CompactNodeTitle = "->", Keywords = "convert cast"))
	static UNeoKinectBody* IndexToBody(const int32 Index);

	/**
	* Cast the Joint type to its joint index number.
	*
	* @param Joint Joint type.
	* @return Index.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (CompactNodeTitle = "->", Keywords = "convert cast", BlueprintThreadSafe))
	static int32 JointToIndex(const EKinectJointType Joint);

	/**
	* Cast a joint index to its Joint type.
	* 
	* @param Index Index.
	* @return Joint type. If index >= joint count or index < 0, it's Spine Base joint.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Body|Joints", Meta = (CompactNodeTitle = "->", Keywords = "convert cast"))
	static EKinectJointType IndexToJoint(const int32 Index);

	/*
	*	FACE TRACKING
	*/

	/**
	* Array of faces (tracked and not tracked). The faces instances are updated
	* automatically, so you don't need to request the array more than once.
	*
	* @return Faces array.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Face", meta = (DisplayName = "Get Kinect Faces"))
	static TArray<UNeoKinectFace*> GetFaces();

	/**
	* Array with only the currently tracked faces.
	*
	* @return The currently tracked faces.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Face", meta = (DisplayName = "Get Kinect Tracked Faces"))
	static TArray<UNeoKinectFace*> GetTrackedFaces();

	/**
	* Gets tracked face whose Head joint is nearest from the sensor.
	*
	* @param bHasAnyFace If there's at least one tracked face.
	* @return The nearest face from the sensor. Null if no faces are tracked.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Face", meta = (DisplayName = "Get Kinect Nearest Face"))
	static UNeoKinectFace* GetNearestFace(bool& bHasAnyFace);

	/**
	* Cast a Face to its index number.
	*
	* @param Face The face instance.
	* @return Face index.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Face", Meta = (CompactNodeTitle = "->", Keywords = "convert cast"))
	static int32 FaceToIndex(const UNeoKinectFace* Face);

	/**
	* Cast a face index to a Face object.
	*
	* @param Index Index.
	* @return Face instance. If index > 5 or index < 0, it's the face at index 0.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Face", Meta = (CompactNodeTitle = "->", Keywords = "convert cast"))
	static UNeoKinectFace* IndexToFace(const int32 Index);

	/*
	*	CONSTANTS
	*/

	/**
	* The number of bodies the Kinect device can track simultaneously.
	* 
	* @return The number of bodies the Kinect device can track simultaneously.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Constants", meta = (Keywords = "length number size capability"))
	static int32 GetTrackableBodyCount();

	/**
	* The number of faces the Kinect device can track simultaneously.
	* 
	* @return The number of faces the Kinect device can track simultaneously.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Constants", meta = (Keywords = "length number size capability"))
	static int32 GetTrackableFaceCount();

	/**
	* The number of joints the Kinect device can track in each tracked body.
	* 
	* @return The number of joints the Kinect device can track in each tracked body.
	*/
	UFUNCTION(BlueprintPure, Category = "NeoKinect|Constants", meta = (Keywords = "length number size capability"))
	static int32 GetJointCount();

	/*////////////////////////////////////////////////////////////////////////////

							BLUEPRINT FUNCTIONS END

	////////////////////////////////////////////////////////////////////////////*/


private:
	/** NeoKinect API sensor */
	static NeoKinect::KinectSensor* Kinect;
	/** Does the sensor polling */
	static FKinectThread* KinectThread;
	/** FTicker handle so we can stop ticking after uninitializing the sensor */
	static FTSTicker::FDelegateHandle TickerHandle;

	//TODO: figure out a way to know when a level is unloaded
	//static bool bAutoShutdown;

	/** Should we read and update the bodies data each frame? */
	static bool bUsingBodyFrame;
	/** Should we remap the bodies transforms into 3D Color space each frame? */
	static bool bUsingJointsColorSpaceTransforms;
	/** Should we read and update the faces data each frame? */
	static bool bUsingFaceFrame;

	/** Initialize the bodies, faces and frames arrays */
	static void initStatics();
	/** Updates bodies, faces and frames each frame */
	static bool OnTick(float DeltaTime);

	static bool IsUsingDepth();
	static bool IsUsingBodyIndex();
	static bool IsUsingColor();
	/** Creates a 2D dynamic texture for frames usage */
	static UTextureRenderTarget2D* CreateTexture(int32 Width, int32 Height, EPixelFormat PixelFormat, bool sRGB);
	/** Updates all currently enabled frames. Executed in the Render thread */
	static void UpdateFrames();
	/** Delete frames textures memory */
	static void ReleaseFrameResources(FFrameTextureData&);

	/** Enable/disable bodies frame reading */
	static void SetUseBodyFrame(bool bUse = true);
	/** Converts bodies joints transforms to Unreal coordinate space */
	static void ProcessBodies(const NeoKinect::KinectBody* KBodies);
	/** Zero all bodies values. Used when uninitializing the sensor */
	static void ResetBodies();

	/** Enable/disable faces frame reading */
	static void SetUseFaceFrame(bool bUse = true);
	/** Converts faces points transforms to Unreal coordinate space */
	static void ProcessFaces(const NeoKinect::KinectFace* KFaces);
	/** Zero all faces values. Used when uninitializing the sensor */
	static void ResetFaces();
	
	UFUNCTION()
	void BodyBeginTrackBroadcast(int32 BodyIndex);
	UFUNCTION()
	void BodyLostBroadcast(int32 BodyIndex);
	UFUNCTION()
	void FaceBeginTrackBroadcast(int32 FaceIndex);
	UFUNCTION()
	void FaceLostBroadcast(int32 FaceIndex);
};
