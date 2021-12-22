#include "HighFrequencyAudioCurve.h"

namespace RubberBand
{


HighFrequencyAudioCurve::HighFrequencyAudioCurve(Parameters parameters) :
    AudioCurveCalculator(parameters)
{
}

HighFrequencyAudioCurve::~HighFrequencyAudioCurve()
{
}

void
HighFrequencyAudioCurve::reset()
{
}

float
HighFrequencyAudioCurve::processFloat(const float *R__ mag, int)
{
    float result = 0.0;

    const int sz = m_lastPerceivedBin;

    for (int n = 0; n <= sz; ++n) {
        result = result + mag[n] * n;
    }

    return result;
}

double HighFrequencyAudioCurve::processDouble(const double *R__ mag, int)
{
    double result = 0.0;

    const int sz = m_lastPerceivedBin;

    for (int n = 0; n <= sz; ++n) {
        result = result + mag[n] * n;
    }
    
    return result;
}

}

