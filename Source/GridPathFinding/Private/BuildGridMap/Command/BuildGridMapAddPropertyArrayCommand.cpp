// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapAddPropertyArrayCommand.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "Types/SerializableTile.h"
#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "TokenFeatureInterface.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"

void UBuildGridMapAddPropertyArrayCommand::Initialize(const FHCubeCoord& InCoord,const int InActorIndex,
	const int InFeatureIndex,const FName& InPropertyArrayName)
{
	SelectedCoord = InCoord;
	ActorIndex = InActorIndex;
	FeatureIndex = InFeatureIndex;
	PropertyArrayName = InPropertyArrayName;
}

bool UBuildGridMapAddPropertyArrayCommand::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetMutEditingTiles();
	auto TokenFeatures = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord,ActorIndex)->GetComponentsByInterface(UTokenFeatureInterface::StaticClass());
	auto TokenFeatureInterface = Cast<ITokenFeatureInterface>(TokenFeatures[FeatureIndex]);
	auto& PropertiesFeature = EditingTiles[SelectedCoord].SerializableTokens[ActorIndex].Features[FeatureIndex];
	TArray<FSerializableTokenProperty> PropertyArray;
	if (auto Property = PropertiesFeature.FindMutPropertyArrayByArrayName(PropertyArrayName))
	{
		PropertyArray  = TokenFeatureInterface->CreatePropertyArray(PropertyArrayName);
		Property->PropertyArray.Add(PropertyArray);
		ArrayIndex = Property->PropertyArray.Num()-1;
	}
	auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, ActorIndex, false);
	check(ExistingTokenActor);

	ExistingTokenActor->DeserializeFeatureData(FeatureIndex,PropertiesFeature);
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenPropertyArray(ActorIndex,FeatureIndex,PropertyArrayName,*PropertiesFeature.FindMutPropertyArrayByArrayName(PropertyArrayName));
	return true;
}

bool UBuildGridMapAddPropertyArrayCommand::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	TMap<FHCubeCoord, FSerializableTile>& EditingTiles = MyGameMode->GetMutEditingTiles();
	FSerializableTokenPropertyArray* PropertyArray = EditingTiles[SelectedCoord].SerializableTokens[ActorIndex].Features[FeatureIndex].FindMutPropertyArrayByArrayName(PropertyArrayName);
	PropertyArray->PropertyArray.RemoveAt(ArrayIndex);

	auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, ActorIndex, false);
	check(ExistingTokenActor);

	ExistingTokenActor->DeserializeFeatureData(FeatureIndex,EditingTiles[SelectedCoord].SerializableTokens[ActorIndex].Features[FeatureIndex]);
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenPropertyArray(ActorIndex,FeatureIndex,PropertyArrayName,*PropertyArray);
	return true;
}

FString UBuildGridMapAddPropertyArrayCommand::GetDescription() const
{
	return FString::Printf(TEXT("增加地块 %s（第 %d 个Token 的第 %d 个Component的 %s 属性数组）"),
	*SelectedCoord.ToString(),ActorIndex+1,FeatureIndex+1,*PropertyArrayName.ToString());
}
