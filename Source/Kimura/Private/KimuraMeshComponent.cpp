//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "KimuraMeshComponent.h"
#include "KimuraMeshSceneProxy.h"
#include "KimuraModule.h"
#include "Kimura.h"
#include "KimuraPlayer.h"
#include "Materials/MaterialInstanceDynamic.h"

//-----------------------------------------------------------------------------
// UKimuraMeshComponent::UKimuraMeshComponent
//-----------------------------------------------------------------------------
UKimuraMeshComponent::UKimuraMeshComponent(const FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{

}


//-----------------------------------------------------------------------------
// UKimuraMeshComponent::Initialize
//-----------------------------------------------------------------------------
void UKimuraMeshComponent::Initialize
	(
		
		const Kimura::MeshInformation& InMeshInfo,
		UMaterialInstanceDynamic* InMaterialInstance
		
	)
{

	this->MeshInfo = InMeshInfo;

	this->MaterialInstance = InMaterialInstance;

	// UMeshComponent::SetMaterial
	Super::SetMaterial(0, InMaterialInstance);

}


//-----------------------------------------------------------------------------
// UKimuraMeshComponent::UKimuraMeshComponent
//-----------------------------------------------------------------------------
FPrimitiveSceneProxy* UKimuraMeshComponent::CreateSceneProxy()
{
	if (this->MaterialInstance == nullptr)
	{
		return nullptr;
	}

	this->KimuraSceneProxy = new FKimuraPrimitiveSceneProxy(this);

	this->SceneProxy = this->KimuraSceneProxy;

	return this->SceneProxy;
}


//-----------------------------------------------------------------------------
// UKimuraMeshComponent::SetMaterial
//-----------------------------------------------------------------------------
void UKimuraMeshComponent::SetMaterial(int32 ElementIndex, UMaterialInterface* Material)
{
	UE_LOG(KimuraLog, Log, TEXT("UKimuraMeshComponent::SetMaterial"), *this->GetName());

#if WITH_EDITOR
	
	if (GetWorld() != nullptr && GetWorld()->WorldType == EWorldType::Editor)
	{
		AKimuraPlayer* pPlayer = Cast<AKimuraPlayer>(this->GetOwner());
		if (pPlayer)
		{
			pPlayer->SetDefaultMaterial(Material);
		}
	}

#endif

	//Super::SetMaterial(ElementIndex, Material);
}


//-----------------------------------------------------------------------------
// UKimuraMeshComponent::GetNumMaterials
//-----------------------------------------------------------------------------
int32 UKimuraMeshComponent::GetNumMaterials() const
{
	return 1;
}


//-----------------------------------------------------------------------------
// UKimuraMeshComponent::EndPlay
//-----------------------------------------------------------------------------
void UKimuraMeshComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}


//-----------------------------------------------------------------------------
// UKimuraMeshComponent::UpdateFrame
//-----------------------------------------------------------------------------
void UKimuraMeshComponent::UpdateFrame(std::shared_ptr<Kimura::IFrame> InFrame, const KimuraMeshFrameParams& InAdditionalParams)
{

	//UE_LOG(KimuraLog, Log, TEXT("UKimuraMeshComponent::UpdateFrame"));

	if (this->KimuraSceneProxy == nullptr)
	{
		return;
	}

	if (this->SceneProxy == nullptr)
	{
		return;
	}

	if (!this->IsVisible())
	{
		return;
	}

	this->CurrentFrame = InFrame;

	// update bounds
	if (InFrame)
	{		
		Kimura::Vector3 boundsCenter = Kimura::Vector3::ZeroVector;
		Kimura::Vector3 boundSize = Kimura::Vector3::ZeroVector;
		InFrame->GetBounds(this->MeshInfo.Index, boundsCenter, boundSize);

		FVector vBoxOrigin = FVector(boundsCenter.X, boundsCenter.Y, boundsCenter.Z);
		FVector vBoxExtent = FVector(boundSize.X, boundSize.Y, boundSize.Z);

		this->Bounds = FBoxSphereBounds(vBoxOrigin, vBoxExtent, vBoxExtent.Size());

		this->UpdateBounds();
		this->UpdateComponentToWorld();

	}

	// enqueue command to be executed from the render thread. 
	ENQUEUE_RENDER_COMMAND(FKimuraMeshUpdate)(
		[this, InFrame, InAdditionalParams](FRHICommandListImmediate& RHICmdList)
	{
		this->KimuraSceneProxy->UpdateFrame(InFrame, MeshInfo.Index, InAdditionalParams);
	});

}


//-----------------------------------------------------------------------------
// UKimuraMeshComponent::CalcBounds
//-----------------------------------------------------------------------------
FBoxSphereBounds UKimuraMeshComponent::CalcBounds(const FTransform& LocalToWorld) const
{
	return Bounds.TransformBy(LocalToWorld);
}

