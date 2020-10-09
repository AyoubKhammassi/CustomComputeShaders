# CustomComputeShader

A minimal Unreal Engine 4 porject for adding and using a compute shader. I published an article that covers the main steps of the process and I use this project as an example. 

## Modules:
* **CustomComputeShader** : The primary game module
* **CustomShadersDeclarations** : The game module that contains all the code for adding and using the compute shader

##  Shaders:
* **WhiteNoiseCS** : A simple compute shader that renders white noise to a texture

## UE4 Version
This project was created and tested using **UE4.24**. Id you're using **UE4.25**, you'll need to include **/Engine/Public/Platform.ush** in your shader file in order for it to compile.

