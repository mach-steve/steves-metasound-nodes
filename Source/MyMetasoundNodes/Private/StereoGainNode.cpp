
#include "MetasoundExecutableOperator.h"
#include "MetasoundParamHelper.h"
#include "MetasoundAudioBuffer.h"
#include "MetasoundPrimitives.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundFacade.h"
#include "DSP/FloatArrayMath.h"




#define LOCTEXT_NAMESPACE "StevesMetasoundNodes_StereoGainNode"


namespace Metasound {


	namespace StevesStereoGainNodeVertexNames {
		METASOUND_PARAM(InputAudioL, "In Audio L", "The left audio input.");
		METASOUND_PARAM(InputAudioR, "In Audio R", "The left audio input.");
		METASOUND_PARAM(InputGainLevel, "Gain Level", "How much gain to apply. Linear.");

		METASOUND_PARAM(OutputAudioL, "Out Audio L", "Output audio with gain applied.");
		METASOUND_PARAM(OutputAudioR, "Out Audio R", "Output audio with gain applied.");

	}


	class FStevesStereoGainNodeOperator : public TExecutableOperator<FStevesStereoGainNodeOperator> {

	public:
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
				{
					const FName OperatorName = TEXT("StereoGainNode");
					const FText NodeDisplayName = METASOUND_LOCTEXT("StereoGainNodeDisplayName", "Stereo gain node");
					const FText NodeDescription = METASOUND_LOCTEXT("StereoGainNodeDescription", "Applies gain to input audio.");

					FNodeClassMetadata Info;
					Info.ClassName = { FName("UE"), OperatorName, FName("Audio") };
					Info.MajorVersion = 1;
					Info.MinorVersion = 0;
					Info.DisplayName = NodeDisplayName;
					Info.Description = NodeDescription;
					Info.Author = "Steve";
					Info.CategoryHierarchy = { METASOUND_LOCTEXT("StereoGainNodeCategory", "Steve's Nodes") };
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();

					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();
			return Info;
		}

		static const FVertexInterface& GetVertexInterface()
		{
			using namespace StevesStereoGainNodeVertexNames;

			static const FVertexInterface Interface(
				FInputVertexInterface(
					TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAudioL)),
					TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAudioR)),
					TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputGainLevel), 1.0f)
				),
				FOutputVertexInterface(
					TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputAudioL)),
					TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputAudioR))

				)
			);

			return Interface;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace StevesStereoGainNodeVertexNames;

			const FInputVertexInterfaceData& InputData = InParams.InputData;

			TDataReadReference<FAudioBuffer> AudioInputL = InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputAudioL), InParams.OperatorSettings);
			TDataReadReference<FAudioBuffer> AudioInputR = InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputAudioR), InParams.OperatorSettings);
			TDataReadReference<float> GainLevel = InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputGainLevel), InParams.OperatorSettings);

			return MakeUnique<FStevesStereoGainNodeOperator>(InParams, AudioInputL, AudioInputR, GainLevel);
		}

		FStevesStereoGainNodeOperator(const FBuildOperatorParams& InParams,
			const FAudioBufferReadRef& InAudioInputL,
			const FAudioBufferReadRef& InAudioInputR,
			const FFloatReadRef& InGainLevel)

			: AudioInL(InAudioInputL)
			, AudioInR(InAudioInputR)
			, GainLevel(InGainLevel)
			, AudioOutL(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
			, AudioOutR(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))


		{
			Reset(InParams);
		}


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace StevesStereoGainNodeVertexNames;

			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputAudioL), AudioInL);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputAudioL), AudioInL);

			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputGainLevel), GainLevel);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace StevesStereoGainNodeVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputAudioL), AudioOutL);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputAudioR), AudioOutR);

		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			AudioOutL->Zero();
			AudioOutR->Zero();

		}

		void Execute()
		{

			if (*GainLevel != currentGainLevel) {

				currentGainLevel = *GainLevel;
			}
			int32 NumFrames = AudioInL->Num();

			const float* InDataL = AudioInL->GetData();
			const float* InDataR = AudioInR->GetData();

			float* OutDataL = AudioOutL->GetData();
			float* OutDataR = AudioOutR->GetData();




			for (auto i = 0; i < NumFrames; ++i)
			{
				OutDataL[i] = InDataL[i] * currentGainLevel;
				OutDataR[i] = InDataR[i] * currentGainLevel;

			}

		}


	private:

		FAudioBufferReadRef AudioInL;
		FAudioBufferReadRef AudioInR;
		FFloatReadRef GainLevel;
		float currentGainLevel = 1.f;
		float prevGainLevel = 1.f;

		FAudioBufferWriteRef AudioOutL;
		FAudioBufferWriteRef AudioOutR;


	};



	//class FMyTestNode : public FNodeFacade
	//{
	//public:
	//	FMyTestNode(const FNodeInitData& InitData)
	//		: FNodeFacade(InitData.InstanceName, InitData.InstanceID, TFacadeOperatorClass<FMyTestNodeOperator>())
	//	{
	//	}
	//};


	using FStevesStereoGainNode = TNodeFacade<FStevesStereoGainNodeOperator>;
	METASOUND_REGISTER_NODE(FStevesStereoGainNode)

} // End Namespace Metasound

#undef LOCTEXT_NAMESPACE