// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"

#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

//This struct act as a container for all the parameter that we'll to the Compute Shader Manager so it can update the state and exexute the shader
struct  FWhiteNoiseCSParameters
{
	UTextureRenderTarget2D* RenderTarget;


	FIntPoint GetRenderTargetSize() const
	{
		return CachedRenderTargetSize;
	}

	FWhiteNoiseCSParameters() { }
	FWhiteNoiseCSParameters(UTextureRenderTarget2D* IORenderTarget)
		: RenderTarget(IORenderTarget)
	{
		CachedRenderTargetSize = RenderTarget ? FIntPoint(RenderTarget->SizeX, RenderTarget->SizeY) : FIntPoint::ZeroValue;
	}

private:
	FIntPoint CachedRenderTargetSize;
};


/**
 * 
 */
class CUSTOMSHADERSDECLARATIONS_API FWhiteNoiseCSManager
{
public:
	//Get the instance
	static FWhiteNoiseCSManager* Get()
	{
		if (!instance)
			instance = new FWhiteNoiseCSManager();
		return instance;
	};

	// Call this when you want to hook onto the renderer and start drawing. The shader will be executed once per frame.
	void BeginRendering();

	// When you are done, call this to stop drawing.
	void EndRendering();

	// Call this whenever you have new parameters to share. You could set this up to update different sets of properties at
	// different intervals to save on locking and GPU transfer time.
	void UpdateParameters(FWhiteNoiseCSParameters& DrawParameters);
	
private:
	//Private constructor to prevent client from instanciating
	FWhiteNoiseCSManager() = default;

	//The singleton instance
	static FWhiteNoiseCSManager* instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	FCriticalSection RenderEveryFrameLock;

	//Cached Shader Parameters
	FWhiteNoiseCSParameters cachedParams;
	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
public:
	void PostResolveSceneColor_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext);
	void Execute_RenderThread(const FWhiteNoiseCSParameters& parameters);
};
