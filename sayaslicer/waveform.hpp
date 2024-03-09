#pragma once

#include "settings.hpp"
#include <SFML/Audio.hpp>
#include <implot.h>

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
        double yMinLimit = -32768;
        double yMaxLimit = 32768;
        double startRect = settings.selection.start / waveformReso;
        double endRect = (settings.selection.isSelectMode ? settings.cursorPos : settings.selection.end) / waveformReso;
        ImPlot::DragRect(0, &startRect, &yMinLimit, &endRect, &yMaxLimit, ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), ImPlotDragToolFlags_NoInputs);
    }
}

void DisplayWaveform(sf::SoundBuffer& buffer, SlicerSettings& settings) {
    double maxDisplayRange = 1500.0;
    double leftMargin = (buffer.getSampleCount() < 400 * waveformReso ? 0 : 400) * waveformReso;

    if (ImPlot::BeginPlot("##lines", ImVec2(-1, 200), ImPlotFlags_NoBoxSelect | ImPlotFlags_NoLegend)) {
        double plotStart = (settings.cursorPos - leftMargin) / waveformReso;
        double plotEnd = plotStart + maxDisplayRange;
        ImPlot::SetupAxisLinks(ImAxis_X1, &plotStart, &plotEnd);
        ImPlot::SetupAxisLimits(ImAxis_Y1, -32768, 32768);

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
            ImPlot::SetupAxisLimitsConstraints(ImAxis_X1, -leftMargin, sampleCount / waveformReso - settings.offset);

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

            auto samples = buffer.getSamples();
            ImPlot::SetNextLineStyle(ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram));
            size_t arrLen = maxDisplayRange;
            if (lastTick > sampleCount)
                arrLen = (sampleCount - settings.cursorPos + leftMargin) / waveformReso;
            size_t arrayOffset = (std::max((long long)(settings.cursorPos - leftMargin), (long long)0) / (waveformReso * numChannels)) * (waveformReso * numChannels);
            ImPlot::PlotLine("Waveform", &samples[arrayOffset], arrLen, 1.0, arrayOffset / (waveformReso), 0, settings.offset, waveformReso * numChannels); // Buffer stores samples as [channel1_i, channel2_i, channel1_i+1, etc.]

            // Display cursor
            double curDisplayPos = settings.cursorPos / waveformReso;
            ImPlot::DragLineX(555, &curDisplayPos, ImGui::GetStyleColorVec4(ImGuiCol_PlotLines), 0.5, ImPlotDragToolFlags_NoInputs);
            if (settings.selection.isSelectMode)
                ImPlot::TagX(curDisplayPos, ImGui::GetStyleColorVec4(ImGuiCol_PlotHistogram), "Select");
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