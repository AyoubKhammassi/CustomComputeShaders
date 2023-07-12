#pragma once

#include "CoreMinimal.h"
#include "Runtime/Engine/Classes/Engine/TextureRenderTarget2D.h"

//This struct act as a container for all the parameters that the client needs to pass to the Compute Shader Manager.
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
public:
	uint32 TimeStamp;
};


/// <summary>
/// A singleton Shader Manager for our Shader Type
/// </summary>
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

	// Call this when you want to hook onto the renderer and start executing the compute shader. The shader will be dispatched once per frame.
	void BeginRendering();

	// Stops compute shader execution
	void EndRendering();

	// Call this whenever you have new parameters to share.
	void UpdateParameters(FWhiteNoiseCSParameters& DrawParameters);
	
private:
	//Private constructor to prevent client from instanciating
	FWhiteNoiseCSManager() = default;

	//The singleton instance
	static FWhiteNoiseCSManager* instance;

	//The delegate handle to our function that will be executed each frame by the renderer
	FDelegateHandle OnPostResolvedSceneColorHandle;

	//Cached Shader Manager Parameters
	FWhiteNoiseCSParameters cachedParams;

	//Whether we have cached parameters to pass to the shader or not
	volatile bool bCachedParamsAreValid;

	//Reference to a pooled render target where the shader will write its output
	TRefCountPtr<IPooledRenderTarget> ComputeShaderOutput;
public:
	void Execute_RenderThread(FRDGBuilder& GraphBuilder, const FSceneTextures& SceneTextures);
};
