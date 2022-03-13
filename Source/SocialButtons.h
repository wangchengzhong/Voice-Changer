
#pragma once

#include <juce_gui_basics/juce_gui_basics.h>
#include <juce_events/juce_events.h>

class SocialButtons : public juce::Component,
    public juce::Button::Listener
{
public:
    SocialButtons()
    {
        setOpaque(false);

        auto* b = buttons.add(new juce::ImageButton());
        b->addListener(this);


    }

    ~SocialButtons() override = default;

    void paint(juce::Graphics& g) override
    {
    }

    void resized() override
    {
        auto bounds = getLocalBounds();
        for (auto* b : buttons)
            b->setBounds(bounds.removeFromLeft(bounds.getHeight()).reduced(3));
    }

    void buttonClicked(juce::Button* b) override
    {
        juce::URL url(b->getComponentID());
        if (url.isWellFormed()) {
            url.launchInDefaultBrowser();
        }
    }

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SocialButtons)

        juce::OwnedArray<juce::ImageButton> buttons;

};
