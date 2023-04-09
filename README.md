# Media editor template

It is a SFML-based general purpose editor (viewer) template.

<img src="readme_images/screenshot_00.png">

## Features:

### Key bindings:
```
m_window->addKeyDownEvent(sf::Keyboard::O, ModifierKey::Control, ...some_function... );
```
### Single time any key down event
```
// export file example
m_window->addKeyDownEvent(sf::Keyboard::E, ModifierKey::Control | ModifierKey::Shift,
    [this]() {
        m_window->setAnyKeyReason("export file type");            // call here
        m_window->setTitle("Export shader as: G-glsl, H-hlsl");
    });
    
// receive here, this function will be called on any key single time after "setAnyKeyReason"
m_window->setAnyKeyDownOnceEvent("export file type",
    [this](KeyWithModifier key) {
        std::string shaderText;
        ShaderType shaderExportType;
        switch (key.key) {
        case sf::Keyboard::G: shaderExportType = UVBSP::ShaderType::GLSL; break;
        case sf::Keyboard::H: shaderExportType = UVBSP::ShaderType::HLSL; break;
        default: { 
            m_window->setTitle("Invalid export letter, press 'G' or 'H' next time.");
            return; }
        }
        // for example: do export stuff
    });
```
### Mouse event bindings:
```
m_window->setMouseDragEvent(sf::Mouse::Middle,
    [this](ivec2 startPos, ivec2 currentPos, ivec2 currentDelta, DragState dragState) {
        vec2 textureSize = toFloat(m_texture.getSize());
        vec2 uvCurrentDelta = m_window->mapPixelToCoords(currentDelta) / textureSize;
        vec2 uvStartPos = m_window->mapPixelToCoords(startPos) / textureSize;
        vec2 uvCurrentPos = m_window->mapPixelToCoords(currentPos) / textureSize;
        vec2 uvCurrentDir = normalized(uvCurrentPos - uvStartPos);

        if (dragState == DragState::StartDrag) {
          // when start dragging
        } else if (dragState == DragState::ContinueDrag) {
          // when continue dragging
        }
        // do something on MMB drag
    });

m_window->setMouseDownEvent(sf::Mouse::Left, [this](ivec2 pos, bool mouseDown) {
        if (mouseDown) { /* on mouse down */  }  else { /* on mouse up */ }
        // maybe it is better to separate these events
    });
```
### Read/write file dialog templates

You can create image reader/writer template.
It saves functions, extensions, current folder and last open filename.
```
// let's create a "Primary" template
// read(write)functions: bool(const std::filesystem::path& fullFilePath) (return success)
addFileInteractionInfo("Primary", "png,jpg", readFunction, writeFunction);

// open file
m_window->addKeyDownEvent(sf::Keyboard::O, ModifierKey::Control,
    std::bind(&Application::openFileDialog, this, "Primary"));

// save file
m_window->addKeyDownEvent(sf::Keyboard::S, ModifierKey::Control,
    std::bind(&Application::saveFileOptionalDialog, this, "Primary", false));

// save file as
m_window->addKeyDownEvent(sf::Keyboard::S, ModifierKey::Control | ModifierKey::Shift,
    std::bind(&Application::saveFileOptionalDialog, this, "Primary", true)); 
// last 'true' is force open dialog window

// you can add additional file import

addFileInteractionInfo("VectorFormatImport", "svg", 
    anotherReadFunction, anotherWriteFunction);

m_window->addKeyDownEvent(sf::Keyboard::I, ModifierKey::Control,
    std::bind(&Application::openFileDialog, this, "VectorFormatImport"));
```

## How to add it to your project
For CMake noobs like me.

This repo is your child project, the parent project is your "main".
CMake in your root project:
```
set(MET "third_party/media-editor-template")   # something like set variable, to get it write ${MET}
add_subdirectory(${MET}) # just do it!

set(SOURCE_FILES main.cpp)        # the "main" of your project
add_executable(${PROJECT_NAME} ${SOURCE_FILES})

include_directories(${MET}/src)         # to find includes do this
target_link_libraries(${PROJECT_NAME} media-editor-template)  # undefined reference without this
```

Trust me, I brute-forced a lot of words in CMake!

Your main:
```
#include "application.h"

int main()
{
    Application app;
    app.init(); 
    // override key bindings and creating save templates here
    
    app.mainLoop();
    // override drawContext() to draw your stuff

    return 0;
}

```
Override Application's functions and be happy!

## Feel free to use and/or make it better!

# Dependencies:
- SFML
