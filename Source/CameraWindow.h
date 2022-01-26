#pragma once
#include"JuceHeader.h"
class CameraWindow :
	 public juce::Component
{
public:
	CameraWindow()
	{
		setSize(500, 500);
		setOpaque(true);

		addAndMakeVisible(cameraSelectorComboBox);
		updateCameraList();
		cameraSelectorComboBox.setSelectedId(1);
		cameraSelectorComboBox.onChange = [this] { cameraChanged(); };

		addAndMakeVisible(snapshotButton);
		snapshotButton.onClick = [this] { takeSnapshot(); };
		snapshotButton.setEnabled(false);

		addAndMakeVisible(recordMovieButton);
		recordMovieButton.onClick = [this] { startRecording(); };
		recordMovieButton.setEnabled(false);

		addAndMakeVisible(lastSnapshot);

		cameraSelectorComboBox.setSelectedId(2);
		 
	}
	void paint(juce::Graphics& g)override
	{
		g.fillAll(juce::Colours::black);
	}
	void resized()override
	{
		auto r = getLocalBounds().reduced(5);
		auto top = r.removeFromTop(25);
		cameraSelectorComboBox.setBounds(top.removeFromLeft(250));

		r.removeFromTop(4);
		top = r.removeFromTop(25);

		snapshotButton.changeWidthToFitText(24);
		snapshotButton.setBounds(top.removeFromLeft(snapshotButton.getWidth()));
		top.removeFromLeft(4);

		recordMovieButton.changeWidthToFitText(24);
		recordMovieButton.setBounds(top.removeFromLeft(recordMovieButton.getWidth()));

		r.removeFromTop(4);
		auto previewArea = shouldUseLandscapeLayout() ? r.removeFromLeft(r.getWidth() / 2)
			: r.removeFromTop(r.getHeight() / 2);
		if (cameraPreviewComp.get() != nullptr)
			cameraPreviewComp->setBounds(previewArea);
		if (shouldUseLandscapeLayout())
			r.removeFromLeft(4);
		else
			r.removeFromTop(4);
		lastSnapshot.setBounds(r);
	}
private:
	std::unique_ptr<juce::CameraDevice> cameraDevice;
	std::unique_ptr<juce::Component> cameraPreviewComp;
	juce::ImageComponent lastSnapshot;

	juce::ComboBox cameraSelectorComboBox{ juce::CharPointer_UTF8("\xe7\x9b\xb8\xe6\x9c\xba") };
	juce::TextButton snapshotButton{ juce::CharPointer_UTF8("\xe6\x88\xaa\xe5\x9b\xbe") };
	juce::TextButton recordMovieButton{ juce::CharPointer_UTF8("\xe6\x88\xaa\xe8\xa7\x86\xe9\xa2\x91") };

	bool recordingMovie = false;
	juce::File recordingFile;
	bool contentSharingPending = false;

	void setPortraitOrientationEnabled(bool shouldBeEnabled)
	{
		auto allowedOrientations = juce::Desktop::getInstance().getOrientationsEnabled();
		if (shouldBeEnabled)
			allowedOrientations |= juce::Desktop::upright;
		else
			allowedOrientations &= ~juce::Desktop::upright;
		juce::Desktop::getInstance().setOrientationsEnabled(allowedOrientations);
	}
	bool shouldUseLandscapeLayout()const noexcept
	{
		return false;
	}
	void updateCameraList()
	{
		cameraSelectorComboBox.clear();
		cameraSelectorComboBox.addItem(juce::CharPointer_UTF8("\xe6\xb2\xa1\xe6\x9c\x89\xe7\x9b\xb8\xe6\x9c\xba"), 1);
		cameraSelectorComboBox.addSeparator();

		auto cameras = juce::CameraDevice::getAvailableDevices();
		for (int i = 0; i < cameras.size(); ++i)
		{
			cameraSelectorComboBox.addItem(cameras[i], i + 2);
		}
	}
	void cameraChanged()
	{
		cameraDevice.reset();
		cameraPreviewComp.reset();
		recordingMovie = false;
		if (cameraSelectorComboBox.getSelectedId() > 1)
		{
			cameraDeviceOpenResult(juce::CameraDevice::openDevice(cameraSelectorComboBox.getSelectedId() - 2), {});

		}
		else
		{
			snapshotButton.setEnabled(cameraDevice != nullptr && !contentSharingPending);
			recordMovieButton.setEnabled(cameraDevice != nullptr && !contentSharingPending);
			resized();
		}
	}
	void openCameraAsync()
	{
		SafePointer<CameraWindow> safeThis(this);
		juce::CameraDevice::openDeviceAsync(cameraSelectorComboBox.getSelectedId() - 2,
			[safeThis](juce::CameraDevice* device, const juce::String& error)mutable
		{
			if (safeThis)
				safeThis->cameraDeviceOpenResult(device, error);
		});
	}
	void cameraDeviceOpenResult(juce::CameraDevice* device, const juce::String& error)
	{
		cameraDevice.reset(device);
		if (cameraDevice.get() != nullptr)
		{
			cameraPreviewComp.reset(cameraDevice->createViewerComponent());
			addAndMakeVisible(cameraPreviewComp.get());

		}
		else
		{
			juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon, juce::CharPointer_UTF8("\xe6\xb2\xa1\xe6\x9c\x89\xe6\x89\x93\xe5\xbc\x80\xe7\x9b\xb8\xe6\x9c\xba"),error);
		}

		snapshotButton.setEnabled(cameraDevice.get() != nullptr && !contentSharingPending);
		recordMovieButton.setEnabled(cameraDevice.get() != nullptr && !contentSharingPending);
		resized();
	}
	void startRecording()
	{
		if (cameraDevice.get() != nullptr)
		{
			if (!recordingMovie)
			{
				recordingMovie = true;
				recordingFile = juce::File::getSpecialLocation(juce::File::userDesktopDirectory).getNonexistentChildFile("voice_changer", juce::CameraDevice::getFileExtension());
				cameraSelectorComboBox.setEnabled(false);
				cameraDevice->startRecordingToFile(recordingFile);
				recordMovieButton.setButtonText(juce::CharPointer_UTF8("\xe5\x81\x9c\xe6\xad\xa2\xe5\xbd\x95\xe5\x88\xb6"));
			}
			else
			{
				recordingMovie = false;
				cameraDevice->stopRecording();

				recordMovieButton.setButtonText(juce::CharPointer_UTF8("\xe5\xbd\x95\xe5\x88\xb6(\xe5\x88\xb0\xe6\xa1\x8c\xe9\x9d\xa2)"));
				cameraSelectorComboBox.setEnabled(true);
			}
		}
	}
	void takeSnapshot()
	{
		SafePointer<CameraWindow>safeThis(this);
		cameraDevice->takeStillPicture([safeThis](const juce::Image& image)mutable { safeThis->imageReceived(image); });
	}
	void imageReceived(const juce::Image& image)
	{
		if (!image.isValid())
			return;
		lastSnapshot.setImage(image);

	}
	void errorOccured(const juce::String& error)
	{
		juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
			juce::CharPointer_UTF8("\xe6\x91\x84\xe5\x83\x8f\xe8\xae\xbe\xe5\xa4\x87\xe9\x94\x99\xe8\xaf\xaf"),error);
		cameraDevice.reset();
		cameraSelectorComboBox.setSelectedId(1);
		snapshotButton.setEnabled(false);
		recordMovieButton.setEnabled(false);
	
	}
	void sharingFinished(bool success, bool isCapture)
	{
		juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::InfoIcon,
			isCapture ? "image sharing result" : "video sharing result",
			success ? "success" : "failed");
		contentSharingPending = false;
		snapshotButton.setEnabled(true);
		recordMovieButton.setEnabled(true);
	}
};