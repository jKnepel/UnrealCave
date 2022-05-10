// Copyright 2016-2018 Rodrigo Villani Pereira

#include "NeoKinectUnreal.h"

#define LOCTEXT_NAMESPACE "FNeoKinectUnrealModule"

void FNeoKinectUnrealModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	
}

void FNeoKinectUnrealModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
	
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FNeoKinectUnrealModule, NeoKinectUnreal)