
#include "ClipComponent.h"

 //==============================================================================
ClipComponent::ClipComponent(EditViewState& evs, tracktion_engine::Clip::Ptr c)
    : editViewState(evs), clip(c)
{
}

void ClipComponent::paint(juce::Graphics& g)
{
    g.fillAll(clip->getColour().withAlpha(0.5f));
    g.setColour(juce::Colours::black);
    g.drawRect(getLocalBounds());

    if (editViewState.selectionManager.isSelected(clip.get()))
    {
        g.setColour(juce::Colours::red);
        g.drawRect(getLocalBounds(), 2);
    }
}

void ClipComponent::mouseDown(const juce::MouseEvent&)
{
    editViewState.selectionManager.selectOnly(clip.get());
}


