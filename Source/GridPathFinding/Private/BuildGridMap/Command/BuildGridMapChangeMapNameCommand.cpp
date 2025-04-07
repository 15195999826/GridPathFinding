// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeMapNameCommand.h"

#include "GridPathFindingBlueprintFunctionLib.h"
#include "GridPathFindingSettings.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/Command/BuildGridMapCommandManager.h"

void UBuildGridMapChangeMapNameCommand::Initialize(const FName& InOldMapName, const FName& InNewMapName)
{
	OldMapName = InOldMapName;
	NewMapName = InNewMapName;
}

bool UBuildGridMapChangeMapNameCommand::Execute()
{
	auto AllMapNames = UGridPathFindingBlueprintFunctionLib::GetAllMapSaveNames();

	// 不许重复
	if (AllMapNames.Contains(NewMapName))
	{
		// 弹出提示
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red, TEXT("地图名称重复，修改失败"));
		return true;
	}

	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = GM->GetMutEditingMapSave();
	// 删除旧的Json文件
	auto Settings = GetDefault<UGridPathFindingSettings>();
	FString OldSavePath = FPaths::ProjectContentDir() / Settings->MapSaveFolder / (OldMapName.ToString() + TEXT("_Map.txt"));
	check(FPaths::FileExists(OldSavePath));
	IFileManager::Get().Delete(*OldSavePath);
	UE_LOG(LogTemp, Display, TEXT("删除旧地图文件: %s"), *OldSavePath);
	
	EditingMapSave->MapName = NewMapName;
				
	// 直接重新保存一次好了
	GM->IntervalMapSaveToFile(*EditingMapSave);

	GM->GetCommandManager()->OnChangeMapName.Broadcast(OldMapName, NewMapName);
	return true;
}

bool UBuildGridMapChangeMapNameCommand::Undo()
{
	// Todo: 实现撤销功能
	return true;
}

FString UBuildGridMapChangeMapNameCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地图名字（从 %s 到 %s）"),
	                       *OldMapName.ToString(), *NewMapName.ToString());
}
