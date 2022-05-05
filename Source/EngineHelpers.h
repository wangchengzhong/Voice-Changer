

#pragma once
#include <JuceHeader.h>

namespace EngineHelpers
{
    tracktion_engine::Project::Ptr createTempProject(tracktion_engine::Engine& engine);
    void showAudioDeviceSettings(tracktion_engine::Engine& engine); ///< 获取音频设备设置
    void browseForAudioFile(tracktion_engine::Engine& engine, std::function<void(const juce::File&)> fileChosenCallback);
    void removeAllClips(tracktion_engine::AudioTrack& track);
    tracktion_engine::AudioTrack* getOrInsertAudioTrackAt(tracktion_engine::Edit& edit, int index);
    tracktion_engine::WaveAudioClip::Ptr loadAudioFileAsClip(tracktion_engine::Edit& edit, const juce::File& file);
    template<typename ClipType> typename ClipType::Ptr loopAroundClip(ClipType& clip);
    void stop(tracktion_engine::Edit& edit); ///停止子引擎
    void togglePlay(tracktion_engine::Edit& edit); ///< 触发编辑器播放
    void toggleRecord(tracktion_engine::Edit& edit); ///< 如果当前轨道允许录音，触发录音
    void armRecordTrack(tracktion_engine::AudioTrack& t, bool arm, int position = 0); /// 设置轨道是否可用录音
    bool isTrackArmed(tracktion_engine::AudioTrack& t, int position = 0);
    bool isInputMonitoringEnabled(tracktion_engine::AudioTrack& t, int position = 0);
    void enableInputMonitoring(tracktion_engine::AudioTrack& t, bool im, int position = 0);
    bool trackHasInput(tracktion_engine::AudioTrack& t, int position = 0);
    std::unique_ptr<juce::KnownPluginList::PluginTree> createPluginTree(tracktion_engine::Engine& engine);
}
