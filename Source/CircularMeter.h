#pragma once
#include"JuceHeader.h"
namespace Gui
{
class CicularMeter :public juce::Component//, public juce::Timer
{
public:
	CicularMeter(std::function<float()>&& valueFunction, const juce::Colour& c)
		: valueSupplier(std::move(valueFunction)), colour(c)
	{
		// startTimerHz(24);
	}
	void paint(juce::Graphics& g)override
	{
		const auto bounds = getLocalBounds().toFloat();
		const auto level = juce::jlimit(0.f, 1.f, juce::jmap(valueSupplier(), -60.f, 6.f, 0.f, 1.f));
		juce::Path referencePath{};
		referencePath.addEllipse(getLocalBounds().toFloat().reduced(bounds.proportionOfHeight(0.099f + 0.4f * (1.f - level))));

		juce::Path ellipse{};
		const auto numberOfPoints = 50;

		const auto ellipseLength = referencePath.getLength();
		const auto startingPoint = generatePointWithRandomness(referencePath.getPointAlongPath(0.f), level);
		ellipse.startNewSubPath(startingPoint);

		for (auto i = 1; i <= numberOfPoints; i++)
		{
			if (i == numberOfPoints)
			{
				ellipse.lineTo(startingPoint);
			}
			else
			{
				ellipse.lineTo(generatePointWithRandomness(referencePath.getPointAlongPath(ellipseLength * i / numberOfPoints), level));

			}
		}
		ellipse.closeSubPath();
		ellipse = ellipse.createPathWithRoundedCorners(2.f);
		const auto affine = juce::AffineTransform::translation(-4.f * level, -4.f * level);
		ellipse.applyTransform(affine);

		const auto gradient = juce::ColourGradient(colour.withAlpha(0.6f), { getWidth() / 2.f , getHeight() / 2.f }, colour.withAlpha(0.f), {}, true);
		g.setGradientFill(gradient);
		g.strokePath(ellipse, juce::PathStrokeType{ 8.f * level,juce::PathStrokeType::JointStyle::curved,juce::PathStrokeType::rounded });
	}
	//void timerCallback() override
	//{
	//	// repaint();
	//}
private:
	juce::Point<float> generatePointWithRandomness(juce::Point<float> p, const float level)
	{
		p.addXY(random.nextFloat() * level * 8.f, random.nextFloat() * level * 8.f);
		return p;
	}
	std::function<float()> valueSupplier;
	const juce::Colour colour;
	juce::Random random{0};// { juce::Time::currentTimeMillis(); }
};
}