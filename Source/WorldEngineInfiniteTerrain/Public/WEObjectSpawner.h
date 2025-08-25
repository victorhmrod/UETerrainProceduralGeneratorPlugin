// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Engine/HitResult.h"
#include "WEObjectSpawner.generated.h"

UCLASS()
class WORLDENGINEINFINITETERRAIN_API AWEObjectSpawner : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AWEObjectSpawner();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SpawnGrid")
	int CellSize = 5000;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SpawnGrid")
	int SubCellSize = 500;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SpawnGrid")
	int SubCellRandomOffset = 200;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SpawnGrid")
	int CellCount = 4;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="SpawnGrid")
	int TraceDistance = 5000;

	TArray<FVector2D> SpawnedTiles = {FVector2D(0,0)};
	

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="SpawnGrid")
	FVector GetPlayerCell();

	UFUNCTION(BlueprintCallable, Category="SpawnGrid")
	void UpdateTiles();

	UFUNCTION(BlueprintCallable, Category="SpawnGrid")
	void UpdateTile(const FVector TileCenter);

	UFUNCTION(BlueprintCallable, Category="SpawnGrid")
	virtual void SpawnObject(const FHitResult Hit, const FVector ParentTileCenter);

	UFUNCTION(BlueprintCallable, Category="SpawnGrid")
	void RemoveFarTiles();

	UFUNCTION(BlueprintCallable, Category="SpawnGrid")
	virtual void RemoveTile(const FVector TileCenter);

};
