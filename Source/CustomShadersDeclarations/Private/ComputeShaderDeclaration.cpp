// Fill out your copyright notice in the Description page of Project Settings.


#include "ComputeShaderDeclaration.h"

#include "GlobalShader.h"
#include "ShaderParameterStruct.h"
#include "RenderGraphUtils.h"
#include "RenderTargetPool.h"


#include "Modules/ModuleManager.h"

#define NUM_THREADS_PER_GROUP_DIMENSION 32

/// <summary>
/// Internal class thet holds the parameters and connects the HLSL Shader to the engine
/// </summary>
class FWhiteNoiseCS : public FGlobalShader
{
public:
	//Declare this class as a global shader
	DECLARE_GLOBAL_SHADER(FWhiteNoiseCS);
	//Tells the engine that this shader uses a structure for  is parameters
	SHADER_USE_PARAMETER_STRUCT(FWhiteNoiseCS, FGlobalShader);
	/// <summary>
	/// DECLARATION OF THE PARAMETER STRUCTURE
	/// The parameters must match the parameters in the HLSL code
	/// For each parameter, provide the C++ type, and the name (Same name used in HLSL code)
	/// </summary>
	BEGIN_SHADER_PARAMETER_STRUCT(FParameters, )
		SHADER_PARAMETER_UAV(RWTexture2D<float>, OutputTexture)
		SHADER_PARAMETER(FVector2D, Dimensions)
		SHADER_PARAMETER(UINT, TimeStamp)
	END_SHADER_PARAMETER_STRUCT()

public:
	//Called by the engine to determine which permutations to compile for this shader
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return IsFeatureLevelSupported(Parameters.Platform, ERHIFeatureLevel::SM5);
	}

	//Modifies the compilations environment of the shader
	static inline void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);

		//We're using it here to add some preprocessor defines. That way we don't have to change both C++ and HLSL code when we change the value for NUM_THREADS_PER_GROUP_DIMENSION
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_X"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Y"), NUM_THREADS_PER_GROUP_DIMENSION);
		OutEnvironment.SetDefine(TEXT("THREADGROUPSIZE_Z"), 1);
	}

};

// This will tell the engine to create the shader and where the shader entry point is.
//                        ShaderType              ShaderPath             Shader function name    Type
IMPLEMENT_GLOBAL_SHADER(FWhiteNoiseCS, "/CustomShaders/WhiteNoiseCS.usf", "MainComputeShader", SF_Compute);


//Static members
FWhiteNoiseCSManager* FWhiteNoiseCSManager::instance = nullptr;


void FWhiteNoiseCSManager::BeginRendering()
{
	//If the handle is already initalized and valid, no need to do anything
	if (OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}
	bCachedParamsAreValid = false;
	//Get the Renderer Module and add our entry to the callbacks so it can be executed each frame after the scene rendering is done
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		OnPostResolvedSceneColorHandle = RendererModule->GetResolvedSceneColorCallbacks().AddRaw(this, &FWhiteNoiseCSManager::Execute_RenderThread);
	}
}

void FWhiteNoiseCSManager::EndRendering()
{
	//If the handle is not valid then there's no cleanup to do
	if (!OnPostResolvedSceneColorHandle.IsValid())
	{
		return;
	}

	//Get the Renderer Module and remove our entry from the ResolvedSceneColorCallbacks
	const FName RendererModuleName("Renderer");
	IRendererModule* RendererModule = FModuleManager::GetModulePtr<IRendererModule>(RendererModuleName);
	if (RendererModule)
	{
		RendererModule->GetResolvedSceneColorCallbacks().Remove(OnPostResolvedSceneColorHandle);
	}

	OnPostResolvedSceneColorHandle.Reset();
}

void FWhiteNoiseCSManager::UpdateParameters(FWhiteNoiseCSParameters& params)
{
	RenderEveryFrameLock.Lock();
	cachedParams = params;
	bCachedParamsAreValid = true;
	RenderEveryFrameLock.Unlock();
}


void FWhiteNoiseCSManager::Execute_RenderThread(FRHICommandListImmediate& RHICmdList, class FSceneRenderTargets& SceneContext)
{
	//If there's no cached parameters to use, skip
	//If no Render Target is supplied in the cachedParams, skip
	if (!(bCachedParamsAreValid && cachedParams.RenderTarget))
	{
		return;
	}

	//Render Thread Assertion
	check(IsInRenderingThread());


	//If the render target is not valid, get an element from the render target pool by supplying a Descriptor
	if (!ComputeShaderOutput.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("Not Valid"));
		FPooledRenderTargetDesc ComputeShaderOutputDesc(FPooledRenderTargetDesc::Create2DDesc(cachedParams.GetRenderTargetSize(), cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI->GetFormat(), FClearValueBinding::None, TexCreate_None, TexCreate_ShaderResource | TexCreate_UAV, false));
		ComputeShaderOutputDesc.DebugName = TEXT("WhiteNoiseCS_Output_RenderTarget");
		GRenderTargetPool.FindFreeElement(RHICmdList, ComputeShaderOutputDesc, ComputeShaderOutput, TEXT("WhiteNoiseCS_Output_RenderTarget"));
	}
	
	//Unbind the previously bound render targets
	UnbindRenderTargets(RHICmdList);

	//Specify the resource transition, we're executing this in post scene rendering so we set it to Graphics to Compute
	RHICmdList.TransitionResource(EResourceTransitionAccess::ERWBarrier, EResourceTransitionPipeline::EGfxToCompute, ComputeShaderOutput->GetRenderTargetItem().UAV);


	//Fill the shader parameters structure with tha cached data supplied by the client
	FWhiteNoiseCS::FParameters PassParameters;
	PassParameters.OutputTexture = ComputeShaderOutput->GetRenderTargetItem().UAV;
	PassParameters.Dimensions = FVector2D(cachedParams.GetRenderTargetSize().X, cachedParams.GetRenderTargetSize().Y);
	PassParameters.TimeStamp = cachedParams.TimeStamp;

	//Get a reference to our shader type from global shader map
	TShaderMapRef<FWhiteNoiseCS> whiteNoiseCS(GetGlobalShaderMap(GMaxRHIFeatureLevel));

	//Dispatch the compute shader
	FComputeShaderUtils::Dispatch(RHICmdList, *whiteNoiseCS, PassParameters,
		FIntVector(FMath::DivideAndRoundUp(cachedParams.GetRenderTargetSize().X, NUM_THREADS_PER_GROUP_DIMENSION),
			FMath::DivideAndRoundUp(cachedParams.GetRenderTargetSize().Y, NUM_THREADS_PER_GROUP_DIMENSION), 1));

	RHICmdList.CopyTexture(ComputeShaderOutput->GetRenderTargetItem().ShaderResourceTexture, cachedParams.RenderTarget->GetRenderTargetResource()->TextureRHI, FRHICopyTextureInfo());

}
