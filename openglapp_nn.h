#ifndef OPENGLAPP_NN_H
#define OPENGLAPP_NN_H

#include "application.h"

#include "graphics/gl_shader.h"
#include "graphics/visualobject.h"

#include "graphics/gl_rendermanager.h"

#include <fstream>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/random.hpp>
#include <glm/gtx/string_cast.hpp>

namespace fs = std::filesystem;
static fs::path projectDir(PROJECT_DIR);
static fs::path resourceDir(RESOURCE_DIR);

// namespace {
// auto reverseInt(int i)
// {
//     unsigned char c1, c2, c3, c4;
//     c1 = i & 255, c2 = (i >> 8) & 255, c3 = (i >> 16) & 255, c4 = (i >> 24) & 255;
//     return ((int)c1 << 24) + ((int)c2 << 16) + ((int)c3 << 8) + c4;
// };

// // slightly changed https://stackoverflow.com/a/33384846/9269700
// std::vector<uint8_t> read_mnist_images(std::string full_path, int& number_of_images, int& image_size)
// {
//     std::ifstream file(full_path, std::ios::binary);

//     if (file.is_open()) {
//         int magic_number = 0, n_rows = 0, n_cols = 0;

//         file.read((char*)&magic_number, sizeof(magic_number));
//         magic_number = reverseInt(magic_number);

//         if (magic_number != 2051)
//             throw std::runtime_error("Invalid MNIST image file!");

//         int number_of_images, image_size;
//         file.read((char*)&number_of_images, sizeof(number_of_images)), number_of_images = reverseInt(number_of_images);
//         file.read((char*)&n_rows, sizeof(n_rows)), n_rows = reverseInt(n_rows);
//         file.read((char*)&n_cols, sizeof(n_cols)), n_cols = reverseInt(n_cols);

//         image_size = n_rows * n_cols;

//         // std::vector<std::vector<uint8_t>> _dataset(number_of_images);
//         // for (int i = 0; i < number_of_images; i++) {
//         //     _dataset[i].resize(image_size);
//         //     file.read((char*)_dataset[i].data(), image_size);
//         // }

//         std::vector<uint8_t> _dataset(number_of_images * image_size);
//         for (int i = 0; i < _dataset.size(); i++) {
//             file.read((char*)_dataset.data(), _dataset.size());
//         }

//         return std::move(_dataset);
//     } else {
//         throw std::runtime_error("Cannot open file `" + full_path + "`!");
//     }
// }
// } // namespace
/////////////////////////////

// I'll delete this mess later

// void writeFloatArray(const std::string& filename,
//     const std::vector<float>& arr1, const std::vector<float>& arr2)
// {
//     std::ofstream file(filename, std::ios::binary);
//     if (!file.is_open()) {
//         std::cerr << "Error opening file for writing: " << filename << std::endl;
//         return;
//     }
//     file.write(reinterpret_cast<const char*>(arr1.data()), arr1.size() * sizeof(float));
//     file.write(reinterpret_cast<const char*>(arr2.data()), arr2.size() * sizeof(float));
//     file.close();
// }

// void readFloatArray(const std::string& filename,
//     std::vector<float>& arr1, std::vector<float>& arr2)
// {
//     std::ifstream file(filename, std::ios::binary);
//     if (!file.is_open()) {
//         std::cerr << "Error opening file for reading: " << filename << std::endl;
//         return;
//     }
//     file.read(reinterpret_cast<char*>(arr1.data()), arr1.size() * sizeof(float));
//     file.read(reinterpret_cast<char*>(arr2.data()), arr2.size() * sizeof(float));
//     file.close();
// }

void load_model(std::vector<float>& w1, std::vector<float>& w2)
{

    const int n1 = 784; //, without bias neuron
    const int n2 = 128;
    const int n3 = 10; // Ten classes: 0 - 9
    w1.resize(n1 * n2);
    w2.resize(n2 * n3);

    // https://github.com/HyTruongSon/Neural-Network-MNIST-CPP/blob/master/model-neural-network.dat
    assert(false && "use dataset above");
    std::ifstream file("../Downloaded/Neural-Network-MNIST-CPP/model-neural-network.dat",
        std::ios::in);
    for (int i = 0; i < w1.size(); ++i)
        file >> w1[i];
    for (int i = 0; i < w2.size(); ++i)
        file >> w2[i];
    file.close();

    // Image::writeFloatImageAsPNG_RGBA8(resourceDir / "weights1.png", glm::ivec2(n2, n1), w1.data());
    // Image::writeFloatImageAsPNG_RGBA8(resourceDir / "weights2.png", glm::ivec2(n3, n2), w2.data());

    // glm::ivec2 size;
    // Image::readFloatImageFromPNG_RGBA8(resourceDir / "weights1.png", size, w1);
    // Image::readFloatImageFromPNG_RGBA8(resourceDir / "weights2.png", size, w2);
    // writeFloatArray(resourceDir / "weights.dat", w1, w2);

    // std::vector<float> w1_check(w1.size());
    // std::vector<float> w2_check(w2.size());
    // readFloatArray(resourceDir / "weights.dat", w1, w2);
}

static const std::string shaderCodeNN_VS = R"(
#version 330 core
layout(location = 0) in vec3 position;
void main() { gl_Position = vec4(position, 1.0); }
)";

static const std::string shaderCodeNN_FS = R"(
#version 330 core

uniform sampler2D inputLayer;
uniform sampler2D weightLayer;
out float OutputLayer;

float sigmoid(float x) { return 1.0 / (1.0 + exp(-x)); }
float getInput(int x) { return texelFetch(inputLayer, ivec2(x, 0), 0).r; }
float getWeight(ivec2 pos) { return texelFetch(weightLayer, pos, 0).r; }

void main()
{
    int inputWidth = textureSize(inputLayer, 0).x;

    if (inputWidth != textureSize(weightLayer, 0).y) { // check inconsistency
        OutputLayer = 1e38 * 1e38;
        return;
    }

    int currentPosX = int(gl_FragCoord.x);

    float acc = 0;
    for (int pixIndex = 0; pixIndex < inputWidth; ++pixIndex)
        acc += getInput(pixIndex) * getWeight(ivec2(currentPosX, pixIndex));

    OutputLayer = sigmoid(acc);
}
)";

static const std::string shaderCode2D_Default_VS = textFromFile(shaderDir / "default2d.vert");
static const std::string shaderCodeBrush_FS = R"(
#version 330 core

in vec2 uv;
uniform vec4 color;
out vec4 FragColor;

void main() {
    vec4 col = color;
    float brushShape = clamp(1. - length(abs(uv * 2. - 1)), 0., 1.);
    col.a *= brushShape * brushShape;
    FragColor = col;
}
)";

constexpr glm::ivec2 ImageSize(28);

class OpenGLApp_NN : public Application {
    GLShader m_shaderDefault2d = GLShader::FromFile("default2d.vert", "default2d.frag");
    GLShader m_shader_NN_forward = GLShader(shaderCodeNN_VS, shaderCodeNN_FS);
    GLShader m_shader_brush = GLShader(shaderCode2D_Default_VS, shaderCodeBrush_FS);

    // I got weights from here: https://github.com/HyTruongSon/Neural-Network-MNIST-CPP/tree/master
    std::shared_ptr<GLTexture2D> m_weightLayer0, m_weightLayer1;
    // m_inputTexture, m_inputTextureForSprite

    // VisualObjectSprite2D m_weightSprite0, m_weightSprite1;
    VisualObjectSprite2D m_neuronOutSprite0, m_neuronOutSprite1, m_neuronOutSprite2;
    VisualObjectSprite2D m_spriteWithImage, m_spriteBrush;
    std::vector<float> m_inputPixelData, m_outputPixelData;

    std::shared_ptr<GLTexture2D> m_neuronTexture0;
    GLFrameBuffer m_inputLayerBuffer, m_neuronBuffer1, m_neuronBuffer2;
    CameraOrtho m_cameraView;

    int m_frameCountIndex = 0;

public:
    void init() override
    {
        m_cameraView.setViewportSize(m_window.getSize());

        ///////////////////////////////////////////////////
        m_window.setMouseScrollEvent( // zoom on scroll
            [this](float diff, glm::ivec2 mousePos) {
                float scaleFactor = pow(1.1f, -diff);
                m_cameraView.multiplyScaleOffseted(scaleFactor, mousePos);
            });

        m_window.setMouseDragEvent(MouseButton::Middle, // drag on MMB
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                m_cameraView.addOffset_View(glm::vec2(-currentDelta));
            });

        m_window.setScreenResizeEvent( // window resize
            [this](glm::ivec2 oldSize, glm::ivec2 newSize) {
                m_cameraView.setViewportSize(glm::vec2(newSize));
            });

        // draw white on LMB down
        m_window.setMouseDownEvent(MouseButton::Left, [this](glm::ivec2 mousePos) {
            m_spriteBrush.setUniform("color", glm::vec4(1, 0, 0, .2));
            draw();
        });

        // draw black on RMB down
        m_window.setMouseDownEvent(MouseButton::Right, [this](glm::ivec2 mousePos) {
            m_spriteBrush.setUniform("color", glm::vec4(0, 0, 0, 2));
            draw();
        });

        // draw white on LMB move
        m_window.setMouseDragEvent(MouseButton::Left,
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                m_spriteBrush.setUniform("color", glm::vec4(1, 0, 0, .2));
                draw();
            });

        // draw black on RMB move
        m_window.setMouseDragEvent(MouseButton::Right,
            [this](glm::ivec2 startPos, glm::ivec2 currentPos, glm::ivec2 currentDelta, DragState dragState) {
                m_spriteBrush.setUniform("color", glm::vec4(0, 0, 0, 2));
                draw();
            });

        // move brush on mouse move
        m_window.setMouseMoveEvent(MouseButton::Left,
            [this](glm::ivec2 currentScreenPos, glm::ivec2 delta) {
                glm::vec2 normalizedScreenPos = m_window.toNormalizedPos(currentScreenPos, true);
                glm::vec2 spritePosWorld = glm::inverse(m_cameraView.getViewMatrix()) * glm::vec4(normalizedScreenPos, 0.f, 1.f);
                float halfImageSize = ImageSize.x / 2.f;
                spritePosWorld = glm::floor(spritePosWorld * halfImageSize) / halfImageSize;
                m_spriteBrush.setPos(spritePosWorld);
            });

        m_window.addKeyDownEvent(SDLK_r, KMOD_NONE, [this]() {
            for (auto& p : m_inputPixelData)
                p = 0.f;
            m_inputLayerBuffer.getTexture()->updateData(m_inputPixelData.data());
            m_neuronTexture0->updateData(m_inputPixelData.data());

            updateNN();
        });

        m_window.setClearColor({ .13f, .14f, .15f, 1.f });
        float twoDivImageSize = 2.f / ImageSize.x;
        m_spriteBrush.setRectSize(glm::vec2(-twoDivImageSize), glm::vec2(2 * twoDivImageSize)); // set brush 1 px size
        m_inputLayerBuffer.create(ImageSize, TexelFormat::R_32F);
        // std::vector<int> topology { 784, 128, 10 };

        const float visualSpriteScale = 1 / 20.f;

        std::vector<float> w1, w2;
        load_model(w1, w2);
        assert(w1.size() == 128 * 784);
        assert(w2.size() == 10 * 128);
        m_weightLayer0 = std::make_shared<GLTexture2D>(glm::ivec2(128, 784), TexelFormat::R_32F, w1.data());
        m_weightLayer1 = std::make_shared<GLTexture2D>(glm::ivec2(10, 128), TexelFormat::R_32F, w2.data());

        // auto dataSet = read_mnist_images("/home/staseg/Downloads/train-images-idx3-ubyte/train-images.idx3-ubyte", number_of_images, image_size);

        m_neuronTexture0 = std::make_shared<GLTexture2D>(glm::ivec2(784, 1), TexelFormat::R_32F, nullptr);
        m_neuronBuffer1.create(glm::ivec2(128, 1), TexelFormat::R_32F);
        m_neuronBuffer2.create(glm::ivec2(10, 1), TexelFormat::R_32F);
        m_outputPixelData.resize(10);

        m_spriteWithImage.setUniform("texture0", m_inputLayerBuffer.getTexture());

        m_neuronOutSprite1.setUniform("inputLayer", m_neuronTexture0);
        m_neuronOutSprite2.setUniform("inputLayer", m_neuronBuffer1.getTexture());

        m_neuronOutSprite1.setUniform("weightLayer", m_weightLayer0);
        m_neuronOutSprite2.setUniform("weightLayer", m_weightLayer1);

        m_neuronOutSprite0.setUniform("texture0", m_neuronTexture0);
        m_neuronOutSprite1.setUniform("texture0", m_neuronBuffer1.getTexture()); // to visualize
        m_neuronOutSprite2.setUniform("texture0", m_neuronBuffer2.getTexture());

        const glm::vec2 spriteSize(3, .2);
        m_neuronOutSprite0.setRectSize(-spriteSize, spriteSize);
        m_neuronOutSprite1.setRectSize(-spriteSize, spriteSize);
        m_neuronOutSprite2.setRectSize(-spriteSize, spriteSize);

        m_neuronOutSprite0.setPos({ 0.f, -2.5f });
        m_neuronOutSprite1.setPos({ 0.f, -2.0f });
        m_neuronOutSprite2.setPos({ 0.f, -1.5f });

        updateNN();
    }

    void updateNN()
    {
        GLRenderManager::draw(&m_neuronBuffer1, &m_shader_NN_forward, nullptr, &m_neuronOutSprite1);
        GLRenderManager::draw(&m_neuronBuffer2, &m_shader_NN_forward, nullptr, &m_neuronOutSprite2);
        m_neuronBuffer2.getDataTmp(m_outputPixelData);
    }

    void draw()
    {
        GLRenderParameters paramWithAlpha { GLBlend::OneMinusAlpha };
        GLRenderManager::draw(&m_inputLayerBuffer, &m_shader_brush, 0, &m_spriteBrush, paramWithAlpha);
        m_inputLayerBuffer.getDataTmp(m_inputPixelData);
        m_neuronTexture0->updateData(m_inputPixelData.data());

        updateNN();
    }

    void updateWindow(float dt) override
    {
        m_window.clear();
        // GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_weightSprite0);
        // GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_weightSprite1);

        GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_neuronOutSprite0);
        GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_neuronOutSprite1);
        GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_neuronOutSprite2);

        GLRenderManager::draw(&m_window, &m_shaderDefault2d, &m_cameraView, &m_spriteWithImage);

        GLRenderParameters paramWithAlpha { GLBlend::OneMinusAlpha };
        GLRenderManager::draw(&m_window, &m_shader_brush, &m_cameraView, &m_spriteBrush, paramWithAlpha);

        int looksLikeNumberIndex = -1;
        float current = 0.f;

        if (m_frameCountIndex % 10 == 0) {
            for (int i = 0; i < m_outputPixelData.size(); ++i) {
                if (m_outputPixelData[i] > 0.2f && m_outputPixelData[i] > current) {
                    current = m_outputPixelData[i];
                    looksLikeNumberIndex = i;
                }
            }

            m_window.setTitle("Looks like "
                + (looksLikeNumberIndex == -1 ? std::string("shit") : std::to_string(looksLikeNumberIndex)));
        }
        m_frameCountIndex++;
    }
};
typedef OpenGLApp_NN App;
#endif // OPENGLAPP_NN_H
