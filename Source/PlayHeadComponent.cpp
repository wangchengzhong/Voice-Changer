#include"PlayHeadComponent.h"
PlayHeadComponent::PlayHeadComponent(tracktion_engine::Edit& e, EditViewState& evs)
	:edit(e),editViewState(evs)
{
	startTimerHz(30);
}
void PlayHeadComponent::paint(juce::Graphics& g)
{
	g.setColour(juce::Colours::yellow);
	g.drawRect(xPosition, 0, 2, getHeight());
}

bool PlayHeadComponent::hitTest(int x, int y)
{
	if (std::abs(x - xPosition) <= 3)
		return true;
	return false;
}

void PlayHeadComponent::mouseDown(const MouseEvent& event)
{
	edit.getTransport().setUserDragging(true);
}

void PlayHeadComponent::mouseUp(const MouseEvent& event)
{
	edit.getTransport().setUserDragging(false);
}
void PlayHeadComponent::mouseDrag(const MouseEvent& event)
{
	double t = editViewState.xPositionToTime(event.x, getWidth());
	edit.getTransport().setCurrentPosition(t);
}

void PlayHeadComponent::timerCallback()
{
	if(firstTimer)
	{
		firstTimer = false;
		setMouseCursor(juce::MouseCursor::LeftEdgeResizeCursor);
	}
	int newX = editViewState.timeToXPosition(edit.getTransport().getCurrentPosition(), getWidth());
	if(newX != xPosition)
	{
		repaint(jmin(newX, xPosition) - 1, 0, jmax(newX, xPosition) - jmin(newX, xPosition) + 3, getHeight());
		xPosition = newX;
	}
}




