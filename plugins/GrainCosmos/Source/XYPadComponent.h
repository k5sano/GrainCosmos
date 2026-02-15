#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>

struct TrailDot
{
    float x;
    float y;
    float alpha;
    int64_t birthTime;
};

class XYPadComponent : public juce::Component, public juce::Timer
{
public:
    XYPadComponent(juce::AudioProcessorValueTreeState& params)
        : parameters(params)
    {
        setMouseCursor(juce::MouseCursor::CrosshairCursor);
        startTimerHz(30);
    }

    ~XYPadComponent() override
    {
        stopTimer();
    }

    void setXParam(const juce::String& id) { xParamID = id; repaint(); }
    void setYParam(const juce::String& id) { yParamID = id; repaint(); }
    void setBackgroundImage(const juce::Image& img) { backgroundImage = img; repaint(); }
    void loadBackgroundImage()
    {
        chooser = std::make_unique<juce::FileChooser>("Select Image",
            juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                .getChildFile("plugin-freedom-system")
                .getChildFile("plugins")
                .getChildFile("GrainCosmos")
                .getChildFile("pics"),
            "*.png;*.jpg;*.jpeg;*.gif");

        chooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles,
            [this](const juce::FileChooser& fc)
            {
                auto file = fc.getResult();
                if (file.existsAsFile())
                {
                    juce::Image img = juce::PNGImageFormat().loadFrom(file);
                    if (img.isNull())
                        img = juce::JPEGImageFormat().loadFrom(file);
                    if (!img.isNull())
                    {
                        backgroundImage = img;
                        repaint();
                    }
                }
            });
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();

        // Background image or black
        if (backgroundImage.isValid())
        {
            g.drawImageWithin(backgroundImage,
                bounds.getX(), bounds.getY(),
                bounds.getWidth(), bounds.getHeight(),
                juce::RectanglePlacement::stretchToFit);
        }
        else
        {
            g.fillAll(juce::Colours::black);
        }

        // Semi-transparent overlay for better visibility
        if (backgroundImage.isValid())
        {
            g.fillAll(juce::Colour(0, 0, 0).withAlpha(0.4f));
        }

        // Border
        g.setColour(juce::Colours::grey);
        g.drawRect(bounds, 2.0f);

        // Grid lines
        g.setColour(juce::Colours::darkgrey.withAlpha(0.5f));

        // Vertical center line
        g.drawLine(juce::Line<float>(bounds.getCentreX(), bounds.getY(),
                                     bounds.getCentreX(), bounds.getBottom()), 1.0f);

        // Horizontal center line
        g.drawLine(juce::Line<float>(bounds.getX(), bounds.getCentreY(),
                                     bounds.getRight(), bounds.getCentreY()), 1.0f);

        // Draw trail dots
        for (const auto& dot : trailDots)
        {
            // Flicker effect with sine wave
            float flicker = 0.5f + 0.5f * std::sin(dot.alpha * 10.0f);
            float finalAlpha = dot.alpha * flicker;

            float radius = 4.0f + (1.0f - dot.alpha) * 3.0f;

            // Purple glow
            g.setColour(juce::Colour(180, 120, 255).withAlpha(finalAlpha * 0.3f));
            float glowRadius = radius * 2.5f;
            g.fillEllipse(dot.x - glowRadius, dot.y - glowRadius, glowRadius * 2.0f, glowRadius * 2.0f);

            // Main dot
            g.setColour(juce::Colour(180, 120, 255).withAlpha(finalAlpha));
            g.fillEllipse(dot.x - radius, dot.y - radius, radius * 2.0f, radius * 2.0f);
        }

        // Current position dot
        auto pos = getCurrentPosition();
        float dotSize = 12.0f;

        // Glow effect
        g.setColour(juce::Colours::purple.withAlpha(0.3f));
        g.fillEllipse(pos.x - dotSize * 1.5f, pos.y - dotSize * 1.5f,
                      dotSize * 3.0f, dotSize * 3.0f);

        // Dot
        g.setColour(juce::Colours::purple.brighter());
        g.fillEllipse(pos.x - dotSize / 2, pos.y - dotSize / 2, dotSize, dotSize);

        // White center
        g.setColour(juce::Colours::white);
        g.fillEllipse(pos.x - dotSize / 4, pos.y - dotSize / 4, dotSize / 2, dotSize / 2);

        // Labels
        g.setColour(juce::Colours::lightgrey);
        g.setFont(12.0f);

        auto xParam = parameters.getParameter(xParamID);
        auto yParam = parameters.getParameter(yParamID);

        if (xParam)
        {
            float xValue = getParamValue(xParam);
            g.drawText(xParam->name + ": " + juce::String(xValue, 2),
                       bounds.getX() + 5, bounds.getY() + 5,
                       150, 20, juce::Justification::left);
        }

        if (yParam)
        {
            float yValue = getParamValue(yParam);
            g.drawText(yParam->name + ": " + juce::String(yValue, 2),
                       bounds.getX() + 5, bounds.getY() + 25,
                       150, 20, juce::Justification::left);
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        // Add trail at current position BEFORE moving
        addTrailDot(lastCursorPos, true);
        // Then move to new position
        updateFromMouse(e.getPosition());
    }

    void mouseDrag(const juce::MouseEvent& e) override
    {
        // Add trail at current position before moving
        addTrailDot(lastCursorPos, false);
        updateFromMouse(e.getPosition());
    }

    void timerCallback() override
    {
        auto now = juce::Time::currentTimeMillis();

        // Remove expired dots
        trailDots.erase(
            std::remove_if(trailDots.begin(), trailDots.end(),
                [&](const TrailDot& d) { return (now - d.birthTime) > trailLifetimeMs; }),
            trailDots.end());

        // Update alpha values
        for (auto& dot : trailDots)
        {
            float age = static_cast<float>(now - dot.birthTime) / static_cast<float>(trailLifetimeMs);
            dot.alpha = 1.0f - age;
        }

        repaint();
    }

private:
    juce::AudioProcessorValueTreeState& parameters;
    juce::String xParamID = "chaos";
    juce::String yParamID = "character";

    juce::Image backgroundImage;
    std::unique_ptr<juce::FileChooser> chooser;

    std::vector<TrailDot> trailDots;
    static constexpr int trailLifetimeMs = 2000;
    static constexpr int maxTrailDots = 50;
    int64_t lastTrailAddTime = 0;
    juce::Point<int> lastCursorPos { 0, 0 };

    void addTrailDot(juce::Point<int> position, bool force = false)
    {
        // Throttle: add dot every 50ms max (unless forced)
        auto now = juce::Time::currentTimeMillis();
        if (!force && (now - lastTrailAddTime < 50))
            return;
        lastTrailAddTime = now;

        TrailDot dot;
        dot.x = static_cast<float>(position.x);
        dot.y = static_cast<float>(position.y);
        dot.alpha = 1.0f;
        dot.birthTime = now;
        trailDots.push_back(dot);

        if (trailDots.size() > maxTrailDots)
            trailDots.erase(trailDots.begin());

        repaint();  // Force repaint to show the dot immediately
    }

    juce::Point<float> getCurrentPosition() const
    {
        auto bounds = getLocalBounds().toFloat();
        auto xParam = parameters.getParameter(xParamID);
        auto yParam = parameters.getParameter(yParamID);

        float xNorm = 0.5f;
        float yNorm = 0.5f;

        if (xParam)
        {
            auto range = xParam->getNormalisableRange();
            xNorm = range.convertTo0to1(getParamValue(xParam));
        }

        if (yParam)
        {
            auto range = yParam->getNormalisableRange();
            yNorm = range.convertTo0to1(getParamValue(yParam));
        }

        return { bounds.getX() + xNorm * bounds.getWidth(),
                 bounds.getY() + (1.0f - yNorm) * bounds.getHeight() };
    }

    float getParamValue(juce::AudioProcessorParameter* param) const
    {
        if (auto* floatParam = dynamic_cast<juce::AudioParameterFloat*>(param))
            return floatParam->get();
        return 0.0f;
    }

    void updateFromMouse(juce::Point<int> mousePos)
    {
        auto bounds = getLocalBounds();

        mousePos.x = juce::jlimit(bounds.getX(), bounds.getRight(), mousePos.x);
        mousePos.y = juce::jlimit(bounds.getY(), bounds.getBottom(), mousePos.y);

        // Update last cursor position BEFORE updating parameters
        lastCursorPos = mousePos;

        float xNorm = static_cast<float>(mousePos.x - bounds.getX()) / bounds.getWidth();
        float yNorm = 1.0f - static_cast<float>(mousePos.y - bounds.getY()) / bounds.getHeight();

        if (auto* xParam = parameters.getParameter(xParamID))
        {
            auto range = xParam->getNormalisableRange();
            float value = range.convertFrom0to1(xNorm);
            parameters.getParameter(xParamID)->setValueNotifyingHost(range.convertTo0to1(value));
        }

        if (auto* yParam = parameters.getParameter(yParamID))
        {
            auto range = yParam->getNormalisableRange();
            float value = range.convertFrom0to1(yNorm);
            parameters.getParameter(yParamID)->setValueNotifyingHost(range.convertTo0to1(value));
        }

        repaint();
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPadComponent)
};


class XYPadPanel : public juce::Component
{
public:
    XYPadPanel(juce::AudioProcessorValueTreeState& params)
        : parameters(params)
        , xyPad(params)
    {
        addAndMakeVisible(xyPad);
        addAndMakeVisible(xSelector);
        addAndMakeVisible(ySelector);
        addAndMakeVisible(loadImageButton);

        loadImageButton.setButtonText("IMG");
        loadImageButton.onClick = [this]() { xyPad.loadBackgroundImage(); };

        xSelector.addItem("Chaos", 1);
        xSelector.addItem("Character", 2);
        xSelector.addItem("Grain Size", 3);
        xSelector.addItem("Delay Time", 4);
        xSelector.addItem("Feedback", 5);
        xSelector.addItem("Mix", 6);
        xSelector.addItem("Distortion", 7);
        xSelector.addItem("Envelope", 8);
        xSelector.addItem("Grain Voices", 9);
        xSelector.addItem("Lim Thresh", 10);
        xSelector.addItem("Lim Release", 11);

        ySelector.addItem("Chaos", 1);
        ySelector.addItem("Character", 2);
        ySelector.addItem("Grain Size", 3);
        ySelector.addItem("Delay Time", 4);
        ySelector.addItem("Feedback", 5);
        ySelector.addItem("Mix", 6);
        ySelector.addItem("Distortion", 7);
        ySelector.addItem("Envelope", 8);
        ySelector.addItem("Grain Voices", 9);
        ySelector.addItem("Lim Thresh", 10);
        ySelector.addItem("Lim Release", 11);

        xSelector.onChange = [this]() { updateXParam(); };
        ySelector.onChange = [this]() { updateYParam(); };

        xSelector.setSelectedId(1, juce::dontSendNotification);
        ySelector.setSelectedId(2, juce::dontSendNotification);

        updateXParam();
        updateYParam();
    }

    ~XYPadPanel() override = default;

    void resized() override
    {
        auto bounds = getLocalBounds();

        int dropdownHeight = 25;
        int dropdownWidth = 150;
        int spacing = 10;

        xSelector.setBounds(bounds.getX() + spacing, bounds.getY() + 5,
                          dropdownWidth, dropdownHeight);
        ySelector.setBounds(bounds.getX() + spacing + dropdownWidth + spacing, bounds.getY() + 5,
                          dropdownWidth, dropdownHeight);

        int buttonX = bounds.getX() + spacing + dropdownWidth + spacing + dropdownWidth + spacing;
        loadImageButton.setBounds(buttonX, bounds.getY() + 5, 40, dropdownHeight);

        xyPad.setBounds(bounds.getX(), bounds.getY() + dropdownHeight + 10,
                       bounds.getWidth(), bounds.getHeight() - dropdownHeight - 10);
    }

    void paint(juce::Graphics& g) override
    {
        g.fillAll(juce::Colours::darkgrey);
    }

    void setDefaultAxes(int xId, int yId)
    {
        xSelector.setSelectedId(xId, juce::sendNotification);
        ySelector.setSelectedId(yId, juce::sendNotification);
    }

private:
    juce::AudioProcessorValueTreeState& parameters;
    XYPadComponent xyPad;

    juce::ComboBox xSelector;
    juce::ComboBox ySelector;
    juce::TextButton loadImageButton;

    void updateXParam()
    {
        int id = xSelector.getSelectedId();
        juce::String paramID;

        switch (id)
        {
            case 1: paramID = "chaos"; break;
            case 2: paramID = "character"; break;
            case 3: paramID = "grain_size"; break;
            case 4: paramID = "delay_time"; break;
            case 5: paramID = "feedback"; break;
            case 6: paramID = "mix"; break;
            case 7: paramID = "distortion_amount"; break;
            case 8: paramID = "envelope_shape"; break;
            case 9: paramID = "grain_voices"; break;
            case 10: paramID = "limiter_threshold"; break;
            case 11: paramID = "limiter_release"; break;
            default: paramID = "chaos"; break;
        }

        xyPad.setXParam(paramID);
    }

    void updateYParam()
    {
        int id = ySelector.getSelectedId();
        juce::String paramID;

        switch (id)
        {
            case 1: paramID = "chaos"; break;
            case 2: paramID = "character"; break;
            case 3: paramID = "grain_size"; break;
            case 4: paramID = "delay_time"; break;
            case 5: paramID = "feedback"; break;
            case 6: paramID = "mix"; break;
            case 7: paramID = "distortion_amount"; break;
            case 8: paramID = "envelope_shape"; break;
            case 9: paramID = "grain_voices"; break;
            case 10: paramID = "limiter_threshold"; break;
            case 11: paramID = "limiter_release"; break;
            default: paramID = "character"; break;
        }

        xyPad.setYParam(paramID);
    }

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(XYPadPanel)
};
