// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGirdMapChangeTokenPropertyArrayCommand.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"

void UBuildGirdMapChangeTokenPropertyArrayCommand::Initialize(const FHCubeCoord& InCoord, const int InActorIndex,
                                                              const int InFeatureIndex, const FName& InPropertyArrayName,
                                                              const int InArrayIndex,const FName& InPropertyName, const FString& InValue)
{
	SelectedCoord = InCoord;
	ActorIndex = InActorIndex;
	FeatureIndex = InFeatureIndex;
	PropertyName = InPropertyName;
	PropertyArrayName = InPropertyArrayName;
	NewValue = InValue;
	ArrayIndex = InArrayIndex;
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	const FSerializableTile& TilePtr = MyGameMode->GetEditingTile(InCoord);
	const auto& SerializableFeature = TilePtr.SerializableTokens[InActorIndex].Features[InFeatureIndex];
	if (const FSerializableTokenPropertyArray* PropertyArray = SerializableFeature.FindPropertyArrayByArrayName(InPropertyArrayName))
	{
		if (const FSerializableTokenProperty* Property = PropertyArray->FindPropertyByPropertyName(InArrayIndex, InPropertyName))
		{
			OldValue = Property->Value;
		}
	}
}

bool UBuildGirdMapChangeTokenPropertyArrayCommand::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto TilePtr = MyGameMode->GetMutEditingTile(SelectedCoord);
	auto& SerializableFeature= TilePtr->SerializableTokens[ActorIndex].Features[FeatureIndex];
	// 找到对应的属性， 并更新数据
	bool bFound = false;
	TArray<FSerializableTokenProperty> PropertyCopy;
	if (FSerializableTokenPropertyArray* PropertyArray = SerializableFeature.FindMutPropertyArrayByArrayName(PropertyArrayName))
	{
		if (FSerializableTokenProperty* Property = PropertyArray->FindMutPropertyByPropertyName(ArrayIndex, PropertyName))
		{
			Property->Value = NewValue;
			bFound = true;
			PropertyCopy = PropertyArray->PropertyArray[ArrayIndex];
		}
	}

	if (!bFound)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenFeaturePropertyChanged] Property not found: %s"), *PropertyName.ToString());
		return false;
	}
	
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	// 更新对应的Actor数据
	auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, ActorIndex, false);
	check(ExistingTokenActor);

	ExistingTokenActor->UpdatePropertyArray(FeatureIndex,PropertyArrayName,ArrayIndex, TilePtr->SerializableTokens[ActorIndex].Features[FeatureIndex].FeatureClass, PropertyCopy);
	return true;
}

bool UBuildGirdMapChangeTokenPropertyArrayCommand::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto TilePtr = MyGameMode->GetMutEditingTile(SelectedCoord);
	auto& SerializableFeature = TilePtr->SerializableTokens[ActorIndex].Features[FeatureIndex];
	// 找到对应的属性， 并更新数据
	bool bFound = false;
	
	TArray<FSerializableTokenProperty> PropertyCopy;
	if (FSerializableTokenPropertyArray* PropertyArray = SerializableFeature.FindMutPropertyArrayByArrayName(PropertyArrayName))
	{
		if (FSerializableTokenProperty* Property = PropertyArray->FindMutPropertyByPropertyName(ArrayIndex, PropertyName))
		{
			Property->Value = OldValue;
			bFound = true;
			PropertyCopy = PropertyArray->PropertyArray[ArrayIndex];
		}
	}

	if (!bFound)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[ABuildGridMapGameMode.OnTokenFeaturePropertyChanged] Property not found: %s"), *PropertyName.ToString());
		return false;
	}
	
	MyGameMode->MarkEditingTilesDirty(SelectedCoord);

	// 更新对应的Actor数据
	auto ExistingTokenActor = MyGameMode->GridMapModel->GetTokenByIndex(SelectedCoord, ActorIndex, false);
	check(ExistingTokenActor);

	ExistingTokenActor->UpdatePropertyArray(FeatureIndex,PropertyArrayName,ArrayIndex, TilePtr->SerializableTokens[ActorIndex].Features[FeatureIndex].FeatureClass, PropertyCopy);

	// 更新UI上的TokenActorPanelLineProperty
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenPropertyInArray(ActorIndex,FeatureIndex,PropertyArrayName,ArrayIndex,
		*SerializableFeature.FindMutPropertyArrayByArrayName(PropertyArrayName)->FindMutPropertyByPropertyName(ArrayIndex, PropertyName));
	
	return true;
}

FString UBuildGirdMapChangeTokenPropertyArrayCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地块 %s（第 %d 个Token 的第 %d 个Component的 %s 属性数组 的 第 %d 个 元素 的 %s 属性 从 %s 到 %s）"),
	*SelectedCoord.ToString(),ActorIndex+1,FeatureIndex+1,*PropertyArrayName.ToString(),ArrayIndex+1,*PropertyName.ToString(),
	*OldValue, 
	*NewValue);
}
