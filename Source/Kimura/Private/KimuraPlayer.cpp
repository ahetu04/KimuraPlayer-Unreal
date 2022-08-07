//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "KimuraPlayer.h"
#include "KimuraModule.h"
#include "Kimura.h"
#include "KimuraMeshComponent.h"
#include "Rendering/Texture2DResource.h"
#include "Materials/MaterialInstanceDynamic.h"

#if WITH_EDITOR
	#include "MoviePipelineQueueSubsystem.h"
	#include "Editor.h"
#endif

//-----------------------------------------------------------------------------
// AKimuraPlayer::AKimuraPlayer
//-----------------------------------------------------------------------------
AKimuraPlayer::AKimuraPlayer()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	this->KimuraMeshRoot = CreateDefaultSubobject<USceneComponent>(TEXT("KimuraMeshRoot"));
	this->RootComponent = this->KimuraMeshRoot;

}

#if WITH_EDITOR

//-----------------------------------------------------------------------------
// AKimuraPlayer::PostEditChangeProperty
//-----------------------------------------------------------------------------
void AKimuraPlayer::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = NAME_None;
	if (PropertyChangedEvent.Property != nullptr) 
	{
		PropertyName = PropertyChangedEvent.MemberProperty->GetFName();
	}

	// Make sure that the filename is relative to the project's content dir, so that it can be packaged as a movie in a 
	// standalone package. 
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(AKimuraPlayer, InputFile)) && !this->InputFile.FilePath.IsEmpty())
	{
		FPaths::MakePathRelativeTo(this->InputFile.FilePath, *FPaths::ProjectContentDir());
	}

	this->ResetPlayerRequested = true;

}


//-----------------------------------------------------------------------------
// AKimuraPlayer::PreEditUndo
//-----------------------------------------------------------------------------
void AKimuraPlayer::PreEditUndo()
{
	Super::PreEditUndo();
}


//-----------------------------------------------------------------------------
// AKimuraPlayer::PostEditUndo
//-----------------------------------------------------------------------------
void AKimuraPlayer::PostEditUndo()
{
	this->ResetPlayerRequested = true;

	Super::PostEditUndo();
}

//-----------------------------------------------------------------------------
// AKimuraPlayer::SetDefaultMaterial
//-----------------------------------------------------------------------------
void AKimuraPlayer::SetDefaultMaterial(UMaterialInterface* Material)
{
	this->DefaultMaterial = Material;
	this->ResetPlayerRequested = true;
}


#endif // WITH_EDITOR


//-----------------------------------------------------------------------------
// AKimuraPlayer::SetInputPlayer
//-----------------------------------------------------------------------------
void AKimuraPlayer::SetInputPlayer(AKimuraPlayer* InInputPlayer)
{
	this->InputPlayer = InInputPlayer;
}


//-----------------------------------------------------------------------------
// AKimuraPlayer::BeginPlay
//-----------------------------------------------------------------------------
void AKimuraPlayer::BeginPlay()
{
	Super::BeginPlay();

#ifdef WITH_EDITOR
	this->ResetPlayerRequested = true;
#endif
}


//-----------------------------------------------------------------------------
// AKimuraPlayer::Tick
//-----------------------------------------------------------------------------
void AKimuraPlayer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// setup KimuraPlayer. This returns true once everything is ready. 
	if (!this->TickKimuraPlayerSetup())
	{
		return;
	}

	int32 desiredFrameIndex = 0;
	switch (this->FrameControl)
	{
		case EKimuraPlayerFrameControl::PlaybackTime:
		{
			this->PlaybackTime += DeltaTime * this->PlaybackTimeSpeed;
			desiredFrameIndex = this->PlaybackTime / (1.0f / this->FrameRate);

			break;
		}

		case EKimuraPlayerFrameControl::SequencerFrame:
		{
			desiredFrameIndex = (int32)this->SequencerFrame;
			break;
		}

		case EKimuraPlayerFrameControl::SequencerTime:
		{
			desiredFrameIndex = this->SequencerTime / (1.0f / this->FrameRate);
			break;
		}

	}

	if (desiredFrameIndex < 0)
	{
		desiredFrameIndex = 0;
	}

	if (this->Loop)
	{
		desiredFrameIndex %= this->FrameCount;
	}
	else
	{
		if (this->DestroyOnCompletion && (uint32) desiredFrameIndex >= this->FrameCount)
		{
			UE_LOG(KimuraLog, Log, TEXT("%s: Kimura player auto-destroyed on completion"), *this->GetName());

			this->Destroy();
			return;
		}
	}

	this->NumFramesBuffered = this->KimuraPlayer->GetBufferedFrameCount();

	bool bForceFrame = false;
	{
		// in UE5, detect when selection changes on the actor. For some reason something gets reset 
		// and the player doesn't refresh the mesh because it rightfully thinks the frame hasn't changed. 
		#if WITH_EDITOR && ENGINE_MAJOR_VERSION >= 5
		{
			if (this->SelectedInEditor && !this->IsSelectedInEditor())
			{
				SelectedInEditor = false;
				bForceFrame = true;
			}
			else if (!this->SelectedInEditor && this->IsSelectedInEditor())
			{
				SelectedInEditor = true;
				bForceFrame = true;
			}
		}
		#endif
	}

	// In editor, block if movie render queue is active
	#if WITH_EDITOR
	{
		bool bmovieRender = false;
		UMoviePipelineQueueSubsystem* PipelineSubsystem = GEditor->GetEditorSubsystem<UMoviePipelineQueueSubsystem>();
		if (PipelineSubsystem->IsRendering())
		{
			bForceFrame = true;
		}
	}
	#endif


	if (bForceFrame || this->LastFrameSet == nullptr || this->LastFrameSet->FrameIndex != desiredFrameIndex)
	{

		std::shared_ptr<Kimura::IFrame> pFrame = this->KimuraPlayer->GetFrameAt(desiredFrameIndex, this->BlockMainThreadIfNecessary || bForceFrame);

		// if using PlaybackTime and if frame is not ready yet, rewind playback time
		if (this->FrameControl == EKimuraPlayerFrameControl::PlaybackTime && this->PausePlaybackTimeWhenStarved && pFrame == nullptr)
		{	
			this->PlaybackTime -= DeltaTime * this->PlaybackTimeSpeed;
		}

		if (!this->Visible)
		{
			pFrame = nullptr;
		}

		// params structure for passing down arguments all the way to the vertex factory shader
		KimuraMeshFrameParams frameParams;
		frameParams.VelocityScale = this->VelocityScale;

		// update the geometry on the meshes
		for (int32 iMesh = 0; iMesh < this->Meshes.Num(); iMesh++)
		{
			if (IsValid(this->Meshes[iMesh].MeshComponent))
			{
				this->Meshes[iMesh].MeshComponent->UpdateFrame(pFrame, frameParams);

				if (pFrame)
				{
					this->Meshes[iMesh].CurrentVertices = pFrame->GetNumVertices(iMesh);
					this->Meshes[iMesh].CurrentSurfaces = pFrame->GetNumSurfaces(iMesh);
				}
			}
		}

		// update the textures from the image sequences
		for (int32 iImageSequence = 0; iImageSequence < this->ImageSequences.Num(); iImageSequence++)
		{
			
			// this is a special case where we only need to update one texture, once.
			if (this->ImageSequences[iImageSequence].Constant)
			{
				if (!this->ImageSequences[iImageSequence].ConstantFrameSet)
				{
					uint32 textureIndex = 0;
					std::shared_ptr<Kimura::IFrame> pConstantFrame = this->KimuraPlayer->GetConstantFrame();

					ENQUEUE_RENDER_COMMAND(FKimuraUpdateTexture)(
						[this, iImageSequence, pConstantFrame, textureIndex](FRHICommandListImmediate& RHICmdList)
					{
						this->UpdateTextureResource(pConstantFrame, iImageSequence, textureIndex);
					});

					this->ImageSequences[iImageSequence].ConstantFrameSet = true;
				}
			}
			else
			{
				// update next available texture from the render thread. 

				// store the current update index so that it may be captured by the lambda
				uint32 textureIndex = this->TextureResourceUpdateIndex;

				ENQUEUE_RENDER_COMMAND(FKimuraUpdateTexture)(
					[this, iImageSequence, pFrame, textureIndex](FRHICommandListImmediate& RHICmdList)
				{
					this->UpdateTextureResource(pFrame, iImageSequence, textureIndex);
				});

			}

		}

		// update the meshes' material instances references to the textures 
		{
			for (int32 iMesh = 0; iMesh < this->Meshes.Num(); iMesh++)
			{
				FKimuraMeshInstance& m = this->Meshes[iMesh];
				if (!IsValid(m.MaterialInstance))
				{
					continue;
				}

				for (const FName& n : m.ImageSequencesUsed)
				{
					// find which image sequence is referenced by name
					const int* pImageSequenceIndex = this->ImageSequenceByName.Find(n);

					// if image sequence was found, update the texture params on the material instance 
					if (pImageSequenceIndex)
					{
						int index = *pImageSequenceIndex;

						// whenever dealing with an image sequence containing only one image (constant), always use first texture
						if (this->ImageSequences[index].Constant)
						{
							m.MaterialInstance->SetTextureParameterValue(n, this->ImageSequences[index].Textures[0]);
						}
						else
						{
							m.MaterialInstance->SetTextureParameterValue(n, this->ImageSequences[index].Textures[this->TextureResourceUpdateIndex]);
						}

					}

				}
				
			}

		}

		this->TextureResourceDrawIndex = this->TextureResourceUpdateIndex;
		this->TextureResourceUpdateIndex++;
		this->TextureResourceUpdateIndex %= this->NumTextureResourcesPerImageSequence;

		this->CurrentFrame = desiredFrameIndex;

		this->LastFrameSet = pFrame;

// 		if (this->LastFrameSet)
// 		{
// 			this->CurrentFrameReadTime = this->LastFrameSet->ReadTimeInMS;
// 		}
// 		else
// 		{
// 			this->CurrentFrameReadTime = 0.0f;
// 		}
	}

#if WITH_EDITOR

	Kimura::PlayerStats stats;
	this->KimuraPlayer->CollectStats(stats);

	this->BytesReadInLastSecond = FText::AsNumber(stats.BytesReadInLastSecond).ToString();
	this->BytesAllocatedForFrames = FText::AsNumber(stats.MemoryUsageForFrames).ToString();
	this->AverageReadTimePerFrame = stats.AvgTimeSpentOnReadingFromDiskPerFrame;
	this->AverageProcessTimePerFrame = stats.AvgTimeSpentOnProcessingPerFrames;

#endif

}


//-----------------------------------------------------------------------------
// AKimuraPlayer::TickKimuraPlayerSetup
//-----------------------------------------------------------------------------
bool AKimuraPlayer::TickKimuraPlayerSetup()
{
	if (this->PluginVersion.IsEmpty())
	{
		this->PluginVersion = Kimura::GetVersion().c_str();
	}

	// detect when player should be terminated	
	if (!this->Enabled && this->KimuraPlayer != nullptr)
	{
		if (this->ResetPlayerRequested == false)
		{
			this->ResetPlayerDelayFrames = 3;
		}

		this->ResetPlayerRequested = true;
	}

	// when we need to reset, we do so right before re-initializing everything
	if (this->ResetPlayerRequested)
	{
		// delay destruction of the player for a few frames
		if (--this->ResetPlayerDelayFrames > 0)
		{
			return false;
		}

		this->ResetKimuraPlayer();
		this->ResetPlayerRequested = false;

		return false;
	}

	if (!this->Enabled)
	{
		return false;
	}

	if (this->DefaultMaterial == nullptr)
	{
		return false;
	}

	if (this->InputFile.FilePath.IsEmpty() && !IsValid(this->InputPlayer))
	{
		return false;
	}

	// when using another player as input source, it must be set to buffer its entire playback
	if (IsValid(this->InputPlayer))
	{
		if (!this->InputPlayer->BufferEntirePlayback)
		{
			return false;
		}
	}

	// always try to create a player, as soon as possible
	if (this->KimuraPlayer == nullptr)
	{
		if (IsValid(this->InputPlayer))
		{
			this->KimuraPlayer = this->InputPlayer->KimuraPlayer;
		}
		else
		{
			TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AKimuraPlayer::CreatePlayer"));

			// convert to full path
			FString relativePath = FPaths::Combine(FPaths::ProjectContentDir(), this->InputFile.FilePath);

			FString fullPath = FPaths::ConvertRelativePathToFull(relativePath);
			std::string s(TCHAR_TO_UTF8(*fullPath));

			UE_LOG(KimuraLog, Log, TEXT("%s: Initializing new player for file %s"), *this->GetName(), *fullPath);

			Kimura::PlayerOptions options;
			options.PreBufferingSize = this->FramesToBuffer;
			options.BufferEntirePlayback = this->BufferEntirePlayback;
			options.Loop = this->Loop;
			this->KimuraPlayer = Kimura::CreatePlayer(s, options);

			UE_LOG(KimuraLog, Log, TEXT("%s: Kimura player created"), *this->GetName());

		}

	}

	if (this->KimuraPlayer == nullptr)
	{
		return false;
	}

	// once we have a player, wait for it to be ready
	if (this->KimuraPlayer->GetStatus() != Kimura::PlayerStatus::Ready)
	{
		return false;
	}

	if (!this->MeshesAndTexturesReady)
	{
		// setup the meshes and textures
		this->SetupMeshesAndTextures();
	}

	return true;

}


//-----------------------------------------------------------------------------
// AKimuraPlayer::SetupMeshesAndTextures
//-----------------------------------------------------------------------------
void AKimuraPlayer::SetupMeshesAndTextures()
{
	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AKimuraPlayer::SetupMeshesAndTextures"));

	this->DefaultMaterialInstance = UMaterialInstanceDynamic::Create(this->DefaultMaterial, this);

	// update the current file's playback information.
	{

		this->FileVersion = this->KimuraPlayer->GetFileVersion().c_str();

		this->KimuraPlayer->RetrievePlaybackInformation(this->PlaybackInfo);

		this->FrameRate = this->PlaybackInfo.FrameRate;
		this->FrameCount = this->PlaybackInfo.FrameCount;
		this->Duration = this->PlaybackInfo.Duration;
		this->NumMeshes = (uint32)this->PlaybackInfo.Meshes.size();
		this->NumImageSequences = (uint32)this->PlaybackInfo.ImageSequences.size();

	}

	if (IsValid(this->InputPlayer))
	{
		// copy the Meshes from the player 
		this->Meshes = this->InputPlayer->Meshes;
	}

	// all material instances and mesh components should have been destroyed by now
	for (int32 iExistingMesh = 0; iExistingMesh < this->Meshes.Num(); iExistingMesh++)
	{
		this->Meshes[iExistingMesh].MaterialInstance = nullptr;
		
		if (!IsValid(this->InputPlayer))
		{
			check(!IsValid(this->Meshes[iExistingMesh].MeshComponent));
		}

		this->Meshes[iExistingMesh].MeshComponent = nullptr;

	}

	// setup the mesh components 
	TArray<FKimuraMeshInstance> newMeshInstanceArray;
	for (Kimura::MeshInformation& meshInfo : this->PlaybackInfo.Meshes)
	{
		// keep this as a local FString
		FString meshName = meshInfo.Name.c_str();
		
		FKimuraMeshInstance kimuraMeshInstance;
		kimuraMeshInstance.Name = meshInfo.Name.c_str();
		kimuraMeshInstance.MaximumVertices = meshInfo.MaximumVertices;
		kimuraMeshInstance.MaximumSurfaces = meshInfo.MaximumSurfaces;
		kimuraMeshInstance.PositionFormat = (EKimuraPositionFormats)meshInfo.PositionFormat_;
		kimuraMeshInstance.NormalFormat = (EKimuraNormalFormats)meshInfo.NormalFormat_;
		kimuraMeshInstance.TangentFormat = (EKimuraTangentFormats)meshInfo.TangentFormat_;
		kimuraMeshInstance.VelocityFormat = (EKimuraVelocityFormats)meshInfo.VelocityFormat_;
		kimuraMeshInstance.ColorFormat = (EKimuraColorFormats)meshInfo.ColorFormat_;
		kimuraMeshInstance.TexCoordFormat = (EKimuraTexCoordFormat)meshInfo.TexCoordFormat_;

		// ok, do we already have a mesh instance where material properties have been set for this mesh
		for (int32 iExistingMesh = 0; iExistingMesh < this->Meshes.Num(); iExistingMesh++)
		{
			if (this->Meshes[iExistingMesh].Name == meshName)
			{
				// we already had this mesh instance listed, copy some of its properties over to the new mesh instance
				kimuraMeshInstance.Material = this->Meshes[iExistingMesh].Material;
				kimuraMeshInstance.ImageSequencesUsed = this->Meshes[iExistingMesh].ImageSequencesUsed;
				break;
			}
		}

		// create a new mesh 
		kimuraMeshInstance.MeshComponent = NewObject<UKimuraMeshComponent>(this, FName(meshInfo.Name.c_str()));

		// if mesh instance has its own material, create an instance from it.
		if (kimuraMeshInstance.Material != nullptr)
		{
			// mesh has its own material interface, create a new instance
			kimuraMeshInstance.MaterialInstance = UMaterialInstanceDynamic::Create(kimuraMeshInstance.Material, this);
		}
		else
		{
			// else, use default material instance
			kimuraMeshInstance.MaterialInstance = this->DefaultMaterialInstance;
		}

		// initialize the mesh 
		kimuraMeshInstance.MeshComponent->Initialize(meshInfo, kimuraMeshInstance.MaterialInstance);
		kimuraMeshInstance.MeshComponent->RegisterComponent();
		kimuraMeshInstance.MeshComponent->AttachToComponent(this->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);

		newMeshInstanceArray.Add(kimuraMeshInstance);
	}

	// new array replaces previous one
	this->Meshes = newMeshInstanceArray;

	// setup the image sequences
	for (Kimura::ImageSequenceInformation& imageSequenceInfo : this->PlaybackInfo.ImageSequences)
	{
		FKimuraImageSequenceInstance imageSequenceInstance;
		imageSequenceInstance.Name = imageSequenceInfo.Name.c_str();
		imageSequenceInstance.Format = imageSequenceInfo.Format.c_str();
		imageSequenceInstance.Width = imageSequenceInfo.Width;
		imageSequenceInstance.Height = imageSequenceInfo.Height;
		imageSequenceInstance.Mipmaps = imageSequenceInfo.Mipmaps;
		imageSequenceInstance.Constant = imageSequenceInfo.Constant;
		if (imageSequenceInstance.Mipmaps > 5)
		{
			imageSequenceInstance.Mipmaps = 5;
		}

		uint32 numTextureResources = imageSequenceInstance.Constant ? 1 : this->NumTextureResourcesPerImageSequence;

		imageSequenceInstance.Textures.SetNum(numTextureResources);
		imageSequenceInstance.TextureResources.SetNum(numTextureResources);

		for (uint32 iResource = 0; iResource < numTextureResources; iResource++)
		{
			imageSequenceInstance.Textures[iResource] = this->CreateTextureResource(	imageSequenceInstance.Format, 
																						imageSequenceInstance.Width, 
																						imageSequenceInstance.Height, 
																						imageSequenceInstance.Mipmaps);
			imageSequenceInstance.Textures[iResource]->SRGB = this->sRGB;
			imageSequenceInstance.Textures[iResource]->UpdateResource();

#if ENGINE_MAJOR_VERSION >= 5
			imageSequenceInstance.TextureResources[iResource] = imageSequenceInstance.Textures[iResource] != nullptr ? (FTexture2DResource*)imageSequenceInstance.Textures[iResource]->GetResource() : nullptr;
#else
			imageSequenceInstance.TextureResources[iResource] = imageSequenceInstance.Textures[iResource] != nullptr ? (FTexture2DResource*)imageSequenceInstance.Textures[iResource]->Resource : nullptr;
#endif
		}

		this->ImageSequences.Add(imageSequenceInstance);
		this->ImageSequenceByName.Add(FName(*imageSequenceInstance.Name), this->ImageSequences.Num()-1);

	}


	this->MeshesAndTexturesReady = true;

	this->PlaybackTime = 0.0f;

	UE_LOG(KimuraLog, Log, TEXT("%s: %d meshes and %d image sequences created."), *this->GetName(), this->NumMeshes, this->NumImageSequences);

}


//-----------------------------------------------------------------------------
// AKimuraPlayer::CreateTextureResource
//-----------------------------------------------------------------------------
UTexture2D* AKimuraPlayer::CreateTextureResource(FString& InPixelFormat, int32 InWidth, int32 InHeight, int32 InMipmaps)
{
	if (InWidth == 0 || InHeight == 0)
	{
		return nullptr;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AKimuraPlayer::CreateTextureResource"));

	EPixelFormat pixelFormat = EPixelFormat::PF_DXT1;
	if (InPixelFormat == "DXT1")
	{
		pixelFormat = EPixelFormat::PF_DXT1;
	}
	else if (InPixelFormat == "DXT3")
	{
		pixelFormat = EPixelFormat::PF_DXT3;
	}
	else if (InPixelFormat == "DXT5")
	{
		pixelFormat = EPixelFormat::PF_DXT5;
	}


	if (InWidth % GPixelFormats[pixelFormat].BlockSizeX != 0)
	{
		return nullptr;
	}

	if (InHeight % GPixelFormats[pixelFormat].BlockSizeY != 0)
	{
		return nullptr;
	}

	if (InMipmaps == 0 || InMipmaps > 8)
	{
		return nullptr;
	}

	UTexture2D* pTexture = NewObject<UTexture2D>(this, NAME_None, RF_Transient);

	FTexturePlatformData* npd = new FTexturePlatformData();
	{
		npd->SizeX = InWidth;
		npd->SizeY = InHeight;
		npd->PixelFormat = pixelFormat;

		int32 mipmapWidth = InWidth;
		int32 mipmapHeight = InHeight;

		for (int32 iMipmap = 0; iMipmap < InMipmaps; iMipmap++)
		{
			int32 blockCountX = mipmapWidth / GPixelFormats[pixelFormat].BlockSizeX;
			int32 blockCountY = mipmapHeight / GPixelFormats[pixelFormat].BlockSizeY;

			FTexture2DMipMap* pMipmap = new FTexture2DMipMap();
			npd->Mips.Add(pMipmap);
			pMipmap->SizeX = mipmapWidth;
			pMipmap->SizeY = mipmapHeight;

			pMipmap->BulkData.Lock(LOCK_READ_WRITE);
			pMipmap->BulkData.Realloc(blockCountX * blockCountY * GPixelFormats[pixelFormat].BlockBytes);
			pMipmap->BulkData.Unlock();

			mipmapWidth /= 2;
			mipmapHeight /= 2;
		}
	}

#if ENGINE_MAJOR_VERSION >= 5

	pTexture->SetPlatformData(npd);

#else

	pTexture->PlatformData = npd;

#endif

	return pTexture;
}


//-----------------------------------------------------------------------------
// AKimuraPlayer::UpdateTextureResource
//-----------------------------------------------------------------------------
void AKimuraPlayer::UpdateTextureResource(std::shared_ptr<Kimura::IFrame> InFrame, uint32 InImageSequence, uint32 InTextureResourceUpdateIndex)
{
	check(IsInRenderingThread());


	if (InFrame == nullptr)
	{
		return;
	}

	if (InImageSequence >= (uint32)this->ImageSequences.Num())
	{
		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("AKimuraPlayer::UpdateTextureResource"));

	FKimuraImageSequenceInstance& instance = this->ImageSequences[InImageSequence];

	FTexture2DResource* pTextureResource = instance.TextureResources[InTextureResourceUpdateIndex];

	for (uint32 iMip = 0; iMip < instance.Mipmaps; iMip++)
	{
		const void* pSourceData = nullptr;
		uint32 sourceDataSize = 0;
		if (InFrame->GetImageData(InImageSequence, iMip, &pSourceData, sourceDataSize))
		{
			uint32 stride;
			void* pDestData = RHILockTexture2D(pTextureResource->GetTexture2DRHI(), iMip, RLM_WriteOnly, stride, false);

			memcpy(pDestData, pSourceData, sourceDataSize);

			RHIUnlockTexture2D(pTextureResource->GetTexture2DRHI(), iMip, false);

		}

	}

}


//-----------------------------------------------------------------------------
// AKimuraPlayer::ResetKimuraPlayer
//-----------------------------------------------------------------------------
void AKimuraPlayer::ResetKimuraPlayer()
{
	// stop and kill the player (if we're the owner)
	{
		// lose our reference to the player. Its destructor will take care of everything
		this->KimuraPlayer = nullptr;
	}

	// clean up all of the meshes
	{
		for (FKimuraMeshInstance& m : this->Meshes)
		{
			if (IsValid(m.MeshComponent))
			{
				m.MeshComponent->DestroyComponent(false);
				m.MeshComponent = nullptr;
			}

			// done with this material
			m.MaterialInstance = nullptr;
		}

		// find all other UKimuraMeshComponent, if any, and destroy them
		TArray<UKimuraMeshComponent*> a;
		this->GetComponents<UKimuraMeshComponent>(a);

		for (UKimuraMeshComponent* c : a)
		{
			if (IsValid(c))
			{
				c->DestroyComponent();
			}
		}

	}

	// clean up all image sequences
	{
		// TODO: do we need to release any texture resources?

		this->ImageSequences.Empty();
		this->ImageSequenceByName.Empty();
	}



	this->MeshesAndTexturesReady = false;
	this->LastFrameSet = nullptr;

	UE_LOG(KimuraLog, Log, TEXT("%s: Kimura player reseted"), *this->GetName());

}

