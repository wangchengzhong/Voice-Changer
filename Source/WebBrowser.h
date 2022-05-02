/*
  ==============================================================================

    WebBrowser.h
    Created: 2 May 2022 4:22:13pm
    Author:  wcz11

  ==============================================================================
*/

#include <JuceHeader.h>
#pragma once
inline Colour getRandomColour(float brightness) noexcept
{
    return Colour::fromHSV(Random::getSystemRandom().nextFloat(), 0.5f, brightness, 1.0f);
}

inline Colour getRandomBrightColour() noexcept { return getRandomColour(0.8f); }
inline Colour getRandomDarkColour() noexcept { return getRandomColour(0.3f); }

inline Colour getUIColourIfAvailable(LookAndFeel_V4::ColourScheme::UIColour uiColour, Colour fallback = Colour(0xff4d4d4d)) noexcept
{
    if (auto* v4 = dynamic_cast<LookAndFeel_V4*> (&LookAndFeel::getDefaultLookAndFeel()))
        return v4->getCurrentColourScheme().getUIColour(uiColour);

    return fallback;
}
class BrowserComponent  : public WebBrowserComponent
{
public:
    //==============================================================================
    BrowserComponent (TextEditor& addressBox)
        : addressTextBox (addressBox)
    {}

    // This method gets called when the browser is about to go to a new URL..
    bool pageAboutToLoad (const String& newURL) override
    {
        // We'll just update our address box to reflect the new location..
        addressTextBox.setText (newURL, false);

        // we could return false here to tell the browser not to go ahead with
        // loading the page.
        return true;
    }

    // This method gets called when the browser is requested to launch a new window
    void newWindowAttemptingToLoad (const String& newURL) override
    {
        // We'll just load the URL into the main window
        goToURL (newURL);
    }

private:
    TextEditor& addressTextBox;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (BrowserComponent)
};


//==============================================================================
class WebBrowser    : public Component
{
public:
    WebBrowser()
    {
        setOpaque (true);

        // Create an address box..
        addAndMakeVisible (addressTextBox);
        // addressTextBox.setTextToShowWhenEmpty ("Enter a web address, e.g. https://www.juce.com", Colours::grey);
        addressTextBox.onReturnKey = [this] { webView->goToURL (addressTextBox.getText()); };

        // create the actual browser component
        webView.reset (new BrowserComponent (addressTextBox));
        addAndMakeVisible (webView.get());

        // add some buttons..
        addAndMakeVisible (goButton);
        goButton.onClick = [this] { webView->goToURL (addressTextBox.getText()); };
        addAndMakeVisible (backButton);
        backButton.onClick = [this] { webView->goBack(); };
        addAndMakeVisible (forwardButton);
        forwardButton.onClick = [this] { webView->goForward(); };

        // send the browser to a start page..
        webView->goToURL ("https://music.163.com/");

        setSize (500, 500);
    }

    void paint (Graphics& g) override
    {
        g.fillAll (getUIColourIfAvailable (LookAndFeel_V4::ColourScheme::UIColour::windowBackground,
                                           Colours::grey));
    }

    void resized() override
    {
        webView->setBounds       (10, 45, getWidth() - 20, getHeight() - 55);
        goButton      .setBounds (getWidth() - 45, 10, 35, 25);
        addressTextBox.setBounds (100, 10, getWidth() - 155, 25);
        backButton    .setBounds (10, 10, 35, 25);
        forwardButton .setBounds (55, 10, 35, 25);
    }

private:
    std::unique_ptr<BrowserComponent> webView;

    TextEditor addressTextBox;

    TextButton goButton      { "Go", "Go to URL" },
               backButton    { "<<", "Back" },
               forwardButton { ">>", "Forward" };

    void lookAndFeelChanged() override
    {
        addressTextBox.applyFontToAllText (addressTextBox.getFont());
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (WebBrowser)
};
