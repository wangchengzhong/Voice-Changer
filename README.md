# VoiceChanger_wcz
voice changer using JUCE platform, rubberband and soundtouch, as well as independent freq based phase vocoder implementation.
## main functions:
compression, EQ(with arbitrary number and filter type), wah_wah, high_quality pitch shift and peak(formant) shift.
![image](https://user-images.githubusercontent.com/55294472/226099197-562a0790-3d54-4e68-b794-5c85ca94141a.png)
![image](https://user-images.githubusercontent.com/55294472/226099238-30bb5bf9-c363-48fb-b052-b638f44b58ac.png)
![image](https://user-images.githubusercontent.com/55294472/226099263-772b9b6c-7ceb-488d-854b-a26637f84e7f.png)
![image](https://user-images.githubusercontent.com/55294472/226099276-be79ff21-78c9-484c-90d4-872c20c2ed15.png)


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

2022.1.3
## add basic "Voice Conversion System" framework

# 参考工程

https://github.com/ffAudio/Frequalizer

https://github.com/GroovemanAndCo/DAWn

https://github.com/juce-framework/JUCE

https://github.com/Tracktion/tracktion_engine

https://github.com/leomccormack/Spatial_Audio_Framework
