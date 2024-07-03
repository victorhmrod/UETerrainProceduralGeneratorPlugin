// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#pragma once

#include "CoreMinimal.h"
#include "WEObjectSpawner.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "Chaos/ChaosEngineInterface.h"
#include "WEGrassSpawner.generated.h"

/**
 * 
 */
UCLASS()
class WORLDENGINEINFINITETERRAIN_API AWEGrassSpawner : public AWEObjectSpawner
{
	GENERATED_BODY()

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grass")
		TArray<UFoliageType_InstancedStaticMesh*> GrassTypes;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grass")
		TArray<UInstancedStaticMeshComponent*> GrassComponents;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grass")
		TEnumAsByte<EPhysicalSurface> SupportedSurfaceType;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Grass")
		FRandomStream RandomStream = 0;

	virtual void SpawnObject(const FHitResult Hit, const FVector ParentTileCenter) override;

	virtual void RemoveTile(const FVector TileCenter) override;
};
