/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "PluginEditor.h"

//==============================================================================
DspmoduleTestAudioProcessor::DspmoduleTestAudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
     : AudioProcessor (BusesProperties()
                     #if ! JucePlugin_IsMidiEffect
                      #if ! JucePlugin_IsSynth
                       .withInput  ("Input",  AudioChannelSet::stereo(), true)
                      #endif
                       .withOutput ("Output", AudioChannelSet::stereo(), true)
                     #endif
                       )
#endif
{
	cutoffParam = new AudioParameterFloat("cutoffParam", "Cross Over Freq (Hz)", 100.f, 10000.0f, 500.f);
	addParameter(cutoffParam);

	filterTypeParam = new AudioParameterChoice("filterTypeParam", "Filter choice", { "JUCE Cascaded IIRs", "Maximilian IIR" }, 0);
	addParameter(filterTypeParam);

}

DspmoduleTestAudioProcessor::~DspmoduleTestAudioProcessor()
{
}

//==============================================================================
const String DspmoduleTestAudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool DspmoduleTestAudioProcessor::acceptsMidi() const
{
   #if JucePlugin_WantsMidiInput
    return true;
   #else
    return false;
   #endif
}

bool DspmoduleTestAudioProcessor::producesMidi() const
{
   #if JucePlugin_ProducesMidiOutput
    return true;
   #else
    return false;
   #endif
}

bool DspmoduleTestAudioProcessor::isMidiEffect() const
{
   #if JucePlugin_IsMidiEffect
    return true;
   #else
    return false;
   #endif
}

double DspmoduleTestAudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

int DspmoduleTestAudioProcessor::getNumPrograms()
{
    return 1;   // NB: some hosts don't cope very well if you tell them there are 0 programs,
                // so this should be at least 1, even if you're not really implementing programs.
}

int DspmoduleTestAudioProcessor::getCurrentProgram()
{
    return 0;
}

void DspmoduleTestAudioProcessor::setCurrentProgram (int index)
{
}

const String DspmoduleTestAudioProcessor::getProgramName (int index)
{
    return {};
}

void DspmoduleTestAudioProcessor::changeProgramName (int index, const String& newName)
{
}

//==============================================================================
void DspmoduleTestAudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
	LPCoefficients = IIRCoefficients::makeLowPass(sampleRate, cutoffParam->get());
	//LPCoefficients = IIRCoefficients(0.0046, 0.092, 0.0046, 1.0, -1.7991, 0.8175); // Custom
	LPFilter1[0].setCoefficients(LPCoefficients);
	LPFilter2[0].setCoefficients(LPCoefficients);
	LPFilter1[1].setCoefficients(LPCoefficients);
	LPFilter2[1].setCoefficients(LPCoefficients);

	HPCoefficients = IIRCoefficients::makeHighPass(sampleRate, cutoffParam->get());
	//HPCoefficients = IIRCoefficients(0.9042, -1.8083, 0.9042, 1.0, -1.7991, 0.8175); // Custom
	HPFilter1[0].setCoefficients(HPCoefficients);
	HPFilter2[1].setCoefficients(HPCoefficients);
	HPFilter1[0].setCoefficients(HPCoefficients);
	HPFilter2[1].setCoefficients(HPCoefficients);

	LPBuffer.setSize(2, samplesPerBlock);
	HPBuffer.setSize(2, samplesPerBlock);

	LPBuffer.clear();
	HPBuffer.clear();

	maxiSettings::setup(sampleRate, getNumInputChannels(), samplesPerBlock);

}

void DspmoduleTestAudioProcessor::releaseResources()
{
    // When playback stops, you can use this as an opportunity to free up any
    // spare memory, etc.
}

#ifndef JucePlugin_PreferredChannelConfigurations
bool DspmoduleTestAudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
  #if JucePlugin_IsMidiEffect
    ignoreUnused (layouts);
    return true;
  #else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != AudioChannelSet::mono()
     && layouts.getMainOutputChannelSet() != AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
   #if ! JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
   #endif

    return true;
  #endif
}
#endif

void DspmoduleTestAudioProcessor::processBlock(AudioBuffer<float>& buffer, MidiBuffer& midiMessages)
{
	ScopedNoDenormals noDenormals;

	// In case we have more outputs than inputs, this code clears any output
	// channels that didn't contain input data
	for (auto i = getTotalNumInputChannels(); i < getTotalNumOutputChannels(); ++i)
		buffer.clear(i, 0, buffer.getNumSamples());

	// Copy input data to temp buffers for processing by filters (for left and right channels)
	LPBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
	LPBuffer.copyFrom(1, 0, buffer, 1, 0, buffer.getNumSamples());
	HPBuffer.copyFrom(0, 0, buffer, 0, 0, buffer.getNumSamples());
	HPBuffer.copyFrom(1, 0, buffer, 1, 0, buffer.getNumSamples());

	if (filterTypeParam->getIndex() == 0) {

		for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
		{
			LPFilter1[channel].processSamples(LPBuffer.getWritePointer(channel), LPBuffer.getNumSamples());
			LPFilter2[channel].processSamples(LPBuffer.getWritePointer(channel), LPBuffer.getNumSamples());
			HPFilter1[channel].processSamples(HPBuffer.getWritePointer(channel), HPBuffer.getNumSamples());
			HPFilter2[channel].processSamples(HPBuffer.getWritePointer(channel), HPBuffer.getNumSamples());
		}

		HPBuffer.applyGain(-1.0f);

	}
	else {

		for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
		{
			maxiLP[channel].lopass(*LPBuffer.getWritePointer(channel), cutoffParam->get());
			maxiHP[channel].hipass(*HPBuffer.getWritePointer(channel), cutoffParam->get());
			LPBuffer.applyGain(0.707f);
			HPBuffer.applyGain(0.707f);
		}
	}

	// Copy data in buffers back to output buffer
	HPBuffer.addFrom(0, 0, LPBuffer, 0, 0, LPBuffer.getNumSamples()); // Mix LP and HP for left channel
	HPBuffer.addFrom(1, 0, LPBuffer, 1, 0, LPBuffer.getNumSamples()); 
	buffer.copyFrom(0, 0, HPBuffer, 0, 0, HPBuffer.getNumSamples());
	buffer.copyFrom(1, 0, HPBuffer, 1, 0, HPBuffer.getNumSamples());
	
}
//==============================================================================
bool DspmoduleTestAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DspmoduleTestAudioProcessor::createEditor()
{
    return new GenericAudioProcessorEditor (this);
}

//==============================================================================
void DspmoduleTestAudioProcessor::getStateInformation (MemoryBlock& destData)
{
    // You should use this method to store your parameters in the memory block.
    // You could do that either as raw data, or use the XML or ValueTree classes
    // as intermediaries to make it easy to save and load complex data.
}

void DspmoduleTestAudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    // You should use this method to restore your parameters from this memory block,
    // whose contents will have been created by the getStateInformation() call.
}

//==============================================================================
// This creates new instances of the plugin..
AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new DspmoduleTestAudioProcessor();
}

void DspmoduleTestAudioProcessor::reset() 
{
	for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
	{
		LPFilter1[channel].reset();
		HPFilter1[channel].reset();
		LPFilter2[channel].reset();
		HPFilter2[channel].reset();
	}

}