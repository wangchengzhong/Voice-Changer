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

    // 当浏览器即将转到新 URL 时，将调用此方法。
    bool pageAboutToLoad (const String& newURL) override
    {
        // 只需更新地址框，以反映新的位置
        addressTextBox.setText (newURL, false);

        // 在这里返回false，告诉浏览器不要继续加载页面。
        return true;
    }

    // 当请求浏览器启动新窗口时
    void newWindowAttemptingToLoad (const String& newURL) override
    {
        //将URL加载到主窗口中
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

        // 创建一个地址框
        addAndMakeVisible (addressTextBox);
        addressTextBox.onReturnKey = [this] { webView->goToURL (addressTextBox.getText()); };

        // 创建实际的浏览器组件
        webView.reset (new BrowserComponent (addressTextBox));
        addAndMakeVisible (webView.get());

        // 增加一些按钮
        addAndMakeVisible (goButton);
        goButton.onClick = [this] { webView->goToURL (addressTextBox.getText()); };
        addAndMakeVisible (backButton);
        backButton.onClick = [this] { webView->goBack(); };
        addAndMakeVisible (forwardButton);
        forwardButton.onClick = [this] { webView->goForward(); };

        // 将浏览器发送到起始页
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
