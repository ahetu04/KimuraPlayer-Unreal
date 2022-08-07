//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "CoreMinimal.h"
#include "Stats/Stats.h"
#include "RHI.h"
#include "RenderResource.h"
#include "UniformBuffer.h"
#include "VertexFactory.h"
#include "KimuraModule.h"

//-----------------------------------------------------------------------------
// FKimuraVertexFactoryUniformBufferParameters
//-----------------------------------------------------------------------------


BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FKimuraVertexFactoryUniformBufferParameters, )
	SHADER_PARAMETER(FKimuraVector3, PositionQuantCenter)
	SHADER_PARAMETER(FKimuraVector3, PositionQuantExtents)
	SHADER_PARAMETER(FKimuraVector3, VelocityQuantCenter)
	SHADER_PARAMETER(FKimuraVector3, VelocityQuantExtents)
	SHADER_PARAMETER(float, VelocityScale)
	SHADER_PARAMETER(FKimuraVector4, ColorQuantExtents)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

typedef TUniformBufferRef<FKimuraVertexFactoryUniformBufferParameters> FKimuraVertexFactoryUniformBufferParametersRef;


//-----------------------------------------------------------------------------
// FKimuraVertexFactory
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraVertexFactory : public FVertexFactory
{

	DECLARE_VERTEX_FACTORY_TYPE(FKimuraVertexFactory);

	typedef FVertexFactory Super;

	friend class FKimuraVertexFactoryShaderParameters;

public:
	FKimuraVertexFactory(ERHIFeatureLevel::Type InFeatureLevel)
		: FVertexFactory(InFeatureLevel)
	{}

	struct FDataType
	{
		FVertexStreamComponent PositionComponent;
		FVertexStreamComponent NormalComponent;
		FVertexStreamComponent TangentComponent;
		TArray<FVertexStreamComponent, TFixedAllocator<MAX_STATIC_TEXCOORDS / 2> > TexCoordsComponents;
		FVertexStreamComponent ColorComponent;
		FVertexStreamComponent VelocityComponent;

	};

	static bool SupportsTessellationShaders() { return true; }

	static void ModifyCompilationEnvironment(const FVertexFactoryShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	static bool ShouldCompilePermutation(const FVertexFactoryShaderPermutationParameters& Parameters);

	void SetData(const FDataType& InData);

	virtual void InitRHI() override;

	FDataType Data;

	// vertex shader params. 
	FKimuraVector3 PositionQuantCenter;
	FKimuraVector3 PositionQuantExtents;
	FKimuraVector3 VelocityQuantCenter;
	FKimuraVector3 VelocityQuantExtents;
	FKimuraVector4 ColorQuantExtents;

	float	VelocityScale;

	// vertex shader params. Updated every frame 
	FKimuraVertexFactoryUniformBufferParametersRef	UniformBufferParams;

};

