// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridTileActorConfigWidget.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "GridPathFindingSettings.h"
#include "HGTypes.h"
#include "IContentBrowserSingleton.h"
#include "JsonObjectConverter.h"
#include "TokenActor.h"
#include "BuildGridMap/BuildGridMapPlayerController.h"

void UBuildGridTileActorConfigWidget::NativeConstruct()
{
	Super::NativeConstruct();
}

void UBuildGridTileActorConfigWidget::OnSpawnActorClick()
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridTileActorConfigWidget.OnSpawnActorClick]"));
	// const UGridPathFindingSettings* Settings = GetDefault<UGridPathFindingSettings>();

	SpawnActorByPath(InputPathTxt->GetText().ToString());
}

void UBuildGridTileActorConfigWidget::OnShowActorClick()
{
	UE_LOG(LogGridPathFinding, Log, TEXT("Todo: [UBuildGridTileActorConfigWidget.OnShowActorClick]"));
}

void UBuildGridTileActorConfigWidget::SpawnActorByPath(const FString& ClassPath)
{
	// if (ClassPath.IsEmpty() || !FPackageName::DoesPackageExist(ClassPath))
	// {
	// 	UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTileActorConfigWidget.SpawnActorByPath] Resource invalid, path: %s"), *ClassPath);
	// 	return;
	// }
	//
	// auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
	// if (!PC)
	// {
	// 	UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTileActorConfigWidget.SpawnActorByPath] PlayerController is invalid!"));
	// 	return;
	// }
	//
	// // "Blueprint'/Game/Blueprints/Main/Actor/BP_TileDemoActor1.BP_TileDemoActor1_C'"
	// const FString Path = FString::Printf(TEXT("Blueprint'%s'"), *ClassPath);
	// auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	// auto EditingMapSave = MyGameMode->GetMutEditingMapSave();
	// const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();
	//
	// TArray<FHCubeCoord> SelectedCoords = PC->GetSelectedCoords();
	// for (FHCubeCoord Coord : SelectedCoords)
	// {
	// 	FHTileOrientation TileOrientation = MyGameMode->GridMapModel->GetTileOrientation(EditingMapSave->MapConfig.MapType, EditingMapSave->MapConfig.TileOrientation);
	// 	auto TileLocation = UGridPathFindingBlueprintFunctionLib::StableCoordToWorld(EditingMapSave->MapConfig, TileOrientation, Coord);
	//
	// 	// 指定生成位置和旋转
	// 	// FVector Location = FVector(0, 0, 0);
	// 	FRotator Rotation = FRotator(0, 0, 0);
	//
	// 	// 生成Actor实例
	// 	FActorSpawnParameters SpawnParams;
	// 	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	// 	SpawnParams.ObjectFlags |= RF_Transient;
	//
	// 	UClass* TokenClass = LoadClass<AActor>(nullptr, *Path);
	// 	if (TokenClass != nullptr)
	// 	{
	// 		ATokenActor* SpawnActor = GetWorld()->SpawnActor<ATokenActor>(TokenClass, TileLocation, Rotation, SpawnParams);
	// 	}
	//
	// 	// 保存数据
	// 	if (EditingTiles.Contains(Coord))
	// 	{
	// 		// 修改
	// 		MyGameMode->GetMutEditingTiles()[Coord].TileEnvData.TokenActorClassPath = ClassPath;
	//
	// 		MyGameMode->GridMapModel->UpdateTileEnv(ETileTokenModifyType::UpdateTileEnv, EditingTiles[Coord]);
	// 		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridTileActorConfigWidget.SpawnActorByPath] 修改"));
	// 	}
	// 	else
	// 	{
	// 		// 新增
	// 		auto NewTile = FSerializableTile();
	// 		NewTile.Coord = Coord;
	// 		auto NewEnvData = FTileEnvData();
	// 		NewEnvData.TokenActorClassPath = ClassPath;
	// 		NewTile.TileEnvData = NewEnvData;
	// 		MyGameMode->GetMutEditingTiles().Add(Coord, NewTile);
	//
	// 		MyGameMode->GridMapModel->UpdateTileEnv(ETileTokenModifyType::Add, NewTile);
	//
	// 		UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridTileActorConfigWidget.SpawnActorByPath] 新增"));
	// 	}
	// 	MyGameMode->MarkEditingTilesDirty(Coord);
	// }
}

void UBuildGridTileActorConfigWidget::ShowInfo(const FSerializableTile& InTileData)
{
	// if (InTileData.TileEnvData.TokenActorClassPath.IsEmpty())
	// {
	// 	ShowBtnTxt->SetText(FText::FromString(TEXT("Actor数据：未配置")));
	// 	InputPathTxt->SetText(FText::FromString(TEXT("")));
	// 	return;
	// }
	//
	// ShowBtnTxt->SetText(FText::FromString(TEXT("Actor数据：已配置")));
	// InputPathTxt->SetText(FText::FromString(InTileData.TileEnvData.TokenActorClassPath));
}

bool UBuildGridTileActorConfigWidget::JsonToStructUtil(const FString& JsonString, FTokenActorStruct& OutActorStruct)
{
	bool bSuccess = FJsonObjectConverter::JsonObjectStringToUStruct(JsonString, &OutActorStruct, 0, 0);
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridTileActorConfigWidget.JsonToStructUtil] bSuccess: %d, JsonString: %s"), bSuccess, *JsonString);
	return bSuccess;
}
