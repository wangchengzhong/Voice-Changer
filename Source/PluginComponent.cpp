
#include "PluginComponent.h"
#include "EditViewState.h"

 //==============================================================================
PluginComponent::PluginComponent(EditViewState& evs, tracktion_engine::Plugin::Ptr p)
    : editViewState(evs), plugin(p)
{
    setButtonText(plugin->getName().substring(0, 1));
}

PluginComponent::~PluginComponent()
{
}

void PluginComponent::clicked(const juce::ModifierKeys& modifiers)
{
    editViewState.selectionManager.selectOnly(plugin.get());
    if (modifiers.isPopupMenu())
    {
        juce::PopupMenu m;
        m.addItem("Delete", [this] { plugin->deleteFromParent(); });
        m.showAt(this);
    }
    else
    {
        plugin->showWindowExplicitly();
    }
}
