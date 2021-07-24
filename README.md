# Sound Meter
[Juce](https://juce.com/) peak meter module with optional fader overlay.<br>
<img align="right" src="https://www.sounddevelopment.nl/sd/resources/images/sound_meter/sound_meter.gif">
*by Marcel Huibers | [Sound Development](https://www.sounddevelopment.nl) 2021 | Published under the [MIT License](https://en.wikipedia.org/wiki/MIT_License)*


Features:
- Fully resize-able.
- Configurable meter ballistics (meter decay).
- Normal, warning and peak zones, with configurable colours.
- Tick-marks (dividing lines on the meter) at user specified levels.
- Peak indicator and optional peak value readout.
- Optional label strip next to the meters (which can double as master fader).
- Optional header identifying the meter's name (set by user) or channel type.
- Adaptive. Will show header, value, tick-marks only when there is space available.
- Optional fader and mute button (in the header).

You can find the API documentation [here](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/)...
<br>
An example project, demonstrating sound_meter can be found [here](https://github.com/SoundDevelopment/sound_meter-example)...

# Usage

All classes are in the namespace `sd::SoundMeter` to avoid collisions. You can either prefix each symbol, or import the namespace. 

### MetersComponent

The [MetersComponent](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/classsd_1_1_sound_meter_1_1_meters_component.html) class creates and controls the meters. 
This would live in your editor.h. 
```
private:
   sd::SoundMeter::MetersComponent  m_meters;
```
<br>

In the constructor you could specify a channel format with [setChannelFormat()](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/classsd_1_1_sound_meter_1_1_meters_component.html#aea27fda8af5ec463436186e8fb3afd20) :
```
m_meters.setChannelFormat (juce::AudioChannelSet::stereo());
```
and configure it's options. For example (for all meter options, see [documentation](https://www.sounddevelopment.nl/sd/resources/documentation/sound_meter/structsd_1_1_sound_meter_1_1_options.html)):
```
sd::SoundMeter::Options meterOptions;
meterOptions.faderEnabled     = true;
meterOptions.headerEnabled    = true;
meterOptions.peakRegion_db    = -3.0f;  
meterOptions.warningRegion_db = -12.0f;
m_meters.setOptions (meterOptions);
```
Finally (still in the constructor) we add the component and make it visible:
```
addAndMakeVisible (m_meters);
```
<br>

In the `resized()` method, you set the bounds (left, right, width, height) of the meters:
```
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
*Sound Development 2021*
