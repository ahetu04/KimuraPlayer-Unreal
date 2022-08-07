//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include <memory>
#include <string>
#include <vector>

namespace Kimura
{

	typedef unsigned char 		byte;
	typedef char 				int8;
	typedef unsigned char 		uint8;
	typedef short int			int16;
	typedef unsigned short int	uint16;
	typedef int					int32;
	typedef unsigned int		uint32;
	typedef long long			int64;
	typedef unsigned long long	uint64;


	std::string GetVersion();

	struct Vector2
	{
		Vector2() {};
		Vector2(float x, float y) : X(x), Y(y) {}

		float X = 0.0f;
		float Y = 0.0f;

		static const Vector2 ZeroVector;
	};

	struct Vector3
	{
		Vector3(){};
		Vector3(float x, float y, float z) : X(x), Y(y), Z(z) {}

		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;


		inline Vector3 operator+(const Vector3& v) const
		{
			return Vector3(this->X + v.X, this->Y + v.Y, this->Z + v.Z);
		}

		inline Vector3 operator-(const Vector3& v) const
		{
			return Vector3(this->X - v.X, this->Y - v.Y, this->Z - v.Z);
		}

		inline Vector3 operator*(float fScalar) const
		{
			return Vector3(this->X * fScalar, this->Y * fScalar, this->Z * fScalar);
		}

		inline void operator+=(const Vector3& v)
		{
			this->X += v.X;
			this->Y += v.Y;
			this->Z += v.Z;
		}

		inline void operator*=(float fScalar)
		{
			this->X *= fScalar;
			this->Y *= fScalar;
			this->Z *= fScalar;
		}

		inline void SwizzleYZ()
		{
			float t = this->Y;
			this->Y = this->Z;
			this->Z = t;
		}

		inline void SwizzleXZ()
		{
			float t = this->X;
			this->X = this->Z;
			this->Z = t;
		}

		static const Vector3 ZeroVector;
		static const Vector3 OneVector;
	};

	struct Vector4
	{
		Vector4() {};
		Vector4(float x, float y, float z, float w) : X(x), Y(y), Z(z), W(w) {}

		float X = 0.0f;
		float Y = 0.0f;
		float Z = 0.0f;
		float W = 0.0f;

		inline void SwizzleYZ()
		{
			float t = this->Y;
			this->Y = this->Z;
			this->Z = t;
		}

		inline void SwizzleXZ()
		{
			float t = this->X;
			this->X = this->Z;
			this->Z = t;
		}

		static const Vector4 ZeroVector;
		static const Vector4 OneVector;

	};

	struct MeshSection
	{
		unsigned int VertexStart = 0;
		unsigned int IndexStart = 0;		
		unsigned int NumSurfaces = 0;
		unsigned int MinVertexIndex = 0;
		unsigned int MaxVertexIndex = 0;
	};

	enum class PlayerStatus
	{
		Initializing, 
		Ready, 
		Failed
	};


	enum class PositionFormat : int
	{
		Full,
		Half
	};

	enum class NormalFormat : int
	{
		Full,
		Half,
		Byte,
		None
	};

	enum class TangentFormat : int
	{
		Full,
		Half,
		Byte,
		None
	};

	enum class VelocityFormat : int
	{
		Full,
		Half,
		Byte,
		None
	};

	enum class TexCoordFormat : int
	{
		Full,
		Half,
		None
	};

	enum class ColorFormat : int
	{
		Full,			// Full HDR, 32bit
		Half,			// Quantized HDR with decent precision
		ByteHDR,		// Quantized HDR with very limited precision
		Byte,			// 0...1
		None
	};

	class IFrame
	{
		public:

			uint32						FrameIndex = 0;
			double						ReadTimeInMS = 0.0;							// how long it tool to load this frame from disk
			double						ProcessTimeInMS = 0.0;						// how long it took to process this frame's data

			virtual uint32				GetNumVertices(uint32 iMeshIndex) = 0;
			virtual uint32				GetNumSurfaces(uint32 iMeshIndex) = 0;

			virtual void				GetSections(uint32 iMeshIndex, std::vector<MeshSection>& OutSections) = 0;

			virtual const uint16*		GetIndicesU16(uint32 InMeshIndex) = 0;
			virtual const uint32*		GetIndicesU32(uint32 InMeshIndex) = 0;

			virtual const Vector3*		GetPositionsF32(uint32 InMeshIndex) = 0;
			virtual const int16*		GetPositionsI16(uint32 InMeshIndex) = 0;
			virtual Vector3				GetPositionQuantizationCenter(uint32 InMeshIndex) = 0;
			virtual Vector3				GetPositionQuantizationExtents(uint32 InMeshIndex) = 0;

			virtual const Vector3*		GetNormalsF32(uint32 InMeshIndex) = 0;
			virtual const int16*		GetNormalsI16(uint32 InMeshIndex) = 0;
			virtual const int8*			GetNormalsI8(uint32 InMeshIndex) = 0;

			virtual const Vector4*		GetTangentsF32(uint32 InMeshIndex) = 0;
			virtual const int16*		GetTangentsI16(uint32 InMeshIndex) = 0;
			virtual const int8*			GetTangentsI8(uint32 InMeshIndex) = 0;

			virtual const Vector3*		GetVelocitiesF32(uint32 InMeshIndex) = 0;
			virtual const int16*		GetVelocitiesI16(uint32 InMeshIndex) = 0;
			virtual const int8*			GetVelocitiesI8(uint32 InMeshIndex) = 0;
			virtual Vector3				GetVelocityQuantizationCenter(uint32 InMeshIndex) = 0;
			virtual Vector3				GetVelocityQuantizationExtents(uint32 InMeshIndex) = 0;

			virtual const Vector2*		GetTexCoordsF32(uint32 InMeshIndex, uint32 iTexCoord) = 0;
			virtual const uint16*		GetTexCoordsU16(uint32 InMeshIndex, uint32 iTexCoord) = 0;

			virtual const Vector4*		GetColorsF32(uint32 InMeshIndex, uint32 iColor) = 0;
			virtual const uint16*		GetColorsU16(uint32 InMeshIndex, uint32 iColor) = 0;
			virtual const uint8*		GetColorsU8(uint32 InMeshIndex, uint32 iColor) = 0;
			virtual Vector4				GetColorQuantizationExtents(uint32 InMeshIndex, uint32 InColorIndex) = 0;

			virtual void				GetBounds(uint32 InMeshIndex, Vector3& OutCenter, Vector3& OutSize) = 0;

			virtual bool				GetImageData(uint32 InImageIndex, uint32 InMipmap, const void** OutData, uint32& OutSize) = 0;

	};

	struct MeshInformation
	{
		uint32				Index = 0;
		std::string			Name;
		uint64				MaximumVertices = 0;
		uint64				MaximumSurfaces = 0;
		bool				Force16BitIndices = false;

		PositionFormat		PositionFormat_ = PositionFormat::Full;
		NormalFormat		NormalFormat_ = NormalFormat::Full;
		TangentFormat		TangentFormat_ = TangentFormat::None;
		VelocityFormat		VelocityFormat_ = VelocityFormat::Full;
		TexCoordFormat		TexCoordFormat_ = TexCoordFormat::Full;
		ColorFormat			ColorFormat_ = ColorFormat::Byte;
	};

	struct ImageSequenceInformation
	{
		uint32 Index = 0;
		std::string Name;
		std::string Format;
		uint32 Width = 0;
		uint32 Height = 0;
		uint32 Mipmaps = 0;

		uint32 FrameCount = 0;
		bool Constant = true;

	};

	struct PlaybackInformation
	{

		float						TimePerFrame = 1.0f / 30.0f;
		float						FrameRate = 30.0f;

		uint32						FrameCount;
		float						Duration;

		std::vector<MeshInformation>			Meshes;
		std::vector<ImageSequenceInformation>	ImageSequences;

		std::vector<std::string>		OptimizationSuggestions;

	};

	struct PlayerStats
	{
		uint32 BufferedFramesStart = 0;
		uint32 BufferedFramesCount = 0;

		uint64 BytesReadInLastSecond = 0;
		uint64 MemoryUsageForFrames = 0;

		uint32 NumFramesProcessedInLastSecond = 0;

		double AvgTimeSpentOnReadingFromDiskPerFrame = 0.0;
		double TotalTimeSpentOnReadingFromDiskInLastSecond = 0.0;

		double TotalTimeSpentOnProcessingFramesInLastSecond = 0.0;
		double AvgTimeSpentOnProcessingPerFrames = 0.0;

	};


	class IPlayer
	{
		public:

			virtual PlayerStatus GetStatus() = 0;
			virtual void GetFailStatusMessage(std::string& OutMessage) = 0;

			virtual std::string GetFileVersion() = 0;

			virtual bool RetrievePlaybackInformation(PlaybackInformation& OutInfo) = 0;

			virtual int GetBufferedFrameCount() = 0;
			virtual std::shared_ptr<IFrame>	GetFrameAt(uint32 iFrame, bool InForceWait) = 0;
			virtual std::shared_ptr<IFrame>	GetConstantFrame() = 0;

			virtual uint32	GetNumFrames() = 0;
			
			virtual bool	IsForcing16BitIndices() = 0;

			virtual void CollectStats(PlayerStats& OutStats) = 0;

	};

	class PlayerOptions
	{
		public:
			
			uint32 PreBufferingSize = 20;
			uint32 BackBufferSize = 10;

			bool BufferEntirePlayback = false;

			bool Loop = true;

	};

	std::shared_ptr<IPlayer>	CreatePlayer(const std::string& InPath, const PlayerOptions& InOptions);

}