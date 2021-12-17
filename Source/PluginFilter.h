#pragma once
#include"JuceHeader.h"

#include"PluginProcessor.h"

class Filter :public juce::IIRFilter
{
public:

    void updateCoefficients(const double discreteFrequency,
        const double qFactor,
        const double gain,
        const int filterType) noexcept
    {
        jassert(discreteFrequency > 0);
        jassert(qFactor > 0);

        double bandwidth = juce::jmin(discreteFrequency / qFactor, juce::MathConstants<float>::pi * 0.99);
        double two_cos_wc = -2.0 * cos(discreteFrequency);
        double tan_half_bw = tan(bandwidth / 2.0);
        double tan_half_wc = tan(discreteFrequency / 2.0);
        double tan_half_wc_2 = tan_half_wc * tan_half_wc;
        double sqrt_gain = sqrt(gain);
        // auto n = 1.0 / std::tan(juce::MathConstants<double>::pi * discreteFrequency / sampleRate);
        switch (filterType)
        {
        case 1:
        {
            coefficients = juce::IIRCoefficients(
                tan_half_wc,
                tan_half_wc,
                0.0,
                tan_half_wc + 1.0,
                tan_half_wc - 1.0,
                0.0);
            break;
        }
        case 2:
        {
            coefficients = juce::IIRCoefficients(
                1.0,
                -1.0,
                0.0,
                tan_half_wc + 1.0,
                tan_half_wc - 1.0,
                0.0
            );
            break;
        }
        case 3:
        {
            coefficients = juce::IIRCoefficients(
                gain * tan_half_wc + sqrt_gain,
                gain * tan_half_wc - sqrt_gain,
                0.0,
                tan_half_wc + sqrt_gain,
                tan_half_wc - sqrt_gain,
                0.0
            );
            break;
        }
        case 4:
        {
            coefficients = juce::IIRCoefficients(
                sqrt_gain * tan_half_wc + gain,
                sqrt_gain + tan_half_wc - gain,
                0.0,
                sqrt_gain * tan_half_wc + 1.0,
                sqrt_gain * tan_half_wc - 1.0,
                0.0
            );
            break;
        }
        case 5:
        {
            coefficients = juce::IIRCoefficients(
                tan_half_bw,
                0.0,
                -tan_half_bw,
                1.0 + tan_half_bw,
                two_cos_wc,
                1.0 - tan_half_bw
            );
            break;
        }
        case 6:
        {
            coefficients = juce::IIRCoefficients(
                1.0,
                two_cos_wc,
                1.0,
                1.0 + tan_half_bw,
                two_cos_wc,
                1.0 - tan_half_bw
            );
            break;
        }
        case 7:
        {
            coefficients = juce::IIRCoefficients(
                sqrt_gain + gain * tan_half_bw,
                sqrt_gain * two_cos_wc,
                sqrt_gain - gain * tan_half_bw,
                sqrt_gain + tan_half_bw,
                sqrt_gain * two_cos_wc,
                sqrt_gain - tan_half_bw
            );
            break;
        }
        case 8: {
            coefficients = juce::IIRCoefficients(
                tan_half_wc_2,
                tan_half_wc_2 * 2,
                tan_half_wc_2,
                tan_half_wc_2 + tan_half_wc / gain + 1.0,
                2 * tan_half_wc_2 - 2.0,
                tan_half_wc_2 - tan_half_wc / gain + 1.0
            );
            break;
        }
        default:
            break;
        }
        setCoefficients(coefficients);

    }

    double freq{ 800.0 };
    double qFactor{ 1.0 };
    double gain{ 0.0 };
    int filterType{ 6 };
};

