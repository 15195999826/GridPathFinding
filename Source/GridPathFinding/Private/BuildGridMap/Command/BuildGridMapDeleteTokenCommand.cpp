// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapDeleteTokenCommand.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "GridMapModel.h"
#include "TokenFeatureInterface.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"
#include "Components/VerticalBox.h"

void UBuildGridMapDeleteTokenCommand::Initialize(const FHCubeCoord& InCoord, int32 InSerializedTokenIndex)
{
	SelectedCoord = InCoord;
	DeleteSerializedTokenIndex = InSerializedTokenIndex;

	const auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	const auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, DeleteSerializedTokenIndex, false);
	if (ExistingTokenActor)
	{
		//保存TokenFeaturesComponent数据
		HasTokenFlag = true;
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

bool UBuildGridMapDeleteTokenCommand::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto TilePtr = MyGameMode->GetMutEditingTiles().Find(SelectedCoord);
	if (!TilePtr)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenDeleteClicked] Tile not found at %s"), *SelectedCoord.ToString());
		return false;
	}

	if (!TilePtr->SerializableTokens.IsValidIndex(DeleteSerializedTokenIndex))
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenDeleteClicked] Invalid token index: %d"), DeleteSerializedTokenIndex);
		return false;
	}
	
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);
	// 删除当前位置上的对应Index的TokenActor
	auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, DeleteSerializedTokenIndex, false);
	if (ExistingTokenActor)
	{
		// 删除Actor
		MyGameMode->GridMapModel->RemoveAndDestroyToken(SelectedCoord, ExistingTokenActor);
	}

	// 删除SerializableTokenData
	TilePtr->SerializableTokens.RemoveAt(DeleteSerializedTokenIndex);

	// 删除UI上的TokenActorPanel
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalDeleteTokenActorPanel(DeleteSerializedTokenIndex);
	return true;
}

bool UBuildGridMapDeleteTokenCommand::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();

	TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetMutEditingTiles();
	if (!EditingTiles.Contains(SelectedCoord))
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenDeleteClicked] Tile not found at %s"), *SelectedCoord.ToString());
		return false;
	}
	
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);
	// 还原当前位置上的对应Index的TokenActor
	auto GridMapModel = MyGameMode->GridMapModel;
	
	// 创建一个新的SerializableToken
	EditingTiles[SelectedCoord].SerializableTokens.Insert(OldTokenData, DeleteSerializedTokenIndex);
	
	// 创建一个新的TokenActorPanel
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalCreateTokenActorPanel(DeleteSerializedTokenIndex, OldTokenData);
	
	if (HasTokenFlag)
	{
		FSerializableTokenData& TokenData = MyGameMode->GetMutEditingTile(SelectedCoord)->SerializableTokens[DeleteSerializedTokenIndex];
		auto TokenActorTypes = MyGameMode->GetTokenActorTypes();
		auto TokenActorTypeStringToIndexMap = MyGameMode->GetTokenActorTypeStringToIndexMap();
		TokenData.TokenClass = OldTokenData.TokenClass;
		
		// 在该位置创建一个新的TokenActor
		FActorSpawnParameters SpawnParams;
		SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
		SpawnParams.ObjectFlags |= RF_Transient;
		auto Location = MyGameMode->GridMapModel->StableCoordToWorld(SelectedCoord);
		auto Rotation = FRotator::ZeroRotator;
		auto NewTokenActor = GetWorld()->SpawnActor<ATokenActor>(TokenData.TokenClass, Location, Rotation, SpawnParams);
		NewTokenActor->SetActorEnableCollision(MyGameMode->GridMapModel->EnableTokenCollision);
		
		//反序列化上一个TokenFeatureComponent数据
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
		MyGameMode->GridMapModel->AppendToken(SelectedCoord, NewTokenActor);
	
		// 更新UI上的TokenActorPanel
		MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenActorPanel(DeleteSerializedTokenIndex,OldTokenData);
		
		return true;
	}
	return true;

}

FString UBuildGridMapDeleteTokenCommand::GetDescription() const
{
	return FString::Printf(TEXT("删除地块 %s (的第 %d 个Token )"),
	*SelectedCoord.ToString(), DeleteSerializedTokenIndex+1);
}
