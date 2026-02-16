#include "PluginEditor.h"

//==============================================================================
DroneCosmosAudioProcessorEditor::DroneCosmosAudioProcessorEditor(DroneCosmosAudioProcessor& p)
    : AudioProcessorEditor(p), audioProcessor(p)
{
    // Set UI size
    setSize(800, 150);

    // Title
    titleLabel.setText("DroneCosmos", juce::dontSendNotification);
    titleLabel.setFont(juce::FontOptions(24.0f, juce::Font::bold));
    titleLabel.setColour(juce::Label::textColourId, juce::Colours::white);
    titleLabel.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(titleLabel);

    // Global preset selector
    globalPresetSelector.setTextWhenNothingSelected("Select Global Preset...");
    globalPresetSelector.onChange = [this]()
    {
        int id = globalPresetSelector.getSelectedId();
        if (id == 1) // Default
        {
            // Load default factory preset
            loadGlobalPreset(juce::File());
        }
        else if (id > 1)
        {
            int userIndex = id - 2;
            if (userIndex >= 0 && userIndex < globalUserPresetFiles.size())
                loadGlobalPreset(globalUserPresetFiles[userIndex]);
        }
    };
    addAndMakeVisible(globalPresetSelector);

    // Global save button
    globalSaveButton.setButtonText("Save");
    globalSaveButton.onClick = [this]() { saveGlobalPreset(); };
    addAndMakeVisible(globalSaveButton);

    // Oscillator A preset selector
    oscAPresetSelector.setTextWhenNothingSelected("A...");
    oscAPresetSelector.onChange = [this]()
    {
        int id = oscAPresetSelector.getSelectedId();
        if (id == 1) // Default
        {
            loadOscPreset(juce::File(), "osc_a");
        }
        else if (id > 1)
        {
            int userIndex = id - 2;
            if (userIndex >= 0 && userIndex < oscAUserPresetFiles.size())
                loadOscPreset(oscAUserPresetFiles[userIndex], "osc_a");
        }
    };
    addAndMakeVisible(oscAPresetSelector);

    oscASaveButton.setButtonText("Save A");
    oscASaveButton.onClick = [this]() { saveOscPreset("osc_a"); };
    addAndMakeVisible(oscASaveButton);

    // Oscillator B preset selector
    oscBPresetSelector.setTextWhenNothingSelected("B...");
    oscBPresetSelector.onChange = [this]()
    {
        int id = oscBPresetSelector.getSelectedId();
        if (id == 1) // Default
        {
            loadOscPreset(juce::File(), "osc_b");
        }
        else if (id > 1)
        {
            int userIndex = id - 2;
            if (userIndex >= 0 && userIndex < oscBUserPresetFiles.size())
                loadOscPreset(oscBUserPresetFiles[userIndex], "osc_b");
        }
    };
    addAndMakeVisible(oscBPresetSelector);

    oscBSaveButton.setButtonText("Save B");
    oscBSaveButton.onClick = [this]() { saveOscPreset("osc_b"); };
    addAndMakeVisible(oscBSaveButton);

    // Oscillator C preset selector
    oscCPresetSelector.setTextWhenNothingSelected("C...");
    oscCPresetSelector.onChange = [this]()
    {
        int id = oscCPresetSelector.getSelectedId();
        if (id == 1) // Default
        {
            loadOscPreset(juce::File(), "osc_c");
        }
        else if (id > 1)
        {
            int userIndex = id - 2;
            if (userIndex >= 0 && userIndex < oscCUserPresetFiles.size())
                loadOscPreset(oscCUserPresetFiles[userIndex], "osc_c");
        }
    };
    addAndMakeVisible(oscCPresetSelector);

    oscCSaveButton.setButtonText("Save C");
    oscCSaveButton.onClick = [this]() { saveOscPreset("osc_c"); };
    addAndMakeVisible(oscCSaveButton);

    // Oscillator D preset selector
    oscDPresetSelector.setTextWhenNothingSelected("D...");
    oscDPresetSelector.onChange = [this]()
    {
        int id = oscDPresetSelector.getSelectedId();
        if (id == 1) // Default
        {
            loadOscPreset(juce::File(), "osc_d");
        }
        else if (id > 1)
        {
            int userIndex = id - 2;
            if (userIndex >= 0 && userIndex < oscDUserPresetFiles.size())
                loadOscPreset(oscDUserPresetFiles[userIndex], "osc_d");
        }
    };
    addAndMakeVisible(oscDPresetSelector);

    oscDSaveButton.setButtonText("Save D");
    oscDSaveButton.onClick = [this]() { saveOscPreset("osc_d"); };
    addAndMakeVisible(oscDSaveButton);

    // Scan presets
    scanPresets();
}

DroneCosmosAudioProcessorEditor::~DroneCosmosAudioProcessorEditor()
{
}

//==============================================================================
void DroneCosmosAudioProcessorEditor::paint(juce::Graphics& g)
{
    g.fillAll(juce::Colours::black);
}

void DroneCosmosAudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced(10);

    // Header row: Title | Global Preset | Save
    auto headerRow = area.removeFromTop(40);
    titleLabel.setBounds(headerRow.removeFromLeft(150));

    auto globalPresetArea = headerRow.removeFromLeft(300);
    globalPresetSelector.setBounds(globalPresetArea.removeFromLeft(200));
    globalSaveButton.setBounds(globalPresetArea);

    // Oscillator presets row
    auto oscRow = area.removeFromTop(50);
    int oscWidth = 170;

    auto oscAArea = oscRow.removeFromLeft(oscWidth);
    oscAPresetSelector.setBounds(oscAArea.removeFromTop(30));
    oscASaveButton.setBounds(oscAArea);

    auto oscBArea = oscRow.removeFromLeft(oscWidth);
    oscBPresetSelector.setBounds(oscBArea.removeFromTop(30));
    oscBSaveButton.setBounds(oscBArea);

    auto oscCArea = oscRow.removeFromLeft(oscWidth);
    oscCPresetSelector.setBounds(oscCArea.removeFromTop(30));
    oscCSaveButton.setBounds(oscCArea);

    auto oscDArea = oscRow.removeFromLeft(oscWidth);
    oscDPresetSelector.setBounds(oscDArea.removeFromTop(30));
    oscDSaveButton.setBounds(oscDArea);
}

//==============================================================================
juce::File DroneCosmosAudioProcessorEditor::getPresetsFolder()
{
    auto folder = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
                      .getChildFile("DroneCosmos")
                      .getChildFile("Presets");
    if (!folder.exists())
        folder.createDirectory();

    // Create subfolders
    folder.getChildFile("Global").createDirectory();
    folder.getChildFile("OscA").createDirectory();
    folder.getChildFile("OscB").createDirectory();
    folder.getChildFile("OscC").createDirectory();
    folder.getChildFile("OscD").createDirectory();

    return folder;
}

juce::File DroneCosmosAudioProcessorEditor::getOscPresetFolder(const juce::String& oscId)
{
    auto folderName = oscId == "osc_a" ? "OscA" :
                      oscId == "osc_b" ? "OscB" :
                      oscId == "osc_c" ? "OscC" : "OscD";
    return getPresetsFolder().getChildFile(folderName);
}

void DroneCosmosAudioProcessorEditor::scanPresets()
{
    // Scan Global presets
    globalUserPresetNames.clear();
    globalUserPresetFiles.clear();
    globalPresetSelector.clear();
    globalPresetSelector.addItem("Default", 1);

    auto globalFolder = getPresetsFolder().getChildFile("Global");
    auto globalFiles = globalFolder.findChildFiles(juce::File::findFiles, false, "*.json");
    globalFiles.sort();

    int id = 2;
    for (auto& file : globalFiles)
    {
        auto name = file.getFileNameWithoutExtension();
        globalUserPresetNames.add(name);
        globalUserPresetFiles.add(file);
        globalPresetSelector.addItem(name, id++);
    }

    // Scan Oscillator presets
    auto scanOscPresets = [&](juce::ComboBox& selector,
                              juce::StringArray& names,
                              juce::Array<juce::File>& files,
                              const juce::String& folderName)
    {
        names.clear();
        files.clear();
        selector.clear();
        selector.addItem("Default", 1);

        auto folder = getPresetsFolder().getChildFile(folderName);
        auto folderFiles = folder.findChildFiles(juce::File::findFiles, false, "*.json");
        folderFiles.sort();

        int oscId = 2;
        for (auto& file : folderFiles)
        {
            auto name = file.getFileNameWithoutExtension();
            names.add(name);
            files.add(file);
            selector.addItem(name, oscId++);
        }
    };

    scanOscPresets(oscAPresetSelector, oscAUserPresetNames, oscAUserPresetFiles, "OscA");
    scanOscPresets(oscBPresetSelector, oscBUserPresetNames, oscBUserPresetFiles, "OscB");
    scanOscPresets(oscCPresetSelector, oscCUserPresetNames, oscCUserPresetFiles, "OscC");
    scanOscPresets(oscDPresetSelector, oscDUserPresetNames, oscDUserPresetFiles, "OscD");
}

void DroneCosmosAudioProcessorEditor::saveGlobalPreset()
{
    auto dlg = std::make_shared<juce::AlertWindow>(
        "Save Global Preset",
        "Enter preset name:",
        juce::AlertWindow::NoIcon);

    dlg->addTextEditor("name", "", "Preset Name");
    dlg->addButton("Save", 1);
    dlg->addButton("Cancel", 0);

    dlg->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, dlg](int result)
        {
            if (result == 0)
                return;

            auto name = dlg->getTextEditorContents("name").trim();
            if (name.isEmpty())
                return;

            auto file = getPresetsFolder().getChildFile("Global").getChildFile(name + ".json");

            juce::DynamicObject::Ptr obj = new juce::DynamicObject();
            auto& params = audioProcessor.parameters;

            // Save all parameters
            juce::StringArray paramIDs = {
                "mod_range", "mod_mode",
                "osc_a_waveform", "osc_a_pitch", "osc_a_detune", "osc_a_level",
                "osc_b_waveform", "osc_b_pitch", "osc_b_detune", "osc_b_level",
                "osc_c_waveform", "osc_c_pitch", "osc_c_detune", "osc_c_level",
                "osc_d_waveform", "osc_d_pitch", "osc_d_detune", "osc_d_level",
                "self_fb_phase", "self_fb_amp", "self_fb_delay", "self_fb_delay_time", "self_fb_pitch",
                "cross_mod", "ring_mod", "chaos_mod",
                "drone_pitch", "output_volume", "limiter_threshold", "limiter_release"
            };

            for (auto& id : paramIDs)
            {
                if (auto* p = params.getParameter(id))
                {
                    auto range = p->getNormalisableRange();
                    float value = range.convertFrom0to1(p->getValue());
                    obj->setProperty(id, value);
                }
            }

            auto json = juce::JSON::toString(juce::var(obj.get()));
            file.replaceWithText(json);

            scanPresets();
        }));
}

void DroneCosmosAudioProcessorEditor::saveOscPreset(const juce::String& oscId)
{
    auto oscName = oscId == "osc_a" ? "A" :
                   oscId == "osc_b" ? "B" :
                   oscId == "osc_c" ? "C" : "D";

    auto dlg = std::make_shared<juce::AlertWindow>(
        "Save Oscillator " + juce::String(oscName) + " Preset",
        "Enter preset name:",
        juce::AlertWindow::NoIcon);

    dlg->addTextEditor("name", "", "Preset Name");
    dlg->addButton("Save", 1);
    dlg->addButton("Cancel", 0);

    dlg->enterModalState(true, juce::ModalCallbackFunction::create(
        [this, dlg, oscId](int result)
        {
            if (result == 0)
                return;

            auto name = dlg->getTextEditorContents("name").trim();
            if (name.isEmpty())
                return;

            auto folderName = oscId == "osc_a" ? "OscA" :
                             oscId == "osc_b" ? "OscB" :
                             oscId == "osc_c" ? "OscC" : "OscD";
            auto file = getPresetsFolder().getChildFile(folderName).getChildFile(name + ".json");

            juce::DynamicObject::Ptr obj = new juce::DynamicObject();
            auto& params = audioProcessor.parameters;

            // Save only oscillator-specific parameters
            juce::StringArray paramIDs = {
                oscId + "_waveform", oscId + "_pitch", oscId + "_detune", oscId + "_level"
            };

            for (auto& id : paramIDs)
            {
                if (auto* p = params.getParameter(id))
                {
                    auto range = p->getNormalisableRange();
                    float value = range.convertFrom0to1(p->getValue());
                    obj->setProperty(id, value);
                }
            }

            auto json = juce::JSON::toString(juce::var(obj.get()));
            file.replaceWithText(json);

            scanPresets();
        }));
}

void DroneCosmosAudioProcessorEditor::loadGlobalPreset(const juce::File& file)
{
    auto& params = audioProcessor.parameters;

    auto setParam = [&](const juce::String& id, float value)
    {
        if (auto* p = params.getParameter(id))
        {
            auto range = p->getNormalisableRange();
            p->setValueNotifyingHost(range.convertTo0to1(value));
        }
    };

    if (!file.exists())
    {
        // Load default factory preset
        setParam("mod_range", 1.0f); // Mid
        setParam("mod_mode", 0.0f);
        setParam("osc_a_waveform", 0.0f);
        setParam("osc_a_pitch", 0.0f);
        setParam("osc_a_detune", 0.0f);
        setParam("osc_a_level", 75.0f);
        setParam("osc_b_waveform", 0.0f);
        setParam("osc_b_pitch", 0.0f);
        setParam("osc_b_detune", 7.0f);
        setParam("osc_b_level", 75.0f);
        setParam("osc_c_waveform", 1.0f);
        setParam("osc_c_pitch", -12.0f);
        setParam("osc_c_detune", -5.0f);
        setParam("osc_c_level", 50.0f);
        setParam("osc_d_waveform", 2.0f);
        setParam("osc_d_pitch", 7.0f);
        setParam("osc_d_detune", 3.0f);
        setParam("osc_d_level", 50.0f);
        setParam("self_fb_phase", 0.0f);
        setParam("self_fb_amp", 0.0f);
        setParam("self_fb_delay", 0.0f);
        setParam("self_fb_delay_time", 5.0f);
        setParam("self_fb_pitch", 0.0f);
        setParam("cross_mod", 0.0f);
        setParam("ring_mod", 0.0f);
        setParam("chaos_mod", 0.0f);
        setParam("drone_pitch", 55.0f);
        setParam("output_volume", 80.0f);
        setParam("limiter_threshold", -1.0f);
        setParam("limiter_release", 100.0f);
        return;
    }

    auto json = juce::JSON::parse(file);
    if (!json.isObject())
        return;

    if (auto* obj = json.getDynamicObject())
    {
        for (auto& prop : obj->getProperties())
        {
            auto id = prop.name.toString();
            float value = static_cast<float>(prop.value);
            setParam(id, value);
        }
    }
}

void DroneCosmosAudioProcessorEditor::loadOscPreset(const juce::File& file, const juce::String& oscId)
{
    auto& params = audioProcessor.parameters;

    auto setParam = [&](const juce::String& id, float value)
    {
        if (auto* p = params.getParameter(id))
        {
            auto range = p->getNormalisableRange();
            p->setValueNotifyingHost(range.convertTo0to1(value));
        }
    };

    if (!file.exists())
    {
        // Load default oscillator settings
        if (oscId == "osc_a")
        {
            setParam(oscId + "_waveform", 0.0f);
            setParam(oscId + "_pitch", 0.0f);
            setParam(oscId + "_detune", 0.0f);
            setParam(oscId + "_level", 75.0f);
        }
        else if (oscId == "osc_b")
        {
            setParam(oscId + "_waveform", 0.0f);
            setParam(oscId + "_pitch", 0.0f);
            setParam(oscId + "_detune", 7.0f);
            setParam(oscId + "_level", 75.0f);
        }
        else if (oscId == "osc_c")
        {
            setParam(oscId + "_waveform", 1.0f);
            setParam(oscId + "_pitch", -12.0f);
            setParam(oscId + "_detune", -5.0f);
            setParam(oscId + "_level", 50.0f);
        }
        else if (oscId == "osc_d")
        {
            setParam(oscId + "_waveform", 2.0f);
            setParam(oscId + "_pitch", 7.0f);
            setParam(oscId + "_detune", 3.0f);
            setParam(oscId + "_level", 50.0f);
        }
        return;
    }

    auto json = juce::JSON::parse(file);
    if (!json.isObject())
        return;

    if (auto* obj = json.getDynamicObject())
    {
        for (auto& prop : obj->getProperties())
        {
            auto id = prop.name.toString();
            float value = static_cast<float>(prop.value);
            setParam(id, value);
        }
    }
}
