// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TokenActorData.generated.h"

// 表示向量的结构体
USTRUCT(BlueprintType)
struct FTokenVectorStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float X = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Y = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Z = 0.0f;

	// 默认构造函数
	FTokenVectorStruct()
	{
	}

	// 从FVector构造
	FTokenVectorStruct(const FVector& Vector)
		: X(Vector.X), Y(Vector.Y), Z(Vector.Z)
	{
	}

	// 转换为FVector
	FVector ToFVector() const
	{
		return FVector(X, Y, Z);
	}
};

// 表示四元数的结构体
USTRUCT(BlueprintType)
struct FTokenQuatStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float X = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Y = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Z = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float W = 1.0f;

	// 默认构造函数
	FTokenQuatStruct()
	{
	}

	// 从FQuat构造
	FTokenQuatStruct(const FQuat& Quat)
		: X(Quat.X), Y(Quat.Y), Z(Quat.Z), W(Quat.W)
	{
	}

	// 转换为FQuat
	FQuat ToFQuat() const
	{
		return FQuat(X, Y, Z, W);
	}
};

// 表示旋转的结构体
USTRUCT(BlueprintType)
struct FTokenRotatorStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Pitch = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Yaw = 0.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Roll = 0.0f;

	// 默认构造函数
	FTokenRotatorStruct()
	{
	}

	// 从FRotator构造
	FTokenRotatorStruct(const FRotator& Rotator)
		: Pitch(Rotator.Pitch), Yaw(Rotator.Yaw), Roll(Rotator.Roll)
	{
	}

	// 转换为FRotator
	FRotator ToFRotator() const
	{
		return FRotator(Pitch, Yaw, Roll);
	}
};

// 表示变换的结构体
USTRUCT(BlueprintType)
struct FTokenTransformStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTokenVectorStruct Location;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTokenQuatStruct Rotation;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTokenVectorStruct Scale;

	// 默认构造函数
	FTokenTransformStruct()
		: Scale(FVector(1.0f, 1.0f, 1.0f))
	{
	}

	// 从FTransform构造
	FTokenTransformStruct(const FTransform& Transform)
		: Location(Transform.GetLocation())
		  , Rotation(Transform.GetRotation())
		  , Scale(Transform.GetScale3D())
	{
	}

	// 转换为FTransform
	FTransform ToFTransform() const
	{
		return FTransform(Rotation.ToFQuat(), Location.ToFVector(), Scale.ToFVector());
	}
};

// 表示材质的结构体
USTRUCT(BlueprintType)
struct FTokenMaterialStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MaterialPath;
};

// 表示组件的基础结构体
USTRUCT(BlueprintType)
struct FTokenComponentStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComponentName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ComponentClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTokenTransformStruct RelativeTransform;
};

// 表示场景组件的结构体
USTRUCT(BlueprintType)
struct FTokenSceneComponentStruct : public FTokenComponentStruct
{
	GENERATED_BODY()

	// 场景组件特有的属性可以在这里添加
};

// 表示静态网格组件的结构体
USTRUCT(BlueprintType)
struct FTokenStaticMeshComponentStruct : public FTokenComponentStruct
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString MeshPath;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TArray<FTokenMaterialStruct> Materials;
};

// 表示TokenActor的完整结构体
USTRUCT(BlueprintType)
struct FTokenActorStruct
{
	GENERATED_BODY()

	FTokenActorStruct()
	{
	}

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActorName;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString ActorClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTokenTransformStruct Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTokenSceneComponentStruct SceneRoot;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTokenStaticMeshComponentStruct MeshComponent;

	bool IsValid() const
	{
		return !ActorName.IsEmpty() && !ActorClass.IsEmpty();
	}
};


// enum class ESeriType
// {
// 	Verctor,
// 	float,
// 	Struct
// };
//
// USTRUCT()
// struct FSeriProperty
// {
// 	GENERATED_BODY()
//
// 	FName Name;
//
// 	FString Value;
//
// 	ESeriType Type;
// };
//
// USTRUCT()
// struct FSeriToken
// {
// 	GENERATED_BODY()
//
// 	TArray<FSeriProperty> Properties;
// };
