// Fill out your copyright notice in the Description page of Project Settings.

#include "WhiteNoiseConsumer.h"

#include "Kismet/GameplayStatics.h"
#include "CustomShadersDeclarations/Private/ComputeShaderDeclaration.h"

// Sets default values
AWhiteNoiseConsumer::AWhiteNoiseConsumer()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	Root = CreateDefaultSubobject<USceneComponent>(TEXT("Root"));
	RootComponent = Root;

	static_mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Static Mesh"));

	TimeStamp = 0;
}

// Called when the game starts or when spawned
void AWhiteNoiseConsumer::BeginPlay()
{
	Super::BeginPlay();
	FWhiteNoiseCSManager::Get()->BeginRendering();


	//Assuming that the static mesh is already using the material that we're targeting, we create an instance and assign it to it
	UMaterialInstanceDynamic* MID = static_mesh->CreateAndSetMaterialInstanceDynamic(0);
	MID->SetTextureParameterValue("InputTexture", (UTexture*)RenderTarget);
}

void AWhiteNoiseConsumer::BeginDestroy()
{
	FWhiteNoiseCSManager::Get()->EndRendering();
	Super::BeginDestroy();

}

// Called every frame
void AWhiteNoiseConsumer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	//Update parameters
	FWhiteNoiseCSParameters parameters(RenderTarget);
	TimeStamp++;
	parameters.TimeStamp = TimeStamp;
	FWhiteNoiseCSManager::Get()->UpdateParameters(parameters);
}

// Called to bind functionality to input
void AWhiteNoiseConsumer::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

