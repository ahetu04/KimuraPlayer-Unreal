//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "KimuraMeshVertexFactory.h"
#include "KimuraModule.h"
#include "SceneView.h"
#include "ShaderParameterUtils.h"
#include "MeshMaterialShader.h"

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FKimuraVertexFactoryUniformBufferParameters, "Kimura");

//-----------------------------------------------------------------------------
// FKimuraVertexFactoryShaderParameters
//-----------------------------------------------------------------------------
class FKimuraVertexFactoryShaderParameters : public FVertexFactoryShaderParameters
{

	/** Shader parameters for use with KimuraVertexFactory*/

	DECLARE_TYPE_LAYOUT(FKimuraVertexFactoryShaderParameters, NonVirtual);
 	

public:

	//-----------------------------------------------------------------------------
	// FKimuraVertexFactoryShaderParameters::Bind
	//-----------------------------------------------------------------------------
	void Bind(const FShaderParameterMap& ParameterMap)
	{
		PositionQuantCenter.Bind(ParameterMap, TEXT("PositionQuantCenter"));
		PositionQuantExtents.Bind(ParameterMap, TEXT("PositionQuantExtents"));
		VelocityQuantCenter.Bind(ParameterMap, TEXT("VelocityQuantCenter"));
		VelocityQuantExtents.Bind(ParameterMap, TEXT("VelocityQuantExtents"));
		VelocityScale.Bind(ParameterMap, TEXT("VelocityScale"));
		ColorQuantExtents.Bind(ParameterMap, TEXT("ColorQuantExtents"));
	}

	//-----------------------------------------------------------------------------
	// FKimuraVertexFactoryShaderParameters::GetElementShaderBindings
	//-----------------------------------------------------------------------------
	void GetElementShaderBindings
		(

			const class FSceneInterface* Scene,
			const FSceneView* View,
			const class FMeshMaterialShader* Shader,
			const EVertexInputStreamType InputStreamType,
			ERHIFeatureLevel::Type FeatureLevel,
			const FVertexFactory* GenericVertexFactory,
			const FMeshBatchElement& BatchElement,
			class FMeshDrawSingleShaderBindings& ShaderBindings,
			FVertexInputStreamArray& VertexStreams
		
		) const
	{
		// Ensure the vertex factory matches this parameter object and cast relevant objects
		check(GenericVertexFactory->GetType() == &FKimuraVertexFactory::StaticType);
		const FKimuraVertexFactory* pKimuraVF = static_cast<const FKimuraVertexFactory*>(GenericVertexFactory);

		ShaderBindings.Add(PositionQuantCenter, pKimuraVF->PositionQuantCenter);
		ShaderBindings.Add(PositionQuantExtents, pKimuraVF->PositionQuantExtents);
		ShaderBindings.Add(VelocityQuantCenter, pKimuraVF->VelocityQuantCenter);
		ShaderBindings.Add(VelocityQuantExtents, pKimuraVF->VelocityQuantExtents);
		ShaderBindings.Add(VelocityScale, pKimuraVF->VelocityScale);
		ShaderBindings.Add(ColorQuantExtents, pKimuraVF->ColorQuantExtents);

		ShaderBindings.Add(Shader->GetUniformBufferParameter<FKimuraVertexFactoryUniformBufferParameters>(), pKimuraVF->UniformBufferParams);
	}

private:

	LAYOUT_FIELD(FShaderParameter, PositionQuantCenter);
	LAYOUT_FIELD(FShaderParameter, PositionQuantExtents);
	LAYOUT_FIELD(FShaderParameter, VelocityQuantCenter);
	LAYOUT_FIELD(FShaderParameter, VelocityQuantExtents);
	LAYOUT_FIELD(FShaderParameter, VelocityScale);
	LAYOUT_FIELD(FShaderParameter, ColorQuantExtents);

};

IMPLEMENT_TYPE_LAYOUT(FKimuraVertexFactoryShaderParameters);


//-----------------------------------------------------------------------------
// FKimuraVertexFactory::ModifyCompilationEnvironment
//-----------------------------------------------------------------------------
void FKimuraVertexFactory::ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& InParameters, FShaderCompilerEnvironment& OutEnvironment)
{
	Super::ModifyCompilationEnvironment(InParameters, OutEnvironment);

#if ENGINE_MAJOR_VERSION >= 5
	OutEnvironment.SetDefine(TEXT("KIMURA_UE5"), 1);
#endif

}


//-----------------------------------------------------------------------------
// FKimuraVertexFactory::SetData
//-----------------------------------------------------------------------------
void FKimuraVertexFactory::SetData(const FDataType& InData)
{
	check(IsInRenderingThread());

	this->Data = InData;

	UpdateRHI();
}


//-----------------------------------------------------------------------------
// FKimuraVertexFactory::InitRHI
//-----------------------------------------------------------------------------
void FKimuraVertexFactory::InitRHI()
{
	// position only
	{
		FVertexDeclarationElementList PositionOnlyStreamElements;
		PositionOnlyStreamElements.Add(AccessStreamComponent(Data.PositionComponent, 0, EVertexInputStreamType::PositionOnly));
		InitDeclaration(PositionOnlyStreamElements, EVertexInputStreamType::PositionOnly);
	}

	// position and normal only
	{
		FVertexDeclarationElementList PositionAndNormalOnlyStreamElements;
		PositionAndNormalOnlyStreamElements.Add(AccessStreamComponent(Data.PositionComponent, 0, EVertexInputStreamType::PositionAndNormalOnly));
		PositionAndNormalOnlyStreamElements.Add(AccessStreamComponent(Data.NormalComponent, 1, EVertexInputStreamType::PositionAndNormalOnly));
		InitDeclaration(PositionAndNormalOnlyStreamElements, EVertexInputStreamType::PositionAndNormalOnly);
	}

	// kimura vertex 
	{
		// positions
		FVertexDeclarationElementList Elements;
		if (Data.PositionComponent.VertexBuffer != NULL)
		{
			Elements.Add(AccessStreamComponent(Data.PositionComponent, 0));
		}

		// tangents
		if (Data.TangentComponent.VertexBuffer != NULL)
		{
			Elements.Add(AccessStreamComponent(Data.TangentComponent, 1));
		}

		// normals
		if (Data.NormalComponent.VertexBuffer != NULL)
		{
			Elements.Add(AccessStreamComponent(Data.NormalComponent, 2));
		}

		// colors
		if (Data.ColorComponent.VertexBuffer)
		{
			Elements.Add(AccessStreamComponent(Data.ColorComponent, 3));
		}
		else
		{
			//If the mesh has no color component, set the null color buffer on a new stream with a stride of 0.
			//This wastes 4 bytes of bandwidth per vertex, but prevents having to compile out twice the number of vertex factories.
			FVertexStreamComponent NullColorComponent(&GNullColorVertexBuffer, 0, 0, VET_Color);
			Elements.Add(AccessStreamComponent(NullColorComponent, 3));
		}

		// velocities
		if (Data.VelocityComponent.VertexBuffer)
		{
			Elements.Add(AccessStreamComponent(Data.VelocityComponent, 4));
		}

		// texture coords
		if (Data.TexCoordsComponents.Num())
		{
			const int32 BaseTexCoordAttribute = 5;
			for (int32 CoordinateIndex = 0; CoordinateIndex < Data.TexCoordsComponents.Num(); CoordinateIndex++)
			{
				Elements.Add(AccessStreamComponent(
					Data.TexCoordsComponents[CoordinateIndex],
					BaseTexCoordAttribute + CoordinateIndex
				));
			}

			for (int32 CoordinateIndex = Data.TexCoordsComponents.Num(); CoordinateIndex < MAX_STATIC_TEXCOORDS / 2; CoordinateIndex++)
			{
				Elements.Add(AccessStreamComponent(
					Data.TexCoordsComponents[Data.TexCoordsComponents.Num() - 1],
					BaseTexCoordAttribute + CoordinateIndex
				));
			}
		}

		check(this->Streams.Num() > 0);

		this->InitDeclaration(Elements);

		check(IsValidRef(this->GetDeclaration()));
	}

	// Create uniform shader params to be updated every frame.
	{
		FKimuraVertexFactoryUniformBufferParameters p;
		p.PositionQuantCenter = FKimuraVector3::ZeroVector;
		p.PositionQuantExtents = FKimuraVector3::OneVector;
		p.VelocityQuantCenter = FKimuraVector3::ZeroVector;
		p.VelocityQuantExtents = FKimuraVector3::OneVector;
		p.VelocityScale = 1.0f;
		p.ColorQuantExtents = FKimuraVector4(1.0f);
		this->UniformBufferParams = FKimuraVertexFactoryUniformBufferParametersRef::CreateUniformBufferImmediate(p, UniformBuffer_MultiFrame);

	}

}


//-----------------------------------------------------------------------------
// FKimuraVertexFactory::ShouldCompilePermutation
//-----------------------------------------------------------------------------
bool FKimuraVertexFactory::ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters)
{
	return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5) || IsMobilePlatform(Parameters.Platform);
}


IMPLEMENT_VERTEX_FACTORY_PARAMETER_TYPE(FKimuraVertexFactory, SF_Vertex, FKimuraVertexFactoryShaderParameters);

#if ENGINE_MAJOR_VERSION >= 5

	// UE5
	IMPLEMENT_VERTEX_FACTORY_TYPE(FKimuraVertexFactory, "/Plugin/Kimura/KimuraVertexFactory.ush",
		  EVertexFactoryFlags::UsedWithMaterials
		| EVertexFactoryFlags::SupportsDynamicLighting
		| EVertexFactoryFlags::SupportsPrecisePrevWorldPos
		| EVertexFactoryFlags::SupportsPositionOnly
	);

#else

	// UE4
	bool bUsedWithMaterials = true;
	bool bSupportsStaticLighting = false;
	bool bSupportsDynamicLighting = true;

#if PLATFORM_ANDROID
	bool bPrecisePrevWorldPos = false;
#else
	bool bPrecisePrevWorldPos = true;
#endif
	bool bSupportsPositionOnly = true;

	IMPLEMENT_VERTEX_FACTORY_TYPE(FKimuraVertexFactory, "/Plugin/Kimura/KimuraVertexFactory.ush", 
		bUsedWithMaterials, 
		bSupportsStaticLighting, 
		bSupportsDynamicLighting, 
		bPrecisePrevWorldPos, 
		bSupportsPositionOnly);

#endif 

