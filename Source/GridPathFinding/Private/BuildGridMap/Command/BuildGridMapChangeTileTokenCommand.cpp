// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeTileTokenCommand.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "TokenFeatureInterface.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"

void UBuildGridMapChangeTileTokenCommand::Initialize(const FHCubeCoord& InCoord,const int InActorIndex,
	const FString& InNewTileToken)
{
	//初始化
	SelectedCoord = InCoord;
	NewActorIndex = InActorIndex;
	NewTileTokenName = InNewTileToken;
	
	const auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();

	//上一个TokenAcotr
	const auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, InActorIndex, false);

	
	if (ExistingTokenActor)
	{
		//保存TokenFeaturesComponent数据
		const auto TokenFeatures = ExistingTokenActor->GetComponentsByInterface(UTokenFeatureInterface::StaticClass());
		for (const UActorComponent* Component : TokenFeatures)
		{
			FSerializableTokenFeature FeatureData;
			FeatureData.FeatureClass = Component->GetClass();
			const auto FeatureInterface = Cast<ITokenFeatureInterface>(Component);
			FeatureData.Properties = FeatureInterface->SerializeFeatureProperties().Properties;
			OldTokenData.Features.Add(FeatureData);
		}
		//保存TokenActor类
		OldTokenData.TokenClass = ExistingTokenActor->GetClass();
	}
}

bool UBuildGridMapChangeTileTokenCommand::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	if (NewTileTokenName == MyGameMode->NoneString)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[ABuildGridMapGameMode.OnTokenActorTypeChanged] 设置为None, 不创建新的TokenActor"));
		MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenActorPanel(NewActorIndex,MyGameMode->GetMutEditingTile(SelectedCoord)->SerializableTokens[NewActorIndex]);
		return false;
	}
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);
	auto MapModel = MyGameMode->GridMapModel;
	// 删除当前位置上的对应Index的TokenActor
	auto ExistingTokenActor = MapModel->GetTokenByIndex(SelectedCoord, NewActorIndex, false);
	if (ExistingTokenActor)
	{
		// 删除Actor
		MapModel->RemoveAndDestroyToken(SelectedCoord, ExistingTokenActor);
	}

	// 如果为空 则不创建新的

	if (!MyGameMode->GetTokenActorTypeStringToIndexMap().Contains(NewTileTokenName))
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenActorTypeChanged] Invalid token actor type: %s"), *NewTileTokenName);
		return false;
	}
	FSerializableTokenData& TokenData = MyGameMode->GetMutEditingTile(SelectedCoord)->SerializableTokens[NewActorIndex];
	auto TokenActorTypes = MyGameMode->GetTokenActorTypes();
	auto TokenActorTypeStringToIndexMap = MyGameMode->GetTokenActorTypeStringToIndexMap();
	TokenData.TokenClass = TokenActorTypes[TokenActorTypeStringToIndexMap[NewTileTokenName]];

	UE_LOG(LogGridPathFinding, Log, TEXT("[ABuildGridMapGameMode.OnTokenActorTypeChanged] Changed token actor type to: %s at %s"),
		*NewTileTokenName, *SelectedCoord.ToString());
	// 在该位置创建一个新的TokenActor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;
	auto Location = MapModel->StableCoordToWorld(SelectedCoord);
	auto Rotation = FRotator::ZeroRotator;
	auto NewTokenActor = GetWorld()->SpawnActor<ATokenActor>(TokenData.TokenClass, Location, Rotation, SpawnParams);
	NewTokenActor->SetActorEnableCollision(MapModel->EnableTokenCollision);
	// 首次写入序列化数据
	const auto InitialTokenData = NewTokenActor->SerializableTokenData();
	check(InitialTokenData.TokenClass == TokenData.TokenClass);
	TokenData = InitialTokenData;
	
	// 保存TokenActor和SerializableTokenData的关联, 通过在MapModel中按照相同的Index保存指针来实现
	MapModel->AppendToken(SelectedCoord, NewTokenActor);

	// 更新UI上的TokenActorPanel
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenActorPanel(NewActorIndex,TokenData);
	return true;
}

bool UBuildGridMapChangeTileTokenCommand::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);
	auto OldTilePtr = MyGameMode->GetMutEditingTile(SelectedCoord);
	if (!OldTilePtr)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenActorTypeChanged] Tile not found at %s"), *SelectedCoord.ToString());
		return false;
	}

	if (!OldTilePtr->SerializableTokens.IsValidIndex(NewActorIndex))
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenActorTypeChanged] Invalid actor index: %d"), NewActorIndex);
		return false;
	}
	auto MapModel = MyGameMode->GridMapModel;
	// 删除当前位置上的对应Index的TokenActor
	auto ExistingTokenActor = MapModel->GetTokenByIndex(SelectedCoord, NewActorIndex, false);
	if (ExistingTokenActor)
	{
		// 删除Actor
		MapModel->RemoveAndDestroyToken(SelectedCoord, ExistingTokenActor);
	}

	// 如果为空 则不创建新的
	if (OldTokenData.TokenClass == nullptr)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("[ABuildGridMapGameMode.OnTokenActorTypeChanged] 设置为None, 不创建新的TokenActor"));
		MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenActorPanel(NewActorIndex,OldTokenData);
		return true;
	}
	
	FSerializableTokenData& TokenData = OldTilePtr->SerializableTokens[NewActorIndex];
	//auto TokenActorTypes = MyGameMode->GetTokenActorTypes();
	//TokenData.TokenClass = TokenActorTypes[OldTileTokenMap];
	TokenData.TokenClass = OldTokenData.TokenClass;
	
	UE_LOG(LogGridPathFinding, Log, TEXT("[ABuildGridMapGameMode.OnTokenActorTypeChanged] Changed token actor type to: %s at %s"),
		*OldTokenData.TokenClass->GetName(), *SelectedCoord.ToString());
	// 在该位置创建一个新的TokenActor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;
	auto Location = MapModel->StableCoordToWorld(SelectedCoord);
	auto Rotation = FRotator::ZeroRotator;
	auto NewTokenActor = GetWorld()->SpawnActor<ATokenActor>(TokenData.TokenClass, Location, Rotation, SpawnParams);
	NewTokenActor->SetActorEnableCollision(MapModel->EnableTokenCollision);

	// 反序列化上一个TokenFeatureComponent数据
	for (FSerializableTokenFeature& Properties : OldTokenData.Features)
	{
		auto TokenFeatures = NewTokenActor->GetComponentsByInterface(UTokenFeatureInterface::StaticClass());
		for (UActorComponent* Component : TokenFeatures)
		{
			auto FeatureInterface = Cast<ITokenFeatureInterface>(Component);
			FeatureInterface->DeserializeFeatureProperties(Properties);
		}
	}
	
	// 保存TokenActor和SerializableTokenData的关联, 通过在MapModel中按照相同的Index保存指针来实现
	MapModel->AppendToken(SelectedCoord, NewTokenActor);
	
	// 更新UI上的TokenActorPanel
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenActorPanel(NewActorIndex,OldTokenData);
	return true;
}

FString UBuildGridMapChangeTileTokenCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地块 %s（从 %s 到 %s）"),
	*SelectedCoord.ToString(),
	OldTokenData.TokenClass!=nullptr ? *OldTokenData.TokenClass->GetName() : TEXT("None"), 
	*NewTileTokenName);
}
