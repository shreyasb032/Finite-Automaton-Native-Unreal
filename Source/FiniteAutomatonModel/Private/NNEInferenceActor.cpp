// Fill out your copyright notice in the Description page of Project Settings.


#include "NNEInferenceActor.h"

// Sets default values
ANNEInferenceActor::ANNEInferenceActor()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ANNEInferenceActor::BeginPlay()
{
	Super::BeginPlay();
	// Example for automated loading
	if (PreLoadedModelData)
	{
		UE_LOG(LogTemp, Display, TEXT("PreLoadedModelData loaded %s"), *PreLoadedModelData->GetName());
		// You can use PreLoadedModelData here to create a model and corresponding model instance(s)
		// PreLoadedModelData will be unloaded when the owning object or actor unloads
	}
	else
	{
		UE_LOG(LogTemp, Error, TEXT("PreLoadedModelData is not set, please assign it in the editor"));
	}

	if (PreLoadedModelData)
	{
		// Example for model creation
		TWeakInterfacePtr<INNERuntimeCPU> Runtime = UE::NNE::GetRuntime<INNERuntimeCPU>(FString("NNERuntimeORTCpu"));
		if (Runtime.IsValid())
		{
			ModelHelper = MakeShared<FMyModelHelper>();

			TUniquePtr<UE::NNE::IModelCPU> Model = Runtime->CreateModel(PreLoadedModelData);
			if (Model.IsValid())
			{
				ModelHelper->ModelInstance = Model->CreateModelInstance();
				if (ModelHelper->ModelInstance.IsValid())
				{
					ModelHelper->bIsRunning = false;

					// Example for querying and testing in- and outputs
					TConstArrayView<UE::NNE::FTensorDesc> InputTensorDescs = ModelHelper->ModelInstance->GetInputTensorDescs();
					UE_LOG(LogTemp, Display, TEXT("InputTensorDescs.Num() %d"), InputTensorDescs.Num());
					//GEngine->AddOnScreenDebugMessage(-1, 15.0f, FColor::Yellow, FString::FromInt(InputTensorDescs.Num()));
					//checkf(InputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single input tensor"));
					UE::NNE::FSymbolicTensorShape SymbolicInputTensorShape = InputTensorDescs[0].GetShape();
					TArray<UE::NNE::FTensorShape> InputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicInputTensorShape) };
					FString out = "(";
					for (uint32 d : SymbolicInputTensorShape.GetData())
					{
						out += FString::FromInt(d);
						out += ",";
					}
					out += ")";
					UE_LOG(LogTemp, Display, TEXT("SymbolicInputTensorShape %s"), *out);

					checkf(SymbolicInputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable input tensor dimensions"));
					ModelHelper->ModelInstance->SetInputTensorShapes(InputTensorShapes);

					TConstArrayView<UE::NNE::FTensorDesc> OutputTensorDescs = ModelHelper->ModelInstance->GetOutputTensorDescs();
					UE_LOG(LogTemp, Display, TEXT("OutputTensorDescs.Num() %d"), OutputTensorDescs.Num());
					//checkf(OutputTensorDescs.Num() == 1, TEXT("The current example supports only models with a single output tensor"));
					UE::NNE::FSymbolicTensorShape SymbolicOutputTensorShape = OutputTensorDescs[0].GetShape();
					out = "(";
					for (uint32 d : SymbolicOutputTensorShape.GetData())
					{
						out += FString::FromInt(d);
						out += ",";
					}
					out += ")";
					UE_LOG(LogTemp, Display, TEXT("SymbolicOutputTensorShape %s"), *out);
					
					checkf(SymbolicOutputTensorShape.IsConcrete(), TEXT("The current example supports only models without variable output tensor dimensions"));
					TArray<UE::NNE::FTensorShape> OutputTensorShapes = { UE::NNE::FTensorShape::MakeFromSymbolic(SymbolicOutputTensorShape) };

					// Example for creating in- and outputs
					ModelHelper->InputData.SetNumZeroed(InputTensorShapes[0].Volume());
					ModelHelper->InputBindings.SetNumZeroed(1);
					ModelHelper->InputBindings[0].Data = ModelHelper->InputData.GetData();
					ModelHelper->InputBindings[0].SizeInBytes = ModelHelper->InputData.Num() * sizeof(float);

					ModelHelper->OutputData.SetNumZeroed(OutputTensorShapes[0].Volume());
					ModelHelper->OutputBindings.SetNumZeroed(1);
					ModelHelper->OutputBindings[0].Data = ModelHelper->OutputData.GetData();
					ModelHelper->OutputBindings[0].SizeInBytes = ModelHelper->OutputData.Num() * sizeof(float);
				}
				else
				{
					UE_LOG(LogTemp, Error, TEXT("Failed to create the model instance"));
					ModelHelper.Reset();
				}
			}
			else
			{
				UE_LOG(LogTemp, Error, TEXT("Failed to create the model"));
				ModelHelper.Reset();
			}
		}
		else
		{
			UE_LOG(LogTemp, Error, TEXT("Cannot find runtime NNERuntimeORTCpu, please enable the corresponding plugin"));
		}
	}
}

void ANNEInferenceActor::RunModel(const TArray<float>& InputData, TArray<float>& OutputData)
{
	ModelHelper->InputData = InputData;

	if (ModelHelper.IsValid())
	{
		if (ModelHelper->ModelInstance->RunSync(ModelHelper->InputBindings, ModelHelper->OutputBindings) != 0)
		{
			UE_LOG(LogTemp, Error, TEXT("Failed to run the model"));
		}

		//// Example for async inference
		//if (!ModelHelper->bIsRunning)
		//{
		//// Process ModelHelper->OutputData from the previous run here
		//// Fill in new data into ModelHelper->InputData here

		//	ModelHelper->bIsRunning = true;
		//	TSharedPtr<FMyModelHelper> ModelHelperPtr = ModelHelper;
		//	AsyncTask(ENamedThreads::AnyNormalThreadNormalTask, [ModelHelperPtr]()
		//	{
		//		if (ModelHelperPtr->ModelInstance->RunSync(ModelHelperPtr->InputBindings, ModelHelperPtr->OutputBindings) != 0)
		//		{
		//			UE_LOG(LogTemp, Error, TEXT("Failed to run the model"));
		//		}
		//		AsyncTask(ENamedThreads::GameThread, [ModelHelperPtr]()
		//			{
		//				ModelHelperPtr->bIsRunning = false;
		//			});
		//	});
		//}
	}
	else {
		UE_LOG(LogTemp, Error, TEXT("ModelHelper is not valid"));
	}
	OutputData = ModelHelper->OutputData;

}
