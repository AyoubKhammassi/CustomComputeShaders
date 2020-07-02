#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "WhiteNoiseConsumer.generated.h"

UCLASS()
class CUSTOMCOMPUTESHADER_API AWhiteNoiseConsumer : public APawn
{
	GENERATED_BODY()

//Properties
public:
	UPROPERTY()
		USceneComponent* Root;

	UPROPERTY(EditAnywhere)
		UStaticMeshComponent* static_mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = ShaderDemo)
		class UTextureRenderTarget2D* RenderTarget;
private:
	uint32 TimeStamp;
public:
	// Sets default values for this pawn's properties
	AWhiteNoiseConsumer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void BeginDestroy() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
