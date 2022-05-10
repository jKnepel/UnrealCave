#pragma once

#include <Windows.h>
#include <Ole2.h>

#include <stdio.h>

#include <Kinect.h>
#include <Kinect.Face.h>

#include <KinectExposure.h>

namespace NeoKinect
{
	constexpr unsigned int KinectColorWidth = 1920;
	constexpr unsigned int KinectColorHeight = 1080;

	constexpr unsigned int KinectDepthWidth = 512;
	constexpr unsigned int KinectDepthHeight = 424;

	constexpr unsigned int KinectBodyCount = BODY_COUNT;
	constexpr unsigned int KinectJointCount = JointType_Count;

	struct Vector2
	{
		float x, y;

		Vector2() : x(0.f), y(0.f) {}
		Vector2(float xy) : x(xy), y(xy) {}
		Vector2(float x, float y) : x(x), y(y) {}
		explicit Vector2(const DepthSpacePoint& dsp) : x(dsp.X), y(dsp.Y) {}
		explicit Vector2(const ColorSpacePoint& csp) : x(csp.X), y(csp.Y) {}
		explicit Vector2(const PointF& pf) : x(pf.X), y(pf.Y) {}

		template<typename T>
		inline Vector2& operator=(const T& b);

		inline DepthSpacePoint toDepthSpacePoint() const;
		inline ColorSpacePoint toColorSpacePoint() const;
	};

	struct Vector3
	{
		float x, y, z;

		Vector3() : x(0.f), y(0.f), z(0.f) {}
		Vector3(float xyz) : x(xyz), y(xyz), z(xyz) {}
		Vector3(float x, float y, float z) : x(x), y(y), z(z) {}
		explicit Vector3(const CameraSpacePoint& csp) : x(csp.X), y(csp.Y), z(csp.Z) {}

		inline Vector3& operator=(const CameraSpacePoint& b);
		inline Vector3& operator/=(const float& b);
		inline const Vector3 operator/(const float& b) const;
		inline Vector3& operator*=(const float& b);
		inline const Vector3 operator*(const float& b) const;

		inline bool IsZero() const;
		float magnitude() const; // not inline as it uses math header and I didn't want to polute the headers
		inline Vector3 normalized() const;

		inline CameraSpacePoint toCameraSpacePoint() const;
	};

	enum class KinectFrameType : BYTE
	{
		Color					= 0,
		Depth					= 1,
		Body					= 2,
		BodyIndex				= 3,
		Infrared				= 4,
		LongExposureInfrared	= 5,
		Face					= 6,
		Count					= 7
	};

	enum class KinectCoordinateSpace : BYTE
	{
		Camera = 0,
		Depth = 1,
		Color = 2,
		Count = 3
	};

	enum class KinectPixelFormat : BYTE
	{
		B8G8R8A8 = 0,
		G8 = 1,
		G16 = 2,
		Bayer = 3,
		None = 4,
		Count = 5
	};

	struct KinectJoint
	{
		Vector4				orientation;
		CameraSpacePoint	position;
		TrackingState		trackingState;
		JointType			type;
	};

	struct KinectBodyEdges
	{
		bool left, right, top, bottom;
	};

	struct KinectBody
	{
		UINT64	trackingId;
		bool	isTracked, isRestricted;

		KinectBodyEdges clippedEdges;

		HandState		handLeftState;
		TrackingState	handLeftTrackingState;

		HandState		handRightState;
		TrackingState	handRightTrackingState;

		TrackingState	leanTrackingState;
		Vector2			lean;

		KinectJoint		joints[JointType_Count];
	};

	struct KinectFacePoints
	{
		Vector2	leftEye;
		Vector2	rightEye;
		Vector2	nose;
		Vector2	leftMouthCorner;
		Vector2	rightMouthCorner;
	};

	struct KinectFace
	{
		UINT64				trackingId;
		// face bounding box in color space
		RectI				boxColor;
		// face bounding box in infrared space
		RectI				boxInfrared;
		// face rotation in quaternion
		Vector4				rotation;
		// face pivot point. The same as the skeleton head joint. It's here for conveninence.
		CameraSpacePoint	pivot;
		// face alignment points in color space
		KinectFacePoints	pointsColor;
		// face alignment points in infrared space
		KinectFacePoints	pointsInfrared;

		DetectionResult		isEngaged;
		DetectionResult		isHappy;
		DetectionResult		isLookingAway;
		DetectionResult		isMouthMoved;
		DetectionResult		isMouthOpen;
		DetectionResult		isLeftEyeClosed;
		DetectionResult		isRightEyeClosed;
		DetectionResult		isWearingGlasses;
		
		bool bIsTracked;
	};

	struct FrameDescription
	{
		bool valid;

		float diagonalFieldOfView,
			horizontalFieldOfView,
			verticalFieldOfView;

		UINT bytesPerPixel;
		UINT lengthInPixels;
		int width, height;

		FrameDescription() : valid(false) {}
	};

	class KinectSensor
	{
	public:
		KinectSensor();
		~KinectSensor();

		/*
		Opens the Microsoft API to the sensor.
		@return Is the sensor API working?
		*/
		bool InitSensor();
		/*
		Is the sensor on already?
		It may take some time for the sensor to effectivelly turn on. Keep trying.
		If it's taking too long, there's probably no connected sensor or it's not working.
		@return True if the sensor is on.
		*/
		bool IsSensorAvailable();
		/*
		Is the Microsoft Kinect API responding?
		@return True if the sensor API is working.
		*/
		bool IsSensorOpen();
		/*
		Shuts the sensor down and closes the Microsoft API to it.
		@return True if it worked.
		*/
		bool UninitSensor();


		/*
		A Vector4 containing the floor normal (xyz) and height (w).
		Valid only if frequently locking and unlocking Body frame. That's when
		it gets updated.
		@return A Vector4 containing the floor normal (xyz) and height (w).
		*/
		Vector4 GetFloorClipPlane();
		/*
		Uses the floor clip plane to calculate how much the sensor is tilted.
		@return The sensor tilt in degrees. Looking down gives negative values.
		*/
		float GetSensorTilt();

		FrameDescription GetFrameDescription(const KinectFrameType frameType) const;

		bool SetUseFrame(const KinectFrameType frameType, const bool use = true);
		bool GetIsUsingFrame(const KinectFrameType frameType) const;
		bool LockLatestFrame(KinectFrameType frameType);
		void UnlockLatestFrame(KinectFrameType frameType);
		bool GetIsFrameLocked(KinectFrameType frameType) const;
		bool GetLatestFrameData(const KinectFrameType frameType, BYTE *& buffer, const KinectPixelFormat pixelFormat = KinectPixelFormat::None);
		bool GetLatestFrameData(KinectBody bodies[BODY_COUNT]);
		bool GetLatestFrameData(KinectFace faces[BODY_COUNT]);

		/*
		Gets a frame remapped to another coordinate system.
		All the remapping methods depend on the Depth frame. So make sure you're
		frequently locking and unlocking it.
		@return True if it worked.
		*/
		bool GetRemappedFrameData(
			const KinectFrameType frameType, const KinectCoordinateSpace targetSpace,
			BYTE *const buffer, const KinectPixelFormat pixelFormat);
		/*
		Gets a point remapped to another coordinate system.
		All the remapping methods depend on the Depth frame. So make sure you're
		frequently locking and unlocking it.
		@return True if it worked.
		*/
		bool GetRemappedPoint(
			const KinectCoordinateSpace fromSpace, const KinectCoordinateSpace toSpace,
			const Vector2& srcPoint, Vector2& resultPoint);
		bool GetRemappedPoint(
			const KinectCoordinateSpace fromSpace, const KinectCoordinateSpace toSpace,
			const Vector2& srcPoint, Vector3& resultPoint) const;
		bool GetRemappedPoint(
			const KinectCoordinateSpace fromSpace, const KinectCoordinateSpace toSpace,
			const Vector3& srcPoint, Vector2& resultPoint) const;

		/*
		Samples distance from the Depth frame.
		The pixel used will be the round result of X and Y.
		Note: this only works if the Depth frame is used and is being locked and
		unlocked frequently.
		*/
		UINT16 SampleDepthFrame(const Vector2& point);
		/*
		Samples distance from the Depth frame using a coordinate from the Color frame.
		The pixel used will be the round result of X and Y after remapped to the Depth frame.
		Note: this only works if the 'Depth' and 'Depth remapped to Color' frames are both
		used and are being locked and unlocked frequently.
		*/
		UINT16 SampleDepthFromColorCoords(const Vector2& colorPoint);

		/*
		Fully automatic exposure setting.
		@param exposureCompensation Negative value gives an underexposed image,
									positive gives an overexposed image.
		*/
		void SetAutoExposure(float exposureCompensation = 0);

		/*
		Sets a pseudo-exposure time in ms.
		
		The actual frame integration time is set to a multiple of fluorescent light period
		that is shorter than the requested time e.g. requesting 16 ms will set 10 ms
		in Australia (100Hz), 8.33 ms in USA (120Hz).
		The gain is automatically set to compensate for the reduced integration time.
		
		Requesting less than a single fluorescent light period will set the integration time
		to the requested value.
		
		To set the shortest non-flickering integration period for any country, simply set
		a pseudo-exposure time of between (10.0, 16.667) ms, which will automatically drop
		the integration time to 10 or 8.3 ms depending on country, while setting the analog
		gain control to a brighter value.

		@param pseudoExposureTime	Pseudo-exposure time in ms. [0.0, 640]ms
		*/
		void SetSemiAutoExposure(float pseudoExposureTime);

		/*
		Manually set true exposure time and analog gain.
		@param frameExposureTime	Time of exposure for each frame, in ms. [0.0, 640]ms
		@param analogGain			Exposure analog gain.
		*/
		void SetManualExposure(float frameExposureTime, float analogGain);

	private:
		IKinectSensor			*pSensor;				// Kinect sensor
		//IMultiSourceFrameReader	*pMultiFrameReader;
		//IMultiSourceFrame		*pMultiFrame;

		KinectExposure			exposureControl;

		// Frame types
#define FRAME_VAR(name) I ##name## Frame *p ##name## Frame; I ##name## FrameReader *p ##name## FrameReader;
		FRAME_VAR(Body);
		FRAME_VAR(BodyIndex);
		FRAME_VAR(Color);
		FRAME_VAR(Depth);
		FRAME_VAR(Infrared);
		FRAME_VAR(LongExposureInfrared);
#undef FRAME_VAR

		// Face frame resources
		/* Features we'll use from face frames */
		static const DWORD	cFaceFrameFeatures =
			FaceFrameFeatures_FaceEngagement
			| FaceFrameFeatures_Happy
			| FaceFrameFeatures_LookingAway
			| FaceFrameFeatures_Glasses
			| FaceFrameFeatures_LeftEyeClosed
			| FaceFrameFeatures_RightEyeClosed
			| FaceFrameFeatures_MouthMoved
			| FaceFrameFeatures_MouthOpen
			| FaceFrameFeatures_RotationOrientation
			| FaceFrameFeatures_PointsInColorSpace
			| FaceFrameFeatures_PointsInInfraredSpace
			| FaceFrameFeatures_BoundingBoxInColorSpace
			| FaceFrameFeatures_BoundingBoxInInfraredSpace;
		IFaceFrameSource	*pFaceFrameSources[BODY_COUNT];
		IFaceFrameReader	*pFaceFrameReaders[BODY_COUNT];
		IFaceFrame			*pFaceFrames[BODY_COUNT];

		// Converts between depth, color and 3d coords
		ICoordinateMapper	*pMapper;
		DepthSpacePoint		*pColorMappedToDepth;
		bool				bIsColorMappedToDepthDirty;
		/* We keep a copy of the latest depth frame so the user can convert points
		between spaces anytime, not only when the depth frame is locked.
		This copy is updated everytime the frame is locked. Because of it, some
		coordinate space convertions may not work if the depth frame is not in use
		nor locked frequently. */
		UINT16				*pDepthRawCopy;

		IBody *bodies[BODY_COUNT];
		Joint joints[JointType_Count]; // keep joints array in memory to avoid frequent allocations
		JointOrientation jointOrientations[JointType_Count];
		Vector4 floorClipPlane;

		inline bool InitMapper();
		inline void UpdateColorMappedToDepth();
		inline int IndexFromXY(int pitch, float x, float y) const;
	};

	/* Print formatted string to the Output window */
	void Printf(const char *szFormat, ...);
	void LogFailedHResult(const HRESULT hr, const char *functionName, const int line, const char *action = "");
	template <typename T>
	inline void SafeRelease(T*& pointer);
	template <typename T>
	inline void SafeReleaseArray(T*& pointer);

	/*-------------------------------------------------------------------------
		Templates and inlines implementation
	-------------------------------------------------------------------------*/
	
	template<typename T>
	Vector2& Vector2::operator=(const T& b)
	{
		if (this != &b)
		{
			this->x = b.X;
			this->y = b.Y;
		}
		return *this;
	}

	DepthSpacePoint Vector2::toDepthSpacePoint() const
	{
		DepthSpacePoint b;
		b.X = this->x;
		b.Y = this->y;
		return b;
	}

	ColorSpacePoint Vector2::toColorSpacePoint() const
	{
		ColorSpacePoint b;
		b.X = this->x;
		b.Y = this->y;
		return b;
	}

	Vector3& Vector3::operator=(const CameraSpacePoint& b)
	{
		this->x = b.X;
		this->y = b.Y;
		this->z = b.Z;
		return *this;
	}

	Vector3& Vector3::operator/=(const float& other)
	{
		x /= other;
		y /= other;
		z /= other;
		return *this;
	}

	const Vector3 Vector3::operator/(const float& other) const
	{
		Vector3 result(*this);
		return result /= other;
	}

	Vector3& Vector3::operator*=(const float& other)
	{
		x *= other;
		y *= other;
		z *= other;
		return *this;
	}

	const Vector3 Vector3::operator*(const float& other) const
	{
		Vector3 result(*this);
		return result *= other;
	}

	bool Vector3::IsZero() const
	{
		return x == 0.f && y == 0.f && z == 0.f;
	}

	Vector3 Vector3::normalized() const
	{
		if (IsZero())
			return Vector3();
		return (*this / magnitude());
	}

	CameraSpacePoint Vector3::toCameraSpacePoint() const
	{
		CameraSpacePoint b;
		b.X = this->x;
		b.Y = this->y;
		b.Z = this->z;
		return b;
	}

	template<typename T>
	inline void SafeRelease(T*& pointer)
	{
		if (pointer)
		{
			pointer->Release();
		}
		pointer = nullptr;
	}

	template<typename T>
	inline void SafeReleaseArray(T*& pointer)
	{
		if (pointer)
		{
			delete[] pointer;
		}
		pointer = nullptr;
	}
}