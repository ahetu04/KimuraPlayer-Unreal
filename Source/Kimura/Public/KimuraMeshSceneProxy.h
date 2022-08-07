//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "PrimitiveSceneProxy.h"
#include "KimuraMeshFrameParams.h"
#include <memory>
#include "Kimura.h"

//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy
//-----------------------------------------------------------------------------
class FKimuraPrimitiveSceneProxy : public FPrimitiveSceneProxy
{

	friend class UKimuraMeshComponent;

	protected:

		FKimuraPrimitiveSceneProxy(class UKimuraMeshComponent* KimuraMeshComponent);
		virtual ~FKimuraPrimitiveSceneProxy();

		SIZE_T GetTypeHash() const override;

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override;
		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const;
		virtual bool CanBeOccluded() const override;

		virtual uint32 GetMemoryFootprint(void) const;

		void UpdateFrame(std::shared_ptr<Kimura::IFrame> InFrame, uint32 iMesh, const KimuraMeshFrameParams& InFrameParams);

		UMaterialInstanceDynamic*				MaterialInstance = nullptr;

		FMaterialRelevance						MaterialRelevance;

		const int NumGeometryFrames = 4;
		TArray<class FKimuraFrameGeometry*>		GeometryFrames;

		uint32									GeometryFrameUpdateIndex = 0;
		uint32									GeometryFrameDrawIndex = 0;

		UKimuraMeshComponent*					MeshComponent;

};