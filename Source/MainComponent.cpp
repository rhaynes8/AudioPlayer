#include "MainComponent.h"

//==============================================================================
MainComponent::MainComponent() : state(Stopped), openButton("Browse"), playButton("Play"), stopButton("Stop"), pauseButton("Pause")
{
    // Make sure you set the size of the component after
    // you add any child components.
    setSize (240, 240);
    addAndMakeVisible(&openButton);
    addAndMakeVisible(&playButton);
    addAndMakeVisible(&stopButton);
    addAndMakeVisible(&pauseButton);
    
    transportStateChanged(Stopped);
    
    openButton.onClick = [this] { openButtonClicked(); };
    playButton.onClick = [this] { playButtonClicked(); };
    stopButton.onClick = [this] { stopButtonClicked(); };
    
    addAndMakeVisible(nameLabel);
    addAndMakeVisible(timeLabel);
    
    openButton.setColour(juce::TextButton::buttonColourId, juce::Colours::blue);
    playButton.setColour(juce::TextButton::buttonColourId, juce::Colours::green);
    stopButton.setColour(juce::TextButton::buttonColourId, juce::Colours::red);
    
    playButton.setEnabled(true);
    stopButton.setEnabled(false);

    // Allows us to register basic wav and aiff files
    formatManager.registerBasicFormats();
    // Add a listener to detect change
    transport.addChangeListener(this);
    
    
    // Some platforms require permissions to open input channels so request that here
    if (juce::RuntimePermissions::isRequired (juce::RuntimePermissions::recordAudio)
        && ! juce::RuntimePermissions::isGranted (juce::RuntimePermissions::recordAudio))
    {
        juce::RuntimePermissions::request (juce::RuntimePermissions::recordAudio,
                                           [&] (bool granted) { setAudioChannels (granted ? 2 : 0, 2); });
    }
    else
    {
        // Specify the number of input and output channels that we want to open
        setAudioChannels (0, 2);
    }
    
    
    
    //playSource = std::make_unique<juce::AudioFormatReaderSource> (transport);
}

MainComponent::~MainComponent()
{
    // This shuts down the audio device and clears the audio source.
    shutdownAudio();
}

//==============================================================================
void MainComponent::prepareToPlay (int samplesPerBlockExpected, double sampleRate)
{
    transport.prepareToPlay(samplesPerBlockExpected, sampleRate);
    
    
}

void MainComponent::openButtonClicked()
{
    DBG("Clicked");

    // Create a chooser that can choose specific types of files
    // Choose a file
    juce::FileChooser chooser ("Choose Audio File", juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.wav; *.aiff; *.mp3");
    
    // if user chooses a file
    if (chooser.browseForFileToOpen())
    {
        juce::File myFile;
        
        //what did user choose
        myFile = chooser.getResult();
        
        // read file
        juce::AudioFormatReader* reader = formatManager.createReaderFor(myFile);
        // If user selects another file
        if (reader != nullptr)
        {
            // get the file ready to play
            // Allocate some memory for the read-in audio file
            // Temporary hold place in memory in order to stream / pass data of file
            std::unique_ptr<juce::AudioFormatReaderSource> tempSource (new juce::AudioFormatReaderSource(reader, true));
            // Set audio source for transport to access and play
            transport.setSource(tempSource.get(), 0, nullptr, reader->sampleRate);
            // Brings next file back to beginning
            transportStateChanged(Stopped);
            
            nameLabel.setText("Name: " + (juce::String) myFile.getFileName(), juce::dontSendNotification);
            //timeLabel.setText("Time: ", juce::dontSendNotification);
            // Pass on data from tempSource to playSource so that tempSource can be re-written smoothly when opening new audio files
            playSource.reset(tempSource.release());
            // Print out the name of the type of file that is being opened
            DBG(reader->getFormatName());
        }
    }
}

void MainComponent::playButtonClicked()
{
    if ((state == Stopped || state == Paused))
    {
        transportStateChanged(Starting);
    } else if (state == Playing) {
        transportStateChanged(Pausing);
    }
}

void MainComponent::stopButtonClicked()
{
    if (state == Paused)
    {
        transportStateChanged(Stopped);
    } else {
        transportStateChanged(Stopping);
    }
    
}

void MainComponent::transportStateChanged(TransportState newState)
{
    if (newState != state)
    {
        state = newState;
        
        switch (state) {
            // Reset transport and enable play button
            case Stopped:
                // Allow play button to be pressed
                playButton.setButtonText ("Play");
                stopButton.setButtonText ("Stop");
                // Don't let stop be pressed
                stopButton.setEnabled (false);
                //playButton.setEnabled(true);
                transport.setPosition(0.0);
                break;
                
            case Playing:
                playButton.setButtonText ("Pause");
                stopButton.setButtonText ("Stop");
                // Allow user to stop music if audio is playing
                stopButton.setEnabled(true);
                break;

            case Starting:
                // Start the transport
                transport.start();
                break;
                
            case Pausing:
                // Stop the audio, but don't return it to zero
                transport.stop();
                break;
            case Paused:
                playButton.setButtonText("Resume");
                stopButton.setButtonText("Return");
                break;
            // Stopping
            // play button to enable
            // transport to stop
            case Stopping:
                transport.stop();
                break;
            // State for adding in new files whilst old files are still playing
            // Disable stop and enable Play so user can play new tune with one click
            
  
        }
    }
}
// Listen out for when transport has been changed
void MainComponent::changeListenerCallback (juce::ChangeBroadcaster *source)
{
    // If the source playing is the same as the transport is currently streaming
    if (source == &transport)
    {
        // And if the transport is currently playing
        if (transport.isPlaying())
        {
            // Change its state
            transportStateChanged(Playing);
        } else if ((state == Stopping || state == Playing)){
            // Else Stop
            transportStateChanged(Stopped);
        } else if (state == Pausing) {
            transportStateChanged(Paused);
        }
    }
}


void MainComponent::getNextAudioBlock (const juce::AudioSourceChannelInfo& bufferToFill)
{
    
    // Check to see if the source is null
    if (playSource.get() == nullptr)
    {
        // If it's null, clear the buffer and return
        bufferToFill.clearActiveBufferRegion();
        return;
    }

    transport.getNextAudioBlock(bufferToFill);

}

void MainComponent::releaseResources()
{

    // For more details, see the help for AudioProcessor::releaseResources()
    // Stops memory exception being thrown
    if (playSource != nullptr) {
        transport.releaseResources();
        
    }
}

//==============================================================================
void MainComponent::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));

}

void MainComponent::resized()
{
    
    openButton.setBounds(10, 10, getWidth() - 20, 30);
    playButton.setBounds(10, 60, getWidth() - 20, 30);
    stopButton.setBounds(10, 110, getWidth() - 20, 30);
    nameLabel.setBounds(10, 160, getWidth() - 20, 20);
    timeLabel.setBounds(10, 180, getWidth() - 20, 20);
}
