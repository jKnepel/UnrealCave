// Copyright 2016-2018 Rodrigo Villani Pereira

#pragma once

#include "Modules/ModuleManager.h"

class FNeoKinectUnrealModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};