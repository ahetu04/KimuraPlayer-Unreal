//
// Copyright (c) Alexandre Hetu.
// Licensed under the MIT License.
//
// https://github.com/ahetu04
//

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

DECLARE_LOG_CATEGORY_EXTERN(KimuraConverter, Log, All);

class FKimuraConverterModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	void	ExtendTheEditor();

	void	SetupConverterMenu(FMenuBuilder& MenuBuilder);
	void	OnOpenConverterMenu();

};
