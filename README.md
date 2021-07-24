# Sound Meter
[Juce](https://juce.com/) peak meter module with optional fader overlay.<br>
<img align="right" src="https://www.sounddevelopment.nl/sd/resources/images/sound_meter/sound_meter.gif">
*by Marcel Huibers | Sound Development 2021*


Features:
- Fully resize-able.
- Configurable meter ballistics (meter decay).
- Normal, warning and peak zones, with configurable colours.
- Tick-marks (dividing lines on the meter) at user specified levels.
- Peak indicator and optional peak value readout.
- Optional header identifying the meter's name (set by user) or channel type.
- Adaptive. Will show header, value, tick-marks only when there is space available.
- Optional fader and mute button (in the header).
- Optional master-fader controlling relative fader value of all meters.

# Usage

All classes are in the namespace `sd::SoundMeter` to avoid collisions. You can either prefix each symbol, or import the namespace. 

### `MetersComponent`

The MetersComponent class creates and controls the meters. 
This would live in your editor.h. 
```
private:
   sd::SoundMeter::MetersComponent  m_meters;
```
<br>

In the constructor you could specify a channel format with `setChannelFormat()`:
```
m_meters.setChannelFormat (juce::AudioChannelSet::stereo());
```
and configure it's options. For example (for all meter options, see documentation):
```
sd::SoundMeter::Options meterOptions;
meterOptions.faderEnabled     = true;
meterOptions.headerEnabled    = true;
meterOptions.peakRegion_db    = -3.0f;  
meterOptions.warningRegion_db = -12.0f;
m_meters.setOptions (meterOptions);
```
