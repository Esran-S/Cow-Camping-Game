#pragma once

#include "Modules/ModuleManager.h"

class FCowFieldCleanupModule : public FDefaultGameModuleImpl
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
