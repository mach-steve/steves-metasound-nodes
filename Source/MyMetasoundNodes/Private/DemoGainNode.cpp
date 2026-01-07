#include "MetasoundExecutableOperator.h"
#include "MetasoundParamHelper.h"
#include "MetasoundAudioBuffer.h"
#include "MetasoundPrimitives.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundFacade.h"
#include "DSP/FloatArrayMath.h"




#define LOCTEXT_NAMESPACE "StevesMetasoundNodes_DemoGainNode"


namespace Metasound {


	namespace StevesDemoGainNodeVertexNames {
		METASOUND_PARAM(InputAudio, "In Audio", "The audio input.");
		METASOUND_PARAM(InputGainLevel, "Gain Level", "How much gain to apply. Linear.");

		METASOUND_PARAM(OutputAudio, "Out Audio", "Output audio with gain applied.");
	}


class FStevesDemoGainNodeOperator : public TExecutableOperator<FStevesDemoGainNodeOperator> {

public:
	static const FNodeClassMetadata& GetNodeInfo()
	{
		auto InitNodeInfo = []() -> FNodeClassMetadata
			{
				const FName OperatorName = TEXT("StevesDemoGainNode");
				const FText NodeDisplayName = METASOUND_LOCTEXT("DemoGainNodeDisplayName", "Demo Gain Node");
				const FText NodeDescription = METASOUND_LOCTEXT("DemoGainNodeDescription", "Applies gain to input audio.");

				FNodeClassMetadata Info;
				Info.ClassName = { FName("UE"), OperatorName, FName("Audio") };
				Info.MajorVersion = 1;
				Info.MinorVersion = 0;
				Info.DisplayName = NodeDisplayName;
				Info.Description = NodeDescription;
				Info.Author = "Steve";
				Info.CategoryHierarchy = { METASOUND_LOCTEXT("DemoGainNodeCategory", "Steve's Nodes") };
				Info.PromptIfMissing = PluginNodeMissingPrompt;
				Info.DefaultInterface = GetVertexInterface();

				return Info;
			};

		static const FNodeClassMetadata Info = InitNodeInfo();
		return Info;
	}

	static const FVertexInterface& GetVertexInterface()
	{
		using namespace StevesDemoGainNodeVertexNames;

		static const FVertexInterface Interface(
			FInputVertexInterface(
				TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAudio)),
				TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputGainLevel), 1.0f)
			),
			FOutputVertexInterface(
				TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputAudio))
			)
		);

		return Interface;
	}

	static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
	{
		using namespace StevesDemoGainNodeVertexNames;

		const FInputVertexInterfaceData& InputData = InParams.InputData;

		TDataReadReference<FAudioBuffer> AudioInput = InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputAudio), InParams.OperatorSettings);
		TDataReadReference<float> GainLevel = InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputGainLevel), InParams.OperatorSettings);

		return MakeUnique<FStevesDemoGainNodeOperator>(InParams, AudioInput, GainLevel);
	}

	FStevesDemoGainNodeOperator(const FBuildOperatorParams& InParams,
		const FAudioBufferReadRef& InAudioInput,
		const FFloatReadRef& InGainLevel)

		: AudioIn(InAudioInput)
		, GainLevel(InGainLevel)
		, AudioOut(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))

	{
		Reset(InParams);
	}


	virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
	{
		using namespace StevesDemoGainNodeVertexNames;

		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputAudio), AudioIn);
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputGainLevel), GainLevel);
	}

	virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
	{
		using namespace StevesDemoGainNodeVertexNames;
		InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputAudio), AudioOut);
	}

	void Reset(const IOperator::FResetParams& InParams)
	{
		AudioOut->Zero();

	}

	void Execute()
	{

		if (*GainLevel != currentGainLevel) {

			currentGainLevel = *GainLevel;
		}
			int32 NumFrames = AudioIn->Num();

			const float* InData = AudioIn->GetData();
			float* OutData = AudioOut->GetData();
			


			for (auto i = 0; i < NumFrames; ++i)
			{
				OutData[i] = InData[i] * currentGainLevel;
			}

	}


private:

	FAudioBufferReadRef AudioIn;
	FFloatReadRef GainLevel;
	float currentGainLevel = 1.f;
	float prevGainLevel = 1.f;

	FAudioBufferWriteRef AudioOut;


};



//class FStevesDemoGainNode : public FNodeFacade
//{
//public:
//	FStevesDemoGainNode(const FNodeInitData& InitData)
//		: FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FStevesDemoGainNodeOperator>())
//	{
//	}
//};


using FStevesDemoGainNode = TNodeFacade<FStevesDemoGainNodeOperator>;
METASOUND_REGISTER_NODE(FStevesDemoGainNode)
	
} // End Namespace Metasound

#undef LOCTEXT_NAMESPACE