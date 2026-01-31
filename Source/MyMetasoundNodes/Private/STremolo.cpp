
#include "MetasoundExecutableOperator.h"
#include "MetasoundParamHelper.h"
#include "MetasoundAudioBuffer.h"
#include "MetasoundPrimitives.h"
#include "MetasoundNodeRegistrationMacro.h"
#include "MetasoundStandardNodesCategories.h"
#include "MetasoundStandardNodesNames.h"
#include "MetasoundFacade.h"
#include "DSP/FloatArrayMath.h"
#include "HarmonixDsp/Modulators/Lfo.h"
#include "HarmonixDsp/Modulators/Settings/LfoSettings.h"






#define LOCTEXT_NAMESPACE "StevesMetasoundNodes_STremoloNode"


namespace Metasound {


	namespace StevesTremoloNodeVertexNames {
		METASOUND_PARAM(InputAudio, "In Audio", "The audio input.");
		METASOUND_PARAM(InputTremoloRate, "Rate", "Tremolo rate, in Hz");
		METASOUND_PARAM(InputTremoloDepth, "Depth", "Tremolo depth.");
		METASOUND_PARAM(InputMakeupGain, "MakeupGain", "Makeup gain.");

		METASOUND_PARAM(OutputAudio, "Out Audio", "Output audio with tremolo applied.");
	}
	

	class FStevesTremoloNodeOperator : public TExecutableOperator<FStevesTremoloNodeOperator> {

	public:
		static const FNodeClassMetadata& GetNodeInfo()
		{
			auto InitNodeInfo = []() -> FNodeClassMetadata
				{
					const FName OperatorName = TEXT("StevesTremoloNode");
					const FText NodeDisplayName = METASOUND_LOCTEXT("TremoloNodeDisplayName", "STremolo");
					const FText NodeDescription = METASOUND_LOCTEXT("TremoloNodeDescription", "Applies tremolo effect to input audio.");

					FNodeClassMetadata Info;
					Info.ClassName = { FName("UE"), OperatorName, FName("Audio") };
					Info.MajorVersion = 1;
					Info.MinorVersion = 0;
					Info.DisplayName = NodeDisplayName;
					Info.Description = NodeDescription;
					Info.Author = "Steve";
					Info.CategoryHierarchy = { METASOUND_LOCTEXT("TremoloNodeCategory", "Steve's Nodes") };
					Info.PromptIfMissing = PluginNodeMissingPrompt;
					Info.DefaultInterface = GetVertexInterface();

					return Info;
				};

			static const FNodeClassMetadata Info = InitNodeInfo();
			return Info;
		}

		static const FVertexInterface& GetVertexInterface()
		{
			using namespace StevesTremoloNodeVertexNames;

			static const FVertexInterface Interface(
				FInputVertexInterface(
					TInputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputAudio)),
					TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputTremoloRate), 1.0f),
					TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputTremoloDepth), 1.0f),
					TInputDataVertex<float>(METASOUND_GET_PARAM_NAME_AND_METADATA(InputMakeupGain), 1.0f)

				),
				FOutputVertexInterface(
					TOutputDataVertex<FAudioBuffer>(METASOUND_GET_PARAM_NAME_AND_METADATA(OutputAudio))
				)
			);

			return Interface;
		}

		static TUniquePtr<IOperator> CreateOperator(const FBuildOperatorParams& InParams, FBuildResults& OutResults)
		{
			using namespace StevesTremoloNodeVertexNames;

			const FInputVertexInterfaceData& InputData = InParams.InputData;
			
			TDataReadReference<FAudioBuffer> AudioInput = InputData.GetOrCreateDefaultDataReadReference<FAudioBuffer>(METASOUND_GET_PARAM_NAME(InputAudio), InParams.OperatorSettings);
			TDataReadReference<float> TremRate = InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputTremoloRate), InParams.OperatorSettings);
			TDataReadReference<float> TremDepth = InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputTremoloDepth), InParams.OperatorSettings);
			TDataReadReference<float> TremMakeupGain = InputData.GetOrCreateDefaultDataReadReference<float>(METASOUND_GET_PARAM_NAME(InputMakeupGain), InParams.OperatorSettings);


			return MakeUnique<FStevesTremoloNodeOperator>(InParams, AudioInput, TremRate, TremDepth, TremMakeupGain);
		}

		FStevesTremoloNodeOperator(const FBuildOperatorParams& InParams,
			const FAudioBufferReadRef& InAudioInput,
			const FFloatReadRef& InTremoloRate,
			const FFloatReadRef& InTremoloDepth,
			const FFloatReadRef& InMakeupGain)

			: AudioIn(InAudioInput)
			, TremoloRate(InTremoloRate)
			, TremoloDepth(InTremoloDepth)
			, MakeupGain(InMakeupGain)
			, AudioOut(FAudioBufferWriteRef::CreateNew(InParams.OperatorSettings))
			//, TremOsc(Audio::FSineOsc(INT32(InParams.OperatorSettings.GetSampleRate()), 440.0f, 1.0f, 0.0f))

		{
			sampleRate = INT32(InParams.OperatorSettings.GetSampleRate());

			Reset(InParams);
			//TremOsc.Init(sampleRate, 2.0f, 1.0f, 0.0f);

		}


		virtual void BindInputs(FInputVertexInterfaceData& InOutVertexData) override
		{
			using namespace StevesTremoloNodeVertexNames;

			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputAudio), AudioIn);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputTremoloRate), TremoloRate);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputTremoloDepth), TremoloDepth);
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(InputMakeupGain), MakeupGain);
		}

		virtual void BindOutputs(FOutputVertexInterfaceData& InOutVertexData) override
		{
			using namespace StevesTremoloNodeVertexNames;
			InOutVertexData.BindReadVertex(METASOUND_GET_PARAM_NAME(OutputAudio), AudioOut);
		}

		void Reset(const IOperator::FResetParams& InParams)
		{
			sampleRate = INT32(InParams.OperatorSettings.GetSampleRate());
			currentTremoloRate = *TremoloRate;
			//UE_LOG(LogTemp, Warning, TEXT("RESET TRIGGERED"));
			TremOsc.Prepare(sampleRate);
			makeLfoSettings();
			AudioOut->Zero();

		}

		void Execute()
		{

			if (*TremoloDepth.Get() != currentTremoloDepth) {

				currentTremoloDepth = *TremoloDepth.Get();

				makeLfoSettings();
			}

			if (*TremoloRate.Get() != currentTremoloRate)
			{
				currentTremoloRate = *TremoloRate.Get();

				makeLfoSettings();
				//UE_LOG(LogTemp, Warning,TEXT("Frequency is: %f"), TremOsc.GetFrequency());
			}

			if (*MakeupGain.Get() != currentMakeupGain) {
				currentMakeupGain = *MakeupGain.Get();
			}


			int32 NumFrames = AudioIn->Num();

			const float* InData = AudioIn->GetData();
			float* OutData = AudioOut->GetData();



			for (auto i = 0; i < NumFrames; ++i)
			{
				TremOsc.Advance(1);
				float oscSampleValue = TremOsc.GetValue();
				float oscMultiplier = 1.0f + (oscSampleValue * currentTremoloDepth);

				//OutData[i] = oscSampleValue;
				OutData[i] = InData[i] *  oscMultiplier * currentMakeupGain;
			}

		}

		void makeLfoSettings() {
			FLfoSettings newSettings;
			newSettings.Target = ELfoTarget::None;
			newSettings.IsEnabled = true;
			newSettings.Shape = EWaveShape::Sine;
			newSettings.ShouldRetrigger = true;
			newSettings.BeatSync = false;
			newSettings.Freq = currentTremoloRate;
			newSettings.Depth = currentTremoloDepth;
			newSettings.InitialPhase = 0.0f;
			newSettings.TempoBPM = 120.f;

			TremOscSettings = newSettings;
			TremOsc.UseSettings(&TremOscSettings);
		}


	private:

		FAudioBufferReadRef AudioIn;
		FFloatReadRef TremoloRate;
		FFloatReadRef TremoloDepth;
		FFloatReadRef MakeupGain;
		float currentTremoloDepth = 1.f;
		float currentTremoloRate = 1.f;
		float currentMakeupGain = 1.f;

		FAudioBufferWriteRef AudioOut;

		Harmonix::Dsp::Modulators::FLfo TremOsc;
		FLfoSettings TremOscSettings;

		float sampleRate = 0.0f;


	};



	using FStevesTremoloNode = TNodeFacade<FStevesTremoloNodeOperator>;
	METASOUND_REGISTER_NODE(FStevesTremoloNode)

} // End Namespace Metasound

#undef LOCTEXT_NAMESPACE
