#pragma once
#include "CoreMinimal.h"
#include "MapConfig.h"
#include "SerializableTile.h"
#include "GridMapSave.generated.h"

USTRUCT(BlueprintType)
struct FGridMapSave
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere)
	FName MapName;

	UPROPERTY(VisibleAnywhere)
	FDateTime CreateTime;

	UPROPERTY(VisibleAnywhere)
	FGridMapConfig MapConfig;

	// 设定: MapTilesSave对应的文件名为: {MapName}_Chunk_N.maptiles, 在创建FGridMapSave时会创建一个唯一的文件夹与之对应， 并且不会被改变
	UPROPERTY()
	FString ChunksDir;
};

USTRUCT(BlueprintType)
struct FGridMapTilesSave
{
	GENERATED_BODY()

	// 版本号，用于兼容未来可能的数据结构变化
	// 如 if Version== N 然后处理各个字节对应的字段
	UPROPERTY(VisibleAnywhere)
	int32 Version = 1;
	
	// 优化数据存储
	// 1. Todo: FSerializableGridTile 拆分为两个结构体， 一个记录复杂数据, 一个记录位置 + 复杂数据索引；创建公用数组, 相同的复杂数据只记录一次；
	// 2. Chunk化存储， 每一个Chunk只保存可配置数量的格子数据， 运行时可以分批加载
	UPROPERTY(VisibleAnywhere)
	TArray<FSerializableTile> GridTiles;
	
	// 自定义序列化操作符
	friend FArchive& operator<<(FArchive& Ar, FGridMapTilesSave& TilesSave)
	{
		// 序列化版本号
		Ar << TilesSave.Version;
		
		// 序列化格子数量
		int32 TileCount = TilesSave.GridTiles.Num();
		Ar << TileCount;
		
		// 加载情况下，需要先分配空间
		if (Ar.IsLoading())
		{
			TilesSave.GridTiles.Empty(TileCount);
			TilesSave.GridTiles.SetNum(TileCount);
		}
		
		// 序列化所有格子数据
		for (int32 i = 0; i < TileCount; ++i)
		{
			// 基本数据类型可以直接序列化
			Ar << TilesSave.GridTiles[i].Coord.QRS.X;
			Ar << TilesSave.GridTiles[i].Coord.QRS.Y;
			Ar << TilesSave.GridTiles[i].Coord.QRS.Z;
			Ar << TilesSave.GridTiles[i].Height;
			
			// 对于EnvironmentType，需要特殊处理
			if (Ar.IsSaving())
			{
				FString EnvTypeStr = TilesSave.GridTiles[i].EnvironmentType.ToString();
				Ar << EnvTypeStr;
			}
			else
			{
				FString EnvTypeStr;
				Ar << EnvTypeStr;
				TilesSave.GridTiles[i].EnvironmentType = FName(*EnvTypeStr);
			}
			
			// TileEnvData只有一个简单数据类型，可以直接序列化
			Ar << TilesSave.GridTiles[i].TileEnvData.TextureIndex;
			
			// 字符串可以直接序列化
			Ar << TilesSave.GridTiles[i].CustomGameplayData;
		}
		
		return Ar;
	}
};

/**
 * 用于测试二进制序列化的结构体
 */
USTRUCT(BlueprintType, Blueprintable)
struct FGGB_SaveData
{
	GENERATED_USTRUCT_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GothGirl|Build")
	int Version{ 1 };

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GothGirl|Build")
	TSubclassOf<AActor> Class;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GothGirl|Build")
	FString Data;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GothGirl|Build")
	FTransform Transform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GothGirl|Build")
	TArray<FTransform> HISM;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GothGirl|Build")
	TArray<bool> IsSlope;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GothGirl|Build")
	TArray<bool> IsInside;

	// This method is responsible for custom serialization
	friend FArchive& operator<<(FArchive& Ar, FGGB_SaveData& MyStruct)
	{
		// Serialize each member variable manually
		Ar << MyStruct.Version;
		
		if (Ar.IsSaving())
		{
			FString PathStr = MyStruct.Class ? MyStruct.Class->GetPathName() : FString();
			Ar << PathStr;
		}
		else if (Ar.IsLoading())
		{
			FString PathStr;
			Ar << PathStr;
			if (!PathStr.IsEmpty())
			{
				MyStruct.Class = LoadClass<AActor>(nullptr, *PathStr);
			}
			else
			{
				MyStruct.Class = nullptr;
			}
		}

		//Ar << MyStruct.Class;
		Ar << MyStruct.Data;
		Ar << MyStruct.Transform;
		Ar << MyStruct.HISM;
		Ar << MyStruct.IsSlope;
		Ar << MyStruct.IsInside;

		return Ar;
	}
};

