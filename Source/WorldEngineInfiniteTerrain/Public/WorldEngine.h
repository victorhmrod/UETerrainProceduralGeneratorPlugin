// Procedural Infinite Terrain - Unreal Engine Plugin - (c) Wise Labs 2020-2023

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "ProceduralMeshComponent.h"
#include "FoliageType_InstancedStaticMesh.h"
#include "WorldEngine.generated.h"

USTRUCT(BlueprintType)
struct FFoliageInstanceData
{
	GENERATED_BODY()
	
	UPROPERTY(BlueprintReadWrite, Category="Foliage")
	TArray<int> Instances = {0};
};

UCLASS()
class WORLDENGINEINFINITETERRAIN_API AWorldEngine : public AActor
{
	GENERATED_BODY()
	
public:	
	AWorldEngine();
	
	// Create variables for procedural terrain generation! 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	int XVertexCount = 50;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	int YVertexCount = 50;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	float CellSize = 1000;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	int NumOfSectionX = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Terrain")
	int NumOfSectionY = 2;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	int MeshSectionIndex = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Replicated, Category="Terrain")
	bool RandomizeTerrainLayout = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float MountainHeight  = 20000;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float BoulderHeight  = 2000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float PlainsHeight  = 200;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float LakeHeight  = 20;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	FVector2D PerlinOffset = {0,0};
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float MountainScale = 100000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float BoulderScale = 10000.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float PlainsScale = 500.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float LakeScale = 100.f;
	
	UPROPERTY(BlueprintReadOnly, EditAnywhere, Category="Terrain")
	UProceduralMeshComponent* PROC_Terrain;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	UMaterialInterface* TerrainMaterial;

	UPROPERTY(BlueprintReadWrite, Category="Terrain")
	bool GeneratorBusy = false;
	
	UPROPERTY(BlueprintReadOnly, Category="Terrain")
	bool TileDataReady = false;

	UPROPERTY(BlueprintReadWrite, Category="Terrain")
	TMap<FIntPoint, FIntPoint> QueuedTiles;
	
	UPROPERTY(BlueprintReadWrite, Category="Terrain")
	TMap<FIntPoint, FIntPoint> RemoveLODQueue;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Terrain")
	float TileReplaceableDistance = 200000.f;
	// -----------------

	// Create variables for procedural foliage generation! 
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	TArray<UFoliageType_InstancedStaticMesh*> FoliageTypes;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	bool RandomizeFoliage = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	int InitialSeed = 0;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	FRandomStream RandomStream = 0;

	UPROPERTY(BlueprintReadWrite, Category="Foliage")
	TArray<FVector> FoliagePoints;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	int GrowthProbabilityPercentage = 30;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	int MaxClusterSize = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	int InstanceOffset = 1000;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	int InstanceOffsetVariation = 200;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	FVector InstanceScale = FVector(1, 1, 1);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	float ScaleVariationMultiplier = 0.2;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	TArray<UInstancedStaticMeshComponent*> FoliageComponents;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Foliage")
	TMap<UInstancedStaticMeshComponent*, FFoliageInstanceData> ReplaceableFoliagePool;
	// -----------------
	
	int SectionIndexX = 0;
	int SectionIndexY = 0;
	int CellLODLevel = 1;
	
	TArray<int32> Triangles;

	// Subset mesh data
	TArray<FVector> SubVertices;
	TArray<FVector2D> SubUVs;
	TArray<int32> SubTriangles;
	TArray<FVector> SubNormals;
	TArray<FProcMeshTangent> SubTangents;

protected:
	virtual void BeginPlay() override;

public:	
	virtual void Tick(float DeltaTime) override;

	UFUNCTION(BlueprintCallable, Category="Terrain")
	void GenerateTerrain(const int InSectionIndexX, const int InSectionIndexY, const int LODFactor);
	
	UFUNCTION(BlueprintCallable, Category="Terrain")
	void GenerateTerrainAsync(const int InSectionIndexX, const int InSectionIndexY, const int LODLevel);

	UFUNCTION(BlueprintCallable, Category="Terrain")
	int DrawTile();

	UFUNCTION(BlueprintCallable, Category="Terrain")
	FVector GetPlayerLocation();

	UFUNCTION(BlueprintCallable, Category="Terrain")
	FVector2D GetTileLocation(FIntPoint TileCoordinate);

	UFUNCTION(BlueprintCallable, Category="Terrain")
	FIntPoint GetClosestQueuedTile();

	UFUNCTION(BlueprintCallable, Category="Terrain")
	int GetFurthestUpdatableTile();
	
	UFUNCTION(BlueprintCallable, Category="Foliage")
	void RemoveFoliageTile(const int TileIndex);

	UFUNCTION(BlueprintCallable, Category="Foliage")
	void AddFoliageInstances(const FVector InLocation);

	UFUNCTION(BlueprintCallable, Category="Foliage")
	void SpawnFoliageCluster(UFoliageType_InstancedStaticMesh* FoliageType, UInstancedStaticMeshComponent* FoliageIsmComponent, const FVector ClusterLocation);

	UFUNCTION(Server, Reliable, Category="Terrain")
	void RandomizeTerrain();
	
	float GetHeight(const FVector2D Location) const;
	
	float PerlinNoiseExtended(const FVector2D Location, const float Scale, const float Amplitude, const FVector2D Offset) const;

	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
};

// Async work system
class FAsyncWorldGeneration : public FNonAbandonableTask

{
public:
	FAsyncWorldGeneration(AWorldEngine* InwWorldGeneration): WorldGeneration(InwWorldGeneration) {}
	FORCEINLINE static TStatId GetStatId()
	{
		RETURN_QUICK_DECLARE_CYCLE_STAT(FAsyncWorldGeneration, STATGROUP_ThreadPoolAsyncTasks);
	}
	
	void DoWork();
	
private:
	AWorldEngine* WorldGeneration;
};