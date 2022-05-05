
#include "PlayheadComponent.h"
 //==============================================================================
PlayheadComponent::PlayheadComponent(tracktion_engine::Edit& e, EditViewState& evs)
    : edit(e), editViewState(evs)
{
    startTimerHz(30);
}

void PlayheadComponent::paint(juce::Graphics& g)
{
    g.setColour(juce::Colours::yellow);
    g.drawRect(xPosition, 0, 2, getHeight());
}

bool PlayheadComponent::hitTest(int x, int)
{
    if (std::abs(x - xPosition) <= 3)
        return true;

    return false;
}

void PlayheadComponent::mouseDown(const juce::MouseEvent&)
{
    edit.getTransport().setUserDragging(true);
}

void PlayheadComponent::mouseUp(const juce::MouseEvent&)
{
    edit.getTransport().setUserDragging(false);
}

void PlayheadComponent::mouseDrag(const juce::MouseEvent& e)
{
    double t = editViewState.xPositionToTime(e.x, getWidth());
    edit.getTransport().setCurrentPosition(t);
    timerCallback();
}

void PlayheadComponent::timerCallback()
{
    if (firstTimer)
    {
        // On Linux, don't set the mouse cursor until after the Component has appeared
        firstTimer = false;
        setMouseCursor(juce::MouseCursor::LeftRightResizeCursor);
    }

    int newX = editViewState.timeToXPosition(edit.getTransport().getCurrentPosition(), getWidth());
    if (newX != xPosition)
    {
        repaint(juce::jmin(newX, xPosition) - 1, 0, juce::jmax(newX, xPosition) - juce::jmin(newX, xPosition) + 3, getHeight());
        xPosition = newX;
    }
}

