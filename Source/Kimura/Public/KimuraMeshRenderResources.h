//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "RenderResource.h"
#include <memory>
#include "Kimura.h"
#include "KimuraMeshVertexFactory.h"
#include "KimuraMeshFrameParams.h"


//-----------------------------------------------------------------------------
// FKimuraIndexBuffer32
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraIndexBuffer32 : public FIndexBuffer
{
public:

	int32 IndiceCount = 0;

	void SetSize(int32 InIndiceCount)
	{
		this->IndiceCount = InIndiceCount;
	}

	virtual void InitRHI() override;
};


//-----------------------------------------------------------------------------
// FKimuraIndexBuffer16
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraIndexBuffer16 : public FIndexBuffer
{
public:

	int32 IndiceCount;

	void SetSize(int32 InIndiceCount)
	{
		this->IndiceCount = InIndiceCount;
	}

	virtual void InitRHI() override;

};


//-----------------------------------------------------------------------------
// FKimuraPositionVertexBuffer
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraPositionVertexBuffer : public FVertexBuffer
{
public:

	void Setup(Kimura::MeshInformation& InMeshInfo)
	{
		this->VertexCount = InMeshInfo.MaximumVertices;
		this->PositionFormat = InMeshInfo.PositionFormat_;

	}

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	int32							VertexCount = 0;
	Kimura::PositionFormat			PositionFormat;

	FVertexStreamComponent			PositionComponent;

};


//-----------------------------------------------------------------------------
// FKimuraVelocityVertexBuffer
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraVelocityVertexBuffer : public FVertexBuffer
{
public:

	void Setup(Kimura::MeshInformation& InMeshInfo)
	{
		this->VertexCount = InMeshInfo.MaximumVertices;
		this->VelocityFormat = InMeshInfo.VelocityFormat_;
	}

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	int32							VertexCount = 0;
	Kimura::VelocityFormat	VelocityFormat;

	FVertexStreamComponent			VelocityComponent;

};


//-----------------------------------------------------------------------------
// FKimuraNormalVertexBuffer
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraNormalVertexBuffer : public FVertexBuffer
{
public:

	void Setup(Kimura::MeshInformation& InMeshInfo)
	{
		this->VertexCount = InMeshInfo.MaximumVertices;
		this->NormalFormat = InMeshInfo.NormalFormat_;
	}

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	int32						VertexCount = 0;
	Kimura::NormalFormat		NormalFormat;

	FVertexStreamComponent		NormalComponent;

};


//-----------------------------------------------------------------------------
// FKimuraTangentVertexBuffer
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraTangentVertexBuffer : public FVertexBuffer
{
public:

	void Setup(Kimura::MeshInformation& InMeshInfo)
	{
		this->VertexCount = InMeshInfo.MaximumVertices;
		this->TangentFormat = InMeshInfo.TangentFormat_;
	}

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	int32						VertexCount = 0;
	Kimura::TangentFormat		TangentFormat;

	FVertexStreamComponent		TangentComponent;

};


//-----------------------------------------------------------------------------
// FKimuraTexCoordVertexBuffer
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraTexCoordVertexBuffer : public FVertexBuffer
{
public:

	void Setup(Kimura::MeshInformation& InMeshInfo)
	{
		this->VertexCount = InMeshInfo.MaximumVertices;
		this->TexCoordFormat = InMeshInfo.TexCoordFormat_;
	}

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	int32							VertexCount = 0;
	Kimura::TexCoordFormat			TexCoordFormat;

	TArray<FVertexStreamComponent, TFixedAllocator<MAX_STATIC_TEXCOORDS / 2>>	TexCoordsComponents;

};


//-----------------------------------------------------------------------------
// FKimuraColorVertexBuffer
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraColorVertexBuffer : public FVertexBuffer
{
public:

	void Setup(Kimura::MeshInformation& InMeshInfo)
	{
		this->VertexCount = InMeshInfo.MaximumVertices;
		this->ColorFormat = InMeshInfo.ColorFormat_;
	}

	virtual void InitRHI() override;
	virtual void ReleaseRHI() override;

	int32						VertexCount = 0;
	Kimura::ColorFormat			ColorFormat;

	FVertexStreamComponent		ColorComponent;

};


//-----------------------------------------------------------------------------
// FKimuraFrameGeometry
//-----------------------------------------------------------------------------
class KIMURA_API FKimuraFrameGeometry
{

	friend class FKimuraPrimitiveSceneProxy;

	public:

		FKimuraFrameGeometry(ERHIFeatureLevel::Type InFeatureLevel, Kimura::MeshInformation& InMeshInfo, const char* InDebugName);
		~FKimuraFrameGeometry();

	protected:
		
		uint32								MaxVertices = 0;
		uint32								MaxSurfaces = 0;

		FKimuraVertexFactory				KimuraVertexFactory;


		bool								Use32BitIndices = false;
		FKimuraIndexBuffer16				SurfaceIndices16;
		FKimuraIndexBuffer32				SurfaceIndices32;

		FKimuraPositionVertexBuffer			Positions;
		FKimuraNormalVertexBuffer			Normals;
		FKimuraTangentVertexBuffer			Tangents;
		FKimuraVelocityVertexBuffer			Velocities;
		FKimuraTexCoordVertexBuffer			Texcoords;
		FKimuraColorVertexBuffer			Colors;

		Kimura::MeshInformation				MeshInfo;
		uint32								NumVerticesSet = 0;
		uint32								NumSurfacesSet = 0;

		std::vector<Kimura::MeshSection>	Sections;

		bool								Initialized = false;

		void InitRenderResource(FRenderResource* InResource);
		void UpdateFromKimuraFrame(std::shared_ptr<Kimura::IFrame> InFrame, uint32 InMeshIndex, const KimuraMeshFrameParams& InFrameParams);

};