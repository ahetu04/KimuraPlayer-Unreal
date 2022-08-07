//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Kimura.h"
#include <memory>
#include "KimuraPlayer.generated.h"

UENUM()
enum class EKimuraPositionFormats
{
	Full, 
	Half
};


UENUM()
enum class EKimuraNormalFormats
{
	Full,
	Half, 
	Byte, 
	None
};

UENUM()
enum class EKimuraTangentFormats
{
	Full,
	Half,
	Byte, 
	None
};

UENUM()
enum class EKimuraVelocityFormats
{
	Full,
	Half,
	Byte,
	None
};

UENUM()
enum class EKimuraTexCoordFormat
{
	Full,
	Half,
	None
};

UENUM()
enum class EKimuraColorFormats
{
	Full,
	Half,
	ByteHDR,
	Byte,
	None
};



USTRUCT()
struct FKimuraMeshInstance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	FString Name;

	UPROPERTY(Transient, VisibleAnywhere)
	uint32 MaximumVertices = 0;

	UPROPERTY(Transient, VisibleAnywhere)
	uint32 MaximumSurfaces = 0;

	UPROPERTY(Transient, VisibleAnywhere)
	uint32 CurrentVertices = 0;

	UPROPERTY(Transient, VisibleAnywhere)
	uint32 CurrentSurfaces = 0;

	UPROPERTY(Transient, VisibleAnywhere)
	EKimuraPositionFormats PositionFormat;

	UPROPERTY(Transient, VisibleAnywhere)
	EKimuraNormalFormats NormalFormat;

	UPROPERTY(Transient, VisibleAnywhere)
	EKimuraTangentFormats TangentFormat;

	UPROPERTY(Transient, VisibleAnywhere)
	EKimuraVelocityFormats VelocityFormat;

	UPROPERTY(Transient, VisibleAnywhere)
	EKimuraColorFormats ColorFormat;

	UPROPERTY(Transient, VisibleAnywhere)
	EKimuraTexCoordFormat TexCoordFormat;

	/* Material assigned to this mesh. A material instance will be created and assigned only to this mesh. */
	UPROPERTY(EditAnywhere)
	UMaterialInterface*			Material = nullptr;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic*	MaterialInstance = nullptr;

	/* Names of image sequences (textures) that should be passed to this mesh's material instance. */
	UPROPERTY(EditAnywhere)
	TArray<FName>				ImageSequencesUsed;

	UPROPERTY(Transient)
	class UKimuraMeshComponent*	MeshComponent = nullptr;

};


USTRUCT()
struct FKimuraImageSequenceInstance
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(VisibleAnywhere)
	FString Name;

	UPROPERTY(VisibleAnywhere)
	FString Format;

	UPROPERTY(VisibleAnywhere)
	uint32 Width = 0;

	UPROPERTY(VisibleAnywhere)
	uint32 Height = 0;

	UPROPERTY(VisibleAnywhere)
	uint32 Mipmaps = 0;

	UPROPERTY(VisibleAnywhere)
	bool Constant = false;

	/* Only necessary when 'Constant' is TRUE. */
	UPROPERTY(VisibleAnywhere)
	bool ConstantFrameSet = false;

	UPROPERTY(Transient, VisibleAnywhere)
	TArray<UTexture2D*>			Textures;

	TArray<FTexture2DResource*>	TextureResources;

};


UENUM(BlueprintType)
enum class EKimuraPlayerFrameControl : uint8
{
	PlaybackTime,
	SequencerFrame,
	SequencerTime
};


UCLASS(Blueprintable)
class KIMURA_API AKimuraPlayer : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AKimuraPlayer();

#if WITH_EDITOR
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual void PreEditUndo() override;
	virtual void PostEditUndo() override;

	void SetDefaultMaterial(UMaterialInterface* Material);

#endif

	UFUNCTION(BlueprintCallable, Category = "Kimura")
	void SetInputPlayer(AKimuraPlayer* InInputPlayer);

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// tick in editor
	virtual bool ShouldTickIfViewportsOnly() const override { return true; }

	// Called every frame
	virtual void Tick(float DeltaTime) override;


	bool TickKimuraPlayerSetup();
	void SetupMeshesAndTextures();
	void ResetKimuraPlayer();

	UTexture2D* CreateTextureResource(FString& InFormat, int32 InWidth, int32 InHeight, int32 InMipmaps);
	void UpdateTextureResource(std::shared_ptr<Kimura::IFrame> InFrame, uint32 iImageSequence, uint32 InTextureResourceUpdateIndex);

protected:

	/* Points to the .k file you'd like to play on this instance. */
	UPROPERTY(EditAnywhere, Category = "Kimura")
	FFilePath					InputFile;

	/* If set, use another Kimura Player actor for reading frames */
	UPROPERTY(EditInstanceOnly, Category = "Kimura")
	AKimuraPlayer*				InputPlayer = nullptr;

	/* Default material applied to all of the meshes. */
	UPROPERTY(EditAnywhere, Category = "Kimura")
	UMaterialInterface*			DefaultMaterial;

	/* When TRUE, the player is active and continuously loads frames from the file. */
	UPROPERTY(EditAnywhere, Interp, Category = "Kimura")
	bool						Enabled = true;

	/* Use this to keep the player active but show/hide the meshes. */
	UPROPERTY(EditAnywhere, Interp, Category = "Kimura")
	bool						Visible = true;

	/* Allow alembic to repeat playback when past the last frame. */
	UPROPERTY(EditAnywhere, Category = "Kimura")
	bool						Loop = true;

	/* Destroy the actor when the player is done playing. Looping must be disabled before this can be set. */
	UPROPERTY(EditAnywhere, Category = "Kimura", meta = (EditCondition = "!Loop"))
	bool						DestroyOnCompletion = false;

	/* When TRUE, the player will attempt to load all of the file's frame into memory. Watch out for big files.  */
	UPROPERTY(EditAnywhere, Category = "Kimura")
	bool						BufferEntirePlayback = false;

	/* The player will always try to keep this number of frames loaded ahead of the current playback position. */
	UPROPERTY(EditAnywhere, Category = "Kimura", meta = (EditCondition = "!BufferEntirePlayback", ClampMin = "1", UIMin = "1"))
	int32						FramesToBuffer = 30;

	/* The player will always try to keep this number of frames loaded ahead of the current playback position. */
	UPROPERTY(EditAnywhere, Category = "Kimura")
	EKimuraPlayerFrameControl	FrameControl = EKimuraPlayerFrameControl::PlaybackTime;

	/* When FrameControl is set to PlaybackTime, the playback position is advanced using the unreal clock. */
	UPROPERTY(VisibleAnywhere, Category = "Kimura", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "FrameControl == EKimuraPlayerFrameControl::PlaybackTime"))
	float						PlaybackTime = 0.0f;

	/* When FrameControl is set to PlaybackTime, waits for frames to be available before time is advanced. */
	UPROPERTY(EditAnywhere, Category = "Kimura", meta = (EditCondition = "FrameControl == EKimuraPlayerFrameControl::PlaybackTime"))
	bool						PausePlaybackTimeWhenStarved = true;

	/* !!CAREFUL!! Forces frames to display even if it means blocking the main thread. Note that you do NOT need to enable this in order to get movie render queues to work; this blocking behavior will kick in whenever a movie render queue is active. */
	UPROPERTY(EditAnywhere, Category = "Kimura")
	bool						BlockMainThreadIfNecessary =  false;

	/* When FrameControl is set to PlaybackTime, use this to slow down or speed up the animation speed. */
	UPROPERTY(EditAnywhere, Category = "Kimura", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "FrameControl == EKimuraPlayerFrameControl::PlaybackTime"))
	float						PlaybackTimeSpeed = 1.0f;
	
	/* When FrameControl is set to SequencerFrame, the playback position is controlled by specifying the frame number. This property can be controlled by a sequencer. */
	UPROPERTY(EditAnywhere, Interp, Category = "Kimura", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "FrameControl == EKimuraPlayerFrameControl::SequencerFrame"))
	float						SequencerFrame = 0.0;

	/* When FrameControl is set to SequencerTime, the playback position is controlled by specifying the time in seconds. This property can be controlled by a sequencer. */
	UPROPERTY(EditAnywhere, Interp, Category = "Kimura", meta = (ClampMin = "0.0", UIMin = "0.0", EditCondition = "FrameControl == EKimuraPlayerFrameControl::SequencerTime"))
	float						SequencerTime = 0.0f;

	/* Use this to reduce or augment the amount of velocity (motion blur) generated by this actor. The source content must include velocity data. This property can be controller by a sequencer. */
	UPROPERTY(EditAnywhere, Interp, Category = "Kimura", meta = (ClampMin = "0.0", UIMin = "0.0"))
	float						VelocityScale = 1.0f;

	/* Tells the current frame requested by the current playback control. */
	UPROPERTY(VisibleAnywhere, Category = "Kimura")
	int32						CurrentFrame = -1;

	/* How many frames are buffered ahead of the current frame. */
	UPROPERTY(VisibleAnywhere, Category = "Kimura")
	int32						NumFramesBuffered = -1;

	/* Whether textures from image sequences are SRGB. */
	UPROPERTY(EditAnywhere, Category = "Kimura", DisplayName = "sRGB")
	bool						sRGB = false;

	UPROPERTY(EditAnywhere, Category = "Kimura")
	TArray<FKimuraMeshInstance>		Meshes;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura")
	TArray<FKimuraImageSequenceInstance>	ImageSequences;

	TMap<FName, int>	ImageSequenceByName;


	/* Current version of the Kimura libraries used by this plugin. */
	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Playback Info")
	FString						PluginVersion;

	/* Version of the Kimura libraries used to generate the current .kabc file. */
	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Playback Info")
	FString						FileVersion;

	/* Number of frames per second provided by the current file. */
	UPROPERTY(Transient, VisibleAnywhere, Category="Kimura|Playback Info")
	float						FrameRate = 0;

	/* Total number of frames in the current file. */
	UPROPERTY(Transient, VisibleAnywhere, Category="Kimura|Playback Info")
	uint32						FrameCount = 0;

	/* Total duration of the playback (in seconds). */
	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Playback Info")
	float						Duration = 0.0f;

	/* Number of mesh components created for the current file.  */
	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Playback Info")
	uint32						NumMeshes = 0;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Playback Info")
	uint32						NumImageSequences = 0;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Profiling")
	FString						BytesReadInLastSecond;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Profiling")
	FString						BytesAllocatedForFrames;

// 	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Profiling")
// 	float						CurrentFrameReadTime = 0.0f;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Profiling")
	float						AverageReadTimePerFrame = 0.0;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Profiling")
	float						AverageProcessTimePerFrame = 0.0;

	UPROPERTY(Transient, VisibleAnywhere, Category = "Kimura|Profiling")
	TArray<FString>				SuggestedOptimizations;

	UPROPERTY()
	USceneComponent*			KimuraMeshRoot;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic*				DefaultMaterialInstance;

	const uint32							NumTextureResourcesPerImageSequence = 3;

	uint32									TextureResourceUpdateIndex = 0;
	uint32									TextureResourceDrawIndex = 0;

	bool									ResetPlayerRequested = false;
	int										ResetPlayerDelayFrames = 0;
	bool									MeshesAndTexturesReady = false;


	std::shared_ptr<Kimura::IPlayer>	KimuraPlayer = nullptr;
	Kimura::PlaybackInformation		PlaybackInfo;

	std::shared_ptr<Kimura::IFrame>	LastFrameSet = nullptr;

	bool SelectedInEditor = false;

};
