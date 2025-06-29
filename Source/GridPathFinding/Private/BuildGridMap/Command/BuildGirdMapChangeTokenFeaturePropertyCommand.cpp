// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGirdMapChangeTokenFeaturePropertyCommand.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "GridPathFinding.h"
#include "GridMapModel.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"

void UBuildGirdMapChangeTokenFeaturePropertyCommand::Initialize(const FHCubeCoord& InCoord, int InActorIndex,
	int InFeatureIndex, const FName& InPropertyName, const FString& InValue)
{
	//初始化
	SelectedCoord = InCoord;
	ActorIndex = InActorIndex;
	FeatureIndex = InFeatureIndex;
	PropertyName = InPropertyName;
	NewValue = InValue;

	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto TilePtr = MyGameMode->GetMutEditingTile(InCoord);
	auto& MuteSerializableFeature = TilePtr->SerializableTokens[InActorIndex].Features[InFeatureIndex];
	for (FSerializableTokenProperty& Property : MuteSerializableFeature.Properties)
	{
		if (Property.PropertyName == InPropertyName)
		{
			// 更新属性值
			OldValue = Property.Value;
			break;
		}
	}
	
}

bool UBuildGirdMapChangeTokenFeaturePropertyCommand::Execute()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto TilePtr = MyGameMode->GetMutEditingTile(SelectedCoord);
	auto& MuteSerializableFeature = TilePtr->SerializableTokens[ActorIndex].Features[FeatureIndex];
	// 找到对应的属性， 并更新数据
	bool bFound = false;
	FSerializableTokenProperty PropertyCopy;
	for (FSerializableTokenProperty& Property : MuteSerializableFeature.Properties)
	{
		if (Property.PropertyName == PropertyName)
		{
			// 更新属性值
			Property.Value = NewValue;
			PropertyCopy = Property; // 复制更新后的属性
			bFound = true;
			break;
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

	ExistingTokenActor->UpdateFeatureProperty(FeatureIndex, MuteSerializableFeature.FeatureClass, PropertyCopy);
	return true;
}

bool UBuildGirdMapChangeTokenFeaturePropertyCommand::Undo()
{
	auto MyGameMode = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	auto TilePtr = MyGameMode->GetMutEditingTile(SelectedCoord);
	auto& MuteSerializableFeature = TilePtr->SerializableTokens[ActorIndex].Features[FeatureIndex];
	// 找到对应的属性， 并更新数据
	bool bFound = false;
	FSerializableTokenProperty PropertyCopy;
	for (FSerializableTokenProperty& Property : MuteSerializableFeature.Properties)
	{
		if (Property.PropertyName == PropertyName)
		{
			// 更新属性值
			Property.Value = OldValue;
			PropertyCopy = Property; // 复制更新后的属性
			bFound = true;
			break;
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

	ExistingTokenActor->UpdateFeatureProperty(FeatureIndex, MuteSerializableFeature.FeatureClass, PropertyCopy);

	// 更新UI上的TokenActorPanelLineProperty
	FSerializableTokenData& OldTokenData = TilePtr->SerializableTokens[ActorIndex];
	for (FSerializableTokenProperty& Property: OldTokenData.Features[FeatureIndex].Properties)
	{
		if (Property.PropertyName == PropertyName)
		{
			Property.Value = OldValue;
		}
	}
	MyGameMode->GetBuildGridMapWindow()->TileConfigWidget->IntervalUpdateTokenActorPanel(ActorIndex,OldTokenData);
	
	return true;
}

FString UBuildGirdMapChangeTokenFeaturePropertyCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地块 %s（第 %d 个Token 的第 %d 个Component的 %s 属性 从 %s 到 %s）"),
	*SelectedCoord.ToString(),ActorIndex+1,FeatureIndex+1,*PropertyName.ToString(),
	*OldValue, 
	*NewValue);
}
