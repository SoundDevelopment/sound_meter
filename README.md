# Sound Meter
<sup>*by Marcel Huibers | [Sound Development](https://www.sounddevelopment.nl) 2025 | Published under the [MIT License](https://en.wikipedia.org/wiki/MIT_License)*</sup>
<img align="right" src="https://www.sounddevelopment.nl/sd/resources/images/sound_meter/sound_meter_demo.gif">
<br><br>**Peak meter JUCE module with optional fader overlay.<br><br>**
Used in the Lobith Audio's waveform audio player [PlayerSpecz](https://www.lobith-audio.com/playerspecz/).<br>
Compatible with JUCE 8. For older JUCE version go check out the [JUCE7 branch](https://github.com/SoundDevelopment/sound_meter/tree/juce7).

### Features:
- Fully **resize-able**.
- **Adaptive**. Will show header, value, tick-marks only when there is space available.
- Unlimited number of user defineable **segments**, with custom ranges and configurable colours or gradients.
- **Efficient**. Only redraws when needed.
- Configurable meter **ballistics** (meter decay).
- **Tick-marks** (dividing lines on the meter) at user specified levels.
- Peak hold **indicator** and optional peak **value** readout.
- Optional **label strip** next to the meters (which can double as master fader).
- Optional **header** identifying the meter's name (set by user) or channel type.
- Optional **fader** and mute button (in the header).

<br>

### **New: Meter Presets!**
- Default (-60 dB - 0 dB)
- SMPTE PPM  (-44 dB - 0 dB)
- EBU PPM (-30 dB - -10 dB)
- Yamaha 60 Mizer (-60 dB - 0 dB)
- Bob Katz K metering (K20, K14, K12)
- Extended Bottom (-96 dB - 0 dB)
- Extended Top (-50 dB - +20 dB)
- Full Range (-96 dB - +100 dB)

You can find the API documentation [here](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/)...
<br>
An example project, demonstrating sound_meter can be found [here](https://github.com/SoundDevelopment/sound_meter-example)...

# Usage

All classes are in the namespace `sd::SoundMeter` to avoid collisions. You can either prefix each symbol, or import the namespace. 

### MetersComponent

The [MetersComponent](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/classsd_1_1SoundMeter_1_1MetersComponent.html) class creates and controls the meters. 
This would live in your **editor.h**. 
```cpp
private:
   sd::SoundMeter::MetersComponent  m_meters;
```
<br>

In the **constructor** you can specify a channel format with [setChannelFormat](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/classsd_1_1SoundMeter_1_1MetersComponent.html#aea27fda8af5ec463436186e8fb3afd20) or set the nummer of channels with [setNumChannels](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/classsd_1_1SoundMeter_1_1MetersComponent.html#a042d84e77a91f501c57377d461957e41):
```cpp
m_meters.setChannelFormat (juce::AudioChannelSet::stereo());
```
and configure it's options: (for all meter options, see [documentation](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/structsd_1_1SoundMeter_1_1Options.html))
```cpp
sd::SoundMeter::Options meterOptions;
meterOptions.faderEnabled     = true;
meterOptions.headerEnabled    = true;
meterOptions.showTickMarks    = true;
m_meters.setOptions (meterOptions);
```
and configure the segments:
```cpp
std::vector<sd::SoundMeter::SegmentOptions> segmentOptions = 
{ // From bottom of the meter (0.0f) to the half. Displaying -60 dB up to -18 dB.
  { { -60.0f, -18.0f }, { 0.0f, 0.5f }, juce::Colours::green, juce::Colours::green }, 
  // From half of the meter to almost the top (0.9f). Displaying -18 dB up to -3 dB.
  { { -18.0f, -3.0f }, { 0.5f, 0.90f }, juce::Colours::green, juce::Colours::yellow },
  // From almost the top to the top of the meter (1.0f). Displaying -3 dB up to 0 dB.
  { { -3.0f, 0.0f }, { 0.90f, 1.0f }, juce::Colours::yellow, juce::Colours::red } };    
m_meters.setMeterSegments (segmentOptions);
```

Finally (still in the **constructor**) we add the component and make it visible:
```cpp
addAndMakeVisible (m_meters);
```

In the `resized()` method, you set the bounds (left, right, width, height) of the meters:
```cpp
m_meters.setBounds (getLocalBounds());
```

### Getting the levels

Basically everything is set up now.<br>
All left to do now is to supply the meter with the level with the method:
`setInputLevel (int channel, float value);`

The recommended way to get the levels from the audio processor is to let the editor poll the audio processor (with a timer for instance).
Preferably it would poll atomic values in the audio processor for thread safety.

A fully working example demonstrating this can be found [here](https://github.com/SoundDevelopment/sound_meter-example)...

<br><br>

-----
*Sound Development 2025*
