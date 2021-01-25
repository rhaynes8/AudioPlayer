#pragma once

#include <JuceHeader.h>

class MainComponent  : public juce::AudioAppComponent,
                       public juce::ChangeListener
{
public:
    //==============================================================================
    MainComponent();
    ~MainComponent() override;

    //==============================================================================
    void prepareToPlay (int samplesPerBlockExpected, double sampleRate) override;
    void getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill) override;
    void releaseResources() override;

    //==============================================================================
    void paint (juce::Graphics& g) override;
    void resized() override;

private:
    enum TransportState
    {
        Stopped,
        Starting,
        Playing,
        Pausing,
        Paused,
        Stopping
    };

    void openButtonClicked();
    void playButtonClicked();
    void stopButtonClicked();
    void transportStateChanged(TransportState newState);

    
    void changeListenerCallback (juce::ChangeBroadcaster *source) override;
    // Place above transport - else transport will try and access it on the moment of its deletion and exception will be thrown
    std::unique_ptr<juce::AudioFormatReaderSource> playSource;
    
    // Manage the type of files that app can read
    juce::AudioFormatManager formatManager;
    juce::AudioTransportSource transport;
    TransportState state;
    
    juce::TextButton openButton;
    juce::TextButton playButton;
    juce::TextButton stopButton;
    juce::TextButton pauseButton;
    juce::Label nameLabel;
    juce::Label timeLabel;
    
    int time = transport.getCurrentPosition();
    juce::int64 totalTime = transport.getLengthInSeconds();
    //==============================================================================
    // Your private member variables go here...


    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (MainComponent)
};
