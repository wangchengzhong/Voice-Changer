#pragma once
#include"JuceHeader.h"

class PluginTreeBase
{
public:
	virtual ~PluginTreeBase() = default;
	virtual juce::String getUniqueName()const = 0;

	void addSubItem(PluginTreeBase* itm)
	{
		subItems.add(itm);
	}
	int getNumSubItems()
	{
		return subItems.size();

	}
	PluginTreeBase* getSubItem(int idx)
	{
		return subItems[idx];
	}

private:
	juce::OwnedArray<PluginTreeBase> subItems;
};