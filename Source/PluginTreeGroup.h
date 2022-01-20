#pragma once
#include <JuceHeader.h>
#include"PluginTreeBase.h"
class PluginTreeGroup:public PluginTreeBase
{
public:
	PluginTreeGroup(tracktion_engine::Edit&, juce::KnownPluginList::PluginTree&, tracktion_engine::Plugin::Type);
	PluginTreeGroup(const juce::String&);

	juce::String getUniqueName() const override
	{
		return name;
	}
	juce::String name;
private:
	void populateFrom(juce::KnownPluginList::PluginTree&);
	void createBuiltInItems(int& num, tracktion_engine::Plugin::Type);

	JUCE_LEAK_DETECTOR(PluginTreeGroup);
};