// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/UI/BuildGridMapPropertyLine.h"

#include "BuildGridMap/BuildGridMapGameMode.h"
#include "Components/ComboBoxString.h"
#include "Components/TextBlock.h"

void UBuildGridMapPropertyLine::InitPropertyLine(const FSerializableTokenProperty& InProperty)
{
	PropertyType = InProperty.PropertyType;
	PropertyName = InProperty.PropertyName;
	LabelText->SetText(FText::FromName(InProperty.PropertyName));
	switch (PropertyType) {
		case ETokenPropertyType::None:
			break;
		case ETokenPropertyType::Float:
		case ETokenPropertyType::Int:
		case ETokenPropertyType::String:
			SetGeneralValue(InProperty.Value);
			break;
		case ETokenPropertyType::Vector:
			{
				FVector VectorValue;
				VectorValue.InitFromString(InProperty.Value);
				SetVectorValue(VectorValue);
			}
			break;
		case ETokenPropertyType::SoftMeshPath:
			{
				// 需要先填充ComboBox的选项
				// 从GameMode里获取可用的SoftMesh
				auto BuildGridMapGM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
				auto MeshMap = BuildGridMapGM->GetAvailableStaticMeshes();
				ComboBoxString->ClearOptions();
				ComboBoxString->AddOption(ABuildGridMapGameMode::NoneString);
				
				for (const auto& Pair : MeshMap)
				{
					ComboBoxString->AddOption(Pair.Key.ToString());
				}
				auto MappedMeshKey = BuildGridMapGM->GetStaticMeshShortName(InProperty.Value);
				SetComboBoxValue(MappedMeshKey.ToString());
			}
			break;
		case ETokenPropertyType::Bool:
			{
				SetCheckBoxValue(InProperty.Value);
			}
			break;
	}
}

void UBuildGridMapPropertyLine::Clean()
{
	OnPropertyValueChanged.Clear();
}

FString UBuildGridMapPropertyLine::GetValueString()
{
	switch (PropertyType) {
		case ETokenPropertyType::None:
		case ETokenPropertyType::Float:
		case ETokenPropertyType::Int:
		case ETokenPropertyType::String:
			return GetGeneralValue();
		case ETokenPropertyType::Vector:
			{
				FVector VectorValue = GetVectorValue();
				return VectorValue.ToString();
			}
		case ETokenPropertyType::SoftMeshPath:
			{
				auto ComboBoxValue = GetComboBoxValue();

				if (ComboBoxValue == ABuildGridMapGameMode::NoneString)
				{
					return FString(); // 返回空字符串表示没有选择任何Mesh
				}

				// 从GM中查询对应的软引用路径
				auto BuildGridMapGM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
				auto SoftPath = BuildGridMapGM->GetStaticMeshPath(FName(*ComboBoxValue));
				return SoftPath.ToString();
			}
		case ETokenPropertyType::Bool:
			{
				return GetCheckBoxValue();
			}
	}

	return FString();
}
