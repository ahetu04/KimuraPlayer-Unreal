//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "KimuraConverterProperties.h"
#include "UObject/Class.h"
#include "UObject/Package.h"
#include "UObject/UnrealType.h"
#include "Misc/Paths.h"

#include <thread>


//-----------------------------------------------------------------------------
// UKimuraConverterProperties::UKimuraConverterProperties
//-----------------------------------------------------------------------------
UKimuraConverterProperties::UKimuraConverterProperties(const FObjectInitializer& ObjectInitializer)
: Super(ObjectInitializer)
{
}


//-----------------------------------------------------------------------------
// UKimuraConverterProperties::GetDefault
//-----------------------------------------------------------------------------
UKimuraConverterProperties* UKimuraConverterProperties::GetDefault()
{
	static UKimuraConverterProperties* DefaultSettings = nullptr;
	if (!DefaultSettings)
	{
		// This is a singleton, use default object
		DefaultSettings = DuplicateObject(GetMutableDefault<UKimuraConverterProperties>(), GetTransientPackage());
		DefaultSettings->AddToRoot();


		DefaultSettings->CPU = std::thread::hardware_concurrency() / 2;

	}

	return DefaultSettings;

}


//-----------------------------------------------------------------------------
// UKimuraConverterProperties::PostEditChangeProperty
//-----------------------------------------------------------------------------
void UKimuraConverterProperties::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{

	Super::PostEditChangeProperty(PropertyChangedEvent);

	FName PropertyName = NAME_None;
	if (PropertyChangedEvent.Property != nullptr)
	{
		PropertyName = PropertyChangedEvent.MemberProperty->GetFName();
	}

	// Make sure that the filename is relative to the project's content dir, so that it can be packaged as a movie in a 
	// standalone package. 
	if ((PropertyName == GET_MEMBER_NAME_CHECKED(UKimuraConverterProperties, InputFile)) && !this->InputFile.FilePath.IsEmpty())
	{
		if (!this->InputFile.FilePath.EndsWith(".abc"))
		{
			this->InputFile.FilePath = "";
		}
		else
		{
			this->OutputFile.FilePath = FPaths::ChangeExtension(this->InputFile.FilePath, ".k");
		}
	}
}


//-----------------------------------------------------------------------------
// UKimuraConverterProperties::GenerateArgs
//-----------------------------------------------------------------------------
void UKimuraConverterProperties::GenerateArgs(TArray<FString>& OutArgv)
{

	OutArgv.Add(FString::Printf(TEXT("i:%s"), *this->InputFile.FilePath));
	OutArgv.Add(FString::Printf(TEXT("o:%s"), *this->OutputFile.FilePath));

	OutArgv.Add(FString::Printf(TEXT("scale:%f"), this->Scale));

	OutArgv.Add(FString::Printf(TEXT("start:%d"), this->StartFrame));
	OutArgv.Add(FString::Printf(TEXT("end:%d"), this->EndFrame));

	OutArgv.Add(FString::Printf(TEXT("cpu:%d"), this->CPU));

	OutArgv.Add(FString::Printf(TEXT("split:%s"), (this->SplitMeshes ? TEXT("true") : TEXT("false"))));

	switch (this->PositionFormat)
	{
		case EPositionFormats::Full:	OutArgv.Add(FString::Printf(TEXT("pFmt:%s"), TEXT("full"))); break;
		case EPositionFormats::Half:	OutArgv.Add(FString::Printf(TEXT("pFmt:%s"), TEXT("half"))); break;
	}

	switch (this->NormalFormat)
	{
		case ENormalFormats::Full:	OutArgv.Add(FString::Printf(TEXT("nFmt:%s"), TEXT("full"))); break;
		case ENormalFormats::Half:	OutArgv.Add(FString::Printf(TEXT("nFmt:%s"), TEXT("half"))); break;
		case ENormalFormats::Byte:	OutArgv.Add(FString::Printf(TEXT("nFmt:%s"), TEXT("byte"))); break;
		case ENormalFormats::None:	OutArgv.Add(FString::Printf(TEXT("nFmt:%s"), TEXT("none"))); break;
	}

	switch (this->TangentFormat)
	{
		case ETangentFormats::Full:	OutArgv.Add(FString::Printf(TEXT("ntFmt:%s"), TEXT("full"))); break;
		case ETangentFormats::Half:	OutArgv.Add(FString::Printf(TEXT("ntFmt:%s"), TEXT("half"))); break;
		case ETangentFormats::Byte:	OutArgv.Add(FString::Printf(TEXT("ntFmt:%s"), TEXT("byte"))); break;
		case ETangentFormats::None:	OutArgv.Add(FString::Printf(TEXT("ntFmt:%s"), TEXT("none"))); break;
	}

	switch (this->VelocityFormat)
	{
		case EVelocityFormats::Full:	OutArgv.Add(FString::Printf(TEXT("vFmt:%s"), TEXT("full"))); break;
		case EVelocityFormats::Half:	OutArgv.Add(FString::Printf(TEXT("vFmt:%s"), TEXT("half"))); break;
		case EVelocityFormats::Byte:	OutArgv.Add(FString::Printf(TEXT("vFmt:%s"), TEXT("byte"))); break;
		case EVelocityFormats::None:	OutArgv.Add(FString::Printf(TEXT("vFmt:%s"), TEXT("none"))); break;
	}

	switch (this->ColorFormat)
	{
 		case EColorFormats::Full:		OutArgv.Add(FString::Printf(TEXT("cFmt:%s"), TEXT("full"))); break;
 		case EColorFormats::Half:		OutArgv.Add(FString::Printf(TEXT("cFmt:%s"), TEXT("half"))); break;
		case EColorFormats::Byte:		OutArgv.Add(FString::Printf(TEXT("cFmt:%s"), TEXT("byte"))); break;
		case EColorFormats::ByteHDR:	OutArgv.Add(FString::Printf(TEXT("cFmt:%s"), TEXT("bytehdr"))); break;
		case EColorFormats::None:		OutArgv.Add(FString::Printf(TEXT("cFmt:%s"), TEXT("none"))); break;
	}

	switch (this->TexcoordFormat)
	{
		case ETexCoordsFormats::Full:	OutArgv.Add(FString::Printf(TEXT("tFmt:%s"), TEXT("full"))); break;
		case ETexCoordsFormats::Half:	OutArgv.Add(FString::Printf(TEXT("tFmt:%s"), TEXT("half"))); break;
		case ETexCoordsFormats::None:	OutArgv.Add(FString::Printf(TEXT("tFmt:%s"), TEXT("none"))); break;
	}

	switch (this->Swizzle)
	{
		case EAxisSwizzle::None:	OutArgv.Add(FString::Printf(TEXT("swizzle:%s"), TEXT("none"))); break;
		case EAxisSwizzle::YZ:		OutArgv.Add(FString::Printf(TEXT("swizzle:%s"), TEXT("yz"))); break;
		case EAxisSwizzle::XZ:		OutArgv.Add(FString::Printf(TEXT("swizzle:%s"), TEXT("xz"))); break;
	}

	OutArgv.Add(FString::Printf(TEXT("flip:%s"), (this->FlipTriangleOrder ? TEXT("true") : TEXT("false"))));
	OutArgv.Add(FString::Printf(TEXT("flipUV:%s"), (this->FlipTexCoordVertically ? TEXT("true") : TEXT("false"))));

	int i=0;
	for (const FKimuraImageProperties& image : this->Images)
	{

		FString s = FPaths::ConvertRelativePathToFull(image.Filename.FilePath);
		FPaths::NormalizeFilename(s);
		OutArgv.Add(FString::Printf(TEXT("image%d:%s"), i, *s));


		switch (image.Format)
		{
			case EKimuraTextureFmt::DXT1:
			{
				OutArgv.Add(FString::Printf(TEXT("image%dfmt:DXT1"), i));
				break;
			}

			case EKimuraTextureFmt::DXT3:
			{
				OutArgv.Add(FString::Printf(TEXT("image%dfmt:DXT3"), i));
				break;
			}

			case EKimuraTextureFmt::DXT5:
			{
				OutArgv.Add(FString::Printf(TEXT("image%dfmt:DXT5"), i));
				break;
			}

			default:
			{
				OutArgv.Add(FString::Printf(TEXT("image%dfmt:DXT1"), i));
			}
		}

		OutArgv.Add(FString::Printf(TEXT("image%dmips:%s"), i, (image.GenerateMipmaps ? TEXT("true") : TEXT("false"))));

		int maxSize = 8192;
/*
		switch (image.MaximumSize)
		{
			case EKimuraTextureSize::_256: maxSize = 256; break;
			case EKimuraTextureSize::_512: maxSize = 512; break;
			case EKimuraTextureSize::_1024: maxSize = 1024; break;
			case EKimuraTextureSize::_2048: maxSize = 2048; break;
			case EKimuraTextureSize::_4096: maxSize = 4096; break;
			case EKimuraTextureSize::_8192: maxSize = 8192; break;
			case EKimuraTextureSize::_16384: maxSize = 16384; break;

			default:
				maxSize = 16384;
				break;
		}
*/

		maxSize = image.MaxSize;

		OutArgv.Add(FString::Printf(TEXT("image%dsize:%d"), i, maxSize));

	}
}


//-----------------------------------------------------------------------------
// UKimuraConverterProperties::GenerateArgs
//-----------------------------------------------------------------------------
void UKimuraConverterProperties::GenerateArgs(std::vector<std::string>& OutArgv)
{

	TArray<FString>	a;
	this->GenerateArgs(a);

	for (const FString& s : a)
	{
		OutArgv.push_back(TCHAR_TO_UTF8(*s));
	}
}

