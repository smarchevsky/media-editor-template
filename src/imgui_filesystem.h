#ifndef IMGUI_UTILITES_H
#define IMGUI_UTILITES_H

#include <filesystem>
#include <functional>

#include <set>
#include <unordered_set>
#include <vector>

namespace ImguiUtils {

namespace fs = std::filesystem;

typedef uint32_t FileVisualColor; // uint8: A G R B

///////////// FILE SYSTEM NAVIGATOR ///////////////

class FileSystemNavigator {
protected: // const, structs, typedefs
    static constexpr int s_inputTextBoxSize = 256;

    struct EntryListInfo {
        EntryListInfo(const fs::directory_entry& entry,
            const std::string& name,
            FileVisualColor color, bool forceParentFolder = false)
            : entry(entry)
            , fileName(name)
            , ImGuiFileName(forceParentFolder ? ".." : "  " + name)
            , visibleNameColor(color)
        {
        }
        const fs::directory_entry entry;
        const std::string fileName;
        const std::string ImGuiFileName;
        FileVisualColor visibleNameColor;
    };

public:
    typedef std::function<bool(const fs::path&)> FileInteractionFunction;

protected: // data
    const std::string m_strThisPtrHash;
    const std::string m_strImGuiWidgetName;
    const std::string m_strImGuiExtSensitiveCheckboxName;
    const std::string m_strImGuiFileListBoxName;
    const std::string m_strImGuiTextBoxName;
    const std::string m_strImGuiOverwriteMsg;
    std::string m_strImGuiWarningMessage;
    std::string m_strImGuiCurrentPath;

    const FileInteractionFunction m_fileActionFunction;
    std::set<std::string> m_supportedExtensions; // supported file extensions

    fs::path m_currentDir;

    std::vector<fs::directory_entry> m_allEntryList;
    std::vector<EntryListInfo> m_visibleEntryListInfo;

    std::string m_strSelectedFilename;

    int m_iSelectedItemIndex = 0;

    int m_iFocusedItemIndex = 0;
    int m_iFocusedItemIndexPrev = 0;

    bool m_bIsOpenInImgui = true;
    bool m_bMustFocusListBox = true;

    bool m_bVisibleEntryListDirty = true;

    bool m_bFilterSupportedExtensions = false;
    bool m_bFilterSupportedExtensionsPrev = false;

    bool m_bFileWithThisNameAlreadyExists = false;

    const uint16_t m_width = 400, m_height = 500;

public:
    // extensions as single string, separated by comma, e.g. "jpg,png,svg,abcde"
    FileSystemNavigator(
        const std::string& name,
        const fs::path& path,
        const std::string& extensions,
        const FileInteractionFunction& function);

    fs::path getCurrentDir() { return m_currentDir; }
    void showWarningMessage(const std::string& msg) { m_strImGuiWarningMessage = msg; }
    void closeWarningMessage() { m_strImGuiWarningMessage.clear(); }
    bool isWarningMessageExists() { return !m_strImGuiWarningMessage.empty(); }

    void shouldClose() { m_bIsOpenInImgui = false; }

    bool isParent(int selectedElementIndex) const { return selectedElementIndex == 0; }
    bool showInImGUI();

protected:
    void retrievePathList(const fs::path& newPath);

    void updateVisibleEntryListInternal();

    inline void updateVisibleEntryList()
    {
        if (m_bVisibleEntryListDirty)
            updateVisibleEntryListInternal();
        m_bVisibleEntryListDirty = false;
    }

    virtual void renderOverwriteWindow() = 0;
    virtual void tryDoFileAction(const fs::path& fileInteractionInfo) = 0;
    virtual bool isWriter() = 0;

    int checkFileWithThisNameAlreadyExists_GetIndex();

    const EntryListInfo* getVisibleEntryByIndex(int index) const
    {
        return (index >= 0 && index < m_visibleEntryListInfo.size())
            ? &m_visibleEntryListInfo[index]
            : nullptr;
    }

    bool isExtensionSupported(const fs::path& ext) const
    {
        return m_supportedExtensions.find(ext) != m_supportedExtensions.end();
    }

    std::set<std::string>::const_iterator getSupportedExtensionByIndex(int requiredIndex)
    {
        int index = 0;
        for (auto it = m_supportedExtensions.begin(); it != m_supportedExtensions.end(); ++it) {
            if (requiredIndex == index)
                return it;
            index++;
        }
        return m_supportedExtensions.end();
    }

    int getSupportingExtensionIndex(const fs::path& ext) const
    {
        int index = 0;
        for (const auto& e : m_supportedExtensions) {
            if (ext == e)
                return index;
            index++;
        }
        return -1;
    }

    bool doFileAction(const fs::path& fileInteractionInfo);
};

///////////// FILE READER /////////////////

class FileReader : public FileSystemNavigator {
protected:
    virtual void renderOverwriteWindow() override { }
    virtual void tryDoFileAction(const fs::path& fileInteractionInfo) override;
    virtual bool isWriter() override { return false; };

    bool readFile(const fs::path& fileInteractionInfo)
    {
        return doFileAction(fileInteractionInfo);
    }

public:
    template <typename... Args>
    FileReader(Args&&... args)
        : FileSystemNavigator(std::forward<Args>(args)...)
    {
    }
};

///////////// FILE WRITER /////////////////

class FileWriter : public FileSystemNavigator {

    std::unique_ptr<fs::path> m_popupOverwriteWindowInfo;
    bool m_popupOverwriteWasInPrevFrame; // to focus to "No", when window appears

    virtual void renderOverwriteWindow() override;
    virtual void tryDoFileAction(const fs::path& fileInteractionInfo) override;
    virtual bool isWriter() override { return true; };

    bool writeFile(const fs::path& fileInteractionInfo)
    {
        return doFileAction(fileInteractionInfo);
    }

public:
    template <typename... Args>
    FileWriter(Args&&... args)
        : FileSystemNavigator(std::forward<Args>(args)...)
    {
    }
};

//

} // ImguiUtils
#endif // IMGUI_UTILITES_H
