// Fill out your copyright notice in the Description page of Project Settings.


#include "TokenMeshFeatureComponent.h"

const FName UTokenMeshFeatureComponent::RelativePositionPropertyName = TEXT("RelativePosition");
const FName UTokenMeshFeatureComponent::RelativeRotationPropertyName = TEXT("Rotation");
const FName UTokenMeshFeatureComponent::RelativeScalePropertyName = TEXT("Scale");
const FName UTokenMeshFeatureComponent::SoftMeshPathPropertyName = TEXT("SoftMeshPath");
// const FName UTokenMeshFeatureComponent::HiddenInGamePropertyName = TEXT("HiddenInGame");

UTokenMeshFeatureComponent::UTokenMeshFeatureComponent(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer)
{
	
}

TArray<FSerializableTokenProperty> UTokenMeshFeatureComponent::SerializeFeatureProperties() const
{
	TArray<FSerializableTokenProperty> Properties;

	// RelativePosition
	auto RelativePosition = GetRelativeLocation();
	FSerializableTokenProperty RelativePositionProperty{ETokenPropertyType::Vector, RelativePositionPropertyName, RelativePosition.ToString()};
	Properties.Add(RelativePositionProperty);
	
	// Scale
	auto Scale = GetRelativeScale3D();
	FSerializableTokenProperty ScaleProperty{ETokenPropertyType::Vector, RelativeScalePropertyName, Scale.ToString()};
	Properties.Add(ScaleProperty);
	
	// Rotation
	auto Rotation = GetRelativeRotation();
	FSerializableTokenProperty RotationProperty{ETokenPropertyType::Vector, RelativeRotationPropertyName, Rotation.Vector().ToString()};
	Properties.Add(RotationProperty);
	
	// SoftMesh
	TSoftObjectPtr<UStaticMesh> SoftMesh = GetStaticMesh();
	// 获取底层的 FSoftObjectPath，然后将其转换为字符串
	FSoftObjectPath SoftObjectPath = SoftMesh.ToSoftObjectPath();
	FSerializableTokenProperty MeshPathProperty{ETokenPropertyType::SoftMeshPath, SoftMeshPathPropertyName, SoftObjectPath.ToString()};
	Properties.Add(MeshPathProperty);

	// // bool Hidden In Game
	// FSerializableTokenProperty HiddenInGameProperty{
	// 	ETokenPropertyType::Bool,
	// 	HiddenInGamePropertyName,
	// 	FString::FromInt(HiddenInProjectGame ? 1 : 0) // 将bool转换为字符串
	// };
	// Properties.Add(HiddenInGameProperty);
	
	return Properties;
}

void UTokenMeshFeatureComponent::DeserializeFeatureProperties(const TArray<FSerializableTokenProperty>& Properties)
{
	for (const auto& Property : Properties)
	{
		UpdateFeatureProperty(Property);
	}
}

void UTokenMeshFeatureComponent::UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty)
{
	if (InNewProperty.PropertyName == RelativePositionPropertyName)
	{
		FVector RelativePosition;
		RelativePosition.InitFromString(InNewProperty.Value);
		SetRelativeLocation(RelativePosition);
	}
	else if (InNewProperty.PropertyName == RelativeRotationPropertyName)
	{
		FVector RotationVector;
		RotationVector.InitFromString(InNewProperty.Value);
		FRotator Rotation = FRotator::MakeFromEuler(RotationVector);
		SetRelativeRotation(Rotation);
	}
	else if (InNewProperty.PropertyName == RelativeScalePropertyName)
	{
		FVector Scale;
		Scale.InitFromString(InNewProperty.Value);
		SetRelativeScale3D(Scale);
	}
	else if (InNewProperty.PropertyName == SoftMeshPathPropertyName)
	{
		// 如果字符串为空， 则设置Mesh为nullptr
		if (InNewProperty.Value.IsEmpty())
		{
			SetStaticMesh(nullptr);
			return;
		}
		
		// 从字符串创建 FSoftObjectPath，然后用它来创建 TSoftObjectPtr
		FSoftObjectPath SoftObjectPath(InNewProperty.Value);
		TSoftObjectPtr<UStaticMesh> SoftMesh(SoftObjectPath);
		SetStaticMesh(SoftMesh.LoadSynchronous());
	}
// 	else if (InNewProperty.PropertyName == HiddenInGamePropertyName)
// 	{
// #if WITH_EDITOR
// 		auto GM = GetWorld()->GetAuthGameMode();
// 		if (Cast<ABuildGridMapGameMode>(GM))
// 		{
// 			return;
// 		}
// #endif
// 		int bHiddenInGameInt = FCString::Atoi(*InNewProperty.Value);
// 		HiddenInProjectGame = (bHiddenInGameInt != 0);
// 		SetVisibility(HiddenInProjectGame);
// 	}
}
