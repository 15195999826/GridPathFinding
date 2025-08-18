// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapDeleteTokenPropertyArrayCommand.h"

#include "GridMapModel.h"
#include "TokenFeatureInterface.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"

void UBuildGridMapDeleteTokenPropertyArrayCommand::Initialize(const FHCubeCoord& InCoord, const int InActorIndex,
                                                              const int InFeatureIndex, const FName& InPropertyArrayName, const int InArrayIndex)
{
	SelectedCoord = InCoord;
	ActorIndex = InActorIndex;
	FeatureIndex = InFeatureIndex;
	PropertyArrayName = InPropertyArrayName;
	ArrayIndex = InArrayIndex;
}

bool UBuildGridMapDeleteTokenPropertyArrayCommand::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetMutEditingTiles();
	auto TokenFeatures = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord,ActorIndex)->GetComponentsByInterface(UTokenFeatureInterface::StaticClass());
	auto& PropertiesFeature = EditingTiles[SelectedCoord].SerializableTokens[ActorIndex].Features[FeatureIndex];
	if (FSerializableTokenPropertyArray* PropertyArray = PropertiesFeature.FindMutPropertyArrayByArrayName(PropertyArrayName))
	{
		DeletedPropertyArrayData = PropertyArray->PropertyArray[ArrayIndex];
		PropertyArray->PropertyArray.RemoveAt(ArrayIndex);
	}
	auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, ActorIndex, false);
	check(ExistingTokenActor);

	ExistingTokenActor->DeserializeFeatureData(FeatureIndex,PropertiesFeature);
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenPropertyArray(ActorIndex, FeatureIndex, PropertyArrayName,*PropertiesFeature.FindMutPropertyArrayByArrayName(PropertyArrayName));
	return true;
}

bool UBuildGridMapDeleteTokenPropertyArrayCommand::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetMutEditingTiles();
	auto& PropertiesFeature = EditingTiles[SelectedCoord].SerializableTokens[ActorIndex].Features[FeatureIndex];
	if (auto PropertyArray = PropertiesFeature.FindMutPropertyArrayByArrayName(PropertyArrayName))
	{
		PropertyArray->PropertyArray.Insert(DeletedPropertyArrayData,ArrayIndex);
	}
	auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, ActorIndex, false);
	check(ExistingTokenActor);

	ExistingTokenActor->DeserializeFeatureData(FeatureIndex,PropertiesFeature);
	
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenPropertyArray(ActorIndex,FeatureIndex,PropertyArrayName,*PropertiesFeature.FindMutPropertyArrayByArrayName(PropertyArrayName));
	return true;
}

FString UBuildGridMapDeleteTokenPropertyArrayCommand::GetDescription() const
{
	return FString::Printf(TEXT("删除地块 %s（第 %d 个Token 的第 %d 个Component的 %s 属性数组的 第 %d 个元素）"),
	*SelectedCoord.ToString(),ActorIndex+1,FeatureIndex+1,*PropertyArrayName.ToString(),
	ArrayIndex+1);
}
