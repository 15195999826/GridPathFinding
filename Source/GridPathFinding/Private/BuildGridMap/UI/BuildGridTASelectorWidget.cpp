// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridTASelectorWidget.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "GridPathFindingSettings.h"
#include "HGTypes.h"
#include "BehaviorTree/BehaviorTreeTypes.h"
#include "BuildGridMap/BuildGridMapPlayerController.h"
#include "Components/ComboBoxString.h"

void UBuildGridTASelectorWidget::NativeConstruct()
{
	Super::NativeConstruct();
	ShowInfo();
}

void UBuildGridTASelectorWidget::OnCreateClick()
{
	UE_LOG(LogTemp, Log, TEXT("[UBuildGridTASelectorWidget.OnCreateClick]"));
	UE_LOG(LogTemp, Log, TEXT("%s"), *SelectedClassPath);

	// TODO: 根据选择的类路径创建 Actor
	if (SelectedClassPath.IsEmpty())
	{
		UE_LOG(LogTemp, Error, TEXT("[UBuildGridTASelectorWidget.OnCreateClick] No Actor class selected!"));
		return;
	}

	SpawnActorByPath(SelectedClassPath);
}

void UBuildGridTASelectorWidget::OnDeleteClick()
{
	UE_LOG(LogTemp, Log, TEXT("[UBuildGridTASelectorWidget.OnDeleteClick]"));
	auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTASelectorWidget.OnDeleteClick] PlayerController is invalid!"));
		return;
	}

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();

	int32 Cnt = 0;
	TArray<FHCubeCoord> SelectedCoords = PC->GetSelectedCoords();
	for (FHCubeCoord Coord : SelectedCoords)
	{
		if (!EditingTiles.Contains(Coord))
		{
			continue;
		}

		// 如果该格子上有 Actor，则删除它
		// auto OldTile = EditingTiles[Coord];
		//
		// MyGameMode->GetMutEditingTiles().Remove(Coord);
		// MyGameMode->GridMapModel->UpdateTileEnv(ETileTokenModifyType::Remove, OldTile);
		// MyGameMode->MarkEditingTilesDirty(Coord);
		Cnt++;
	}
	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridTASelectorWidget.OnDeleteClick] 删除了 %d 个 Actor"), Cnt);
}

void UBuildGridTASelectorWidget::OnTypeNameSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	switch (SelectionType)
	{
	case ESelectInfo::OnMouseClick:
		{
			UE_LOG(LogTemp, Log, TEXT("[UBuildGridTASelectorWidget.OnTypeNameSelectionChanged] %s"), *SelectedItem)
			SelectedClassPath = "";

			// 根据选择的类型刷新 Actor 选择器
			UpdateActorComboBox();
			break;
		}
	default: break;
	}
}

void UBuildGridTASelectorWidget::OnActorClassSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType)
{
	switch (SelectionType)
	{
	case ESelectInfo::OnMouseClick:
		{
			UE_LOG(LogTemp, Log, TEXT("[UBuildGridTASelectorWidget.OnActorClassSelectionChanged] %s"), *SelectedItem)
			// SelectedActorIndex = ActorComboBox->GetSelectedIndex();
			SelectedClassPath = SelectedItem;
			break;
		}
	default: break;
	}
}

void UBuildGridTASelectorWidget::ShowInfo()
{
	ActorSelectorGo->SetVisibility(ESlateVisibility::Hidden);
	// 刷新 Type 选择器
	UpdateTypeComboBox();
}

void UBuildGridTASelectorWidget::UpdateTypeComboBox()
{
	TypeComboBox->ClearOptions();

	const UGridPathFindingSettings* Settings = GetDefault<UGridPathFindingSettings>();
	for (auto Pair : Settings->TokenActorClassMap)
	{
		FString TypeName = UEnum::GetValueAsString(Pair.Key);
		if (Pair.Value.TokenActorClasses.Num() <= 0)
		{
			UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTASelectorWidget.ShowInfo] Array is empty for type: %s"), *TypeName)
			continue;
		}

		TypeComboBox->AddOption(TypeName);
	}

	TypeComboBox->SetSelectedIndex(-1);
	TypeComboBox->OnSelectionChanged.AddDynamic(this, &UBuildGridTASelectorWidget::OnTypeNameSelectionChanged);
}

void UBuildGridTASelectorWidget::UpdateActorComboBox()
{
	FString CurrentSelectType = TypeComboBox->GetOptionAtIndex(TypeComboBox->GetSelectedIndex());
	// 将字符串转换为ETokenActorType枚举
	const UGridPathFindingSettings* Settings = GetDefault<UGridPathFindingSettings>();
	ETokenActorType ActorType = ETokenActorType::None;
	for (auto Pair : Settings->TokenActorClassMap)
	{
		if (UEnum::GetValueAsString(Pair.Key) == CurrentSelectType)
		{
			ActorType = Pair.Key;
			break;
		}
	}

	if (ActorType == ETokenActorType::None)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTASelectorWidget.UpdateActorComboBox] Invalid Actor Type: %s"), *CurrentSelectType);
		return;
	}

	ActorComboBox->OnSelectionChanged.RemoveDynamic(this, &UBuildGridTASelectorWidget::OnActorClassSelectionChanged);
	ActorComboBox->ClearOptions();
	// 获取该类型的TokenActor类数组
	TArray<TSubclassOf<ATokenActor>> ActorClasses = GetTokenActors(ActorType);
	for (TSubclassOf<ATokenActor> ActorClass : ActorClasses)
	{
		if (ActorClass)
		{
			ActorComboBox->AddOption(ActorClass->GetClassPathName().ToString());
		}
	}

	// 如果有Actor类，显示ActorSelectorGo组件
	if (ActorClasses.Num() > 0)
	{
		ActorSelectorGo->SetVisibility(ESlateVisibility::Visible);
	}
	else
	{
		ActorSelectorGo->SetVisibility(ESlateVisibility::Hidden);
	}


	ActorComboBox->SetSelectedIndex(-1);
	ActorComboBox->OnSelectionChanged.AddDynamic(this, &UBuildGridTASelectorWidget::OnActorClassSelectionChanged);
}

void UBuildGridTASelectorWidget::SpawnActorByPath(const FString& ClassPath)
{
	if (ClassPath.IsEmpty() || !FPackageName::DoesPackageExist(ClassPath))
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTASelectorWidget.SpawnActorByPath] Resource invalid, path: %s"), *ClassPath);
		return;
	}

	auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
	if (!PC)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTASelectorWidget.SpawnActorByPath] PlayerController is invalid!"));
		return;
	}

	// "Blueprint'/Game/Blueprints/Main/Actor/BP_TileDemoActor1.BP_TileDemoActor1_C'"
	const FString Path = FString::Printf(TEXT("Blueprint'%s'"), *ClassPath);
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto EditingMapSave = MyGameMode->GetMutEditingMapSave();
	const TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetEditingTiles();

	TArray<FHCubeCoord> SelectedCoords = PC->GetSelectedCoords();
	for (FHCubeCoord Coord : SelectedCoords)
	{
		FHTileOrientation TileOrientation = MyGameMode->GridMapModel->GetTileOrientation(EditingMapSave->MapConfig.MapType, EditingMapSave->MapConfig.TileOrientation);
		auto TileLocation = UGridPathFindingBlueprintFunctionLib::StableCoordToWorld(EditingMapSave->MapConfig, TileOrientation, Coord);

		// 指定生成位置和旋转
		// FVector Location = FVector(0, 0, 0);
		FRotator Rotation = FRotator(0, 0, 0);

		// 生成Actor实例
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;

		// 保存数据
		// if (EditingTiles.Contains(Coord))
		// {
		// 	if (ClassPath == EditingTiles[Coord].TileEnvData.TokenActorClassPath)
		// 	{
		// 		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTASelectorWidget.SpawnActorByPath] already have this class"));
		// 		continue;
		// 	}
		//
		// 	UClass* TokenClass = LoadClass<AActor>(nullptr, *Path);
		// 	check(TokenClass);
		//
		// 	GetWorld()->SpawnActor<ATokenActor>(TokenClass, TileLocation, Rotation, SpawnParams);
		// 	// 修改
		// 	MyGameMode->GetMutEditingTiles()[Coord].TileEnvData.TokenActorClassPath = ClassPath;
		//
		// 	MyGameMode->GridMapModel->UpdateTileEnv(ETileTokenModifyType::UpdateTileEnv, EditingTiles[Coord]);
		// 	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridTASelectorWidget.SpawnActorByPath] 修改"));
		// }
		// else
		// {
		// 	UClass* TokenClass = LoadClass<AActor>(nullptr, *Path);
		// 	check(TokenClass);
		// 	ATokenActor* SpawnActor = GetWorld()->SpawnActor<ATokenActor>(TokenClass, TileLocation, Rotation, SpawnParams);
		// 	// add to scene
		// 	MyGameMode->GridMapModel->UpdateStandingActor(FHCubeCoord::Invalid, Coord, SpawnActor);
		//
		// 	// 新增
		// 	auto NewTile = FSerializableTile();
		// 	NewTile.Coord = Coord;
		// 	auto NewEnvData = FTileEnvData();
		// 	NewEnvData.TokenActorClassPath = ClassPath;
		// 	NewTile.TileEnvData = NewEnvData;
		// 	MyGameMode->GetMutEditingTiles().Add(Coord, NewTile);
		//
		// 	MyGameMode->GridMapModel->UpdateTileEnv(ETileTokenModifyType::Add, NewTile);
		//
		// 	UE_LOG(LogGridPathFinding, Log, TEXT("[UBuildGridTASelectorWidget.SpawnActorByPath] 新增"));
		// }
		MyGameMode->MarkEditingTilesDirty(Coord);
	}
}
