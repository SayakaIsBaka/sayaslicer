# sayaslicer

> [!WARNING]
> Currently a work-in-progress; no public builds are currently available (you can compile it yourself if you know how to though)

A cross-platform audio slicer with BMS-related features. Aims to be a drop-in replacement for [woslicerII](https://cerebralmuddystream.nekokan.dyndns.info/soft/woslicerII.zip) [(english version)](https://github.com/SayakaIsBaka/woslicerII-english) by [wosderge](https://cerebralmuddystream.nekokan.dyndns.info) with similar features and newly added ones, while also fixing some long-standing limitations and ergonomic issues.

## Features
- BMSE clipboard support (copy and paste, can be used with [BMHelper](https://excln.github.io/bmhelper.html))
- File drag and drop (only on Windows)
- Offset setting
- Arbitrary snapping (from 1/1 to 1/192)
- Toggleable base-62 support for BMSE clipboard data copy
- Silent tail remover with configurable threshold
- Fadeout
- Partial BPM changes support (only for audio slicing, a bit hackish but it works)
- Load / save project

## Keyboard shortcuts
Keyboard shortcuts are the same as woslicerII except the keyboard layout is taken into account:
- `O`: open audio file
- `Z`: add slice marker
- `C`: clear all markers
- `V`: copy markers as BMSE clipboard data
- `B`: import markers from clipboard (using BMSE clipboard data)
- `M`: export keysounds
- `P`: preview current keysound and move to the next one
- `Enter`: preview current keysound
- `LeftArrow / RightArrow`: move position cursor
- `Shift + LeftArrow / Shift + RightArrow`: move position cursor (jump to closest snap)
- `UpArrow / DownArrow`: set snapping

Additionally, clicking on a marker in the table will jump the cursor to its position; right-clicking it will delete it.

## Planned features
- Import markers from a MIDI file
- Import marker names from [mid2bms](https://mid2bms.net)

The following features are less likely to be implemented but might be done depending on whether I feel like it or if there's enough demand for it:
- Copy / paste measures (doesn't make that much sense since markers are stored using their absolute value, maybe something similar to Vim's visual mode?)
- Better BPM change support (BMSE clipboard support and proper grid display)

## Credits
sayaslicer uses the following libraries:
- [Dear ImGui](https://github.com/ocornut/imgui)
- [SFML](https://github.com/SFML/SFML)
- [imgui-sfml](https://github.com/SFML/imgui-sfml)
- [implot](https://github.com/epezent/implot)
- [tinyfiledialogs](https://sourceforge.net/projects/tinyfiledialogs/)
- [clip](https://github.com/dacap/clip)
- [ImGuiNotify](https://github.com/TyomaVader/ImGuiNotify)
- [Moonlight (ImGui theme)](https://github.com/Madam-Herta/Moonlight/)
