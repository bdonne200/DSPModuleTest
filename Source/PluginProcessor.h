/*
  ==============================================================================

    This file was auto-generated!

    It contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once


#include "../JuceLibraryCode/JuceHeader.h"
#include "../../Maximilian/maximilian.h"
//==============================================================================
/**
*/
class DspmoduleTestAudioProcessor  : public AudioProcessor
{
public:
    //==============================================================================
    DspmoduleTestAudioProcessor();
    ~DspmoduleTestAudioProcessor();

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;

   #ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
   #endif

    void processBlock (AudioBuffer<float>&, MidiBuffer&) override;

    //==============================================================================
    AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const String getProgramName (int index) override;
    void changeProgramName (int index, const String& newName) override;

    //==============================================================================
    void getStateInformation (MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

	void reset() override;

private:

    // Two parameters
	AudioParameterFloat* cutoffParam;
	AudioParameterChoice* filterTypeParam;
    
    // One JUCE IIR filter per channel per frequency band
    // we will cascade two low pass in series and two high pass in processBlock() to create
    // a Linkwitz-Riley filter which is ideal for creating crossover filters for multiband audio applications
	IIRFilter LPFilter1[2];
	IIRFilter LPFilter2[2];
	IIRFilter HPFilter1[2];
	IIRFilter HPFilter2[2];

    // Coefficients to be used with the JUCE IIR Filters
	IIRCoefficients LPCoefficients;
	IIRCoefficients HPCoefficients;

    // Buffers to copy input data to for processing
	AudioSampleBuffer LPBuffer;
	AudioSampleBuffer HPBuffer;

    // Maximilian filters (we need one per channel for our stereo plugin)
	maxiFilter maxiLP[2];
	maxiFilter maxiHP[2];

    //==============================================================================
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (DspmoduleTestAudioProcessor)
};
