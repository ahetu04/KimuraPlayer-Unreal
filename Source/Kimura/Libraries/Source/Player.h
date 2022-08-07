//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "Kimura.h"

#if defined(KIMURA_UNREAL)

	#define KIMURA_TRACE(x) TRACE_CPUPROFILER_EVENT_SCOPE(TEXT(#x))

#elif defined(_WIN32)

	#define KIMURA_WINDOWS 1

	#define KIMURA_TRACE(x)

#else

	// default input stream
	#include <fstream>

	#define KIMURA_TRACE(x)

#endif

namespace Kimura
{

	struct Version
	{
		uint8 A = 0;
		uint8 B = 5;
		uint8 C = 0;
		uint8 NotUsed = 0;

		bool SameAs(Version other)
		{
			return this->A == other.A && this->B == other.B && this->C == other.C;
		}

		bool CompatibleWith(Version other)
		{
			return this->A == other.A && this->B == other.B;
		}

		std::string ToString()
		{
			char buf[128];
			snprintf(buf, sizeof(buf), "%d.%d.%d", this->A, this->B, this->C);
			return std::string(buf);
		}
	};

	static const uint32					MaxTextureCoords = 4;
	static const uint32					MaxColorChannels = 2;
	static const uint32					MaxMipmaps = 8;


	class TOCMesh
	{
		public:
			std::string			Name;

			bool				Constant = false;

			uint64				MaxVertices = 0;
			uint64				MaxSurfaces = 0;

			PositionFormat		PositionFormat_ = PositionFormat::Full;
			NormalFormat		NormalFormat_ = NormalFormat::Full;
			TangentFormat		TangentFormat_ = TangentFormat::None;
			VelocityFormat		VelocityFormat_ = VelocityFormat::Full;
			TexCoordFormat		TexCoordFormat_ = TexCoordFormat::Full;
			ColorFormat			ColorFormat_ = ColorFormat::Byte;

	};

	enum class ImageFormat
	{
		RGBA8 = 0,
		DXT1,
		DXT3,
		DXT5
	};

	class TOCImageSequence
	{
		public:
			std::string Name;

			ImageFormat	Format;

			uint32		Width;
			uint32		Height;
			uint32		MipMapCount;

			bool		Constant = false;

	};

	class TOCFrameMeshSection
	{
		public:

			unsigned int VertexStart = 0;		// offset in the vertex buffer
			unsigned int IndexStart = 0;		// offset in the index buffer
			unsigned int NumSurfaces = 0;
			unsigned int MinVertexIndex = 0;
			unsigned int MaxVertexIndex = 0;

	};

	class TOCFrameMesh
	{
		public:

			uint32 Vertices = 0;
			uint32 Surfaces = 0;

			int32 SeekIndices = 0;
			uint32 SizeIndices = 0;
		
			int32 SeekPositions = 0;
			uint32 SizePositions = 0;
			Kimura::Vector3		PositionQuantizationCenter;
			Kimura::Vector3		PositionQuantizationExtents;

			int32 SeekNormals = 0;
			uint32 SizeNormals = 0;

			int32 SeekTangents = 0;
			uint32 SizeTangents = 0;

			int32 SeekVelocities = 0;
			uint32 SizeVelocities = 0;
			Kimura::Vector3		VelocityQuantizationCenter;
			Kimura::Vector3		VelocityQuantizationExtents;


			static_assert(MaxTextureCoords == 4, "Maximum texcoord count changed");
			int32 SeekTexCoords[MaxTextureCoords] = { 0, 0, 0, 0 };
			uint32 SizeTexCoords[MaxTextureCoords] = { 0, 0, 0, 0 };

			static_assert(MaxColorChannels == 2, "Maximum color count changed");
			int32 SeekColors[MaxColorChannels] = { 0, 0 };
			uint32 SizeColors[MaxColorChannels] = { 0, 0 };
			Kimura::Vector4		ColorQuantizationExtents[MaxColorChannels];

 			Kimura::Vector3		BoundingCenter;
			Kimura::Vector3		BoundingSize;

			bool DependsOnPreviousFrame = false;
			uint32 FrameIndexDependency = 0;

			std::vector<TOCFrameMeshSection>	Sections;


	};

	class TOCMipmap
	{
		public:
			uint32		Width = 0;
			uint32		Height = 0;
			uint32		RowPitch = 0;
			uint32		SlicePitch = 0;

			int32		SeekPosition = 0;
			uint32		Size = 0;
	};

	class TOCFrameImage
	{
		static_assert(MaxMipmaps == 8, "Maximum mipmaps changed");
	
		public:
			uint32			NumMipmaps = 0;
			TOCMipmap		Mipmaps[MaxMipmaps];

	};

	class TOCFrame
	{
		public:
			
			inline bool IsDependantOnPreviousFrame()
			{
				return this->DependsOnPreviousFrame;
			}

			uint64	FilePosition = 0;
			uint64	BufferSize = 0;

			bool DependsOnPreviousFrame = false;
			uint32 FrameIndexDependency = 0;

			std::vector<TOCFrameMesh>		Meshes;
			std::vector<TOCFrameImage>		Images;

	};

	class TableOfContent
	{
		public:

			Version						Version_;

			std::string					SourceFile;
			std::string					CreationDate;

			float						TimePerFrame = 1.0f / 30.0f;
			float						FrameRate = 30.0f;

			bool						Force16BitIndices = 0;

			// info on each mesh present in the document
			std::vector<TOCMesh>			Meshes;

			// info on each image sequence present in the document
			std::vector<TOCImageSequence>	ImageSequences;		

			// info on each single frame present in the document
			std::vector<TOCFrame>			Frames;

	};

	class FrameMesh
	{
		public:

			// 
			uint32					Vertices = 0;
			uint32					Surfaces = 0;
		
			// bounds
			Vector3					BoundingCenter;
			Vector3					BoundingSize;

			// sections
			std::vector<TOCFrameMeshSection>	Sections;

			// indices
			const uint32*			IndicesU32 = nullptr;
			const uint16*			IndicesU16 = nullptr;

			// positions
			const Vector3*			PositionsF32 = nullptr;
			const int16*			PositionsI16 = nullptr;

			Vector3					PositionQuantizationCenter;
			Vector3					PositionQuantizationExtents;

			// normals
			const Vector3*			NormalsF32 = nullptr;
			const int16*			NormalsI16 = nullptr;
			const int8*				NormalsI8 = nullptr;

			// tangents
			const Vector4*			TangentsF32 = nullptr;
			const int16*			TangentsI16 = nullptr;
			const int8*				TangentsI8 = nullptr;

			// velocities
			const Vector3*			VelocitiesF32 = nullptr;
			const int16*			VelocitiesI16 = nullptr;
			const int8*				VelocitiesI8 = nullptr;

			Vector3					VelocityQuantizationCenter;
			Vector3					VelocityQuantizationExtents;

			// texture coords
			const float*			TexCoordsF32[MaxTextureCoords] = { nullptr, nullptr, nullptr, nullptr };
			const uint16*			TexCoordsU16[MaxTextureCoords] = { nullptr, nullptr, nullptr, nullptr };

			// colors
			const float*			ColorsF32[MaxColorChannels] = { nullptr, nullptr };
			const uint16*			ColorsU16[MaxColorChannels] = { nullptr, nullptr };
			const uint8*			ColorsU8[MaxColorChannels] = { nullptr, nullptr };
			Vector4					ColorQuantizationExtents[MaxColorChannels];


	};

	struct FrameImageMipmap
	{
		const void* Data = nullptr;
		uint32 Size;
	};

	class FrameImage
	{
		public:

			uint32			 NumMipmaps = 0;
			FrameImageMipmap Mipmaps[MaxMipmaps];
	};


	class Frame : public IFrame
	{
		public:
			
			virtual ~Frame() {}

			virtual uint32			GetNumVertices(uint32 InMeshIndex) override;
			virtual uint32			GetNumSurfaces(uint32 InMeshIndex) override;

			virtual void			GetSections(uint32 iMeshIndex, std::vector<MeshSection>& OutSections) override;

			virtual const uint16*	GetIndicesU16(uint32 InMeshIndex) override;
			virtual const uint32*	GetIndicesU32(uint32 InMeshIndex) override;

			virtual const Vector3*	GetPositionsF32(uint32 InMeshIndex) override;
			virtual const int16*	GetPositionsI16(uint32 InMeshIndex) override;
			virtual Vector3			GetPositionQuantizationCenter(uint32 InMeshIndex) override;
			virtual Vector3			GetPositionQuantizationExtents(uint32 InMeshIndex) override;

			virtual const Vector3*	GetNormalsF32(uint32 InMeshIndex) override;
			virtual const int16*	GetNormalsI16(uint32 InMeshIndex) override;
			virtual const int8*		GetNormalsI8(uint32 InMeshIndex) override;

			virtual const Vector4*	GetTangentsF32(uint32 InMeshIndex) override;
			virtual const int16*	GetTangentsI16(uint32 InMeshIndex) override;
			virtual const int8*		GetTangentsI8(uint32 InMeshIndex) override;


			virtual const Vector3*	GetVelocitiesF32(uint32 InMeshIndex) override;
			virtual const int16*	GetVelocitiesI16(uint32 InMeshIndex) override;
			virtual const int8*		GetVelocitiesI8(uint32 InMeshIndex) override;
			virtual Vector3			GetVelocityQuantizationCenter(uint32 InMeshIndex) override;
			virtual Vector3			GetVelocityQuantizationExtents(uint32 InMeshIndex) override;

			virtual const Vector2*	GetTexCoordsF32(uint32 InMeshIndex, uint32 iTexCoord) override;
			virtual const uint16*	GetTexCoordsU16(uint32 InMeshIndex, uint32 iTexCoord) override;

			virtual const Vector4*	GetColorsF32(uint32 InMeshIndex, uint32 iColor) override;
			virtual const uint16*	GetColorsU16(uint32 InMeshIndex, uint32 iColor) override;
			virtual const uint8*	GetColorsU8(uint32 InMeshIndex, uint32 iColor) override;
			virtual Vector4			GetColorQuantizationExtents(uint32 InMeshIndex, uint32 InColorIndex) override;

			virtual void			GetBounds(uint32 InMeshIndex, Vector3& OutCenter, Vector3& OutSize) override;

			virtual bool			GetImageData(uint32 InImageIndex, uint32 InMipmap, const void** OutData, uint32& OutSize) override;


			std::vector<byte>		Buffer;

			std::vector<FrameMesh>	Meshes;
			std::vector<FrameImage>	Images;

			// whenever a frame is dependent on a previous frame, we keep a reference to it 
			// to keep it alive
			std::vector<std::shared_ptr<Frame>>	FrameDependencies;
	};



	class Player : public IPlayer
	{
		public:

			Player(const std::string& InPath, const PlayerOptions& InOptions);
			virtual ~Player();

			virtual PlayerStatus GetStatus() override;

			virtual void GetFailStatusMessage(std::string& OutMessage) override;

			virtual std::string GetFileVersion() override;

			virtual bool RetrievePlaybackInformation(PlaybackInformation& OutInfo) override;

			virtual uint32 GetNumFrames() override;
			virtual int GetBufferedFrameCount() override;
			virtual std::shared_ptr<IFrame>	GetFrameAt(uint32 iFrame, bool InForceWait) override;
			virtual std::shared_ptr<IFrame>	GetConstantFrame() override;

			virtual bool	IsForcing16BitIndices() override;


			virtual void CollectStats(PlayerStats& OutStats) override;


		protected:

			void Failure(std::string InErrorMessage);

			void ThreadExecute();

			void Stop(bool InWaitToComplete);

			bool ReadTOC();

			bool BufferNextFrame();
			void LoadFrameAt(uint32 iFrame);

			template<typename T>
			uint32 Read(T& Out, uint32 InCount = 1);
			uint32 Read(std::string& s);


			std::string		InputFilePath;
			PlayerOptions	Options;

			PlayerStatus	Status = PlayerStatus::Initializing;

			std::string		ErrorMessage;

			TableOfContent	TOC;

			std::thread*				Thread;
			std::mutex					ThreadEventMutex;
			std::condition_variable		WakeUpBufferThreadEvent;

			std::mutex					WaitForFrameBufferedMutex;
			std::condition_variable		WaitForFrameBufferedEvent;
			bool						StopThreadExecution = false;

#if defined(KIMURA_UNREAL)
			class IFileHandle*			UEFileHandle = nullptr;
#elif defined(KIMURA_WINDOWS)
			int							FileHandle = -1;
#else
			std::ifstream				InputFile;
#endif

			std::mutex								FrameAccessMutex;

			uint64									FrameDataFilePosition = 0;

			/* Frames located between FullyBufferedFramesStart and (FullyBufferedFramesStart + FullyBufferedFramesCount ) are fully loaded */
			uint32									FullyBufferedFramesStart = 0;
			uint32									FullyBufferedFramesCount = 0;
			std::vector<std::shared_ptr<Frame>>		Frames;

			std::shared_ptr<Frame>					FirstFrame = nullptr;


			std::mutex								ProfilingMutex;

			PlayerStats		Profiling;
			PlayerStats		StoredProfiling;
			std::chrono::time_point<std::chrono::high_resolution_clock>	NextStatsCollection;



	};


	class ScopedTime
	{
		public:
			ScopedTime() :
				start(std::chrono::steady_clock::now())
			{
			}

			inline double Duration()
			{
				std::chrono::duration<double> time_span = std::chrono::duration_cast<std::chrono::duration<double>>(std::chrono::steady_clock::now() - start);
				return time_span.count();
			}

			std::chrono::steady_clock::time_point start;
	};

}