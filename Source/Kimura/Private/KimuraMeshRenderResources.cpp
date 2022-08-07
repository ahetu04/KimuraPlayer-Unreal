//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#include "KimuraMeshRenderResources.h"
#include "KimuraMeshVertexFactory.h"
#include "KimuraModule.h"
#include "Components.h"
#include "Stats/Stats2.h"

#if ENGINE_MAJOR_VERSION >= 5
	#define KimuraLockVertexBuffer RHILockBuffer
	#define KimuraUnlockVertexBuffer RHIUnlockBuffer
	#define KimuraLockIndexBuffer RHILockBuffer
	#define KimuraUnlockIndexBuffer RHIUnlockBuffer
#else
	#define KimuraLockVertexBuffer RHILockVertexBuffer
	#define KimuraUnlockVertexBuffer RHIUnlockVertexBuffer
	#define KimuraLockIndexBuffer RHILockIndexBuffer
	#define KimuraUnlockIndexBuffer RHIUnlockIndexBuffer
#endif

#if ENGINE_MAJOR_VERSION >= 5

	const EBufferUsageFlags c_BufferUsage = EBufferUsageFlags::Static;//BUF_Static /*| BUF_ShaderResource*/;

#else	

	#if PLATFORM_ANDROID
		const uint32 c_BufferUsage = BUF_Dynamic | BUF_ShaderResource;
	#else
		const uint32 c_BufferUsage = (uint32)BUF_Static /*| BUF_ShaderResource*/;
	#endif

#endif


//-----------------------------------------------------------------------------
// FKimuraIndexBuffer32::InitRHI
//-----------------------------------------------------------------------------
void FKimuraIndexBuffer32::InitRHI()
{
	FRHIResourceCreateInfo rci(TEXT("KimuraIndexBuffer32"));
	this->IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint32), this->IndiceCount * sizeof(uint32), c_BufferUsage, rci);
}


//-----------------------------------------------------------------------------
// FKimuraIndexBuffer16::InitRHI
//-----------------------------------------------------------------------------
void FKimuraIndexBuffer16::InitRHI()
{
	FRHIResourceCreateInfo rci(TEXT("KimuraIndexBuffer16"));
	this->IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), this->IndiceCount * sizeof(uint16), c_BufferUsage, rci);
}


//-----------------------------------------------------------------------------
// FKimuraPositionVertexBuffer::InitRHI
//-----------------------------------------------------------------------------
void FKimuraPositionVertexBuffer::InitRHI()
{
	FRHIResourceCreateInfo rci(TEXT("FKimuraPositionVertexBuffer"));

	if (this->PositionFormat == Kimura::PositionFormat::Full)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer(this->VertexCount * sizeof(FKimuraVector3), c_BufferUsage, rci);
		this->PositionComponent = FVertexStreamComponent(this, 0, 12, VET_Float3);

	}
	else if (this->PositionFormat == Kimura::PositionFormat::Half)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount+1) * (sizeof(int16)*3), c_BufferUsage, rci);
		this->PositionComponent = FVertexStreamComponent(this, 0, 6, VET_Short4N);

	}

}


//-----------------------------------------------------------------------------
// FKimuraPositionVertexBuffer::ReleaseRHI
//-----------------------------------------------------------------------------
void FKimuraPositionVertexBuffer::ReleaseRHI()
{
	FVertexBuffer::ReleaseRHI();
}


//-----------------------------------------------------------------------------
// FKimuraVelocityVertexBuffer::InitRHI
//-----------------------------------------------------------------------------
void FKimuraVelocityVertexBuffer::InitRHI()
{
	FRHIResourceCreateInfo rci(TEXT("FKimuraVelocityVertexBuffer"));

	if (this->VelocityFormat == Kimura::VelocityFormat::Full)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer(VertexCount * sizeof(FKimuraVector3), c_BufferUsage, rci);

		this->VelocityComponent = FVertexStreamComponent(this, 0, 12, VET_Float3);

	}
	else if (this->VelocityFormat == Kimura::VelocityFormat::Half)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int16) * 3), c_BufferUsage, rci);
		this->VelocityComponent = FVertexStreamComponent(this, 0, 6, VET_Short4N);

	}
	else if (this->VelocityFormat == Kimura::VelocityFormat::Byte)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int8) * 3), c_BufferUsage, rci);
		this->VelocityComponent = FVertexStreamComponent(this, 0, 3, VET_PackedNormal);

	}
	else 
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int8) * 3), c_BufferUsage, rci);
		this->VelocityComponent = FVertexStreamComponent(this, 0, 3, VET_PackedNormal);

		// zero memory
		{
			int size = this->VertexCount * sizeof(int8) * 3;
			void* pBuffer = KimuraLockVertexBuffer(this->VertexBufferRHI, 0, size, RLM_WriteOnly);
			memset(pBuffer, 0, size);
			KimuraUnlockVertexBuffer(this->VertexBufferRHI);
		}

	}
}


//-----------------------------------------------------------------------------
// FKimuraVelocityVertexBuffer::ReleaseRHI
//-----------------------------------------------------------------------------
void FKimuraVelocityVertexBuffer::ReleaseRHI()
{
	FVertexBuffer::ReleaseRHI();
}


//-----------------------------------------------------------------------------
// FKimuraNormalVertexBuffer::InitRHI
//-----------------------------------------------------------------------------
void FKimuraNormalVertexBuffer::InitRHI()
{

	FRHIResourceCreateInfo rci(TEXT("FKimuraNormalVertexBuffer"));


	if (this->NormalFormat == Kimura::NormalFormat::Full)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer(VertexCount * sizeof(FKimuraVector3), c_BufferUsage, rci);

		this->NormalComponent = FVertexStreamComponent(this, 0, 12, VET_Float3);

	}
	else if (this->NormalFormat == Kimura::NormalFormat::Half)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int16) * 3), c_BufferUsage, rci);
		this->NormalComponent = FVertexStreamComponent(this, 0, 6, VET_Short4N);

	}
	else if (this->NormalFormat == Kimura::NormalFormat::Byte)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int8) * 3), c_BufferUsage, rci);
		this->NormalComponent = FVertexStreamComponent(this, 0, 3, VET_PackedNormal);

	}
	else if (this->NormalFormat == Kimura::NormalFormat::None)
	{
		// create an empty vertex buffer (1 byte per component)
		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int8) * 3), c_BufferUsage, rci);
		this->NormalComponent = FVertexStreamComponent(this, 0, 3, VET_PackedNormal);

		// zero memory
		{
			int size = this->VertexCount * sizeof(int8) * 3;
			void* pBuffer = KimuraLockVertexBuffer(this->VertexBufferRHI, 0, size, RLM_WriteOnly);
			memset(pBuffer, 0, size);
			KimuraUnlockVertexBuffer(this->VertexBufferRHI);
		}

	}

}


//-----------------------------------------------------------------------------
// FKimuraNormalVertexBuffer::ReleaseRHI
//-----------------------------------------------------------------------------
void FKimuraNormalVertexBuffer::ReleaseRHI()
{
	FVertexBuffer::ReleaseRHI();
}


//-----------------------------------------------------------------------------
// FKimuraTangentVertexBuffer::InitRHI
//-----------------------------------------------------------------------------
void FKimuraTangentVertexBuffer::InitRHI()
{

	FRHIResourceCreateInfo rci(TEXT("FKimuraTangentVertexBuffer"));


	if (this->TangentFormat == Kimura::TangentFormat::Full)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer(VertexCount * sizeof(FKimuraVector4), c_BufferUsage, rci);

		this->TangentComponent = FVertexStreamComponent(this, 0, 16, VET_Float4);

	}
	else if (this->TangentFormat == Kimura::TangentFormat::Half)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int16) * 4), c_BufferUsage, rci);
		this->TangentComponent = FVertexStreamComponent(this, 0, 8, VET_Short4N);

	}
	else if (this->TangentFormat == Kimura::TangentFormat::Byte)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int8) * 4), c_BufferUsage, rci);
		this->TangentComponent = FVertexStreamComponent(this, 0, 4, VET_PackedNormal);

	}
	else if (this->TangentFormat == Kimura::TangentFormat::None)
	{
		// create an empty vertex buffer (1 byte per component)
		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * (sizeof(int8) * 4), c_BufferUsage, rci);
		this->TangentComponent = FVertexStreamComponent(this, 0, 4, VET_PackedNormal);

		// zero memory
		{
			int size = this->VertexCount * sizeof(int8) * 4;
			void* pBuffer = KimuraLockVertexBuffer(this->VertexBufferRHI, 0, size, RLM_WriteOnly);
			memset(pBuffer, 0, size);
			KimuraUnlockVertexBuffer(this->VertexBufferRHI);
		}

	}

}


//-----------------------------------------------------------------------------
// FKimuraTangentVertexBuffer::ReleaseRHI
//-----------------------------------------------------------------------------
void FKimuraTangentVertexBuffer::ReleaseRHI()
{
	FVertexBuffer::ReleaseRHI();
}


//-----------------------------------------------------------------------------
// FKimuraTexCoordVertexBuffer::InitRHI
//-----------------------------------------------------------------------------
void FKimuraTexCoordVertexBuffer::InitRHI()
{
	FRHIResourceCreateInfo rci(TEXT("FKimuraTexCoordVertexBuffer"));

	if (this->TexCoordFormat == Kimura::TexCoordFormat::Full)
	{

		int size = (this->VertexCount + 1) * sizeof(float) * 2;
		int stride = sizeof(float) * 2;
		this->VertexBufferRHI = RHICreateVertexBuffer(size, c_BufferUsage, rci);
		this->TexCoordsComponents.Add(FVertexStreamComponent(this, 0, stride, VET_Float2));//, EVertexStreamUsage::ManualFetch));

	}
	else if (this->TexCoordFormat == Kimura::TexCoordFormat::Half)
	{
		int size = (this->VertexCount + 1) * sizeof(uint16) * 2;
		int stride = sizeof(uint16) * 2;
		this->VertexBufferRHI = RHICreateVertexBuffer(size, c_BufferUsage, rci);
		this->TexCoordsComponents.Add(FVertexStreamComponent(this, 0, stride, VET_UShort2N));//, EVertexStreamUsage::ManualFetch));
	}
	else
	{
		int size = (this->VertexCount + 1) * sizeof(uint8) * 4;
		int stride = sizeof(uint8) * 4;
		this->VertexBufferRHI = RHICreateVertexBuffer(size, c_BufferUsage, rci);
		this->TexCoordsComponents.Add(FVertexStreamComponent(this, 0, stride, VET_UByte4N));//, EVertexStreamUsage::ManualFetch));

		// set to black
		void* pBuffer = KimuraLockVertexBuffer(this->VertexBufferRHI, 0, size, RLM_WriteOnly);
		memset(pBuffer, 0, size);
		KimuraUnlockVertexBuffer(this->VertexBufferRHI);
	}


}


//-----------------------------------------------------------------------------
// FKimuraTexCoordVertexBuffer::ReleaseRHI
//-----------------------------------------------------------------------------
void FKimuraTexCoordVertexBuffer::ReleaseRHI()
{
	FVertexBuffer::ReleaseRHI();
}


//-----------------------------------------------------------------------------
// FKimuraColorVertexBuffer::InitRHI
//-----------------------------------------------------------------------------
void FKimuraColorVertexBuffer::InitRHI()
{
	// Create the vertex buffer.
	FRHIResourceCreateInfo rci(TEXT("FKimuraColorVertexBuffer"));


	if (this->ColorFormat == Kimura::ColorFormat::Full)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * 16, c_BufferUsage, rci);
		this->ColorComponent = FVertexStreamComponent(this, 0, 16, VET_Float4);//, EVertexStreamUsage::ManualFetch);

	}
	else if (this->ColorFormat == Kimura::ColorFormat::Half)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount + 1) * 8, c_BufferUsage, rci);
		this->ColorComponent = FVertexStreamComponent(this, 0, 8, VET_UShort4N);//, EVertexStreamUsage::ManualFetch);

	}
	else if (this->ColorFormat == Kimura::ColorFormat::Byte || this->ColorFormat == Kimura::ColorFormat::ByteHDR)
	{

		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount+1) * 4, c_BufferUsage, rci);
		this->ColorComponent = FVertexStreamComponent(this, 0, 4, VET_UByte4N);//, EVertexStreamUsage::ManualFetch);

	}
	else
	{
		// default 8bit vertex buffer
		this->VertexBufferRHI = RHICreateVertexBuffer((this->VertexCount+1) * 4, c_BufferUsage, rci);
		this->ColorComponent = FVertexStreamComponent(this, 0, 4, VET_UByte4N);//, EVertexStreamUsage::ManualFetch);

		// set to black
		int size = this->VertexCount * sizeof(int8) * 4;
		void* pBuffer = KimuraLockVertexBuffer(this->VertexBufferRHI, 0, size, RLM_WriteOnly);
		memset(pBuffer, 0, size);
		KimuraUnlockVertexBuffer(this->VertexBufferRHI);

	}

}


//-----------------------------------------------------------------------------
// FKimuraColorVertexBuffer::ReleaseRHI
//-----------------------------------------------------------------------------
void FKimuraColorVertexBuffer::ReleaseRHI()
{
	FVertexBuffer::ReleaseRHI();
}


//-----------------------------------------------------------------------------
// FKimuraFrameGeometry::FKimuraFrameGeometry
//-----------------------------------------------------------------------------
FKimuraFrameGeometry::FKimuraFrameGeometry(ERHIFeatureLevel::Type InFeatureLevel, Kimura::MeshInformation& InMeshInfo, const char* InDebugName)
	:
	KimuraVertexFactory(InFeatureLevel)
{

	this->MeshInfo = InMeshInfo;

	this->MaxVertices = InMeshInfo.MaximumVertices;
	this->MaxSurfaces = InMeshInfo.MaximumSurfaces;

	if (this->MaxVertices == 0 || this->MaxSurfaces == 0)
	{
		return;
	}


	// always have a 16bit index buffer ready
	this->SurfaceIndices16.SetSize(this->MaxSurfaces * 3);

	// only allocate a 32bit index buffer if one of this mesh's frames is going to require one, 
	// and when conversion didn't optimize for 16bit indices.
	{
		this->Use32BitIndices = this->MaxVertices > 0xfffe && !this->MeshInfo.Force16BitIndices;
		if (this->Use32BitIndices)
		{
			this->SurfaceIndices32.SetSize(this->MaxSurfaces * 3);
		}
	}

	this->Positions.Setup(this->MeshInfo);
	this->Normals.Setup(this->MeshInfo);
	this->Tangents.Setup(this->MeshInfo);
	this->Velocities.Setup(this->MeshInfo);
	this->Texcoords.Setup(this->MeshInfo);
	this->Colors.Setup(this->MeshInfo);

	ENQUEUE_RENDER_COMMAND(FKimuraFrameGeometryInit)(
		[this](FRHICommandListImmediate& RHICmdList)
	{

		this->InitRenderResource(&this->SurfaceIndices16);

		if (this->Use32BitIndices)
		{
			this->InitRenderResource(&this->SurfaceIndices32);
		}

		this->InitRenderResource(&this->Positions);

		// normals are a special case
		this->InitRenderResource(&this->Normals);
		this->InitRenderResource(&this->Tangents);
		this->InitRenderResource(&this->Velocities);
		this->InitRenderResource(&this->Texcoords);
		this->InitRenderResource(&this->Colors);

		// initialize the vertex factory
		{
			FKimuraVertexFactory::FDataType KVFData;

			KVFData.PositionComponent = this->Positions.PositionComponent;

			KVFData.NormalComponent = this->Normals.NormalComponent;

			KVFData.TangentComponent = this->Tangents.TangentComponent;

			KVFData.VelocityComponent = this->Velocities.VelocityComponent;

			for (const auto& x : this->Texcoords.TexCoordsComponents)
			{
				KVFData.TexCoordsComponents.Add(x);
			}

			KVFData.ColorComponent = this->Colors.ColorComponent;

			this->KimuraVertexFactory.SetData(KVFData);
			this->InitRenderResource(&this->KimuraVertexFactory);
		}

		this->Initialized = true;

	});

}


//-----------------------------------------------------------------------------
// FKimuraFrameGeometry::~FKimuraFrameGeometry
//-----------------------------------------------------------------------------
FKimuraFrameGeometry::~FKimuraFrameGeometry()
{
	this->Positions.ReleaseResource();
	
	this->Normals.ReleaseResource();

	this->Tangents.ReleaseResource();

	this->Velocities.ReleaseResource();
	this->Texcoords.ReleaseResource();
	this->Colors.ReleaseResource();

	this->SurfaceIndices16.ReleaseResource();
	if (this->Use32BitIndices)
	{
		this->SurfaceIndices32.ReleaseResource();
	}

	this->KimuraVertexFactory.ReleaseResource();
}


//-----------------------------------------------------------------------------
// FKimuraFrameGeometry::InitOrUpdateResource
//-----------------------------------------------------------------------------
void FKimuraFrameGeometry::InitRenderResource(FRenderResource* InResource)
{
	if (!InResource->IsInitialized())
	{
		InResource->InitResource();
	}
	else
	{
		InResource->UpdateRHI();
	}
}


//-----------------------------------------------------------------------------
// FKimuraFrameGeometry::UpdateFromKimuraFrame
//-----------------------------------------------------------------------------
void FKimuraFrameGeometry::UpdateFromKimuraFrame(std::shared_ptr<Kimura::IFrame> InFrame, uint32 InMeshIndex, const KimuraMeshFrameParams& InFrameParams)
{
	check(IsInRenderingThread());

	if (!this->Initialized)
	{
		UE_LOG(KimuraLog, Warning, TEXT("Kimura frame resource updated before it was initialized. Ignoring..."));
		return;
	}

	if (InFrame == nullptr)
	{
		this->NumVerticesSet = 0;
		this->NumSurfacesSet = 0;

		return;
	}

	TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("UpdateFromKimuraFrame"));

	this->NumVerticesSet = InFrame->GetNumVertices(InMeshIndex);
	this->NumSurfacesSet = InFrame->GetNumSurfaces(InMeshIndex);

	InFrame->GetSections(InMeshIndex, this->Sections);

	if (this->NumVerticesSet == 0 || this->NumSurfacesSet == 0)
	{
		return;
	}


	// index buffers (can be either 32bit of 16bit indices)
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("IndexBuffer"));

		const uint32* pIndices32 = InFrame->GetIndicesU32(InMeshIndex);
		const uint16* pIndices16 = InFrame->GetIndicesU16(InMeshIndex);


		if (pIndices32 != nullptr)
		{
			//UE_LOG(KimuraLog, Log, TEXT("updating index buffer 32 bit"));
			uint32* pBuffer = (uint32*)KimuraLockIndexBuffer(this->SurfaceIndices32.IndexBufferRHI, 0, this->NumSurfacesSet * 3 * sizeof(uint32), RLM_WriteOnly);
			memcpy(pBuffer, pIndices32, this->NumSurfacesSet * 3 * sizeof(uint32));
			KimuraUnlockIndexBuffer(this->SurfaceIndices32.IndexBufferRHI);

		}
		else if (pIndices16 != nullptr)
		{
			//UE_LOG(KimuraLog, Log, TEXT("updating index buffer 16 bit"));
			uint16* pBuffer = (uint16*)KimuraLockIndexBuffer(this->SurfaceIndices16.IndexBufferRHI, 0, this->NumSurfacesSet * 3 * sizeof(uint16), RLM_WriteOnly);
			memcpy(pBuffer, pIndices16, this->NumSurfacesSet * 3 * sizeof(uint16));
			KimuraUnlockIndexBuffer(this->SurfaceIndices16.IndexBufferRHI);
		}

	}

	// positions
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("Positions"));

		// if format is 32bits
		if (this->MeshInfo.PositionFormat_ == Kimura::PositionFormat::Full)
		{
			const Kimura::Vector3* pPositionsF32 = InFrame->GetPositionsF32(InMeshIndex);

			if (pPositionsF32)
			{

				//UE_LOG(KimuraLog, Log, TEXT("updating position 32bit"));

				FKimuraVector3* pBuffer = (FKimuraVector3*)KimuraLockVertexBuffer(this->Positions.VertexBufferRHI, 0, this->NumVerticesSet * sizeof(FKimuraVector3), RLM_WriteOnly);
				memcpy(pBuffer, pPositionsF32, this->NumVerticesSet * sizeof(FKimuraVector3));
				KimuraUnlockVertexBuffer(this->Positions.VertexBufferRHI);
			}
		}
		else if (this->MeshInfo.PositionFormat_ == Kimura::PositionFormat::Half)
		{
			const Kimura::int16* pPositionsI16 = InFrame->GetPositionsI16(InMeshIndex);

			if (pPositionsI16)
			{
				//UE_LOG(KimuraLog, Log, TEXT("updating position 16bit"));

				int size = this->NumVerticesSet * sizeof(int16) * 3;
				void* pBuffer = (void*)KimuraLockVertexBuffer(this->Positions.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pPositionsI16, size);
				KimuraUnlockVertexBuffer(this->Positions.VertexBufferRHI);
			}
		}

	}

	// velocities
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("Velocities"));

		if (this->MeshInfo.VelocityFormat_ == Kimura::VelocityFormat::Full)
		{
			const Kimura::Vector3* pVelocitiesF32 = InFrame->GetVelocitiesF32(InMeshIndex);

			if (pVelocitiesF32)
			{

				FKimuraVector3* pBuffer = (FKimuraVector3*)KimuraLockVertexBuffer(this->Velocities.VertexBufferRHI, 0, this->NumVerticesSet * sizeof(FKimuraVector3), RLM_WriteOnly);
				memcpy(pBuffer, pVelocitiesF32, this->NumVerticesSet * sizeof(FKimuraVector3));

				KimuraUnlockVertexBuffer(this->Velocities.VertexBufferRHI);
			}
		}
		else if (this->MeshInfo.VelocityFormat_ == Kimura::VelocityFormat::Half)
		{
			const Kimura::int16* pVelocitiesI16= InFrame->GetVelocitiesI16(InMeshIndex);

			if (pVelocitiesI16)
			{
				int size = this->NumVerticesSet * sizeof(int16) * 3;
				FKimuraVector3* pBuffer = (FKimuraVector3*)KimuraLockVertexBuffer(this->Velocities.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pVelocitiesI16, size);
				KimuraUnlockVertexBuffer(this->Velocities.VertexBufferRHI);
			}
		}
		else if (this->MeshInfo.VelocityFormat_ == Kimura::VelocityFormat::Byte)
		{
			const Kimura::int8* pVelocitiesI8 = InFrame->GetVelocitiesI8(InMeshIndex);

			if (pVelocitiesI8)
			{
				int size = this->NumVerticesSet * sizeof(int8) * 3;
				void* pBuffer = KimuraLockVertexBuffer(this->Velocities.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pVelocitiesI8, size);
				KimuraUnlockVertexBuffer(this->Velocities.VertexBufferRHI);
			}
		}
	}

	// normals
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("Normals"));

		if (this->MeshInfo.NormalFormat_ == Kimura::NormalFormat::Full)
		{
			const Kimura::Vector3* pNormalF32 = InFrame->GetNormalsF32(InMeshIndex);

			if (pNormalF32)
			{
				FKimuraVector3* pBuffer = (FKimuraVector3*)KimuraLockVertexBuffer(this->Normals.VertexBufferRHI, 0, this->NumVerticesSet * sizeof(FKimuraVector3), RLM_WriteOnly);
				memcpy(pBuffer, pNormalF32, this->NumVerticesSet * sizeof(FKimuraVector3));

				KimuraUnlockVertexBuffer(this->Normals.VertexBufferRHI);
			}

		}
		else if (this->MeshInfo.NormalFormat_ == Kimura::NormalFormat::Half)
		{
			const Kimura::int16* pNormalsI16 = InFrame->GetNormalsI16(InMeshIndex);

			if (pNormalsI16)
			{
				int size = this->NumVerticesSet * sizeof(int16) * 3;
				FKimuraVector3* pBuffer = (FKimuraVector3*)KimuraLockVertexBuffer(this->Normals.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pNormalsI16, size);
				KimuraUnlockVertexBuffer(this->Normals.VertexBufferRHI);
			}

		}
		else if (this->MeshInfo.NormalFormat_ == Kimura::NormalFormat::Byte)
		{
			const Kimura::int8* pNormalsI8 = InFrame->GetNormalsI8(InMeshIndex);

			if (pNormalsI8)
			{
				int size = this->NumVerticesSet * sizeof(int8) * 3;
				void* pBuffer = KimuraLockVertexBuffer(this->Normals.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pNormalsI8, size);
				KimuraUnlockVertexBuffer(this->Normals.VertexBufferRHI);
			}

		}

	}


	// tangents
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("Tangents"));

		if (this->MeshInfo.TangentFormat_ == Kimura::TangentFormat::Full)
		{
			const Kimura::Vector4* pTangentF32 = InFrame->GetTangentsF32(InMeshIndex);

			if (pTangentF32)
			{
				FKimuraVector4* pBuffer = (FKimuraVector4*)KimuraLockVertexBuffer(this->Tangents.VertexBufferRHI, 0, this->NumVerticesSet * sizeof(FKimuraVector4), RLM_WriteOnly);
				memcpy(pBuffer, pTangentF32, this->NumVerticesSet * sizeof(FKimuraVector4));

				KimuraUnlockVertexBuffer(this->Tangents.VertexBufferRHI);
			}
		}
		else if (this->MeshInfo.TangentFormat_ == Kimura::TangentFormat::Half)
		{

			const Kimura::int16* pTangentsI16 = InFrame->GetTangentsI16(InMeshIndex);

			if (pTangentsI16)
			{
				int size = this->NumVerticesSet * sizeof(int16) * 4;
				FKimuraVector3* pBuffer = (FKimuraVector3*)KimuraLockVertexBuffer(this->Tangents.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pTangentsI16, size);
				KimuraUnlockVertexBuffer(this->Tangents.VertexBufferRHI);
			}

		}
		else if (this->MeshInfo.TangentFormat_ == Kimura::TangentFormat::Byte)
		{
			const Kimura::int8* pTangentsI8 = InFrame->GetTangentsI8(InMeshIndex);

			if (pTangentsI8)
			{
				int size = this->NumVerticesSet * sizeof(int8) * 4;
				void* pBuffer = KimuraLockVertexBuffer(this->Tangents.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pTangentsI8, size);
				KimuraUnlockVertexBuffer(this->Tangents.VertexBufferRHI);
			}
		}
	}


	// UVs
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("Texture Coords"));

		if (this->MeshInfo.TexCoordFormat_ == Kimura::TexCoordFormat::Full)
		{
			const Kimura::Vector2* pTexcoords0 = InFrame->GetTexCoordsF32(InMeshIndex, 0);

			if (pTexcoords0)
			{
				typedef TStaticMeshVertexUVsTypeSelector<EStaticMeshVertexUVType::HighPrecision>::UVsTypeT uvType;
				float* pBuffer = (float*)KimuraLockVertexBuffer(this->Texcoords.VertexBufferRHI, 0, this->NumVerticesSet * sizeof(uvType), RLM_WriteOnly);
				memcpy(pBuffer, pTexcoords0, this->NumVerticesSet * sizeof(uvType));
				KimuraUnlockVertexBuffer(this->Texcoords.VertexBufferRHI);
			}

		}
		else if (this->MeshInfo.TexCoordFormat_ == Kimura::TexCoordFormat::Half)
		{
			const Kimura::uint16* pTexcoords0 = InFrame->GetTexCoordsU16(InMeshIndex, 0);

			if (pTexcoords0)
			{
				int size = this->NumVerticesSet * sizeof(uint16) * 2;

				float* pBuffer = (float*)KimuraLockVertexBuffer(this->Texcoords.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pTexcoords0, size);
				KimuraUnlockVertexBuffer(this->Texcoords.VertexBufferRHI);
			}

		}

	}

	// Colors
	{

		TRACE_CPUPROFILER_EVENT_SCOPE(TEXT("Colors"));

		if (this->MeshInfo.ColorFormat_ == Kimura::ColorFormat::Full)
		{
			const Kimura::Vector4* pColors = InFrame->GetColorsF32(InMeshIndex, 0);
			if (pColors)
			{
				int size = this->NumVerticesSet * sizeof(float) * 4;

				void* pBuffer = KimuraLockVertexBuffer(this->Colors.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pColors, size);
				KimuraUnlockVertexBuffer(this->Colors.VertexBufferRHI);
			}

		}
		else if (this->MeshInfo.ColorFormat_ == Kimura::ColorFormat::Half)
		{
			const uint16* pColors = InFrame->GetColorsU16(InMeshIndex, 0);
			if (pColors)
			{
				int s = sizeof(uint16);
				int size = this->NumVerticesSet * sizeof(uint16) * 4;

				void* pBuffer = KimuraLockVertexBuffer(this->Colors.VertexBufferRHI, 0, size, RLM_WriteOnly);
				memcpy(pBuffer, pColors, size);
				KimuraUnlockVertexBuffer(this->Colors.VertexBufferRHI);
			}

		}
		else if (this->MeshInfo.ColorFormat_ == Kimura::ColorFormat::Byte || this->MeshInfo.ColorFormat_ == Kimura::ColorFormat::ByteHDR)
		{
			const uint8* pColors = InFrame->GetColorsU8(InMeshIndex, 0);

			if (pColors)
			{
				FColor* pBuffer = (FColor*)KimuraLockVertexBuffer(this->Colors.VertexBufferRHI, 0, this->NumVerticesSet * sizeof(FColor), RLM_WriteOnly);
				memcpy(pBuffer, pColors, this->NumVerticesSet * sizeof(FColor));
				KimuraUnlockVertexBuffer(this->Colors.VertexBufferRHI);
			}
		}
	}

	// update the vertex factory's uniform buffer parameters here
	{

		Kimura::Vector3 vPosQuantCenter = InFrame->GetPositionQuantizationCenter(InMeshIndex);
		Kimura::Vector3 vPosQuantExtents = InFrame->GetPositionQuantizationExtents(InMeshIndex);

		Kimura::Vector3 vVelocityQuantCenter = InFrame->GetVelocityQuantizationCenter(InMeshIndex);
		Kimura::Vector3 vVelocityQuantExtents = InFrame->GetVelocityQuantizationExtents(InMeshIndex);

		Kimura::Vector4 vColorQuantExtents = InFrame->GetColorQuantizationExtents(InMeshIndex, 0);

		FKimuraVertexFactoryUniformBufferParameters p;
		p.PositionQuantCenter = FKimuraVector3(vPosQuantCenter.X, vPosQuantCenter.Y, vPosQuantCenter.Z);
		p.PositionQuantExtents = FKimuraVector3(vPosQuantExtents.X, vPosQuantExtents.Y, vPosQuantExtents.Z);
		p.VelocityQuantCenter = FKimuraVector3(vVelocityQuantCenter.X, vVelocityQuantCenter.Y, vVelocityQuantCenter.Z);
		p.VelocityQuantExtents = FKimuraVector3(vVelocityQuantExtents.X, vVelocityQuantExtents.Y, vVelocityQuantExtents.Z);
		p.VelocityScale = InFrameParams.VelocityScale;
		p.ColorQuantExtents = FKimuraVector4(vColorQuantExtents.X, vColorQuantExtents.Y, vColorQuantExtents.Z, vColorQuantExtents.W);

		this->KimuraVertexFactory.UniformBufferParams.UpdateUniformBufferImmediate(p);
	}

}

