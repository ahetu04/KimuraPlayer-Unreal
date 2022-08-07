//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "Player.h"

#if defined(KIMURA_UNREAL)

	// files
	#include "GenericPlatform/GenericPlatformFile.h"
	#include "Misc/Paths.h"

#elif defined(KIMURA_WINDOWS)

	#include <io.h>
 	#include <fcntl.h>

#else

	// default, use fstream. Already included in player.h
	#include <fstream>

#endif

const Kimura::Vector2 Kimura::Vector2::ZeroVector(0.0f, 0.0f);
const Kimura::Vector3 Kimura::Vector3::ZeroVector(0.0f, 0.0f, 0.0f);
const Kimura::Vector4 Kimura::Vector4::ZeroVector(0.0f, 0.0f, 0.0f, 0.0f);

const Kimura::Vector3 Kimura::Vector3::OneVector(1.0f, 1.0f, 1.0f);
const Kimura::Vector4 Kimura::Vector4::OneVector(1.0f, 1.0f, 1.0f, 1.0f);


//-----------------------------------------------------------------------------
// Kimura::CreatePlayer
//-----------------------------------------------------------------------------
std::shared_ptr<Kimura::IPlayer>	Kimura::CreatePlayer(const std::string& InPath, const Kimura::PlayerOptions& InOptions)
{
	return std::make_shared<Player>(InPath, InOptions);
}


//-----------------------------------------------------------------------------
// Kimura::GetVersion
//-----------------------------------------------------------------------------
std::string Kimura::GetVersion()
{
	return Kimura::Version().ToString();
}

//-----------------------------------------------------------------------------
// Player::Player
//-----------------------------------------------------------------------------
Kimura::Player::Player(const std::string& InPath, const Kimura::PlayerOptions& InOptions)
	:
	Options(InOptions)
{
	this->InputFilePath = InPath;

	this->Thread = new std::thread([this](){this->ThreadExecute();});
}


//-----------------------------------------------------------------------------
// Player::~Player
//-----------------------------------------------------------------------------
Kimura::Player::~Player()
{
	this->Stop(true);
}


//-----------------------------------------------------------------------------
// Player::GetStatus
//-----------------------------------------------------------------------------
Kimura::PlayerStatus Kimura::Player::GetStatus()
{
	return this->Status;
}


//-----------------------------------------------------------------------------
// Player::GetFailStatusMessage
//-----------------------------------------------------------------------------
void Kimura::Player::GetFailStatusMessage(std::string& OutMessage)
{
	OutMessage = this->ErrorMessage;
}


//-----------------------------------------------------------------------------
// Frame::GetFileVersion
//-----------------------------------------------------------------------------
std::string Kimura::Player::GetFileVersion()
{
	if (this->Status == PlayerStatus::Ready)
	{
		return this->TOC.Version_.ToString();
	}

	return "";
}


//-----------------------------------------------------------------------------
// Player::Stop
//-----------------------------------------------------------------------------
void Kimura::Player::Stop(bool InWaitToComplete)
{
	KIMURA_TRACE("Kimura::Player::Stop");

	this->StopThreadExecution = true;
	this->WakeUpBufferThreadEvent.notify_one();
	if (InWaitToComplete)
	{
		this->Thread->join();
		delete this->Thread;
	}

}


//-----------------------------------------------------------------------------
// Player::Failure
//-----------------------------------------------------------------------------
void Kimura::Player::Failure(std::string InErrorMessage)
{
	this->Status = PlayerStatus::Failed;
	this->ErrorMessage = InErrorMessage;

	// stop execution of the running thread
	this->Stop(false);
}


//-----------------------------------------------------------------------------
// Player::ReadTOC
//-----------------------------------------------------------------------------
bool Kimura::Player::ReadTOC()
{
	KIMURA_TRACE("Kimura::Player::ReadTOC");

	this->Read<Version>(this->TOC.Version_);

	if (!this->TOC.Version_.CompatibleWith(Version()))
	{
		this->Failure("Incompatible version");
		return false;
	}

	this->Read(this->TOC.SourceFile);
	this->Read(this->TOC.CreationDate);

	this->Read<float>(this->TOC.TimePerFrame);
	this->Read<float>(this->TOC.FrameRate);

	uint32 b16BitIndices = 0;
	this->Read<uint32>(b16BitIndices);
	this->TOC.Force16BitIndices = b16BitIndices ? true : false;

	// meshes
	{
		uint32 numMeshes = 0;
		this->Read<uint32>(numMeshes);

		this->TOC.Meshes.resize(numMeshes);

		for (uint32 iMesh = 0; iMesh < numMeshes; iMesh++)
		{
			TOCMesh& m = this->TOC.Meshes[iMesh];

			this->Read(m.Name);

			this->Read<bool>(m.Constant);
			this->Read<uint64>(m.MaxVertices);
			this->Read<uint64>(m.MaxSurfaces);
			this->Read<PositionFormat>(m.PositionFormat_);
			this->Read<NormalFormat>(m.NormalFormat_);
			this->Read<TangentFormat>(m.TangentFormat_);
			this->Read<VelocityFormat>(m.VelocityFormat_);
			this->Read<TexCoordFormat>(m.TexCoordFormat_);
			this->Read<ColorFormat>(m.ColorFormat_);

		}
	}



	// image sequences
	{
		uint32 numImageSequences = 0;		
		this->Read<uint32>(numImageSequences);

		this->TOC.ImageSequences.resize(numImageSequences);

		for (uint32 iIS = 0; iIS < numImageSequences; iIS++)
		{
			TOCImageSequence& IS = this->TOC.ImageSequences[iIS];

			this->Read(IS.Name);
			this->Read<ImageFormat>(IS.Format);

			this->Read<bool>(IS.Constant);
			this->Read<uint32>(IS.Width);
			this->Read<uint32>(IS.Height);
			this->Read<uint32>(IS.MipMapCount);


		}

	}


	// frames
	{

		uint32 numFrames = 0;
		this->Read<uint32>(numFrames);

		this->TOC.Frames.resize(numFrames);

		this->Frames.resize(numFrames);

		for (uint32 iFrame = 0; iFrame < numFrames; iFrame++)
		{
			TOCFrame& f = this->TOC.Frames[iFrame];

			this->Read<uint64>(f.FilePosition);
			this->Read<uint64>(f.BufferSize);

			f.Meshes.resize(this->TOC.Meshes.size());

			for (uint32 iMesh = 0; iMesh < this->TOC.Meshes.size(); iMesh++)
			{
				TOCFrameMesh& fm = f.Meshes[iMesh];

				this->Read<uint32>(fm.Vertices);
				this->Read<uint32>(fm.Surfaces);

				// read the mesh's sections
				uint32 numSections = 0;
				this->Read<uint32>(numSections);

				fm.Sections.resize(numSections);
				for (TOCFrameMeshSection& s : fm.Sections)
				{

					this->Read<uint32>(s.VertexStart);
					this->Read<uint32>(s.IndexStart);
					this->Read<uint32>(s.NumSurfaces);
					this->Read<uint32>(s.MinVertexIndex);
					this->Read<uint32>(s.MaxVertexIndex);

				}

				this->Read<int32>(fm.SeekIndices);
				this->Read<uint32>(fm.SizeIndices);

				this->Read<int32>(fm.SeekPositions);
				this->Read<uint32>(fm.SizePositions);
				this->Read<Kimura::Vector3>(fm.PositionQuantizationCenter);
				this->Read<Kimura::Vector3>(fm.PositionQuantizationExtents);

				this->Read<int32>(fm.SeekNormals);
				this->Read<uint32>(fm.SizeNormals);

				this->Read<int32>(fm.SeekTangents);
				this->Read<uint32>(fm.SizeTangents);

				this->Read<int32>(fm.SeekVelocities);
				this->Read<uint32>(fm.SizeVelocities);
				this->Read<Kimura::Vector3>(fm.VelocityQuantizationCenter);
				this->Read<Kimura::Vector3>(fm.VelocityQuantizationExtents);

				this->Read<int32>(fm.SeekTexCoords[0], MaxTextureCoords);
				this->Read<uint32>(fm.SizeTexCoords[0], MaxTextureCoords);

				this->Read<int32>(fm.SeekColors[0], MaxColorChannels);
				this->Read<uint32>(fm.SizeColors[0], MaxColorChannels);
				this->Read<Vector4>(fm.ColorQuantizationExtents[0], MaxColorChannels);

				this->Read<Kimura::Vector3>(fm.BoundingCenter);
				this->Read<Kimura::Vector3>(fm.BoundingSize);

				// determine dependency on previous frames
				{
					if (fm.SeekIndices == -1 ||
						fm.SeekPositions == -1 ||
						fm.SeekNormals == -1 ||
						fm.SeekTangents == -1 ||
						fm.SeekVelocities == -1)
					{
						fm.DependsOnPreviousFrame = true;
					}

					for (uint32 iTexCoord = 0; iTexCoord < MaxTextureCoords; iTexCoord++)
					{
						if (fm.SeekTexCoords[iTexCoord] == -1)
						{
							fm.DependsOnPreviousFrame = true;
						}
					}

					for (uint32 iColor = 0; iColor < MaxColorChannels; iColor++)
					{
						if (fm.SeekColors[iColor] == -1)
						{
							fm.DependsOnPreviousFrame = true;
						}
					}

					if (fm.DependsOnPreviousFrame)
					{

						uint32 iBackFrame = iFrame;

						bool bFoundIndices = false;
						bool bFoundPositions = false;
						bool bFoundNormals = false;
						bool bFoundTangents = false;
						bool bFoundVelocities = false;
						bool bFoundTextCoords0 = false;
						bool bFoundTextCoords1 = false;
						bool bFoundTextCoords2 = false;
						bool bFoundTextCoords3 = false;
						bool bFoundColor0 = false;
						bool bFoundColor1 = false;

						// backtrack until we find all the components needed for this mesh on this frame
						while (iBackFrame > 0)
						{

							TOCFrameMesh& previousFrameMesh = this->TOC.Frames[iBackFrame].Meshes[iMesh];

							if (previousFrameMesh.SeekIndices != -1)
								bFoundIndices = true;

							if (previousFrameMesh.SeekPositions != -1)
								bFoundPositions = true;

							if (previousFrameMesh.SeekNormals != -1)
								bFoundNormals = true;

							if (previousFrameMesh.SeekTangents != -1)
								bFoundTangents = true;

							if (previousFrameMesh.SeekVelocities != -1)
								bFoundVelocities = true;

							static_assert(MaxTextureCoords == 4, "Maximum texcoord count changed");
							if (previousFrameMesh.SeekTexCoords[0] != -1) { bFoundTextCoords0 = true; }
							if (previousFrameMesh.SeekTexCoords[1] != -1) { bFoundTextCoords1 = true; }
							if (previousFrameMesh.SeekTexCoords[2] != -1) { bFoundTextCoords2 = true; }
							if (previousFrameMesh.SeekTexCoords[3] != -1) { bFoundTextCoords3 = true; }

							static_assert(MaxColorChannels == 2, "Maximum color count changed");
							if (previousFrameMesh.SeekColors[0] != -1) { bFoundColor0 = true; }
							if (previousFrameMesh.SeekColors[1] != -1) { bFoundColor1 = true; }

							// do we have all the data we need at this point?
							if (bFoundIndices &&
								bFoundPositions &&
								bFoundNormals &&
								bFoundTangents &&
								bFoundVelocities &&
								bFoundTextCoords0 &&
								bFoundTextCoords1 &&
								bFoundTextCoords2 &&
								bFoundTextCoords3 &&
								bFoundColor0 &&
								bFoundColor1)
							{
								break;
							}

							iBackFrame--;
						}

						fm.FrameIndexDependency = iBackFrame;

						// frame containing this mesh must also be updated. 
						f.DependsOnPreviousFrame = true;
						if (f.FrameIndexDependency < fm.FrameIndexDependency)
						{
							f.FrameIndexDependency = fm.FrameIndexDependency;
						}

					}

				}

			}

			// image sequences for this frame... 
			f.Images.resize(this->TOC.ImageSequences.size());
			for (uint32 iIS = 0; iIS < this->TOC.ImageSequences.size(); iIS++)
			{
				TOCFrameImage& fi = f.Images[iIS];

				this->Read<uint32>(fi.NumMipmaps);
				for (uint32 iMipmap = 0; iMipmap < MaxMipmaps; iMipmap++)
				{
					this->Read<uint32>(fi.Mipmaps[iMipmap].Width);
					this->Read<uint32>(fi.Mipmaps[iMipmap].Height);
					this->Read<uint32>(fi.Mipmaps[iMipmap].RowPitch);
					this->Read<uint32>(fi.Mipmaps[iMipmap].SlicePitch);

					this->Read<int32>(fi.Mipmaps[iMipmap].SeekPosition);
					this->Read<uint32>(fi.Mipmaps[iMipmap].Size);

				}

			}

		}

	}


#if defined(KIMURA_UNREAL)
	this->FrameDataFilePosition = this->UEFileHandle->Tell();
#elif defined(KIMURA_WINDOWS)
	// right after the TOC comes the frame data, keep that position offset
	this->FrameDataFilePosition = _tell(this->FileHandle);
#else
	this->FrameDataFilePosition = this->InputFile.tellg();
#endif

	return true;

}


//-----------------------------------------------------------------------------
// Player::Read
//-----------------------------------------------------------------------------
template<typename T>
Kimura::uint32 Kimura::Player::Read(T& Out, uint32 InCount /*= 1*/)
{

#if defined(KIMURA_UNREAL)

	this->UEFileHandle->Read((uint8*)&Out, sizeof(Out) * InCount);
	return sizeof(Out) * InCount;

#elif defined(KIMURA_WINDOWS)

	int bytesRead = _read(this->FileHandle, (void*)&Out, sizeof(Out) * InCount);
	return bytesRead;

#else
	uint64 posBefore = this->InputFile.tellg();

	this->InputFile.read((char*)&Out, sizeof(Out) * InCount);

	uint64 posAfter = this->InputFile.tellg();

	return (uint32)(posAfter - posBefore);

#endif

}


//-----------------------------------------------------------------------------
// Player::Read
//-----------------------------------------------------------------------------
Kimura::uint32 Kimura::Player::Read(std::string& s)
{

#if defined(KIMURA_UNREAL)

	int size_ = 0;
	bool bRead = this->UEFileHandle->Read((uint8*)&size_, sizeof(size_));

	if (size_ > 0)
	{
		char str[1024];
		this->UEFileHandle->Read((uint8*)str, size_);
		str[size_] = 0;

		s = str;
	}
	return sizeof(size_) + size_;

#elif defined(KIMURA_WINDOWS)

	int bytesRead = 0;

	int size = 0;
	bytesRead = _read(this->FileHandle, (void*)&size, sizeof(size));
	if (size > 0)
	{
		char str[1024];
		bytesRead += _read(this->FileHandle, str, size);
		str[size] = 0;

		s = str;
	}
	return bytesRead;

#else
	uint64 posBefore = this->InputFile.tellg();

	int size = 0;
	this->InputFile.read((char*)&size, sizeof(size));
	if (size > 0)
	{
		char str[1024];
		this->InputFile.read(str, sizeof(char) * size);
		str[size] = 0;

		s = str;
	}

	uint64 posAfter = this->InputFile.tellg();

	return (uint32)(posAfter - posBefore);
#endif
}


//-----------------------------------------------------------------------------
// Player::ThreadExecute
//-----------------------------------------------------------------------------
void Kimura::Player::ThreadExecute()
{
	std::unique_lock<std::mutex> threadLock(this->ThreadEventMutex);


	// open the file
	{

#if defined(KIMURA_UNREAL)

		IPlatformFile& PlatformFile = FPlatformFileManager::Get().GetPlatformFile();

		FString sourceFilename(InputFilePath.c_str());


		// check if the file exists
		if (!PlatformFile.FileExists(*sourceFilename))
		{
			return this->Failure("Failed to open the input file: ");
		}

		this->UEFileHandle = PlatformFile.OpenRead(*sourceFilename);

		if (this->UEFileHandle == nullptr)
		{
			return this->Failure("Failed to open the input file");
		}


#elif defined(KIMURA_WINDOWS)
		if (_sopen_s(&this->FileHandle, this->InputFilePath.c_str(), _O_RDONLY | _O_BINARY, _SH_DENYNO, 0))
		{
			return this->Failure("Failed to open the input file: ");
		}
#else

/* requires c++ 17

		// validate access to alembic document
		if (!std::filesystem::exists(this->InputFilePath))
		{
			return this->Failure("Couldn't find input file");
		}
*/

		this->InputFile.open(this->InputFilePath, std::ios::in | std::ios::binary);
		if (!this->InputFile.is_open())
		{
			return this->Failure("Failed to open the input file: ");
		}
#endif
	}

	// read the table of content from the file
	{
		if (!this->ReadTOC())
		{
			// failed
			return;
		}

		// success! ready to start loading frames
		this->Status = PlayerStatus::Ready;
	}

	// adjust buffering sizes
	if (this->Options.BufferEntirePlayback || this->Options.PreBufferingSize > (uint32)this->TOC.Frames.size())
	{
		this->Options.PreBufferingSize = (uint32)this->TOC.Frames.size();
	}

	// if any of the image sequences stored in the file is flagged as constant, we should read that frame and store it 
	// immediately. 
	bool bBufferFirstFrame = false;
	for (const auto& imageSequence : this->TOC.ImageSequences)
	{
		if (imageSequence.Constant)
		{
			bBufferFirstFrame = true;
			break;
		}
	}
	if (bBufferFirstFrame)
	{
		this->LoadFrameAt(0);
		this->FirstFrame = this->Frames[0];
	}

	while (!this->StopThreadExecution)
	{
		if (!this->BufferNextFrame())
		{
			// when buffer is full or contains sufficient frames, pause the thread
			this->WakeUpBufferThreadEvent.wait(threadLock);
		}	
	}

#if defined(KIMURA_UNREAL)
	if (this->UEFileHandle != nullptr)
	{
		delete this->UEFileHandle;
		this->UEFileHandle = nullptr;
	}
#elif defined(KIMURA_WINDOWS)
	_close(this->FileHandle);
	this->FileHandle = -1;
#else
	this->InputFile.close();
#endif
}


//-----------------------------------------------------------------------------
// Player::BufferNextFrame
//-----------------------------------------------------------------------------
bool Kimura::Player::BufferNextFrame()
{
	// find the index of the next frame to buffer
	uint32 indexOfFrameToLoad = 0;	
	{
		std::unique_lock<std::mutex> threadLock(this->FrameAccessMutex);

		if (this->FullyBufferedFramesCount >= this->Options.PreBufferingSize)
		{
			// sufficient number of frames are already buffered. There's no need to buffer another frame at this time. 
			return false;
		}

		indexOfFrameToLoad = this->FullyBufferedFramesStart + this->FullyBufferedFramesCount;
		
		if (this->Options.Loop)
		{
			// wrap around
			indexOfFrameToLoad %= (uint32)this->TOC.Frames.size();
		}
		else if (indexOfFrameToLoad >= (uint32)this->TOC.Frames.size())
		{
			// Reached the end of the playback. No more frames to buffer
			return false;
		}
	}

	TOCFrame& tocFrame = this->TOC.Frames[indexOfFrameToLoad];

	// get ref to previous frame
	std::shared_ptr<Frame> previousFrame = indexOfFrameToLoad > 0 ? this->Frames[indexOfFrameToLoad - 1] : nullptr;

	// if previous frame is required but isn't loaded, we need to backtrack a bit
	if (previousFrame == nullptr && tocFrame.IsDependantOnPreviousFrame())
	{
		for (uint32 i = tocFrame.FrameIndexDependency; i < indexOfFrameToLoad; i++)
		{	
			// load as many frames as needed. However!! These frames cannot be considered as fully loaded and 
			// buffered (because their very own dependencies might not be met)
			this->LoadFrameAt(i);
		}
	}

	// 
// 	if (indexOfFrameToLoad == 0 && this->FirstFrame != nullptr)
// 	{
// 		this->Frames[indexOfFrameToLoad] = this->FirstFrame;
// 	}
// 	else
	{
		this->LoadFrameAt(indexOfFrameToLoad);
	}

	{
		std::unique_lock<std::mutex> threadLock(this->FrameAccessMutex);

		uint32 indexOfFrameWeReallyWantLoadedNext = (this->FullyBufferedFramesStart + FullyBufferedFramesCount ) % (uint32)this->TOC.Frames.size();

		if (indexOfFrameToLoad == indexOfFrameWeReallyWantLoadedNext)
		{
			this->FullyBufferedFramesCount++;
		}
		else
		{
			// adjust memory footprint
			if (this->Frames[indexOfFrameToLoad] != nullptr)
			{
				this->Profiling.MemoryUsageForFrames -= (uint64)this->Frames[indexOfFrameToLoad]->Buffer.size();
			}

			this->Frames[indexOfFrameToLoad] = nullptr;
		}
	}

	// 
	{
		this->WaitForFrameBufferedEvent.notify_one();
	}

	return true;

}


//-----------------------------------------------------------------------------
// Player::LoadFrameAt
//-----------------------------------------------------------------------------
void Kimura::Player::LoadFrameAt(uint32 iFrame)
{
	KIMURA_TRACE("Kimura::Player::LoadFrameAt");

	TOCFrame& tocFrame = this->TOC.Frames[iFrame];

	// get ref to previous frame (if any or necessary)
	std::shared_ptr<Frame> previousFrame = iFrame > 0 ? this->Frames[iFrame - 1] : nullptr;

	std::shared_ptr<Frame> newFrame = std::make_shared<Frame>();
	newFrame->FrameIndex = iFrame;

	// keep references to other frames alive as long as this frame is
	if (tocFrame.IsDependantOnPreviousFrame())
	{
		uint32 numFramesDependentOn = iFrame - tocFrame.FrameIndexDependency;

		newFrame->FrameDependencies.reserve(numFramesDependentOn);
		for (uint32 i = tocFrame.FrameIndexDependency; i < iFrame; i++)
		{
			newFrame->FrameDependencies.push_back(this->Frames[i]);
		}

	}

	// allocate a buffer large enough to contain the entire frame
	newFrame->Buffer.resize(tocFrame.BufferSize);
	this->Profiling.BytesReadInLastSecond += tocFrame.BufferSize;
	this->Profiling.MemoryUsageForFrames += tocFrame.BufferSize;

	{
		ScopedTime s;

		KIMURA_TRACE("Kimura::Player::LoadFrameAt::read");

		// seek and read the frame's content into the buffer

		uint64 positionOfFrameInFile = this->FrameDataFilePosition + tocFrame.FilePosition;

#if defined(KIMURA_UNREAL)

		bool bSeek = this->UEFileHandle->Seek(positionOfFrameInFile);
		if (!bSeek)
		{
			return this->Failure("Failed to seek in file");
		}

		bool bRead = this->UEFileHandle->Read((uint8*)newFrame->Buffer.data(), tocFrame.BufferSize);
		if (!bRead)
		{
			return this->Failure("Failed to read frame data from file");
		}

#elif defined(KIMURA_WINDOWS) 


		Kimura::uint64 newOffset = _lseeki64(this->FileHandle, positionOfFrameInFile, SEEK_SET);

		int bytesRead = _read(this->FileHandle, (void*)newFrame->Buffer.data(), (unsigned int)tocFrame.BufferSize);

#else
		this->InputFile.seekg(positionOfFrameInFile);
		this->InputFile.read((char*)newFrame->Buffer.data(), tocFrame.BufferSize);
#endif

		this->Profiling.TotalTimeSpentOnReadingFromDiskInLastSecond += s.Duration();

	}

	// allocate mesh instances for this frame
	newFrame->Meshes.resize(tocFrame.Meshes.size());

	byte* bufferAddress = newFrame->Buffer.data();

	ScopedTime timeProcessingFrame;


	for (uint32 iMesh = 0; iMesh < (uint32)newFrame->Meshes.size(); iMesh++)
	{
		FrameMesh& frameMesh = newFrame->Meshes[iMesh];

		TOCMesh& tocMesh = this->TOC.Meshes[iMesh];
		TOCFrameMesh& tocFrameMesh = tocFrame.Meshes[iMesh];

		frameMesh.Vertices = tocFrameMesh.Vertices;
		frameMesh.Surfaces = tocFrameMesh.Surfaces;

		// a simple copy of the frame mesh's sections
		frameMesh.Sections = tocFrameMesh.Sections;

		frameMesh.PositionQuantizationCenter = tocFrameMesh.PositionQuantizationCenter;
		frameMesh.PositionQuantizationExtents = tocFrameMesh.PositionQuantizationExtents;

		frameMesh.VelocityQuantizationCenter = tocFrameMesh.VelocityQuantizationCenter;
		frameMesh.VelocityQuantizationExtents = tocFrameMesh.VelocityQuantizationExtents;

		frameMesh.ColorQuantizationExtents[0] = tocFrameMesh.ColorQuantizationExtents[0];
		frameMesh.ColorQuantizationExtents[1] = tocFrameMesh.ColorQuantizationExtents[1];

		// copy bounds info
		frameMesh.BoundingCenter = tocFrameMesh.BoundingCenter;
		frameMesh.BoundingSize = tocFrameMesh.BoundingSize;

		// number of vertices stored in this frame determines the type of index buffer used
		if (frameMesh.Vertices <= 0xfffe || this->TOC.Force16BitIndices)
		{
			// 16bit indices

			if (tocFrameMesh.SeekIndices == -1)
			{
				// re-use previous frame's indices
				if (previousFrame != nullptr)
				{
					frameMesh.IndicesU16 = previousFrame->Meshes[iMesh].IndicesU16;
				}
			}
			else if (tocFrameMesh.SizeIndices > 0)
			{
				frameMesh.IndicesU16 = (uint16*)&bufferAddress[tocFrameMesh.SeekIndices];
			}
		}
		else
		{
			// 32bit indices

			if (tocFrameMesh.SeekIndices == -1)
			{
				// re-use previous frame's indices
				if (previousFrame != nullptr)
				{
					frameMesh.IndicesU32 = previousFrame->Meshes[iMesh].IndicesU32;
				}
			}
			else if (tocFrameMesh.SizeIndices > 0)
			{
				frameMesh.IndicesU32 = (uint32*)&bufferAddress[tocFrameMesh.SeekIndices];
			}
		}

		// position
		switch (tocMesh.PositionFormat_)
		{
			case PositionFormat::Full:
			{
				if (tocFrameMesh.SeekPositions == -1)
				{
					// re-use previous frame's positions
					if (previousFrame != nullptr)
					{
						frameMesh.PositionsF32 = previousFrame->Meshes[iMesh].PositionsF32;
					}
				}
				else if (tocFrameMesh.SizePositions > 0)
				{
					frameMesh.PositionsF32 = (Vector3*) &bufferAddress[tocFrameMesh.SeekPositions];
				}

				break;

			}

			case PositionFormat::Half:
			{				
				if (tocFrameMesh.SeekPositions == -1)
				{
					// re-use previous frame's positions
					if (previousFrame != nullptr)
					{
						frameMesh.PositionsI16 = previousFrame->Meshes[iMesh].PositionsI16;
					}
				}
				else if (tocFrameMesh.SizePositions > 0)
				{
					frameMesh.PositionsI16 = (int16*)&bufferAddress[tocFrameMesh.SeekPositions];
				}

				break;
			}
		}

		// normal
		switch (tocMesh.NormalFormat_)
		{
			case NormalFormat::Full:
			{
				if (tocFrameMesh.SeekNormals == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.NormalsF32 = previousFrame->Meshes[iMesh].NormalsF32;
					}
				}
				else if (tocFrameMesh.SizeNormals > 0)
				{
					frameMesh.NormalsF32 = (Vector3*) &bufferAddress[tocFrameMesh.SeekNormals];
				}

				break;
			}

			case NormalFormat::Half:
			{
				if (tocFrameMesh.SeekNormals == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.NormalsI16 = previousFrame->Meshes[iMesh].NormalsI16;
					}
				}
				else if (tocFrameMesh.SizeNormals > 0)
				{
					frameMesh.NormalsI16 = (int16*)&bufferAddress[tocFrameMesh.SeekNormals];
				}

				break;
			}

			case NormalFormat::Byte:
			{
				if (tocFrameMesh.SeekNormals == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.NormalsI8 = previousFrame->Meshes[iMesh].NormalsI8;
					}
				}
				else if (tocFrameMesh.SizeNormals > 0)
				{
					frameMesh.NormalsI8 = (int8*)&bufferAddress[tocFrameMesh.SeekNormals];
				}

				break;
			}

			case NormalFormat::None:
			default:
			{
				break;
			}

		}


		// Tangent
		switch (tocMesh.TangentFormat_)
		{
			case TangentFormat::Full:
			{
				if (tocFrameMesh.SeekTangents == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.TangentsF32 = previousFrame->Meshes[iMesh].TangentsF32;
					}
				}
				else if (tocFrameMesh.SizeTangents > 0)
				{
					frameMesh.TangentsF32 = (Vector4*) &bufferAddress[tocFrameMesh.SeekTangents];
				}

				break;
			}

			case TangentFormat::Half:
			{
				if (tocFrameMesh.SeekTangents == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.TangentsI16 = previousFrame->Meshes[iMesh].TangentsI16;
					}
				}
				else if (tocFrameMesh.SizeTangents > 0)
				{
					frameMesh.TangentsI16 = (int16*)&bufferAddress[tocFrameMesh.SeekTangents];
				}

				break;
			}

			case TangentFormat::Byte:
			{
				if (tocFrameMesh.SeekTangents == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.TangentsI8 = previousFrame->Meshes[iMesh].TangentsI8;
					}
				}
				else if (tocFrameMesh.SizeTangents > 0)
				{
					frameMesh.TangentsI8 = (int8*)&bufferAddress[tocFrameMesh.SeekTangents];
				}

				break;
			}

			case TangentFormat::None:
			default:
			{
				break;
			}

		}



		// velocity
		switch (tocMesh.VelocityFormat_)
		{

			case VelocityFormat::Full:
			{
				if (tocFrameMesh.SeekVelocities == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.VelocitiesF32 = previousFrame->Meshes[iMesh].VelocitiesF32;
					}
				}
				else if (tocFrameMesh.SizeVelocities > 0)
				{
					frameMesh.VelocitiesF32 = (Vector3*)&bufferAddress[tocFrameMesh.SeekVelocities];
				}

				break;
			}

			case VelocityFormat::Half:
			{
				if (tocFrameMesh.SeekVelocities == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.VelocitiesI16 = previousFrame->Meshes[iMesh].VelocitiesI16;
					}
				}
				else if (tocFrameMesh.SizeVelocities > 0)
				{
					frameMesh.VelocitiesI16 = (int16*)&bufferAddress[tocFrameMesh.SeekVelocities];
				}

				break;
			}

			case VelocityFormat::Byte:
			{
				if (tocFrameMesh.SeekVelocities == -1)
				{
					if (previousFrame != nullptr)
					{
						frameMesh.VelocitiesI8 = previousFrame->Meshes[iMesh].VelocitiesI8;
					}
				}
				else if (tocFrameMesh.SizeVelocities > 0)
				{
					frameMesh.VelocitiesI8 = (int8*)&bufferAddress[tocFrameMesh.SeekVelocities];
				}

				break;
			}

			case VelocityFormat::None:
			default:
			{
				break;
			}			

		}

		// texture coords
		switch (tocMesh.TexCoordFormat_)
		{
			case TexCoordFormat::Full:
			{
				for (uint32 iTC = 0; iTC < MaxTextureCoords; iTC++)
				{
					if (tocFrameMesh.SeekTexCoords[iTC] == -1)
					{
						if (previousFrame != nullptr)
						{
							frameMesh.TexCoordsF32[iTC] = previousFrame->Meshes[iMesh].TexCoordsF32[iTC];
						}
					}
					else if (tocFrameMesh.SizeTexCoords[iTC] > 0)
					{
						frameMesh.TexCoordsF32[iTC] = (float*)&bufferAddress[tocFrameMesh.SeekTexCoords[iTC]];
					}
				}

				break;
			}

			case TexCoordFormat::Half:
			{
				for (uint32 iTC = 0; iTC < MaxTextureCoords; iTC++)
				{
					if (tocFrameMesh.SeekTexCoords[iTC] == -1)
					{
						if (previousFrame != nullptr)
						{
							frameMesh.TexCoordsU16[iTC] = previousFrame->Meshes[iMesh].TexCoordsU16[iTC];
						}
					}
					else if (tocFrameMesh.SizeTexCoords[iTC] > 0)
					{
						frameMesh.TexCoordsU16[iTC] = (uint16*)&bufferAddress[tocFrameMesh.SeekTexCoords[iTC]];
					}
				}

				break;
			}

			case TexCoordFormat::None:
			default:
			{
				break;
			}			

		}


		switch (tocMesh.ColorFormat_)
		{
			case ColorFormat::Full:
			{
				for (uint32 iCC = 0; iCC < MaxColorChannels; iCC++)
				{
					if (tocFrameMesh.SeekColors[iCC] == -1)
					{
						if (previousFrame != nullptr)
						{
							frameMesh.ColorsF32[iCC] = previousFrame->Meshes[iMesh].ColorsF32[iCC];
						}
					}
					else if (tocFrameMesh.SizeColors[iCC] > 0)
					{
						frameMesh.ColorsF32[iCC] = (float*)&bufferAddress[tocFrameMesh.SeekColors[iCC]];
					}
				}
			
				break;
			}

			case ColorFormat::Half:
			{
				for (uint32 iCC = 0; iCC < MaxColorChannels; iCC++)
				{
					if (tocFrameMesh.SeekColors[iCC] == -1)
					{
						if (previousFrame != nullptr)
						{
							frameMesh.ColorsU16[iCC] = previousFrame->Meshes[iMesh].ColorsU16[iCC];
						}
					}
					else if (tocFrameMesh.SizeColors[iCC] > 0)
					{
						frameMesh.ColorsU16[iCC] = (uint16*)&bufferAddress[tocFrameMesh.SeekColors[iCC]];
					}
				}
				break;
			}

			case ColorFormat::Byte:
			case ColorFormat::ByteHDR:
			{

				for (uint32 iCC = 0; iCC < MaxColorChannels; iCC++)
				{
					if (tocFrameMesh.SeekColors[iCC] == -1)
					{
						if (previousFrame != nullptr)
						{
							frameMesh.ColorsU8[iCC] = previousFrame->Meshes[iMesh].ColorsU8[iCC];
						}
					}
					else if (tocFrameMesh.SizeColors[iCC] > 0)
					{
						frameMesh.ColorsU8[iCC] = (uint8*)&bufferAddress[tocFrameMesh.SeekColors[iCC]];
					}
				}

				break;
			}

			case ColorFormat::None:
			default:
			{
				break;
			}			
		}
	}

	// setup the frame's image sequence data
	newFrame->Images.resize(tocFrame.Images.size());
	for (uint32 iImageSequence = 0; iImageSequence < newFrame->Images.size(); iImageSequence++)
	{
		// copy number of mipmaps used
		newFrame->Images[iImageSequence].NumMipmaps = tocFrame.Images[iImageSequence].NumMipmaps;

		// for each mipmap, store pointer to data + size of data
		TOCMipmap* pTOCMipmap = tocFrame.Images[iImageSequence].Mipmaps;
		FrameImageMipmap* pFrameMipmap = newFrame->Images[iImageSequence].Mipmaps;
		for (uint32 iMipmap = 0; iMipmap < tocFrame.Images[iImageSequence].NumMipmaps; iMipmap++)
		{
			if (pTOCMipmap->SeekPosition == -1)
			{
				if (previousFrame != nullptr)
				{
					pFrameMipmap->Data = previousFrame->Images[iImageSequence].Mipmaps[iMipmap].Data;
					pFrameMipmap->Size = previousFrame->Images[iImageSequence].Mipmaps[iMipmap].Size;
				}
			}
			else
			{
				pFrameMipmap->Data = (void*)&bufferAddress[pTOCMipmap->SeekPosition];
				pFrameMipmap->Size = pTOCMipmap->Size;
			}

			pFrameMipmap++;
			pTOCMipmap++;
		}

	}

	this->Profiling.TotalTimeSpentOnProcessingFramesInLastSecond += timeProcessingFrame.Duration();
	this->Profiling.NumFramesProcessedInLastSecond++;

	// store the frame
	{
		std::unique_lock<std::mutex> threadLock(this->FrameAccessMutex);
		this->Frames[iFrame] = newFrame;
	}

}


//-----------------------------------------------------------------------------
// Player::GetNumFrames
//-----------------------------------------------------------------------------
Kimura::uint32 Kimura::Player::GetNumFrames()
{
	if (this->Status != Kimura::PlayerStatus::Ready)
	{
		return 0;
	}

	return (Kimura::uint32)this->Frames.size();
}


//-----------------------------------------------------------------------------
// Player::GetFrameAt
//-----------------------------------------------------------------------------
std::shared_ptr<Kimura::IFrame> Kimura::Player::GetFrameAt(uint32 iFrame, bool InForceWait)
{
	std::shared_ptr<Kimura::IFrame> r = nullptr;

	uint32 numFramesTotal = (uint32)this->Frames.size();

	// expected that the frame index be within the full range of the playback
	if (iFrame >= numFramesTotal)
	{
		// complain
		return nullptr;
	}

	// special case when 'buffer entire playback' is on
	if (this->Options.BufferEntirePlayback)
	{
		std::unique_lock<std::mutex> threadLock(this->FrameAccessMutex);

		bool bFrameBuffered = (iFrame >= this->FullyBufferedFramesStart) && (iFrame < this->FullyBufferedFramesStart + this->FullyBufferedFramesCount);
		bFrameBuffered |= ((iFrame + numFramesTotal) >= this->FullyBufferedFramesStart) && ((iFrame + numFramesTotal) < this->FullyBufferedFramesStart + this->FullyBufferedFramesCount);

		if (bFrameBuffered)
		{
			return this->Frames[iFrame];
		}

	}
	else
	{
		std::unique_lock<std::mutex> threadLock(this->FrameAccessMutex);

		// first, is this frame buffered? or in queue to be buffered?
		bool bFrameBuffered = (iFrame >= this->FullyBufferedFramesStart) && (iFrame < this->FullyBufferedFramesStart + this->FullyBufferedFramesCount);
		bFrameBuffered |= ((iFrame+numFramesTotal) >= this->FullyBufferedFramesStart) && ((iFrame+numFramesTotal)< this->FullyBufferedFramesStart + this->FullyBufferedFramesCount);

		bool bFrameIntentedToBeBuffered = (iFrame >= this->FullyBufferedFramesStart) && (iFrame < this->FullyBufferedFramesStart + this->Options.PreBufferingSize);
		bFrameIntentedToBeBuffered |= ((iFrame+numFramesTotal) >= this->FullyBufferedFramesStart) && ((iFrame + numFramesTotal) < this->FullyBufferedFramesStart + this->Options.PreBufferingSize);

		if (bFrameBuffered)
		{

			//std::printf("Obtaining frame %d\n", iFrame);

			// this is the frame we want to return
			r = this->Frames[iFrame];

			// remove previous frames 
			if (!this->Options.BufferEntirePlayback)
			{
				while (this->FullyBufferedFramesStart != iFrame)
				{
					//std::printf("Removing frame %d\n", this->FullyBufferedFramesStart);

					// adjust memory footprint
					if (this->Frames[this->FullyBufferedFramesStart] != nullptr)
					{
						this->Profiling.MemoryUsageForFrames -= (uint64)this->Frames[this->FullyBufferedFramesStart]->Buffer.size();
					}

					this->Frames[this->FullyBufferedFramesStart] = nullptr;
					this->FullyBufferedFramesStart++;
					this->FullyBufferedFramesStart %= numFramesTotal;

					this->FullyBufferedFramesCount--;
				}
			}
		}
		else if (bFrameIntentedToBeBuffered)
		{
			// frame isn't loaded at this time but the player is already working to get there. Do nothing.
			//std::printf("Frame %d isn't available yet but will be. Waiting...\n", iFrame);
		}
		else
		{

			//std::printf("Requesting frame from non-buffered section. Clearing %d buffered frames and jumping to frame %d \n", this->FullyBufferedFramesCount, iFrame);

			// clear all buffered frames
			while (this->FullyBufferedFramesCount > 0)
			{
				// adjust memory footprint
				if (this->Frames[this->FullyBufferedFramesStart] != nullptr)
				{
					this->Profiling.MemoryUsageForFrames -= this->Frames[this->FullyBufferedFramesStart]->Buffer.size();
				}

				this->Frames[this->FullyBufferedFramesStart] = nullptr;
				this->FullyBufferedFramesStart++;
				this->FullyBufferedFramesStart %= numFramesTotal;

				this->FullyBufferedFramesCount--;
			}

			// set new buffer start 
			this->FullyBufferedFramesStart = iFrame;
		}

	}

	// wake up the player's thread and look for more work to do. 
	this->WakeUpBufferThreadEvent.notify_one();

	// This will force blocking until the desired frame is ready
	while (r == nullptr && InForceWait)
	{
		// wait until a frame has been obtained
		std::unique_lock<std::mutex> threadLock(this->WaitForFrameBufferedMutex);
		this->WaitForFrameBufferedEvent.wait(threadLock);

		// *try* to get the frame but do not wait this time
		r = this->GetFrameAt(iFrame, false);

	}

	return r;
}


//-----------------------------------------------------------------------------
// Player::GetConstantFrame
//-----------------------------------------------------------------------------
std::shared_ptr<Kimura::IFrame> Kimura::Player::GetConstantFrame()
{
	return this->FirstFrame;
}


//-----------------------------------------------------------------------------
// Player::IsForcing16BitIndices
//-----------------------------------------------------------------------------
bool Kimura::Player::IsForcing16BitIndices()
{
	return this->TOC.Force16BitIndices;
}


//-----------------------------------------------------------------------------
// Player::RetrievePlaybackInformation
//-----------------------------------------------------------------------------
bool Kimura::Player::RetrievePlaybackInformation(PlaybackInformation& OutInfo)
{
	if (this->Status != PlayerStatus::Ready)
	{
		return false;
	}

	OutInfo.FrameRate = this->TOC.FrameRate;
	OutInfo.TimePerFrame = this->TOC.TimePerFrame;
	OutInfo.FrameCount = (uint32) this->TOC.Frames.size();
	OutInfo.Duration = OutInfo.FrameCount * OutInfo.TimePerFrame;

	OutInfo.Meshes.resize(this->TOC.Meshes.size());
	for (uint32 iMesh = 0; iMesh < this->TOC.Meshes.size(); iMesh++)
	{
		OutInfo.Meshes[iMesh].Index = iMesh;
		OutInfo.Meshes[iMesh].Name = this->TOC.Meshes[iMesh].Name;
		OutInfo.Meshes[iMesh].MaximumVertices = this->TOC.Meshes[iMesh].MaxVertices;
		OutInfo.Meshes[iMesh].MaximumSurfaces= this->TOC.Meshes[iMesh].MaxSurfaces;
		OutInfo.Meshes[iMesh].Force16BitIndices = this->TOC.Force16BitIndices;
		OutInfo.Meshes[iMesh].PositionFormat_ = this->TOC.Meshes[iMesh].PositionFormat_;
		OutInfo.Meshes[iMesh].NormalFormat_ = this->TOC.Meshes[iMesh].NormalFormat_;
		OutInfo.Meshes[iMesh].TangentFormat_ = this->TOC.Meshes[iMesh].TangentFormat_;
		OutInfo.Meshes[iMesh].VelocityFormat_ = this->TOC.Meshes[iMesh].VelocityFormat_;
		OutInfo.Meshes[iMesh].TexCoordFormat_ = this->TOC.Meshes[iMesh].TexCoordFormat_;
		OutInfo.Meshes[iMesh].ColorFormat_ = this->TOC.Meshes[iMesh].ColorFormat_;
	}

	OutInfo.ImageSequences.resize(this->TOC.ImageSequences.size());
	for (uint32 i = 0; i < this->TOC.ImageSequences.size(); i++)
	{
		OutInfo.ImageSequences[i].Index = i;
		OutInfo.ImageSequences[i].Name = this->TOC.ImageSequences[i].Name;
		OutInfo.ImageSequences[i].Constant = this->TOC.ImageSequences[i].Constant;
		OutInfo.ImageSequences[i].Width = this->TOC.ImageSequences[i].Width;
		OutInfo.ImageSequences[i].Height = this->TOC.ImageSequences[i].Height;
		OutInfo.ImageSequences[i].Mipmaps = this->TOC.ImageSequences[i].MipMapCount;
		switch (this->TOC.ImageSequences[i].Format)
		{
			case ImageFormat::RGBA8:
			{
				OutInfo.ImageSequences[i].Format = "RGBA8";
				break;
			}

			case ImageFormat::DXT1:
			{
				OutInfo.ImageSequences[i].Format = "DXT1";
				break;
			}

			case ImageFormat::DXT3:
			{
				OutInfo.ImageSequences[i].Format = "DXT3";
				break;
			}

			case ImageFormat::DXT5:
			{
				OutInfo.ImageSequences[i].Format = "DXT5";
				break;
			}

		}

	}

	return true;

}


//-----------------------------------------------------------------------------
// Frame::CollectStats
//-----------------------------------------------------------------------------
void Kimura::Player::CollectStats(PlayerStats& OutStats)
{

	const std::chrono::time_point<std::chrono::high_resolution_clock> now = std::chrono::high_resolution_clock::now();
	if (now > this->NextStatsCollection)
	{
		std::unique_lock<std::mutex> threadLock(this->ProfilingMutex);

		this->StoredProfiling.BytesReadInLastSecond = this->Profiling.BytesReadInLastSecond;
		this->StoredProfiling.MemoryUsageForFrames = this->Profiling.MemoryUsageForFrames;

		// update stats
		this->StoredProfiling.AvgTimeSpentOnReadingFromDiskPerFrame = this->Profiling.TotalTimeSpentOnReadingFromDiskInLastSecond / (double)this->Profiling.NumFramesProcessedInLastSecond;
		this->StoredProfiling.AvgTimeSpentOnProcessingPerFrames = this->Profiling.TotalTimeSpentOnProcessingFramesInLastSecond / (double)this->Profiling.NumFramesProcessedInLastSecond;
		this->StoredProfiling.TotalTimeSpentOnReadingFromDiskInLastSecond = this->Profiling.TotalTimeSpentOnReadingFromDiskInLastSecond;
		this->StoredProfiling.TotalTimeSpentOnProcessingFramesInLastSecond = this->Profiling.TotalTimeSpentOnProcessingFramesInLastSecond;
		this->StoredProfiling.NumFramesProcessedInLastSecond = this->Profiling.NumFramesProcessedInLastSecond;

		this->Profiling.BytesReadInLastSecond = 0;
		this->Profiling.TotalTimeSpentOnProcessingFramesInLastSecond = 0;
		this->Profiling.TotalTimeSpentOnReadingFromDiskInLastSecond = 0;
		this->Profiling.NumFramesProcessedInLastSecond = 0;

		// c++ 14
		//using namespace std::literals;
		//this->NextStatsCollection = now + 1s;

		// c++ 11
		this->NextStatsCollection = now + std::chrono::seconds{1};

	}

	OutStats = this->StoredProfiling;

	OutStats.BufferedFramesStart = this->FullyBufferedFramesStart;
	OutStats.BufferedFramesCount = this->FullyBufferedFramesCount;


}


//-----------------------------------------------------------------------------
// Frame::GetBufferedFrameCount
//-----------------------------------------------------------------------------
int Kimura::Player::GetBufferedFrameCount()
{
	return this->FullyBufferedFramesCount;
}


//-----------------------------------------------------------------------------
// Frame::GetIndicesU16
//-----------------------------------------------------------------------------
const Kimura::uint16* Kimura::Frame::GetIndicesU16(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].IndicesU16;

}


//-----------------------------------------------------------------------------
// Frame::GetIndicesU32
//-----------------------------------------------------------------------------
const Kimura::uint32* Kimura::Frame::GetIndicesU32(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].IndicesU32;

}


//-----------------------------------------------------------------------------
// Frame::GetPositionsF32
//-----------------------------------------------------------------------------
const Kimura::Vector3* Kimura::Frame::GetPositionsF32(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].PositionsF32;

}


//-----------------------------------------------------------------------------
// Frame::GetPositionsI16
//-----------------------------------------------------------------------------
const Kimura::int16* Kimura::Frame::GetPositionsI16(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].PositionsI16;
}


//-----------------------------------------------------------------------------
// Frame::GetPositionQuantizationCenter
//-----------------------------------------------------------------------------
Kimura::Vector3 Kimura::Frame::GetPositionQuantizationCenter(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return Kimura::Vector3::ZeroVector;
	}

	return this->Meshes[InMeshIndex].PositionQuantizationCenter;
}


//-----------------------------------------------------------------------------
// Frame::GetPositionQuantizationExtents
//-----------------------------------------------------------------------------
Kimura::Vector3 Kimura::Frame::GetPositionQuantizationExtents(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return Kimura::Vector3::OneVector;
	}

	return this->Meshes[InMeshIndex].PositionQuantizationExtents;
}


//-----------------------------------------------------------------------------
// Frame::GetNormalsF32
//-----------------------------------------------------------------------------
const Kimura::Vector3* Kimura::Frame::GetNormalsF32(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].NormalsF32;

}


//-----------------------------------------------------------------------------
// Frame::GetNormalsI16
//-----------------------------------------------------------------------------
const Kimura::int16* Kimura::Frame::GetNormalsI16(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].NormalsI16;

}


//-----------------------------------------------------------------------------
// Frame::GetNormalsI8
//-----------------------------------------------------------------------------
const Kimura::int8* Kimura::Frame::GetNormalsI8(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].NormalsI8;
}


//-----------------------------------------------------------------------------
// Frame::GetTangentsF32
//-----------------------------------------------------------------------------
const Kimura::Vector4* Kimura::Frame::GetTangentsF32(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].TangentsF32;

}


//-----------------------------------------------------------------------------
// Frame::GetTangentsI16
//-----------------------------------------------------------------------------
const Kimura::int16* Kimura::Frame::GetTangentsI16(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].TangentsI16;

}


//-----------------------------------------------------------------------------
// Frame::GetTangentsI8
//-----------------------------------------------------------------------------
const Kimura::int8* Kimura::Frame::GetTangentsI8(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].TangentsI8;
}


//-----------------------------------------------------------------------------
// Frame::GetVelocitiesF32
//-----------------------------------------------------------------------------
const Kimura::Vector3* Kimura::Frame::GetVelocitiesF32(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].VelocitiesF32;
}


//-----------------------------------------------------------------------------
// Frame::GetVelocitiesI16
//-----------------------------------------------------------------------------
const Kimura::int16* Kimura::Frame::GetVelocitiesI16(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].VelocitiesI16;
}



//-----------------------------------------------------------------------------
// Frame::GetVelocitiesI8
//-----------------------------------------------------------------------------
const Kimura::int8* Kimura::Frame::GetVelocitiesI8(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].VelocitiesI8;

}


//-----------------------------------------------------------------------------
// Frame::GetVelocityQuantizationCenter
//-----------------------------------------------------------------------------
Kimura::Vector3 Kimura::Frame::GetVelocityQuantizationCenter(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return Kimura::Vector3::ZeroVector;
	}

	return this->Meshes[InMeshIndex].VelocityQuantizationCenter;
}


//-----------------------------------------------------------------------------
// Frame::GetVelocityQuantizationExtents
//-----------------------------------------------------------------------------
Kimura::Vector3 Kimura::Frame::GetVelocityQuantizationExtents(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return Kimura::Vector3::OneVector;
	}

	return this->Meshes[InMeshIndex].VelocityQuantizationExtents;
}


//-----------------------------------------------------------------------------
// Frame::GetTexCoordsF32
//-----------------------------------------------------------------------------
const Kimura::Vector2* Kimura::Frame::GetTexCoordsF32(uint32 InMeshIndex, uint32 iTexCoord)
{
	if (InMeshIndex >= this->Meshes.size() || iTexCoord >= MaxTextureCoords)
	{
		return nullptr;
	}

	return (Vector2*) this->Meshes[InMeshIndex].TexCoordsF32[iTexCoord];

}


//-----------------------------------------------------------------------------
// Frame::GetTexCoordsU16
//-----------------------------------------------------------------------------
const Kimura::uint16* Kimura::Frame::GetTexCoordsU16(uint32 InMeshIndex, uint32 iTexCoord)
{
	if (InMeshIndex >= this->Meshes.size() || iTexCoord >= MaxTextureCoords)
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].TexCoordsU16[iTexCoord];
}


//-----------------------------------------------------------------------------
// Frame::GetColorsF32
//-----------------------------------------------------------------------------
const Kimura::Vector4* Kimura::Frame::GetColorsF32(uint32 InMeshIndex, uint32 iColor)
{
	if (InMeshIndex >= this->Meshes.size() || iColor >= MaxColorChannels)
	{
		return nullptr;
	}

	return (Vector4*)this->Meshes[InMeshIndex].ColorsF32[iColor];

}


//-----------------------------------------------------------------------------
// Frame::GetColorsU16
//-----------------------------------------------------------------------------
const Kimura::uint16* Kimura::Frame::GetColorsU16(uint32 InMeshIndex, uint32 iColor)
{
	if (InMeshIndex >= this->Meshes.size() || iColor >= MaxColorChannels)
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].ColorsU16[iColor];

}


//-----------------------------------------------------------------------------
// Frame::GetColorsU8
//-----------------------------------------------------------------------------
const Kimura::uint8* Kimura::Frame::GetColorsU8(uint32 InMeshIndex, uint32 iColor)
{
	if (InMeshIndex >= this->Meshes.size() || iColor >= MaxColorChannels)
	{
		return nullptr;
	}

	return this->Meshes[InMeshIndex].ColorsU8[iColor];

}


//-----------------------------------------------------------------------------
// Frame::GetColorQuantizationExtents
//-----------------------------------------------------------------------------
Kimura::Vector4 Kimura::Frame::GetColorQuantizationExtents(uint32 InMeshIndex, uint32 InColorIndex)
{
	if (InMeshIndex >= this->Meshes.size() || InColorIndex >= MaxColorChannels)
	{
		return Kimura::Vector4::ZeroVector;
	}

	return this->Meshes[InMeshIndex].ColorQuantizationExtents[InColorIndex];

}


//-----------------------------------------------------------------------------
// Frame::GetBounds
//-----------------------------------------------------------------------------
void Kimura::Frame::GetBounds(uint32 InMeshIndex, Vector3& OutCenter, Vector3& OutSize)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		OutCenter = Vector3::ZeroVector;
		OutSize = Vector3::ZeroVector;
		return;
	}

	OutCenter = this->Meshes[InMeshIndex].BoundingCenter;
	OutSize = this->Meshes[InMeshIndex].BoundingSize;

	return;

}


//-----------------------------------------------------------------------------
// Frame::GetNumVertices
//-----------------------------------------------------------------------------
Kimura::uint32 Kimura::Frame::GetNumVertices(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return 0;
	}

	return this->Meshes[InMeshIndex].Vertices;

}


//-----------------------------------------------------------------------------
// Frame::GetNumSurfaces
//-----------------------------------------------------------------------------
Kimura::uint32 Kimura::Frame::GetNumSurfaces(uint32 InMeshIndex)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		return 0;
	}

	return this->Meshes[InMeshIndex].Surfaces;
}


//-----------------------------------------------------------------------------
// Frame::GetSections
//-----------------------------------------------------------------------------
void Kimura::Frame::GetSections(uint32 InMeshIndex, std::vector<MeshSection>& OutSections)
{
	if (InMeshIndex >= this->Meshes.size())
	{
		OutSections.clear();
	}

	FrameMesh& fm = this->Meshes[InMeshIndex];

	OutSections.resize(fm.Sections.size());

	uint32 i = 0;
	for (TOCFrameMeshSection& s : fm.Sections)
	{
		MeshSection& outSection = OutSections[i];

		outSection.VertexStart = s.VertexStart;
		outSection.IndexStart = s.IndexStart;
		outSection.NumSurfaces = s.NumSurfaces;
		outSection.MinVertexIndex = s.MinVertexIndex;
		outSection.MaxVertexIndex = s.MaxVertexIndex;

		i++;
	}

}


//-----------------------------------------------------------------------------
// Frame::GetImageData
//-----------------------------------------------------------------------------
bool Kimura::Frame::GetImageData(uint32 InImageIndex, uint32 InMipmap, const void** OutData, uint32& OutSize)
{

	if (InImageIndex >= this->Images.size() || OutData == nullptr)
	{
		OutSize = 0;
		return false;
	}

	if (InMipmap >= this->Images[InImageIndex].NumMipmaps)
	{
		OutSize = 0;
		*OutData = nullptr;
		return false;
	}

	*OutData = this->Images[InImageIndex].Mipmaps[InMipmap].Data;
	OutSize = this->Images[InImageIndex].Mipmaps[InMipmap].Size;

	return true;

}


