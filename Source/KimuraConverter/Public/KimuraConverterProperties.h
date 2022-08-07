//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "CoreMinimal.h"
#include <vector>
#include <string>
#include "KimuraConverterProperties.generated.h"

UENUM()
enum class EPositionFormats
{
	Full UMETA(DisplayName = "32-bits", ToolTip = "Full precision"),
	Half UMETA(DisplayName = "16-bits", ToolTip = "Faster but less precise for very wide meshes.")
};

UENUM()
enum class ENormalFormats
{
	Full UMETA(DisplayName = "32-bits", ToolTip = "Full precision normals, no tangent. Slow and unecessary in Unreal Engine"),
	Half UMETA(DisplayName = "16-bits", ToolTip = "High precision normals, no tangent"),
	Byte UMETA(DisplayName = "8-bits", ToolTip = "Low precision normals, no tangent"),
	None

};

UENUM()
enum class ETangentFormats
{
	Full UMETA(DisplayName = "32-bits", ToolTip = "Full precision tangent. Slow and unecessary in Unreal Engine"),
	Half UMETA(DisplayName = "16-bits", ToolTip = "High precision tangent"),
	Byte UMETA(DisplayName = "8-bits", ToolTip = "Low precision tangent"),
	None

};

UENUM()
enum class EVelocityFormats
{
	Full UMETA(DisplayName = "32-bits", ToolTip = "Full precision velocity."),
	Half UMETA(DisplayName = "16-bits", ToolTip = "Half precision velocity. Faster."),
	Byte UMETA(DisplayName = "8-bits", ToolTip = "Low precision velocity. Even faster."),
	None
};

UENUM()
enum class EColorFormats
{
	Full UMETA(DisplayName = "32-bits HDR", ToolTip = "Full precision HDR."),
	Half UMETA(DisplayName = "16-bits HDR", ToolTip = "Half precision HDR."),
	ByteHDR UMETA(DisplayName = "8-bits HDR", ToolTip = "Low precision HDR."),
	Byte UMETA(DisplayName = "8-bits", ToolTip = "Default precision."),
	None

};

UENUM()
enum class ETexCoordsFormats
{
	Full UMETA(DisplayName = "32-bits", ToolTip = "Full precision texture coords."),
	Half UMETA(DisplayName = "16-bits", ToolTip = "Half precision texture coords."),
	None

};

UENUM()
enum class EAxisSwizzle
{
	None,
	YZ,
	XZ
};

UENUM()
enum class EKimuraTextureFmt
{
	DXT1, 
	DXT3,
	DXT5
};

UENUM(BlueprintType)
enum class EKimuraTextureSize : uint8
{
	_16384	UMETA(DisplayName = "16384"),
	_8192	UMETA(DisplayName = "8192"),
	_4096	UMETA(DisplayName = "4096"),
	_2048	UMETA(DisplayName = "2048"),
	_1024	UMETA(DisplayName = "1024"),
	_512	UMETA(DisplayName = "512"),
	_256	UMETA(DisplayName = "256")
};

USTRUCT()
struct FKimuraImageProperties
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere)
	FFilePath Filename;

// 	UPROPERTY(EditAnywhere)
// 	EKimuraTextureSize MaximumSize = EKimuraTextureSize::_16384;

	UPROPERTY(EditAnywhere)
	int32 MaxSize = 16384;

	UPROPERTY(EditAnywhere)
	bool GenerateMipmaps = true;

	UPROPERTY(EditAnywhere)
	EKimuraTextureFmt	Format = EKimuraTextureFmt::DXT1;
	
};

UCLASS(Blueprintable)
class KIMURACONVERTER_API UKimuraConverterProperties : public UDataAsset
{
	GENERATED_UCLASS_BODY()

public:

	static UKimuraConverterProperties* GetDefault();

	/* File to convert from. Must point to an alembic(.abc) file. */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Files, meta=(FilePathFilter="Alembic|*.abc"))
	FFilePath InputFile;

	/* File to convert to (.k).*/
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Files, meta = (FilePathFilter = "Kimura|*.k"))
	FFilePath OutputFile;

	/* Number of frames that can be converted concurrently. By default, it tries to be set to the number of cores available. If you need to process extremely large frame and run out of memory (or the system has to swap memory), try reducing this number. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	int					CPU = 1;

	/* Scale the geometry by this value during conversion. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0.001", UIMin = "0.001"), Category = Arguments)
	float				Scale = 1.0f;

	/* The frame index at which the conversion process will start. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "0", UIMin = "0"), Category = Arguments)
	uint32				StartFrame = 0;

	/* The frame index at which the conversion process will end. */
	UPROPERTY(EditAnywhere, meta = (ClampMin = "1", UIMin = "1"), Category = Arguments)
	uint32				EndFrame = 99999999;

	/* Split and optimize large meshes into groups of 64k vertices and 16bit index buffers. 
	This optimization can lead to significant reduction in file size for very large meshes where 
	triangles share multiple vertices (ex: fluid simulations). However, in some cases it might 
	lead to larger meshes! Make sure to compare the results. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	bool				SplitMeshes = false;

	/* Format in which the positions are stored. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	EPositionFormats	PositionFormat = EPositionFormats::Full;

	/* Format in which the vertex normals are stored. The input file must contain normals. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	ENormalFormats		NormalFormat = ENormalFormats::Half;

	/* Format in which the vertex normals tangents are stored. The input file must contain normals and texture coords. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	ETangentFormats		TangentFormat = ETangentFormats::Half;

	/* Format in which the velocities are stored. The input file must contain velocity data. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	EVelocityFormats	VelocityFormat = EVelocityFormats::Byte;

	/* Format in which the colors are stored. The input file must contain color data. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	EColorFormats		ColorFormat = EColorFormats::Byte;

	/* Format in which the texture coordinates are stored. The input file must contain texture coords. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	ETexCoordsFormats	TexcoordFormat = ETexCoordsFormats::Full;

	/* Swap axises. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	EAxisSwizzle		Swizzle = EAxisSwizzle::YZ;

	/* Whether triangle indices specify triangle points in clockwise or counter clockwise order. */
	UPROPERTY(EditAnywhere, Category = Arguments)
	bool				FlipTriangleOrder = true;

	/* Whether to flips the V component of the texture coords (UV). V = 1.0 - V */
	UPROPERTY(EditAnywhere, Category = Arguments)
	bool				FlipTexCoordVertically = false;

	UPROPERTY(EditAnywhere, Category = Arguments)
	TArray<FKimuraImageProperties>		Images;

#if WITH_EDITOR
	//Called when a property on this object has been modified externally
	virtual void PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void GenerateArgs(TArray<FString>& OutArgv);
	void GenerateArgs(std::vector<std::string>& OutArgv);

};
