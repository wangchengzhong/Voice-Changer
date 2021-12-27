# VoiceChanger_wcz
voice changer using JUCE platform, rubberband and soundtouch, as well as independent freq based phase vocoder implementation.
## main functions:
compression, EQ(with arbitrary number and filter type), wah_wah, high_quality pitch shift and peak(formant) shift.

##  当前主流的实时pitch shift库
- rubberband
- elastique pro
- soundtouch
- melodyne


## Rubberband

- frequency domain
- Options:
  * windowLength(long,short,std)
  * smoothing(open/close)
  * pitch(highSpeed, highQuality, highConsistency)
  * channels(apart, together)
  * detector(compound, percussive, soft)
  * realtime mode(singel thread), offline mode(multithread support)
### algorithm
- improved phase vocoder
  * basic analysis-synthesis structure
  * record corresponding ana and syn sample numbers
  * when encounting transient component in specific frame, or the frame is silent, the phase is reset
 
  * the "transient" frame is determined by both "percussive" detector and "high frequency" detector in realtime mode
  * the phase increment is calculated by adjacent frequency bin if there is an increasing phase estimation error, and has the tendency to go into the next freq bin
- speex resampling method(linear interpolation also works)
