#include "waveform.hpp"

#include "audio.hpp"

using namespace i18n::literals;

int MeterFormatter(double value, char* buff, int size, void* data) {
    SlicerSettings settings = *(SlicerSettings*)data;
    const double reducedSamplesPerMeasure = settings.samplesPerSnap * settings.snapping / waveformReso;
    double tmp = fmod(value, reducedSamplesPerMeasure);
    double delta = 0.0001;
    if (tmp <= delta || tmp >= reducedSamplesPerMeasure - delta)
        return snprintf(buff, size, "%d", (int)round(value / reducedSamplesPerMeasure));
    else
        return snprintf(buff, size, "");
}

void DrawSelection(SlicerSettings& settings) {
    if (settings.selection.start != -1.0) {
        double yMinLimit = -1.0;
        double yMaxLimit = 1.0;
        double startRect = settings.selection.start / waveformReso;
        double endRect = (settings.selection.isSelectMode ? settings.cursorPos : settings.selection.end) / waveformReso;
        ImPlot::DragRect(0, &startRect, &yMinLimit, &endRect, &yMaxLimit, ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), ImPlotDragToolFlags_NoInputs);
    }
}

std::vector<float> ApplyAudioEffects(SoundBuffer& buffer, SlicerSettings& settings, size_t arrayOffset, size_t arrLen) { // TODO: optimize by reducing the amount to copy on each pass
    auto& samples = buffer.getSamples();
    unsigned long long offsetSamples = (long long)settings.offset * (long long)waveformReso;
    std::vector<double> markers;
    settings.markers.sort();
    unsigned long long endOffset = samples.size();

    for (size_t i = 0; i < settings.markers.size(); i++) {
        auto markerPos = settings.markers.get(i).position + offsetSamples;
        if (markerPos > arrayOffset + arrLen) {
            if (markerPos < samples.size())
                endOffset = markerPos;
            break;
        }
        if (markerPos < arrayOffset)
            continue;
        if (markers.empty())
            markers.push_back(settings.markers.get(i == 0 ? 0 : i - 1).position + offsetSamples);
        if (i != 0)
            markers.push_back(markerPos);
    }
    if (markers.empty())
        return std::vector<float>();

    auto base = markers[0] > arrayOffset ? arrayOffset : markers[0];
    std::vector<float> newBuf(samples.begin() + base, samples.begin() + endOffset);
    for (size_t i = 0; i < markers.size(); i++) {
        auto begin = markers[i] - base;
        auto end = newBuf.size();
        if (i + 1 < markers.size())
            end = markers[i + 1] - base;
        tcb::span<float> view(&newBuf[begin], &newBuf[end - 1]);
        if (settings.selectedGateThreshold != 0)
            ApplyNoiseGate(view, gateThresholds[settings.selectedGateThreshold], buffer.getChannelCount());
        if (settings.fadein != 0)
            ApplyFadein(view, settings.fadein, buffer.getSampleRate(), buffer.getChannelCount());
        if (settings.fadeout != 0)
            ApplyFadeout(view, settings.fadeout, buffer.getSampleRate(), buffer.getChannelCount());
    }
    if (base != arrayOffset)
        newBuf.erase(newBuf.begin(), newBuf.begin() + (arrayOffset - base));
    newBuf.resize(arrLen);
    return newBuf;
}

void DisplayWaveform(SoundBuffer& buffer, SlicerSettings& settings) {
    double maxDisplayRange = settings.maxDisplayRange;
    double marginConst = 1600.0 * maxDisplayRange / minZoom;
    double leftMargin = (buffer.getSampleCount() < marginConst * waveformReso ? 0 : marginConst) * waveformReso;

    if (ImPlot::BeginPlot("##lines", ImVec2(-1, 200), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoLegend)) {
        double plotStart = (settings.cursorPos - leftMargin) / waveformReso;
        double plotEnd = plotStart + maxDisplayRange;
        ImPlot::SetupAxisLinks(ImAxis_X1, &plotStart, &plotEnd);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -1.0f, 1.0f);

        ImPlot::SetupAxis(ImAxis_Y1, "", ImPlotAxisFlags_NoGridLines | ImPlotAxisFlags_NoTickLabels | ImPlotAxisFlags_Lock | ImPlotAxisFlags_NoTickMarks);
        ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_Foreground | ImPlotAxisFlags_NoTickLabels);

        auto sampleCount = buffer.getSampleCount();
        auto sampleRate = buffer.getSampleRate();
        auto numChannels = buffer.getChannelCount();

        double samplesPerBeat = sampleRate ? 60.0 / (double)settings.bpm * ((double)sampleRate * (double)numChannels) : 1.0;
        settings.samplesPerSnap = samplesPerBeat / (double)settings.snapping * 4.0;
        int beatsToDisplayLeft = (leftMargin / samplesPerBeat) - (int)(leftMargin / samplesPerBeat) % 4 + 4;

        double leftPartSamples = fmod(settings.cursorPos, samplesPerBeat * beatsToDisplayLeft);
        double fixMod = settings.cursorPos > leftMargin && leftPartSamples < leftMargin ? samplesPerBeat * beatsToDisplayLeft : 0.0;
        double startTick = settings.cursorPos - leftPartSamples - fixMod;
        double lastTick = settings.cursorPos + maxDisplayRange * waveformReso;

        std::vector<double> ticks;
        if (sampleCount > 0) {
            for (double i = startTick; i < lastTick; i += settings.samplesPerSnap)
                ticks.push_back(i / waveformReso);
        }
        else {
            ticks.push_back(0.0);
        }
        int nbTicksToDraw = ticks.size();

        ImPlot::SetupAxisFormat(ImAxis_X1, MeterFormatter, &settings);
        ImPlot::SetupAxisTicks(ImAxis_X1, ticks.data(), nbTicksToDraw);

        if (sampleCount > 0) {
            ImPlot::SetupAxis(ImAxis_X1, "", ImPlotAxisFlags_Foreground);
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, -marginConst, sampleCount / waveformReso - settings.offset);

            // Draw barlines
            for (double j = 0; j < lastTick; j += samplesPerBeat * 4) {
                if (j < settings.cursorPos - leftMargin)
                    continue;
                double tmp = j / waveformReso;
                ImPlot::DragLineX(555, &tmp, ImVec4(1, 1, 1, 0.25), 0.1, ImPlotDragToolFlags_NoInputs);
            }

            // Draw beat lines if snapping is a multiple of 4
            if (settings.snapping % 4 == 0) {
                for (double j = 0; j < lastTick; j += samplesPerBeat) {
                    if (j < settings.cursorPos - leftMargin)
                        continue;
                    double tmp = j / waveformReso;
                    ImPlot::DragLineX(555, &tmp, ImVec4(1, 1, 1, 0.075), 0.05, ImPlotDragToolFlags_NoInputs);
                }
            }

            auto& samples = buffer.getSamples();
            ImPlot::SetNextLineStyle(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram));
            size_t arrLen = maxDisplayRange + settings.offset;
            size_t arrayOffset = (std::max((long long)(settings.cursorPos - leftMargin), (long long)0) / (waveformReso * numChannels)) * (waveformReso * numChannels);
            int stride = waveformReso;
            if (lastTick > sampleCount)
                arrLen = (samples.size() - arrayOffset) / stride;

            auto wavBuf = &samples[arrayOffset];
            
            // Apply audio effects to waveform
            std::vector<float> tmp;
            if (settings.selectedGateThreshold != 0 || settings.fadeout != 0 || settings.fadein != 0) {
                tmp = ApplyAudioEffects(buffer, settings, arrayOffset, arrLen * stride);
                if (!tmp.empty())
                    wavBuf = tmp.data();
            }

            ImPlot::PlotLine("Waveform", wavBuf, arrLen, 1.0, arrayOffset / (waveformReso), 0, settings.offset, stride * sizeof(float)); // Buffer stores samples as [channel1_i, channel2_i, channel1_i+1, etc.]

            // Display cursor
            double curDisplayPos = settings.cursorPos / waveformReso;
            ImPlot::DragLineX(555, &curDisplayPos, ImGui::GetStyleColorVec4(ImGuiCol_PlotLines), 0.5, ImPlotDragToolFlags_NoInputs);
            if (settings.selection.isSelectMode)
                ImPlot::TagX(curDisplayPos, ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), "select"_t.c_str());
            else
                ImPlot::TagX(curDisplayPos, ImGui::GetStyleColorVec4(ImGuiCol_PlotLines));

            for (auto m : settings.markers) {
                if (m.position < settings.cursorPos - leftMargin)
                    continue;
                double mTmp = m.position / waveformReso;
                ImPlot::DragLineX(0, &mTmp, ImVec4(1, 1, 1, 1), 1, ImPlotDragToolFlags_NoInputs);
            }

            DrawSelection(settings);
        }
        ImPlot::EndPlot();
        settings.cursorPos = plotStart * waveformReso + leftMargin;
    }
}