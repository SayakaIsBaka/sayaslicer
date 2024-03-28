#include "markdown.hpp"

void LinkCallback(ImGui::MarkdownLinkCallbackData data_)
{
    std::string url(data_.link, data_.linkLength);
    if (!data_.isImage)
    {
#ifdef _WIN32
        ShellExecuteA(NULL, "open", url.c_str(), NULL, NULL, SW_SHOWNORMAL);
#else
#if __APPLE__
        const char* open_executable = "open";
#else
        const char* open_executable = "xdg-open";
#endif
        char command[256];
        snprintf(command, 256, "%s \"%s\"", open_executable, url.c_str());
        system(command);
#endif
    }
}

void MyMarkdownFormatCallback(const ImGui::MarkdownFormatInfo& markdownFormatInfo_, bool start_)
{
    // Call the default first so any settings can be overwritten by our implementation.
    // Alternatively could be called or not called in a switch statement on a case by case basis.
    // See defaultMarkdownFormatCallback definition for furhter examples of how to use it.
    ImGui::defaultMarkdownFormatCallback(markdownFormatInfo_, start_);

    switch (markdownFormatInfo_.type)
    {
        // example: change the colour of heading level 2
    case ImGui::MarkdownFormatType::LINK:
    {

        if (start_)
        {
            ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyle().Colors[ImGuiCol_PlotLines]);
        }
        else
        {
            ImGui::PopStyleColor();
        }
        break;
    }
    default:
    {
        break;
    }
    }
}

void Markdown(const std::string& markdown_)
{
    ImGui::MarkdownConfig mdConfig{ LinkCallback };
    mdConfig.formatCallback = MyMarkdownFormatCallback;
    ImGui::Markdown(markdown_.c_str(), markdown_.length(), mdConfig);
}