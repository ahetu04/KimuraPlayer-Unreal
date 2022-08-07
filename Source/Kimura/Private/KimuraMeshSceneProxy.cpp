//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "KimuraMeshSceneProxy.h"
#include "KimuraMeshComponent.h"
#include "KimuraMeshRenderResources.h"
#include "Materials/MaterialInstanceDynamic.h"

//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::FKimuraPrimitiveSceneProxy
//-----------------------------------------------------------------------------
FKimuraPrimitiveSceneProxy::FKimuraPrimitiveSceneProxy(class UKimuraMeshComponent* KimuraMeshComponent)
	:
	FPrimitiveSceneProxy(KimuraMeshComponent),
	MaterialRelevance(KimuraMeshComponent->GetMaterialRelevance(GetScene().GetFeatureLevel()))
{
	
	this->MaterialInstance = KimuraMeshComponent->MaterialInstance;

	for (int iFrame = 0; iFrame < NumGeometryFrames; iFrame++)
	{
		FKimuraFrameGeometry* pFrameGeometry = new FKimuraFrameGeometry(GetScene().GetFeatureLevel(), KimuraMeshComponent->MeshInfo, "FKimuraFrameGeometry");
		this->GeometryFrames.Add(pFrameGeometry);
	}
}


//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::~FKimuraPrimitiveSceneProxy
//-----------------------------------------------------------------------------
FKimuraPrimitiveSceneProxy::~FKimuraPrimitiveSceneProxy()
{
	// clean up the geometry frames
	{
		for (int iFrame = 0; iFrame < NumGeometryFrames; iFrame++)
		{
			if (this->GeometryFrames[iFrame] != nullptr)
			{
				delete this->GeometryFrames[iFrame];
			}
		}

		this->GeometryFrames.Empty();
	}
}


//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::GetTypeHash
//-----------------------------------------------------------------------------
SIZE_T FKimuraPrimitiveSceneProxy::GetTypeHash() const
{
	static size_t FKimuraPrimitiveSceneProxy_UniquePointer;
	return reinterpret_cast<size_t>(&FKimuraPrimitiveSceneProxy_UniquePointer);
}


//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::GetDynamicMeshElements
//-----------------------------------------------------------------------------
void FKimuraPrimitiveSceneProxy::GetDynamicMeshElements(const TArray<const FSceneView*>& InViews, const FSceneViewFamily& InViewFamily, uint32 InVisibilityMap, FMeshElementCollector& OutCollector) const
{

	FKimuraFrameGeometry* pFrameGeometry = this->GeometryFrames[this->GeometryFrameDrawIndex];

	// no triangles to render
	if (pFrameGeometry->NumSurfacesSet == 0)
	{
		return;
	}



	// Set up wireframe material (if needed)
	const bool bWireframe = AllowDebugViewmodes() && InViewFamily.EngineShowFlags.Wireframe;

	FColoredMaterialRenderProxy* wireframeMaterialInstance = NULL;
	if (bWireframe)
	{
		wireframeMaterialInstance = new FColoredMaterialRenderProxy(
			GEngine->WireframeMaterial ? GEngine->WireframeMaterial->GetRenderProxy() : NULL,
			FLinearColor(0, 0.5f, 1.f)
		);

		OutCollector.RegisterOneFrameMaterialProxy(wireframeMaterialInstance);
	}

	FMaterialRenderProxy* MaterialProxy = (this->MaterialInstance == nullptr || bWireframe )? wireframeMaterialInstance : this->MaterialInstance->GetRenderProxy();

	// For each view..
	for (int32 ViewIndex = 0; ViewIndex < InViews.Num(); ViewIndex++)
	{
		if (InVisibilityMap & (1 << ViewIndex))
		{
			const FSceneView* View = InViews[ViewIndex];

			// setup the mesh
			FMeshBatch& mesh = OutCollector.AllocateMesh();
			{
				mesh.bWireframe = bWireframe;
				mesh.VertexFactory = &pFrameGeometry->KimuraVertexFactory;

				mesh.MaterialRenderProxy = MaterialProxy;
				mesh.ReverseCulling = 0;
				mesh.Type = PT_TriangleList;
				mesh.DepthPriorityGroup = SDPG_World;
				mesh.bCanApplyViewModeOverrides = false;
			}


			bool bHasPrecomputedVolumetricLightmap;
			FMatrix previousLocalToWorld;
			int32 singleCaptureIndex;
			bool outputVelocity;
			GetScene().GetPrimitiveUniformShaderParameters_RenderThread(GetPrimitiveSceneInfo(), bHasPrecomputedVolumetricLightmap, previousLocalToWorld, singleCaptureIndex, outputVelocity);

			mesh.Elements.SetNum((int32)pFrameGeometry->Sections.size());

			uint32 iMeshElement = 0;
			for (Kimura::MeshSection& s : pFrameGeometry->Sections)
			{

				FMeshBatchElement& batchElement = mesh.Elements[iMeshElement];

				if (pFrameGeometry->NumVerticesSet <= 0xfffe || pFrameGeometry->MeshInfo.Force16BitIndices)
				{
					batchElement.IndexBuffer = &pFrameGeometry->SurfaceIndices16;
				}
				else
				{
					batchElement.IndexBuffer = &pFrameGeometry->SurfaceIndices32;
				}

				FDynamicPrimitiveUniformBuffer& dynamicPrimitiveUniformBuffer = OutCollector.AllocateOneFrameResource<FDynamicPrimitiveUniformBuffer>();

#if ENGINE_MAJOR_VERSION >= 5
				dynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), previousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, outputVelocity);
#else
				dynamicPrimitiveUniformBuffer.Set(GetLocalToWorld(), previousLocalToWorld, GetBounds(), GetLocalBounds(), true, bHasPrecomputedVolumetricLightmap, DrawsVelocity(), outputVelocity);
#endif
				batchElement.PrimitiveUniformBuffer = dynamicPrimitiveUniformBuffer.UniformBuffer.GetUniformBufferRHI();

				batchElement.BaseVertexIndex = s.VertexStart;
				batchElement.FirstIndex = s.IndexStart;
				batchElement.NumPrimitives = s.NumSurfaces;
				batchElement.MinVertexIndex = s.MinVertexIndex;
				batchElement.MaxVertexIndex = s.MaxVertexIndex;

				iMeshElement++;

			}

			OutCollector.AddMesh(ViewIndex, mesh);

			//UE_LOG(KimuraLog, Log, TEXT("GetDynamicMeshElements: mesh added to collector"));

		}

	}

	// Draw bounds
#if !(UE_BUILD_SHIPPING || UE_BUILD_TEST)
	for (int32 viewIndex = 0; viewIndex < InViews.Num(); viewIndex++)
	{
		if (InVisibilityMap & (1 << viewIndex))
		{
			// Render bounds
			RenderBounds(OutCollector.GetPDI(viewIndex), InViewFamily.EngineShowFlags, GetBounds(), IsSelected());
		}
	}
#endif

}


//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::GetViewRelevance
//-----------------------------------------------------------------------------
FPrimitiveViewRelevance FKimuraPrimitiveSceneProxy::GetViewRelevance(const FSceneView* View) const
{
	FPrimitiveViewRelevance result;
	result.bDrawRelevance = IsShown(View);
	result.bShadowRelevance = IsShadowCast(View);
	result.bDynamicRelevance = true;
	result.bRenderInMainPass = ShouldRenderInMainPass();
	result.bUsesLightingChannels = GetLightingChannelMask() != GetDefaultLightingChannelMask();
	result.bRenderCustomDepth = ShouldRenderCustomDepth();
	MaterialRelevance.SetPrimitiveViewRelevance(result);

	//bhgfeywfb
	result.bVelocityRelevance = IsMovable() && result.bOpaque && result.bRenderInMainPass;

	return result;
}


//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::CanBeOccluded
//-----------------------------------------------------------------------------
bool FKimuraPrimitiveSceneProxy::CanBeOccluded() const
{
	return !MaterialRelevance.bDisableDepthTest;
}


//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::GetMemoryFootprint
//-----------------------------------------------------------------------------
uint32 FKimuraPrimitiveSceneProxy::GetMemoryFootprint(void) const
{
	return(sizeof(*this) + GetAllocatedSize());
}


//-----------------------------------------------------------------------------
// FKimuraPrimitiveSceneProxy::UpdateFrame
//-----------------------------------------------------------------------------
void FKimuraPrimitiveSceneProxy::UpdateFrame(std::shared_ptr<Kimura::IFrame> InFrame, uint32 iMesh, const KimuraMeshFrameParams& InFrameParams)
{

	//UE_LOG(KimuraLog, Log, TEXT("FKimuraPrimitiveSceneProxy::UpdateFrame"));

	check(IsInRenderingThread());

	// get next available geometry frame
	FKimuraFrameGeometry* pFrameGeometryToUpdate = this->GeometryFrames[this->GeometryFrameUpdateIndex];

	// update it with the content of the frame 
	pFrameGeometryToUpdate->UpdateFromKimuraFrame(InFrame, iMesh, InFrameParams);

	// next draw will use the geometry frame 
	this->GeometryFrameDrawIndex = this->GeometryFrameUpdateIndex;

	this->GeometryFrameUpdateIndex++;
	this->GeometryFrameUpdateIndex %= this->GeometryFrames.Num();

}

