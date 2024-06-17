// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#pragma once

#include "CoreMinimal.h"
#include "ObjectSpawner.h"
#include "Engine/HitResult.h"
#include "GameFramework/Actor.h"
#include "TiledAnimalSpawner.generated.h"

/**
 * 
 */

USTRUCT(BlueprintType)
struct FAnimalType
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animals")
	TSubclassOf<AActor> AnimalClass;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animals")
	int HerdSize = 1;
};

USTRUCT(BlueprintType)
struct FTileAnimalData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animals")
	TArray<AActor*> SpawnedAnimals;
};

UCLASS()
class WORLDENGINEINFINITETERRAIN_API ATiledAnimalSpawner : public AObjectSpawner
{
	GENERATED_BODY()

public:
	virtual void SpawnObject(const FHitResult Hit, const FVector ParentTileCenter) override;
	virtual void RemoveTile(const FVector TileCenter) override;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animals")
	TArray<FAnimalType> AnimalTypes;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animals")
	TEnumAsByte<EPhysicalSurface> SupportedSurfaceType;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animals")
	TMap<FVector, FTileAnimalData> AnimalsOfTile;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Animals")
	TMap<TSubclassOf<AActor>, FTileAnimalData> PooledAnimals;

	AActor* GetAnimalFromPool(const TSubclassOf<class AActor> AnimalClass);
};
