#pragma once

#include "PluginProcessor.h"
#include <juce_gui_extra/juce_gui_extra.h>

struct SinglePageBrowser : juce::WebBrowserComponent
{
    using WebBrowserComponent::WebBrowserComponent;

    bool pageAboutToLoad(const juce::String& newURL) override
    {
        return newURL == juce::String("/") ||
               newURL == getResourceProviderRoot();
    }
};

//==============================================================================
class WebViewDemoAudioProcessorEditor : public juce::AudioProcessorEditor
{
public:
    explicit WebViewDemoAudioProcessorEditor(WebViewDemoAudioProcessor&);
    ~WebViewDemoAudioProcessorEditor() override;

    void paint(juce::Graphics&) override;
    void resized() override;

    int getControlParameterIndex(juce::Component&) override
    {
        return controlParameterIndexReceiver.getControlParameterIndex();
    }

    std::optional<juce::WebBrowserComponent::Resource> getResource(const juce::String& url);

private:
    WebViewDemoAudioProcessor& processorRef;

    // Relays
    juce::WebSliderRelay cutoffSliderRelay { "cutoff" };
    juce::WebToggleButtonRelay muteToggleRelay { "mute" };
    juce::WebComboBoxRelay filterTypeComboRelay { "filterType" };
    juce::WebControlParameterIndexReceiver controlParameterIndexReceiver;

    // WebView with direct Options{} chain initialization
    SinglePageBrowser webComponent
    {
        juce::WebBrowserComponent::Options{}
            .withNativeIntegrationEnabled()
            .withKeepPageLoadedWhenBrowserIsHidden()
            .withOptionsFrom(cutoffSliderRelay)
            .withOptionsFrom(muteToggleRelay)
            .withOptionsFrom(filterTypeComboRelay)
            .withOptionsFrom(controlParameterIndexReceiver)
            .withResourceProvider(
                [this](const auto& url) { return getResource(url); },
                juce::WebBrowserComponent::getResourceProviderRoot())
    };

    // Attachments with direct member initialization
    juce::WebSliderParameterAttachment cutoffAttachment
    {
        *processorRef.parameters.getParameter("cutoff"), cutoffSliderRelay, nullptr
    };
    juce::WebToggleButtonParameterAttachment muteAttachment
    {
        *processorRef.parameters.getParameter("mute"), muteToggleRelay, nullptr
    };
    juce::WebComboBoxParameterAttachment filterTypeAttachment
    {
        *processorRef.parameters.getParameter("filterType"), filterTypeComboRelay, nullptr
    };

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(WebViewDemoAudioProcessorEditor)
};
