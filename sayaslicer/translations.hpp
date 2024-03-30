#pragma once

#include <i18n_keyval/src/i18n_keyval/i18n.hpp>
#include <i18n_keyval/src/i18n_keyval/translators/basic.hpp>
#include <vector>

static i18n::translations translations{
	{"en", {
		{"_lang", "English"},
		{"general", "General"},
		{"zoom", "Zoom"},
		{"offset", "Offset"},
		{"position", "Position"},
		{"bpm", "BPM"},
		{"snapping", "Snapping"},
		{"starting_key", "Starting key"},
		{"decimal_value", "Decimal value"},
		{"enable_b62", "Enable base-62"},
		{"zerocross_markers", "Zero-cross markers"},
		{"file", "File"},
		{"open_project", "Open project"},
		{"save_project", "Save project"},
		{"import_audio_file", "Import audio file"},
		{"export_keysounds", "Export keysounds"},
		{"quit", "Quit"},
		{"edit", "Edit"},
		{"import_midi", "Import slices from MIDI"},
		{"import_mid2bms", "Import Mid2BMS renamer file"},
		{"export_bmse", "Copy BMSE clipboard data"},
		{"export_ibmsc", "Copy iBMSC clipboard data"},
		{"import_bmse", "Paste BMSE clipboard data"},
		{"export_keysound_list", "Copy keysound list to clipboard"},
		{"clear_all_markers", "Clear all markers"},
		{"clear_all_markers_with_0", "Clear all markers (including 0)"},
		{"preferences", "Preferences"},
		{"export_settings", "Export settings"},
		{"disabled", "Disabled"},
		{"noise_gate", "Noise gate"},
		{"fadeout", "Fadeout"},
		{"process", "Process"},
		{"waveform", "Waveform"},
		{"markers", "Markers"},
		{"id", "ID"},
		{"name", "Name"},
		{"no_markers_set", "No markers set..."},
		{"select_midi_track", "Select MIDI track"},
		{"select_track_import", "Select the track to import"},
		{"all_tracks", "All tracks"},
		{"clear_existing_markers", "Clear existing markers"},
		{"import", "Import"},
		{"cancel", "Cancel"},
		{"autodetect_starting_keysound", "Auto-detect starting keysound"},
		{"language", "Language"},
		{"save", "Save"},
		{"opened_file", "Opened file"},
		{"selected_file_not_supported", "Selected file isn't supported!"},
		{"exported_keysounds_to_folder", "Exported keysounds to the following folder"},
		{"success", "Success"},
		{"warning", "Warning"},
		{"error", "Error"},
		{"select", "Select"},
		{"file_doesnt_exist", "Selected file does not exist!"},
		{"load_file_first", "Please load an audio file first!"},
		{"moved_markers_zerocrossing", "Moved markers to zero-crossing points!"},
		{"imported_markers_clipboard", "Successfully imported markers from the clipboard!"},
		{"clipboard_no_bmse_data", "Clipboard does not contain any BMSE data!"},
		{"copied_markers_bmse_data", "Copied markers as BMSE clipboard data!"},
		{"copied_markers_ibmsc_data", "Copied markers as iBMSC clipboard data!"},
		{"select_region_first", "Please select a region first!"},
		{"selected_region_no_markers", "Selected region does not contain any markers!"},
		{"clipboard_empty", "Clipboard is empty!"},
		{"cut_markers", "Cut {count} marker!"},
		{"cut_markers_plural", "Cut {count} markers!"},
		{"copied_markers", "Copied {count} marker!"},
		{"copied_markers_plural", "Copied {count} markers!"},
		{"pasted_markers", "Pasted {count} marker!"},
		{"pasted_markers_plural", "Pasted {count} markers!"},
		{"deleted_markers", "Deleted {count} marker!"},
		{"deleted_markers_plural", "Deleted {count} markers!"},
		{"imported_midi_markers", "Successfully imported markers from MIDI file!"},
		{"saved_project_to", "Saved project to"},
		{"opened_project", "Opened project"},
		{"copied_keysound_list_clipboard", "Copied keysound list to clipboard!"},
		{"marker_names_import_success", "Successfully imported marker names!"},
		{"marker_names_import_warning", "Imported marker names but the number of names did not perfectly match the number of markers"},
		{"close", "Close"},
		{"about", "About..."},
		{"help", "Help"},
		{"help_menu", "?"},
		{"info", "Info"},
		{"check_for_updates", "Check for updates"},
		{"check_for_updates_startup", "Check for updates on startup"},
		{"latest_version", "You have the latest version!"},
		{"update_available", "An update is available!"},
		{"error_update_checking", "Error checking for updates"},
		{"credits", "Credits"},
		{"used_libraries", "Used libraries"},
		{"download_update", "Download update"},
		{"downloading_update", "Downloading update..."},
		{"downloaded_update_to", "Downloaded latest version to"},
		{"error_downloading_update", "Error downloading update"},
		{"keyboard_shortcuts", "Keyboard shortcuts"},
		{"keyboard_shortcuts_list", u8R"(  * O: open audio file
  * Z: add slice marker
  * C: clear all markers (but adds a marker at 0, similar behaviour to woslicerII)
  * Shift+C: clear ALL markers (including marker at 0)
  * V: copy markers as BMSE clipboard data
  * Shift+V: copy markers as iBMSC clipboard data
  * B: import markers from clipboard (using BMSE clipboard data)
  * K: copy keysound list to clipboard
  * Shift+K: append keysound list to BMS (if in the same folder as the audio file)
  * M: export keysounds
  * P: preview current keysound and move to the next one
  * Enter: preview current keysound
  * LeftArrow / RightArrow: move position cursor
  * Shift + LeftArrow / Shift + RightArrow: move position cursor (jump to closest snap)
  * UpArrow / DownArrow: set snapping
  * Ctrl+O: load project
  * Ctrl+S: save project
  * Home: jump to the beginning of the waveform
  * End: jump to the end of the waveform
  * Space: enter / exit select mode
  * Ctrl+C: copy selected markers
  * Ctrl+X: cut selected markers
  * Ctrl+V: paste previously copied markers
  * Del: delete selected markers
  * Ctrl+Z: undo action
  * Ctrl+Y: redo action
)"},
		{"append_keysound_list_bms", "Append keysound list to BMS"},
		{"no_suitable_bms_found", "No suitable BMS file found"},
		{"appended_keysound_list_to_file", "Appended keysound list to the following file"},
		{"more_than_one_bms_found_in_folder", "More than one BMS file found in the folder"},
	}},
	{"jp", {
		{"_lang", u8"日本語"},
		{"general", u8"基本設定"},
		{"zoom", u8"ズーム"},
		{"offset", u8"ずらし"},
		{"position", u8"位置"},
		{"bpm", u8"BPM"},
		{"snapping", u8"Grid"},
		{"starting_key", u8"始番号"},
		{"decimal_value", u8"10進数値"},
		{"enable_b62", u8"62進数モード"},
		{"zerocross_markers", u8"ゼロクロス切断配置"},
		{"file", u8"ファイル"},
		{"open_project", u8"プロジェクトを開く"},
		{"save_project", u8"プロジェクトを保存"},
		{"import_audio_file", u8"音声ファイルを読み込む"},
		{"export_keysounds", u8"キー音を出力"},
		{"quit", u8"終了"},
		{"edit", u8"編集"},
		{"import_midi", u8"MIDIファイルから切断配置を読み込む"},
		{"import_mid2bms", u8"text5_renamer_array.txtを読み込む"},
		{"export_bmse", u8"BMSEシーケンスをコーピ"},
		{"export_ibmsc", u8"iBMSCシーケンスをコーピ"},
		{"import_bmse", u8"BMSEシーケンスをペースト"},
		{"export_keysound_list", u8"WAV定義情報をクリップボードに書き出す"},
		{"clear_all_markers", u8"切断配置を全消去"},
		{"clear_all_markers_with_0", u8"切断配置を全消去（ID0も含めて）"},
		{"preferences", u8"環境設定"},
		{"export_settings", u8"出力設定"},
		{"disabled", u8"無効"},
		{"noise_gate", u8"無音切"},
		{"fadeout", u8"末端"},
		{"process", u8"処理"},
		{"waveform", u8"波形"},
		{"markers", u8"切断配置"},
		{"id", u8"ID"},
		{"name", u8"ファイル名"},
		{"no_markers_set", u8"切断配置がありません"},
		{"select_midi_track", u8"MIDIチャンネル選択"},
		{"select_track_import", u8"MIDIチャンネルを選んでください"},
		{"all_tracks", u8"全部"},
		{"clear_existing_markers", u8"既存の切断配置を消す"},
		{"import", u8"読み込む"},
		{"cancel", u8"キャンセル"},
		{"autodetect_starting_keysound", u8"始番号の自動調整"},
		{"language", u8"言語選択"},
		{"save", u8"OK"},
		{"opened_file", u8"ファイルを読み込みました"},
		{"selected_file_not_supported", u8"未対応のファイル形式です"},
		{"exported_keysounds_to_folder", u8"以下のフォルダーにキー音を出力しました"},
		{"success", u8"成功"},
		{"warning", u8"注意"},
		{"error", u8"エラー"},
		{"select", u8"選択"},
		{"file_doesnt_exist", u8"指定したファイルは存在しません"},
		{"load_file_first", u8"まず音声ファイルを開いてください"},
		{"moved_markers_zerocrossing", u8"切断配置をゼロクロス箇所に移しました"},
		{"imported_markers_clipboard", u8"切断配置をクリップボードから読み込みました"},
		{"clipboard_no_bmse_data", u8"クリップボードにBMSEシーケンスデータがありません"},
		{"copied_markers_bmse_data", u8"切断配置をBMSEシーケンスデータとしてコーピしました"},
		{"copied_markers_ibmsc_data", u8"切断配置をiBMSCシーケンスデータとしてコーピしました"},
		{"select_region_first", u8"まず領域を選択してください"},
		{"selected_region_no_markers", u8"選択中の領域に切断配置がありません"},
		{"clipboard_empty", u8"クリップボードは空です"},
		{"cut_markers", u8"{count}つの切断配置をカットしました"},
		{"cut_markers_plural", u8"{count}つの切断配置をカットしました"},
		{"copied_markers", u8"{count}つの切断配置をコーピしました"},
		{"copied_markers_plural", u8"{count}つの切断配置をコーピしました"},
		{"pasted_markers", u8"{count}つの切断配置をペーストしました"},
		{"pasted_markers_plural", u8"{count}つの切断配置をペーストしました"},
		{"deleted_markers", u8"{count}つの切断配置を消しました"},
		{"deleted_markers_plural", u8"{count}つの切断配置を消しました"},
		{"imported_midi_markers", u8"切断配置をMIDIファイルから読み込みました"},
		{"saved_project_to", u8"以下のフォルダーにプロジェクトを保存しました"},
		{"opened_project", u8"プロジェクトを開きました"},
		{"copied_keysound_list_clipboard", u8"WAV定義情報をクリップボードに書き出しました"},
		{"marker_names_import_success", u8"ファイル名のインポートに成功しました"},
		{"marker_names_import_warning", u8"ファイル名をインポートしましたがファイル名の数値と切断配置の数値が一致していません"},
		{"close", u8"閉じる"},
		{"about", u8"このソフトについて"},
		{"help", u8"ヘルプ"},
		{"help_menu", u8"ヘルプ"},
		{"info", u8"情報"},
		{"check_for_updates", u8"最新バージョンの有無を確認"},
		{"check_for_updates_startup", u8"起動時に最新バージョンの有無を確認"},
		{"latest_version", u8"お使いのソフトは最新版です"},
		{"update_available", u8"新しいバージョンがあります"},
		{"error_update_checking", u8"最新バージョンの有無の確認に失敗しました"},
		{"credits", u8"クレジット"},
		{"used_libraries", u8"利用したライブラリ"},
		{"download_update", u8"アップデートをダウンロードする"},
		{"downloading_update", u8"アップデートをダウンロード中"},
		{"downloaded_update_to", u8"以下のフォルダーにアップデートをダウンロードしました"},
		{"error_downloading_update", u8"アップデートのダウンロードに失敗しました"},
		{"keyboard_shortcuts", u8"キーボード操作"},
		{"keyboard_shortcuts_list", u8R"(  * O: 音声ファイルを開く
  * Z: 切断配置を置く
  * C: 切断配置を全消去、始点に切断配置を置く (woslicerIIと同様)
  * Shift+C: 切断配置を全消去 (始点の切断配置も含めて)
  * V: 切断配置をBMSEシーケンスデータとしてコーピする
  * Shift+V: 切断配置をiBMSCシーケンスデータとしてコーピする
  * B: 切断配置をクリップボードから読み込む (BMSEシーケンスデータのみ)
  * K: WAV定義情報をクリップボードに書き出す
  * Shift+K: BMSファイルにWAV定義情報をぶち込む (音声ファイルを収納しているフォルダーにBMSファイルが存在する場合のみ)
  * M: キー音を出力
  * P: ホバー中のキー音を再生してから次のキー音に移動
  * Enter: ホバー中のキー音を再生
  * [←] / [→]: カーソルの位置を移動
  * Shift + [←] / Shift + [→]: 最も近い切断配置に飛ばす
  * [↑] / [↓]: Gridの調整
  * Ctrl+O: プロジェクトを開く
  * Ctrl+S: プロジェクトを保存
  * Home: 波形の始点に移動
  * End: 波形の終点に移動
  * Space: 選択モードに切り替える
  * Ctrl+C: 切断配置をコーピ
  * Ctrl+X: 切断配置をカット
  * Ctrl+V: クリップボード内の切断配置をペースト
  * Del: 選択中の切断配置を消す
  * Ctrl+Z: 元に戻す
  * Ctrl+Y: やり直す
)"},
		{"append_keysound_list_bms", u8"BMSファイルにWAV定義情報をぶち込む"},
		{"no_suitable_bms_found", u8"適切なBMSファイルが見つかりませんでした"},
		{"appended_keysound_list_to_file", u8"以下のBMSファイルにWAV定義情報をぶち込みました"},
		{"more_than_one_bms_found_in_folder", u8"フォルダー内に複数のBMSファイルがあります"},
	}},
	{ "kr", {
		{"_lang", u8"한국어"},
		{"general", u8"일반"},
		{"zoom", u8"확대"},
		{"offset", u8"오프셋"},
		{"position", u8"위치"},
		{"bpm", u8"BPM"},
		{"snapping", u8"격자에 맞춤"},
		{"starting_key", u8"시작 키"},
		{"decimal_value", u8"10진 값"},
		{"enable_b62", u8"base-62 활성화"},
		{"zerocross_markers", u8"영점 교차 마커"},
		{"file", u8"파일"},
		{"open_project", u8"열기"},
		{"save_project", u8"저장"},
		{"import_audio_file", u8"오디오 가져오기"},
		{"export_keysounds", u8"키음으로 내보내기"},
		{"quit", u8"종료"},
		{"edit", u8"수정"},
		{"import_midi", u8"MIDI에서 조각 가져오기"},
		{"import_mid2bms", u8"Mid2BMS renamer 파일 가져오기"},
		{"export_bmse", u8"BMSE 클립보드 데이터 복사"},
		{"export_ibmsc", u8"iBMSC 클립보드 데이터 복사"},
		{"import_bmse", u8"BMSE 클립보드 데이터 붙여넣기"},
		{"export_keysound_list", u8"키음 목록 클립보드에 복사"},
		{"clear_all_markers", u8"마커 초기화"},
		{"clear_all_markers_with_0", u8"마커 초기화 (0 포함)"},
		{"preferences", u8"설정"},
		{"export_settings", u8"설정 내보내기"},
		{"disabled", u8"사용 불가"},
		{"noise_gate", u8"노이즈 게이트"},
		{"fadeout", u8"페이드아웃"},
		{"process", u8"처리"},
		{"waveform", u8"파형"},
		{"markers", u8"마커"},
		{"id", u8"ID"},
		{"name", u8"이름"},
		{"no_markers_set", u8"마커 없음..."},
		{"select_midi_track", u8"MIDI 트랙을 고르세요"},
		{"select_track_import", u8"가져올 트랙을 선택하세요"},
		{"all_tracks", u8"모든 트랙"},
		{"clear_existing_markers", u8"현재 마커 초기화"},
		{"import", u8"가져오기"},
		{"cancel", u8"취소"},
		{"autodetect_starting_keysound", u8"첫 키음 자동 감지"},
		{"language", u8"언어"},
		{"save", u8"저장"},
		{"opened_file", u8"열린 파일"},
		{"selected_file_not_supported", u8"미지원 파일입니다!"},
		{"exported_keysounds_to_folder", u8"아래 폴더에 키음을 추출했습니다:"},
		{"success", u8"성공"},
		{"warning", u8"경고"},
		{"error", u8"에러"},
		{"select", u8"선택"},
		{"file_doesnt_exist", u8"파일이 존재하지 않습니다!"},
		{"load_file_first", u8"오디오 파일을 로드해 주세요!"},
		{"moved_markers_zerocrossing", u8"마커를 영점 교차 시점으로 이동했습니다!"},
		{"imported_markers_clipboard", u8"마커를 클립보드로 가져왔습니다!"},
		{"clipboard_no_bmse_data", u8"클립보드에 BMSE 데이터가 없습니다!"},
		{"copied_markers_bmse_data", u8"마커를 BMSE 클립보드 데이터로 복사했습니다!"},
		{"copied_markers_ibmsc_data", u8"마커를 iBMSC 클립보드 데이터로 복사했습니다!"},
		{"select_region_first", u8"영역을 먼저 설정해 주세요!"},
		{"selected_region_no_markers", u8"선택 영역에 마커가 없습니다!"},
		{"clipboard_empty", u8"클립보드가 비었습니다!"},
		{"cut_markers", u8"마커를 {count}개 자릅니다!"},
		{"cut_markers_plural", u8"마커를 {count}개 자릅니다!"},
		{"copied_markers", u8"마커 {count}개를 복사했습니다!"},
		{"copied_markers_plural", u8"마커 {count}개를 복사했습니다!"},
		{"pasted_markers", u8"마커 {count}개를 붙여넣었습니다!"},
		{"pasted_markers_plural", u8"마커 {count}개를 붙여넣었습니다!"},
		{"deleted_markers", u8"마커 {count}개를 지웠습니다!"},
		{"deleted_markers_plural", u8"마커 {count}개를 지웠습니다!"},
		{"imported_midi_markers", u8"MIDI 파일로부터 마커를 가져왔습니다!"},
		{"saved_project_to", u8"프로젝트를 다음 경로에 저장함: "},
		{"opened_project", u8"열린 프로젝트"},
		{"copied_keysound_list_clipboard", u8"키음 목록을 클립보드에 복사했습니다!"},
		{"marker_names_import_success", u8"마커 이름을 가져왔습니다!"},
		{"marker_names_import_warning", u8"마커 이름을 가져왔지만 마커와 이름의 갯수가 다릅니다."},
		{"close", u8"닫기"},
		{"about", u8"크레딧..."},
		{"help", u8"도움말"},
		{"help_menu", u8"?"},
		{"info", u8"정보"},
		{"check_for_updates", u8"업데이트 체크"},
		{"check_for_updates_startup", u8"시작할 때 업데이트 체크"},
		{"latest_version", u8"최신 버전입니다!"},
		{"update_available", u8"업데이트가 있습니다!"},
		{"error_update_checking", u8"업데이트 체크에 문제가 있습니다"},
		{"credits", u8"크레딧"},
		{"used_libraries", u8"사용한 라이브러리"},
		{"download_update", u8"업데이트 받기"},
		{"downloading_update", u8"업데이트 받는 중"},
		{"downloaded_update_to", u8"업데이트를 다음 경로에 받았습니다"},
		{"error_downloading_update", u8"업데이트 다운로드 에러 발생"},
		{"keyboard_shortcuts", u8"키보드 단축키"},
		{"keyboard_shortcuts_list", u8R"(  * O: 오디오 파일 열기
  * Z: 슬라이스 마커 추가
  * C: 마커 초기화 (0에는 1개 추가. woslicerII와 같은 동작입니다)
  * Shift+C: 마커 완전 초기화 (0까지도)
  * V: BMSE 클립보드 데이터로 마커 복사
  * Shift+V: iBMSC 클립보드 데이터로 마커 복사
  * B: 클립보드에서 마커 가져오기 (BMSE 클립보드 데이터 사용)
  * K: 클립보드로 키음 목록 복사
  * Shift+K: 키음 목록 BMS에 덧붙이기 (오디오 파일과 같은 경로라면)
  * M: 키음 내보내기
  * P: 키음 미리듣기 및 다음 키음으로 이동
  * Enter: 키음 미리듣기
  * [←] / [→]: 커서 위치 이동
  * Shift + [←] / Shift + [→]: 가장 가까운 격자선으로
  * [↑] / [↓]: 격자 설정
  * Ctrl+O: 프로젝트 불러오기
  * Ctrl+S: 프로젝트 저장
  * Home: 파형의 처음으로 이동
  * End: 파형의 끝으로 이동
  * Space: 선택 모드 시작/종료
  * Ctrl+C: 선택된 마커 복사
  * Ctrl+X: 선택된 마커 잘라내기
  * Ctrl+V: 복사한 마커 붙여넣기
  * Del: 선택된 마커 지우기
  * Ctrl+Z: 실행 취소
  * Ctrl+Y: 다시 실행
)"},
		{"append_keysound_list_bms", u8"키음 목록 BMS에 덧붙이기"},
		{"no_suitable_bms_found", u8"적절한 BMS 파일을 찾지 못했습니다"},
		{"appended_keysound_list_to_file", u8"다음 파일에 키음 목록을 덧붙였습니다"},
		{"more_than_one_bms_found_in_folder", u8"폴더의 BMS 파일 수가 하나를 넘습니다"},
	}},
};

void InitTranslations(std::string locale);
std::vector<std::string> GetLanguages();
std::vector<std::string> GetLanguagesPretty();