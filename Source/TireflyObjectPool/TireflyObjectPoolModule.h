// Copyright Tirefly. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FTireflyObjectPoolModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
