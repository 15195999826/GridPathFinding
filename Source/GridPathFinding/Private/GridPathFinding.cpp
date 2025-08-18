// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridPathFinding.h"


#define LOCTEXT_NAMESPACE "FGridPathFindingModule"

void FGridPathFindingModule::StartupModule()
{
}

void FGridPathFindingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGridPathFindingModule, GridPathFinding)
