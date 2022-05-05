
#pragma once
#include <JuceHeader.h>
#include "PluginTreeBase.h"

 //==============================================================================
class PluginTreeItem : public PluginTreeBase
{
public:
    PluginTreeItem(const juce::PluginDescription&);
    PluginTreeItem(const juce::String& uniqueId, const juce::String& name, const juce::String& xmlType, bool isSynth, bool isPlugin);

    tracktion_engine::Plugin::Ptr create(tracktion_engine::Edit&);

    juce::String getUniqueName() const override
    {
        if (desc.fileOrIdentifier.startsWith(tracktion_engine::RackType::getRackPresetPrefix()))
            return desc.fileOrIdentifier;

        return desc.createIdentifierString();
    }

    juce::PluginDescription desc;
    juce::String xmlType;
    bool isPlugin = true;

    JUCE_LEAK_DETECTOR(PluginTreeItem)
};
