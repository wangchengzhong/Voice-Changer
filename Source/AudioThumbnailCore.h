#pragma once

#include "../JuceLibraryCode/JuceHeader.h"

class AudioThumbnailComp :
    public Component,
    public ChangeListener,
    public FileDragAndDropTarget,
    public ChangeBroadcaster,
    private ScrollBar::Listener,
    private Timer
{
public:
    AudioThumbnailComp(
        AudioFormatManager& formatManager,
        AudioTransportSource& transport,
        AudioThumbnailCache& thumbCache,
        const File& existingFile = File());

    ~AudioThumbnailComp();

    void setFile(const File& file);

    File getLastDroppedFile() const noexcept;

    void setZoomFactor(double amount);

    void setRange(Range<double> newRange);

    void setFollowsTransport(bool shouldFollow);

    void paint(Graphics& g) override;

    void resized() override;

    void changeListenerCallback(ChangeBroadcaster*) override;

    bool isInterestedInFileDrag(const StringArray& files) override { return true; }

    void filesDropped(const StringArray& files, int x, int y) override;

    void mouseDown(const MouseEvent& e) override;

    void mouseDrag(const MouseEvent& e) override;

    void mouseUp(const MouseEvent&) override;

    void mouseWheelMove(const MouseEvent&, const MouseWheelDetails& wheel) override;
    File lastFileDropped;
private:
    AudioTransportSource& transportSource;
    ScrollBar scrollbar;

    AudioThumbnail thumbnail;
    Range<double> visibleRange;
    bool isFollowingTransport;
    

    DrawableRectangle currentPositionMarker;

    float timeToX(const double time) const;

    double xToTime(const float x) const;

    bool canMoveTransport() const noexcept;

    void scrollBarMoved(ScrollBar* scrollBarThatHasMoved, double newRangeStart) override;

    void timerCallback() override;

    void updateCursorPosition();
};
