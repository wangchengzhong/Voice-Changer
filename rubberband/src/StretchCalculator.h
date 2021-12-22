

#ifndef RUBBERBAND_STRETCH_CALCULATOR_H
#define RUBBERBAND_STRETCH_CALCULATOR_H

#include <sys/types.h>

#include <vector>
#include <map>

namespace RubberBand
{

class StretchCalculator
{
public:
    StretchCalculator(size_t sampleRate, size_t inputIncrement, bool useHardPeaks);
    virtual ~StretchCalculator();

    /**
     * Provide a set of mappings from "before" to "after" sample
     * numbers so as to enforce a particular stretch profile.  This
     * must be called before calculate().  The argument is a map from
     * audio sample frame number in the source material to the
     * corresponding sample frame number in the stretched output.
     */
    void setKeyFrameMap(const std::map<size_t, size_t> &mapping);
    
    /**
     * Calculate phase increments for a region of audio, given the
     * overall target stretch ratio, input duration in audio samples,
     * and the audio curves to use for identifying phase lock points
     * (lockAudioCurve) and for allocating stretches to relatively
     * less prominent points (stretchAudioCurve).
     */
    std::vector<int> calculate(double ratio, size_t inputDuration,
                               const std::vector<float> &lockAudioCurve,
                               const std::vector<float> &stretchAudioCurve);

    /**
     * Calculate the phase increment for a single audio block, given
     * the overall target stretch ratio and the block's value on the
     * phase-lock audio curve.  State is retained between calls in the
     * StretchCalculator object; call reset() to reset it.  This uses
     * a less sophisticated method than the offline calculate().
     *
     * If increment is non-zero, use it for the input increment for
     * this block in preference to m_inbufJumpSampleNum.
     */
    int calculateSingle(double outputTimeStretchRatio,
                        double frac_1_outputPitchRatio,
                        float curveValue,
                        size_t increment,
                        size_t analysisWindowSize,
                        size_t synthesisWindowSize);

    void setUseHardPeaks(bool use) { m_useHardPeaks = use; }

    void reset();
  
    void setDebugLevel(int level) { m_debugLevel = level; }

    struct Peak {
        size_t chunk;
        bool hard;
    };
    std::vector<Peak> getLastCalculatedPeaks() const 
    { 
        return m_peaks; 
    }

    std::vector<float> smoothDF(const std::vector<float> &df);

protected:
    std::vector<Peak> findPeaks(const std::vector<float> &audioCurve);

    void mapPeaks(std::vector<Peak> &peaks, std::vector<size_t> &targets,
                  size_t outputDuration, size_t totalCount);

    std::vector<int> distributeRegion(const std::vector<float> &regionCurve,
                                      size_t outputDuration, float ratio,
                                      bool shouldResetPhase);

    void calculateDisplacements(const std::vector<float> &df,
                                float &maxDf,
                                double &totalDisplacement,
                                double &maxDisplacement,
                                float adj) const;

    size_t m_sampleRate;
    size_t m_inbufJumpSampleNum;
    float m_prevDf;
    double m_prevOverallRatio;
    double m_prevOutputTimeRatio;
    int m_transientAmnesty; // only in RT mode; handled differently offline
    int m_debugLevel;
    bool m_useHardPeaks;
    int64_t m_inSampleCounter;
    std::pair<int64_t, int64_t> m_frameCheckpoint;
    int64_t expectedOutFrame(int64_t inFrame, double outputTimeStretchRatio);
    double m_outSampleCounter;

    std::map<size_t, size_t> m_keyFrameMap;
    std::vector<Peak> m_peaks;
};

}

#endif
