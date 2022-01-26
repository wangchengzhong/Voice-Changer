# include"DawComponent.h"

using namespace juce;
DawComponent::DawComponent(juce::Component& par)
	:factory(this,this),parent(par)
{
	settingsButton.onClick = [this] {EngineHelpers::showAudioDeviceSettings(engine); };
	pluginsButton.onClick = [this]
	{
		DialogWindow::LaunchOptions o;
		o.dialogTitle = TRANS("Plugins");
		o.dialogBackgroundColour = Colours::black;
		o.escapeKeyTriggersCloseButton = true;
		o.useNativeTitleBar = true;
		o.resizable = true;
		o.useBottomRightCornerResizer = true;


		auto v = new PluginListComponent(engine.getPluginManager().pluginFormatManager,
			engine.getPluginManager().knownPluginList,
			engine.getTemporaryFileManager().getTempFile("PluginScanDeadMansPedal"),
			tracktion_engine::getApplicationSettings());
		v->setSize(800, 600);
		o.content.setOwned(v);
		o.launchAsync();
	};
	newEditButton.onClick = [this] {createOrLoadEdit(); };

	updatePlayButton();
	updateRecordButtonText();

	editNameLabel.setJustificationType(Justification::centred);

	Helpers::addAndMakeVisible(*this, {
		&settingsButton, &pluginsButton,
		&showEditButton, &newTrackButton,
		&deleteButton, &clearTracksButton, &aboutButton

		});

	deleteButton.setEnabled(false);

	auto d = File::getSpecialLocation(File::tempDirectory).getChildFile("DawComponent");
	d.createDirectory();

	auto f = Helpers::findRecentEdit(d);
	if (f.existsAsFile())
		createOrLoadEdit(f);
	else
	{
		createOrLoadEdit(d.getNonexistentChildFile("Test", ".tracktionedit", false));
	}
	selectionManager.addChangeListener(this);

	setupGUI();

	setSize(600, 400);
}

DawComponent::~DawComponent()
{

	tracktion_engine::EditFileOperations(*edit).save(true, true, false);
	engine.getTemporaryFileManager().getTempDirectory().deleteRecursively();

}

void DawComponent::paint(Graphics& g)
{
	auto col = juce::Colour(0xff101525);
	g.fillAll(col);
}

void DawComponent::resized()
{
	const auto toolbarThickness = 32;
	auto r = getLocalBounds();
	r.removeFromTop(toolbarThickness);

	auto topR = r.removeFromTop(30);
	auto w = r.getWidth() / 7;

	toolbar.setBounds(toolbar.isVertical() ? getLocalBounds().removeFromLeft(toolbarThickness) : getLocalBounds().removeFromTop(toolbarThickness));

	settingsButton.setBounds(topR.removeFromLeft(w).reduced(2));
	pluginsButton.setBounds(topR.removeFromLeft(w).reduced(2));

	showEditButton.setBounds(topR.removeFromLeft(w).reduced(2));
	newTrackButton.setBounds(topR.removeFromLeft(w).reduced(2));

	deleteButton.setBounds(topR.removeFromLeft(w).reduced(2));
	clearTracksButton.setBounds(topR.removeFromLeft(w).reduced(2));
	aboutButton.setBounds(topR.removeFromLeft(2).reduced(2));
	topR = r.removeFromTop(30);
	editNameLabel.setBounds(topR);

	if (editComponent != nullptr)editComponent->setBounds(r);
}


void DawComponent::setupGUI()
{
	addAndMakeVisible(toolbar);
	toolbar.addDefaultItems(factory);
	playPauseButton.onClick = [this]
	{
		EngineHelpers::togglePlay(*edit);
	};
	recordButton.onClick = [this] {onRecordTracks(); };
	newTrackButton.onClick = [this]
	{
		edit->ensureNumberOfAudioTracks(getAudioTracks(*edit).size() + 1);
	};

	deleteButton.onClick = [this]
	{
		auto* sel = selectionManager.getSelectedObject(0);
		if (auto* clip = dynamic_cast<tracktion_engine::Clip*>(sel))
		{
			clip->removeFromParentTrack();
		}

		else if (auto* track = dynamic_cast<tracktion_engine::Track*>(sel))
		{
			if (!(track->isMarkerTrack() || track->isTempoTrack() || track->isChordTrack()))
				edit->deleteTrack(track);
		}
		else if (auto* plugin = dynamic_cast<tracktion_engine::Plugin*>(sel))
			plugin->deleteFromParent();
	};

	showWaveformButton.onClick = [this]
	{
		auto& evs = editComponent->getEditViewState();
		evs.drawWaveforms = !evs.drawWaveforms.get();
		showWaveformButton.setToggleState(evs.drawWaveforms, dontSendNotification);
	};

	clearTracksButton.onClick = [this]
	{
		const auto userIsSure = engine.getUIBehaviour().showOkCancelAlertBox(TRANS("MIDI Clip"),
			TRANS("sure?"),
			TRANS("Clear all"),
			TRANS("Ignore")) == 1;
		if (!userIsSure) return;
		for (auto* t : tracktion_engine::getAudioTracks(*edit))
			edit->deleteTrack(t);
	};

	aboutButton.onClick = [this]
	{
		AlertWindow::showMessageBoxAsync(AlertWindow::InfoIcon,
			TRANS(String("voice changer wcz") + ProjectInfo::versionString + "..."),
			TRANS("voice changer, support realtime processing\n"
			"as well as basic DAW functionalities"));
	};
}

void DawComponent::buttonClicked(Button* button)
{
	const auto name = button->getName().toLowerCase();
	if (name == "new") createOrLoadEdit();
	else if (name == "open") createOrLoadEdit(File{}, true);
	else if (name == "save")
	{
		tracktion_engine::EditFileOperations(*edit).save(true, true, false);
		TRACKTION_LOG("edit file " + edit->getName() + " saved. ");

	}
	else if (name == "save as")
	{
		juce::FileChooser fc("Save As...",
			juce::File::getSpecialLocation(juce::File::userDocumentsDirectory),
			"*.tracktionedit");
		if (fc.browseForFileToSave(true))
		{
			auto editFile = fc.getResult();
			tracktion_engine::EditFileOperations(*edit).saveAs(editFile);
			TRACKTION_LOG("Edit file saved as " + edit->getName() + " saved.");

		}
	}
	else if (name == "start")
	{
		EngineHelpers::togglePlay(*edit);
	}
	else if (name == "stop") EngineHelpers::stop(*edit);
	else if (name == "record") onRecordTracks();
	else
	{
		TRACKTION_LOG(juce::String("Unknown Button " + button->getName() + "pressed"));
	}

}

void DawComponent::sliderValueChanged(Slider* slider)
{
	if(slider->getName()==TransportToolbarItemFactory::TempoSliderName)
	{
		if (edit != nullptr)edit->tempoSequence.getTempos()[0]->setBpm(slider->getValue());
	}
}

void DawComponent::onRecordTracks()
{
	const auto wasRecording = edit->getTransport().isRecording();
	EngineHelpers::toggleRecord(*edit);
	if (wasRecording) tracktion_engine::EditFileOperations(*edit).save(true, true, false);
}
void DawComponent::setSongTitle(const juce::String& title)
{
	parent.setName("current project - " + title);
}
void DawComponent::createOrLoadEdit(juce::File editFile, bool loadOnly)
{
	if(editFile==juce::File())
	{
		auto title = juce::String(loadOnly?"Load":"New") + "Project";
		juce::FileChooser fc(title, juce::File::getSpecialLocation(juce::File::userDesktopDirectory), "*.tracktionedit");
		auto result = loadOnly ? fc.browseForFileToOpen() : fc.browseForFileToSave(false);
		if (result)
			editFile = fc.getResult();
		else
		{
			return;
		}
	}
	selectionManager.deselectAll();
	editComponent = nullptr;

	if (editFile.existsAsFile())
		edit = std::make_unique<tracktion_engine::Edit>(engine, ValueTree::fromXml(editFile.loadFileAsString()), tracktion_engine::Edit::forEditing, nullptr, 0);
	else
	{
		edit = std::make_unique<tracktion_engine::Edit>(engine, tracktion_engine::createEmptyEdit(engine), tracktion_engine::Edit::forEditing, nullptr, 0);
	}

	auto& transport = edit->getTransport();
	transport.addChangeListener(this);
	setSongTitle(editFile.getFileNameWithoutExtension());
	showEditButton.onClick = [this, editFile]
	{
		tracktion_engine::EditFileOperations(*edit).save(true, true, false);
		editFile.revealToUser();
	};
	createTracksAndAssignInputs();
	tracktion_engine::EditFileOperations(*edit).save(true, true, false);

	editComponent = std::make_unique<EditComponent>(*edit, selectionManager);
	editComponent->getEditViewState().showFooters = true;
	editComponent->getEditViewState().showMidiDevices = true;
	editComponent->getEditViewState().showWaveDevices = true;


	addAndMakeVisible(*editComponent);
	resized();

}

