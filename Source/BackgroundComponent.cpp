#include"JuceHeader.h"
#include"BackgroundComponent.h"

BackgroundComponent::BackgroundComponent()
{

}
BackgroundComponent::~BackgroundComponent()
{
}

void BackgroundComponent::paint(juce::Graphics& g)
{
	juce::Rectangle<float> bkg{ 545, 350, 380, 220 };
	// juce::Rectangle<float> bkg{ 10,10,10,10 };
	g.fillRect(bkg);
	g.setColour(juce::Colours::pink);
	g.drawRect(bkg);
}
void BackgroundComponent::resized()
{


}