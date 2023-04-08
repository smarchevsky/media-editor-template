#include "imgui_filesystem.h"
#include "imgui/imgui.h"
#include "imgui/imgui_stdlib.h"

#include <iostream>
#include <sstream>
namespace ImguiUtils {

#ifndef LOG
#define LOG(x) std::cout << x << std::endl
#endif

static constexpr FileVisualColor s_folderColor = -1;
static constexpr FileVisualColor s_unsupportedFileColor = IM_COL32(200, 200, 200, 255);
static FileVisualColor getSupportedFileColorByIndex(int index)
{
    static constexpr int l = 127, h = 255; // low, high intensity color component
    static constexpr FileVisualColor s_supportedFileColor[] {
        IM_COL32(h, h, l, h),
        IM_COL32(l, h, l, h),
        IM_COL32(l, h, h, h),
        IM_COL32(l, l, h, h),
        IM_COL32(h, l, h, h),
        IM_COL32(h, l, l, h),
        IM_COL32(h, h, l, h),
        IM_COL32(l, h, l, h),
    };
    static constexpr int arraySize = sizeof(s_supportedFileColor) / sizeof(FileVisualColor);
    return s_supportedFileColor[index % arraySize];
}
static const ImVec2 getButtonSize() { return { ImGui::GetFontSize() * 7.0f, 0.0f }; };

template <class T> // make hex string, because I can (copy from StackOverflow)
static std::string toStringCustom(T t, std::ios_base& (*f)(std::ios_base&))
{
    std::ostringstream oss;
    oss << f << t;
    return oss.str();
}

auto splitStringByComma(std::string singleLineExtensions)
{
    std::string tmp;

    for (const char symbol : singleLineExtensions)
        if ((symbol > 'a' && symbol < 'z') || (symbol > 'A' && symbol < 'Z')
            || (symbol > '0' && symbol < '9') || symbol == ',') {
            tmp += symbol;
        }

    singleLineExtensions = std::move(tmp);

    std::stringstream singleLineExtensionsStream(singleLineExtensions);

    std::set<std::string> resultSet; // avoid copies
    while (singleLineExtensionsStream.good()) {
        std::string substr;
        std::getline(singleLineExtensionsStream, substr, ',');
        if (substr.size() > 0)
            resultSet.insert('.' + substr);
    }

    for (const auto& e : resultSet) {
        std::cout << e << "  ";
    }

    std::cout << std::endl;
    return resultSet;
}

FileSystemNavigator::FileSystemNavigator(
    const std::string& name,
    const fs::path& path,
    const std::string& extensionsSingleLine,
    const FileInteractionFunction& function)

    : m_strThisPtrHash(toStringCustom((size_t)this, std::hex))
    , m_strImGuiWidgetName(name + "###" + m_strThisPtrHash)
    , m_strImGuiExtSensitiveCheckboxName("Extension sensitive###" + m_strThisPtrHash)
    , m_strImGuiFileListBoxName("###FileList" + m_strThisPtrHash)
    , m_strImGuiTextBoxName("###InputTextBlock" + m_strThisPtrHash)
    , m_strImGuiOverwriteMsg(std::string("Overwrite"))
    , m_currentDir(path)
    , m_supportedExtensions(splitStringByComma(extensionsSingleLine))
    , m_fileActionFunction(function)
{
    assert(!!function && "function must exist");
    assert(m_supportedExtensions.size() && "No extensions");
    retrievePathList(m_currentDir);
}

void FileSystemNavigator::retrievePathList(const fs::path& newPath)
{
    fs::directory_entry newEntry(newPath);
    if (newEntry.exists() && newEntry.is_directory()) {

        decltype(m_allEntryList) newEntryList;

        fs::path parentPath(newPath.parent_path());
        fs::directory_entry parentEntry(parentPath);

        newEntryList.push_back(parentEntry);

        for (const fs::directory_entry& entry : fs::directory_iterator(newEntry,
                 fs::directory_options::skip_permission_denied)) {
            newEntryList.push_back(entry);
        }

        m_allEntryList = std::move(newEntryList);

        m_currentDir = newEntry.path();
        m_strImGuiCurrentPath = m_currentDir;

        m_strSelectedFilename.clear();

        m_bMustFocusListBox = true;
        m_iSelectedItemIndex = 0;
    }

    m_bVisibleEntryListDirty = true;
    static int retrievePathListCounter = 0;
    LOG("Path retrieved num: " << ++retrievePathListCounter);
}

void FileSystemNavigator::updateVisibleEntryListInternal()
{
    m_visibleEntryListInfo.clear();
    std::vector<fs::directory_entry> m_folderEntries;
    std::vector<fs::directory_entry> m_fileEntries;

    for (const auto& e : m_allEntryList) {
        if (e.is_directory())
            m_folderEntries.push_back(e);
        else if (e.is_regular_file())
            m_fileEntries.push_back(e);
    }

    for (int i = 0; i < m_folderEntries.size(); ++i) {
        const auto& e = m_folderEntries[i];
        std::string fileNameStr(e.path().filename());
        m_visibleEntryListInfo.push_back(EntryListInfo(e, fileNameStr, s_folderColor, i == 0));
    }

    for (const auto& e : m_fileEntries) {
        FileVisualColor fileColor = s_unsupportedFileColor;

        const fs::path& path = e.path();

        int supportedExtensionIndex = getSupportingExtensionIndex(path.extension());
        bool isSupportingExtension = supportedExtensionIndex != -1;

        if (isSupportingExtension)
            fileColor = getSupportedFileColorByIndex(supportedExtensionIndex);

        if (isSupportingExtension || !m_bFilterSupportedExtensions) {
            m_visibleEntryListInfo.push_back(EntryListInfo(e, std::string(path.filename()), fileColor));
        }
    }

    static int refreshPathListCounter = 0;
    LOG("List refreshed: " << ++refreshPathListCounter);
    m_bVisibleEntryListDirty = false;
}

int FileSystemNavigator::checkFileWithThisNameAlreadyExists_GetIndex()
{
    m_bFileWithThisNameAlreadyExists = false;
    int loopIndex = 0;
    for (const auto& entryListInfo : m_visibleEntryListInfo) {
        if (entryListInfo.fileName == m_strSelectedFilename) {
            m_bFileWithThisNameAlreadyExists = true;
            return loopIndex;
        }
        loopIndex++;
    };

    return -1;
}

bool FileSystemNavigator::showInImGUI()
{
    updateVisibleEntryList();
    //////////////// SAVE DIALOG BOX ////////////////////
    ImGui::SetNextWindowSize(ImVec2(m_width, m_height));
    if (ImGui::Begin(m_strImGuiWidgetName.c_str(), &m_bIsOpenInImgui,
            ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar)) {

        bool enterPressed = ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter));
        if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape)))
            shouldClose();

        ImGui::Text("%s", m_strImGuiCurrentPath.c_str());

        if (ImGui::Checkbox("Extension sensitive", &m_bFilterSupportedExtensions)
            && m_bFilterSupportedExtensions != m_bFilterSupportedExtensionsPrev) {
            m_bFilterSupportedExtensionsPrev = m_bFilterSupportedExtensions;
            m_bVisibleEntryListDirty = true;
        }

        //////////////// LIST BOX ////////////////////
        if (ImGui::BeginListBox(m_strImGuiFileListBoxName.c_str(), ImVec2(m_width, m_height - 126))) {

            if (m_bMustFocusListBox) {
                ImGui::SetKeyboardFocusHere(0);
                m_bMustFocusListBox = false;
            }

            for (int i = 0; i < m_visibleEntryListInfo.size(); i++) {
                const bool is_selected = (m_iSelectedItemIndex == i);
                const std::string& filename = m_visibleEntryListInfo[i].ImGuiFileName;

                //////////////// SELECTABLE ITEM ////////////////////
                ImGui::PushStyleColor(ImGuiCol_Text, m_visibleEntryListInfo[i].visibleNameColor);
                if (ImGui::Selectable(filename.c_str(), is_selected)) { // on item selected
                    m_iSelectedItemIndex = i;

                    if (const auto* selectedEntryPtr = getVisibleEntryByIndex(m_iSelectedItemIndex)) {

                        if (selectedEntryPtr->entry.is_directory()) {
                            retrievePathList(selectedEntryPtr->entry.path());

                        } else if (selectedEntryPtr->entry.is_regular_file()) {
                            const fs::directory_entry& fileEntry = selectedEntryPtr->entry;
                            const fs::path& ext = fileEntry.path().extension();

                            if (isExtensionSupported(ext)) {
                                m_bFileWithThisNameAlreadyExists = true;
                                tryDoFileAction(fileEntry);

                            } else
                                showWarningMessage("Trying to overwrite unsupported file");
                        }
                    }
                }
                ImGui::PopStyleColor();

                if (ImGui::IsItemFocused())
                    m_iFocusedItemIndex = i;
            }

            // on lsit element focus changed change text in text box
            if (m_iFocusedItemIndex != m_iFocusedItemIndexPrev) {
                m_iFocusedItemIndexPrev = m_iFocusedItemIndex;
                if (const auto* e = getVisibleEntryByIndex(m_iFocusedItemIndex)) {
                    m_strSelectedFilename = e->entry.is_directory() ? "" : e->fileName;
                    m_bFileWithThisNameAlreadyExists = true;
                }
            }
            ImGui::EndListBox(); // end list
        }

        //////////////// TEXT BOX /////////////////
        ImGui::SetNextItemWidth(m_width);
        const auto& textColor = (m_bFileWithThisNameAlreadyExists == isWriter())
            ? IM_COL32(255, 50, 50, 255)
            : IM_COL32(255, 255, 255, 255);

        ImGui::PushStyleColor(ImGuiCol_Text, textColor);
        if (ImGui::InputText(m_strImGuiTextBoxName.c_str(), &m_strSelectedFilename)) { // text input
            checkFileWithThisNameAlreadyExists_GetIndex();
        }
        ImGui::PopStyleColor();

        if (enterPressed && ImGui::IsItemFocused()) { // if enter pressed in TextBox
            if (!m_strSelectedFilename.empty()) {
                fs::path fileNameAsPath(m_strSelectedFilename);

                if (isExtensionSupported(fileNameAsPath.extension())) { // extension supported?
                    checkFileWithThisNameAlreadyExists_GetIndex();

                    fs::path fileEntry(m_currentDir / fileNameAsPath);
                    tryDoFileAction(fileEntry);

                } else {
                    auto it = getSupportedExtensionByIndex(0); // append first supported extension to filename
                    if (it != m_supportedExtensions.end()) {
                        LOG("Extension appended");
                        m_strSelectedFilename += *it;
                        checkFileWithThisNameAlreadyExists_GetIndex();
                    }
                }
            }
        }

        renderOverwriteWindow();

        if (isWarningMessageExists()) {
            ImGui::SetNextWindowSize(ImVec2(ImGui::GetFontSize() * m_strImGuiWarningMessage.size() * 0.46f, 0));
            ImGui::OpenPopup(m_strImGuiWarningMessage.c_str());
            if (ImGui::BeginPopupModal(m_strImGuiWarningMessage.c_str(), nullptr, ImGuiWindowFlags_Popup)) {
                ImGui::SetKeyboardFocusHere(0);
                if (ImGui::Button("Ok", getButtonSize())) {
                    closeWarningMessage();
                }

                ImGui::EndPopup();
            }
        }

        ImGui::End();
    }

    return m_bIsOpenInImgui;
}

bool FileSystemNavigator::doFileAction(const fs::path& fileDir)
{
    bool success = false;

    if (m_fileActionFunction)
        success = m_fileActionFunction(fileDir);

    if (success)
        shouldClose();

    return success;
}
///////////// FILE READER /////////////////

void FileReader::tryDoFileAction(const fs::path& fileDir)
{
    if (m_bFileWithThisNameAlreadyExists) { // if this filename already exists
        if (fs::directory_entry(fileDir).is_regular_file())
            readFile(fileDir);
    } else {
        showWarningMessage("No such file: " + std::string(fileDir));
    }
    LOG("File must be read");
}

///////////// FILE WRITER /////////////////

void FileWriter::renderOverwriteWindow()
{
    if (m_popupOverwriteWindowInfo) {
        ImGui::OpenPopup(m_strImGuiOverwriteMsg.c_str());
        if (ImGui::BeginPopupModal(m_strImGuiOverwriteMsg.c_str(),
                nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_Popup)) {

            if (!m_popupOverwriteWasInPrevFrame)
                ImGui::SetKeyboardFocusHere(1); // focus to "No"

            bool clickedYes = ImGui::Button("Yes", getButtonSize());
            bool clickedNo = ImGui::Button("No", getButtonSize());
            if (clickedYes || clickedNo) {
                if (clickedYes) {
                    writeFile(*m_popupOverwriteWindowInfo);
                }
                ImGui::CloseCurrentPopup();
                m_popupOverwriteWindowInfo.reset();
            }

            ImGui::EndPopup();
        }
    }
    m_popupOverwriteWasInPrevFrame = !!m_popupOverwriteWindowInfo;
}

void FileWriter::tryDoFileAction(const fs::path& fileDir)
{
    if (m_bFileWithThisNameAlreadyExists) { // if this filename already exists
        if (fs::directory_entry(fileDir).is_regular_file())
            m_popupOverwriteWindowInfo.reset(new fs::path(fileDir)); // overwrite dialog
        else
            showWarningMessage("You are trying to overwrite not a regular file");
    } else {
        writeFile(fileDir);
        m_popupOverwriteWindowInfo.reset();
    }
}

} // ImguiUtils
