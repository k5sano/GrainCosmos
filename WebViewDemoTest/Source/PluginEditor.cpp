#include "PluginEditor.h"

//==============================================================================
WebViewDemoAudioProcessorEditor::WebViewDemoAudioProcessorEditor(WebViewDemoAudioProcessor& p)
    : AudioProcessorEditor(&p)
    , processorRef(p)
{
    // Fixed size for the demo
    setResizable(false, false);
    setSize(500, 500);

    // All members are initialized via in-class member initializers
    addAndMakeVisible(webComponent);
    webComponent.goToURL(juce::WebBrowserComponent::getResourceProviderRoot());
}

WebViewDemoAudioProcessorEditor::~WebViewDemoAudioProcessorEditor()
{
}

//==============================================================================
void WebViewDemoAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void WebViewDemoAudioProcessorEditor::resized()
{
    webComponent.setBounds(getLocalBounds());
}

//==============================================================================
std::optional<juce::WebBrowserComponent::Resource> WebViewDemoAudioProcessorEditor::getResource(const juce::String& url)
{
    // Simple HTML content for the demo UI
    static const char* htmlContent = R"html(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>WebView Demo</title>
    <style>
        body {
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif;
            background: linear-gradient(135deg, #1a1a2e 0%, #16213e 100%);
            color: #eee;
            margin: 0;
            padding: 20px;
            display: flex;
            flex-direction: column;
            align-items: center;
            min-height: 100vh;
        }
        h1 {
            color: #4fc3f7;
            text-shadow: 0 0 10px rgba(79, 195, 247, 0.5);
        }
        .control-group {
            background: rgba(255,255,255,0.05);
            border-radius: 10px;
            padding: 20px;
            margin: 10px 0;
            width: 300px;
            border: 1px solid rgba(79, 195, 247, 0.3);
        }
        label {
            display: block;
            margin-bottom: 8px;
            color: #81d4fa;
        }
        input[type="range"] {
            width: 100%;
            margin: 10px 0;
        }
        button {
            background: linear-gradient(135deg, #4fc3f7 0%, #29b6f6 100%);
            border: none;
            border-radius: 5px;
            padding: 10px 20px;
            color: #1a1a2e;
            font-weight: bold;
            cursor: pointer;
            width: 100%;
        }
        select {
            width: 100%;
            padding: 8px;
            background: rgba(255,255,255,0.1);
            border: 1px solid rgba(79, 195, 247, 0.5);
            border-radius: 5px;
            color: #eee;
        }
    </style>
</head>
<body>
    <h1>WebView Demo</h1>
    <p>JUCE 8.0.4 - Native Integration Test</p>

    <div class="control-group">
        <label for="cutoff">Cutoff Frequency</label>
        <input type="range" id="cutoff" data-juce-type="slider" min="200" max="14000" value="11000">
        <span id="cutoff-value">11000 Hz</span>
    </div>

    <div class="control-group">
        <label for="filterType">Filter Type</label>
        <select id="filterType" data-juce-type="combobox">
            <option value="0">Low-pass</option>
            <option value="1">High-pass</option>
            <option value="2">Band-pass</option>
        </select>
    </div>

    <div class="control-group">
        <button id="mute" data-juce-type="togglebutton">Mute: OFF</button>
    </div>

    <script type="module">
        import * as juce from "./js/juce/index.js";

        // Initialize JUCE integration
        await juce.init();

        // Get parameter elements
        const cutoffSlider = document.getElementById('cutoff');
        const cutoffValue = document.getElementById('cutoff-value');
        const muteButton = document.getElementById('mute');
        const filterTypeSelect = document.getElementById('filterType');

        // Listen for parameter changes from native side
        juce.parameters.cutoff.addListener((value) => {
            cutoffSlider.value = value;
            cutoffValue.textContent = Math.round(value) + ' Hz';
        });

        juce.parameters.mute.addListener((value) => {
            muteButton.textContent = 'Mute: ' + (value ? 'ON' : 'OFF');
        });

        juce.parameters.filterType.addListener((value) => {
            filterTypeSelect.value = value;
        });

        // Send UI changes to native side
        cutoffSlider.addEventListener('input', (e) => {
            cutoffValue.textContent = Math.round(e.target.value) + ' Hz';
        });

        muteButton.addEventListener('click', () => {
            // Toggle will be handled by JUCE
        });
    </script>
</body>
</html>
)html";

    // Handle root URL or index.html
    if (url == "/" || url.endsWith("/index.html") || url == "index.html")
    {
        auto makeVector = [](const char* data, size_t size) {
            return std::vector<std::byte>(
                reinterpret_cast<const std::byte*>(data),
                reinterpret_cast<const std::byte*>(data) + size
            );
        };
        return juce::WebBrowserComponent::Resource {
            makeVector(htmlContent, std::strlen(htmlContent)),
            juce::String("text/html")
        };
    }

    // JUCE frontend library - minimal stub
    if (url == "/js/juce/index.js" || url.endsWith("/js/juce/index.js"))
    {
        static const char* jsContent = R"js(
// Minimal JUCE WebView frontend stub
export let parameters = {};

export async function init() {
    if (window.juceNative) {
        // Get initial parameter values
        parameters = await window.juceNative.getParameterInfos();
    }
}
)js";
        auto makeVector = [](const char* data, size_t size) {
            return std::vector<std::byte>(
                reinterpret_cast<const std::byte*>(data),
                reinterpret_cast<const std::byte*>(data) + size
            );
        };
        return juce::WebBrowserComponent::Resource {
            makeVector(jsContent, std::strlen(jsContent)),
            juce::String("text/javascript")
        };
    }

    return std::nullopt;
}
