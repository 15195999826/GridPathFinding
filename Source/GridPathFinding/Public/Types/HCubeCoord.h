#pragma once

#include "CoreMinimal.h"

#include "HCubeCoord.generated.h"
/**
 * @see https://www.redblobgames.com/grids/hexagons/#coordinates
 * @see https://www.redblobgames.com/grids/hexagons/implementation.html#hex
 */
USTRUCT(BlueprintType)
struct FHCubeCoord
{
	GENERATED_USTRUCT_BODY()
	
	static const FHCubeCoord Invalid;

	FHCubeCoord() {}

	FHCubeCoord(int32 q, int32 r, int32 s)
	{
		check(q + r + s == 0);
		QRS.X = q;
		QRS.Y = r;
		QRS.Z = s;
	}

	FHCubeCoord(FIntVector _v) : QRS(_v) {}


	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "GraphAStarExample|HGTypes|Cube Coord")
	FIntVector QRS {
		FIntVector::ZeroValue
	};


	friend FHCubeCoord operator+(const FHCubeCoord &lhs, const FHCubeCoord &rhs)
	{
		return FHCubeCoord{ lhs.QRS + rhs.QRS };
	}

	friend FHCubeCoord operator-(const FHCubeCoord &lhs, const FHCubeCoord &rhs)
	{
		return FHCubeCoord{ lhs.QRS - rhs.QRS };
	}

	friend FHCubeCoord operator*(const FHCubeCoord &lhs, int32 k)
	{
		return FHCubeCoord{ lhs.QRS * k };
	}

	friend bool operator==(const FHCubeCoord &lhs, const FHCubeCoord &rhs)
	{
		return lhs.QRS == rhs.QRS;
	}

	friend bool operator!=(const FHCubeCoord &lhs, const FHCubeCoord &rhs)
	{
		return lhs.QRS != rhs.QRS;
	}

	friend uint32 GetTypeHash(const FHCubeCoord &Other)
	{
		FString TypeHash{ Other.QRS.ToString() };
		return GetTypeHash(TypeHash);
	}

	FString ToString() const
	{
		FString Ret;
		Ret.Append("{");
		Ret.Append(FString::FromInt(QRS.X));
		Ret.Append(", ");
		Ret.Append(FString::FromInt(QRS.Y));
		Ret.Append(", ");
		Ret.Append(FString::FromInt(QRS.Z));
		Ret.Append("}");
		return Ret;
	}
};