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
    // Setup the two parameters
	cutoffParam = new AudioParameterFloat("cutoffParam", "Cross Over Freq (Hz)", 100.f, 10000.0f, 500.f);
	addParameter(cutoffParam);

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
	LPCoefficients = IIRCoefficients::makeLowPass(sampleRate, cutoffParam->get()); // Call makeLowPass to generate a set of coefficients for the IIR filter
	//LPCoefficients = IIRCoefficients(0.0046, 0.092, 0.0046, 1.0, -1.7991, 0.8175); // Or you can enter your own coefficients
	LPFilter1[0].setCoefficients(LPCoefficients);
	LPFilter2[0].setCoefficients(LPCoefficients);
	LPFilter1[1].setCoefficients(LPCoefficients);
	LPFilter2[1].setCoefficients(LPCoefficients);

	HPCoefficients = IIRCoefficients::makeHighPass(sampleRate, cutoffParam->get()); // Call makeHighPass to generate a set of coefficients for the IIR filter
	//HPCoefficients = IIRCoefficients(0.9042, -1.8083, 0.9042, 1.0, -1.7991, 0.8175); //  Or you can enter your own coefficients
	HPFilter1[0].setCoefficients(HPCoefficients);
	HPFilter2[1].setCoefficients(HPCoefficients);
	HPFilter1[0].setCoefficients(HPCoefficients);
	HPFilter2[1].setCoefficients(HPCoefficients);

    // Set size of temporary buffers
	LPBuffer.setSize(2, samplesPerBlock);
	HPBuffer.setSize(2, samplesPerBlock);

    // Clear buffers
	LPBuffer.clear();
	HPBuffer.clear();

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

	for (int channel = 0; channel < getTotalNumInputChannels(); ++channel)
	{
        // Copy samples into temp low pass and high pass buffers ready for filtering
        LPBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
        HPBuffer.copyFrom(channel, 0, buffer, channel, 0, buffer.getNumSamples());
            
        // Apply filter to samples in temp buffers (2 x butter low pass filter is equivelent to Linkwitz-Riley Filter
        // which are ideal for crossover filters due to their flat passband
        // https://www.rane.com/note160.html
		LPFilter1[channel].processSamples(LPBuffer.getWritePointer(channel), LPBuffer.getNumSamples());
		LPFilter2[channel].processSamples(LPBuffer.getWritePointer(channel), LPBuffer.getNumSamples());
		HPFilter1[channel].processSamples(HPBuffer.getWritePointer(channel), HPBuffer.getNumSamples());
		HPFilter2[channel].processSamples(HPBuffer.getWritePointer(channel), HPBuffer.getNumSamples());
            
		// Here could be were you would put your processing code for each band

        // Multipy high pass by –1 so that the branches of your crossover pair are in-phase
        HPBuffer.applyGain(channel, 0, HPBuffer.getNumSamples(), -1.0f);

        HPBuffer.addFrom(channel, 0, LPBuffer, channel, 0, LPBuffer.getNumSamples()); // Mix low pass and high pass samples together
        buffer.copyFrom(channel, 0, HPBuffer, channel, 0, HPBuffer.getNumSamples()); // Copy mixed samples to output buffer
	}
}
//==============================================================================
bool DspmoduleTestAudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

AudioProcessorEditor* DspmoduleTestAudioProcessor::createEditor()
{
    return new GenericAudioProcessorEditor (*this);
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
