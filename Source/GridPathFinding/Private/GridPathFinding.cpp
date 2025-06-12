// Copyright Epic Games, Inc. All Rights Reserved.

#include "GridPathFinding.h"

#include "TokenActor.h"
#include "BuildGridMap/UI/TokenActorDetails.h"

#define LOCTEXT_NAMESPACE "FGridPathFindingModule"

void FGridPathFindingModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FPropertyEditorModule& PropertyModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
	PropertyModule.RegisterCustomClassLayout(
		ATokenActor::StaticClass()->GetFName(),
		FOnGetDetailCustomizationInstance::CreateStatic(&FTokenActorDetails::MakeInstance)
	);
}

void FGridPathFindingModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FGridPathFindingModule, GridPathFinding)
