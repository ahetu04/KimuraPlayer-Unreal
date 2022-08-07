//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(KimuraLog, Log, All);


#if ENGINE_MAJOR_VERSION >= 5

	static_assert(ENGINE_MINOR_VERSION >= 1, "Kimura supports 4.27 and 5.1 but not Unreal 5 Early Access");
	
	// 5.1+: alias to FVector3f	and FVector4f
	typedef FVector3f FKimuraVector3;
	typedef FVector4f FKimuraVector4;

#else
	// 4.27: alias to FVector and FVector4
	typedef FVector FKimuraVector3;
	typedef FVector4 FKimuraVector4;
#endif


class FKimuraModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

