//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "CoreMinimal.h"
#include "Kimura.h"
#include <memory>
#include "Components/MeshComponent.h"
#include "KimuraMeshFrameParams.h"
#include "KimuraMeshComponent.generated.h"

class FPrimitiveSceneProxy;
class FKimuraPrimitiveSceneProxy;


//-----------------------------------------------------------------------------
// UKimuraMeshComponent
//-----------------------------------------------------------------------------
UCLASS()
class KIMURA_API UKimuraMeshComponent : public UMeshComponent
{

	GENERATED_UCLASS_BODY()

	friend class AKimuraPlayer;
	friend class FKimuraPrimitiveSceneProxy;

public:

	void Initialize(const Kimura::MeshInformation& InMeshInfo, UMaterialInstanceDynamic* InMaterialInstance);
	virtual FPrimitiveSceneProxy* CreateSceneProxy() override;

	virtual void SetMaterial(int32 ElementIndex, UMaterialInterface* Material) override;

	virtual int32 GetNumMaterials() const override;

	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
	void UpdateFrame(std::shared_ptr<Kimura::IFrame> InFrame, const KimuraMeshFrameParams& InAdditionalParams);

protected:

    virtual FBoxSphereBounds CalcBounds(const FTransform& LocalToWorld) const override;

	Kimura::MeshInformation			MeshInfo;

	UPROPERTY(Transient)
	UMaterialInstanceDynamic*				MaterialInstance = nullptr;


	FKimuraPrimitiveSceneProxy*				KimuraSceneProxy = nullptr;

	std::shared_ptr<Kimura::IFrame>	CurrentFrame = nullptr;

	FBoxSphereBounds						Bounds;

};


