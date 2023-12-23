#include "shadercodeparser.h"

#include <cstring>
#include <regex>

#include "common.h"

namespace {
std::string removeComments(const std::string& code)
{
    // God bless ChatGPT !!111
    std::regex commentRegex(R"((//.*?$|/\*[^*]*\*+(?:[^/*][^*]*\*+)*/))", std::regex::multiline);
    return std::regex_replace(code, commentRegex, "");
}

void parseDefaultVariable(UniformVariant& var, const std::string& defaultVarString)
{
    enum class VarDataType : uint8_t { FLOAT,
        INT };

    if (defaultVarString.empty())
        return;

    float floatData[16] {};
    int intData[16] {};

    auto stringToFloat = [](const std::string& str, float& outFloat) {
        char* endptr;
        outFloat = strtof(str.c_str(), &endptr);
        // if (endptr != str.c_str()) { // handle invalid number
        //     return false;
        // }
        return true;
    };

    auto stringToInt = [](const std::string& str, int& outInt) {
        char* endptr;
        outInt = std::strtol(str.c_str(), &endptr, 10);
        // if (endptr != str.c_str()) {
        //     return false;
        // }
        return true;
    };

    auto convertDataIfCan = [&](const std::smatch& matches, int num, VarDataType type) {
        for (int i = 0; i < num; ++i) {
            const auto& str = matches[i + 1].str(); // first is whole expression

            switch (type) {
            case VarDataType::FLOAT: {
                stringToFloat(str, floatData[i]);
            } break;
            case VarDataType::INT: {
                stringToInt(str, intData[i]);
            } break;
            default:
                break;
            }
        }
    };

    std::smatch matches;

    // thanks to ChatGPT, I have no glue, how regex works
    switch (var.index()) {
    case GET_INDEX(float): {
        float v;
        if (stringToFloat(defaultVarString, v)) {
            var = v;
        }
        return;
    }

    case GET_INDEX(glm::vec2): {
        if (std::regex_search(defaultVarString, matches,
                std::regex("vec2\\(([^,]+),([^)]+)\\)"))) {
            convertDataIfCan(matches, 2, VarDataType::FLOAT);
            var = glm::vec2(floatData[0], floatData[1]);
        }
        return;
    }

    case GET_INDEX(glm::vec3): {
        if (std::regex_search(defaultVarString, matches,
                std::regex("vec3\\(([^,]+),\\s*([^,]+),\\s*([^)]+)\\)"))) {
            convertDataIfCan(matches, 3, VarDataType::FLOAT);
            var = glm::vec3(floatData[0], floatData[1], floatData[2]);
        }
        return;
    }

    case GET_INDEX(glm::vec4): {
        glm::vec4 result(0);
        if (std::regex_search(defaultVarString, matches,
                std::regex("vec4\\(([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*([^)]+)\\)"))) {
            convertDataIfCan(matches, 4, VarDataType::FLOAT);
            var = glm::vec4(floatData[0], floatData[1], floatData[2], floatData[3]);
        }
        return;
    }

    case GET_INDEX(glm::mat4): { // identity by default, not zero matrix
        if (std::regex_search(defaultVarString, matches,
                std::regex("mat4\\("
                           "([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*"
                           "([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*"
                           "([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*"
                           "([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*([^)]+)\\)"))) {
            convertDataIfCan(matches, 16, VarDataType::FLOAT);
            glm::mat4 result;
            std::memcpy(&result, floatData, sizeof(glm::mat4));
            var = result;
        }
        return;
    }

    case GET_INDEX(int): {
        int v;
        if (stringToInt(defaultVarString, v)) {
            var = v;
        }
        return;
    }

    case GET_INDEX(glm::ivec2): {
        if (std::regex_search(defaultVarString, matches,
                std::regex("ivec2\\(([^,]+),([^)]+)\\)"))) {
            convertDataIfCan(matches, 2, VarDataType::INT);
            var = glm::ivec2(intData[0], intData[1]);
        }
        return;
    }

    case GET_INDEX(glm::ivec3): {
        if (std::regex_search(defaultVarString, matches,
                std::regex("ivec3\\(([^,]+),\\s*([^,]+),\\s*([^)]+)\\)"))) {
            convertDataIfCan(matches, 3, VarDataType::INT);
            var = glm::ivec3(intData[0], intData[1], intData[2]);
        }
        return;
    }

    case GET_INDEX(glm::ivec4): {
        if (std::regex_search(defaultVarString, matches,
                std::regex("ivec4\\(([^,]+),\\s*([^,]+),\\s*([^,]+),\\s*([^)]+)\\)"))) {
            convertDataIfCan(matches, 4, VarDataType::INT);
            var = glm::ivec4(intData[0], intData[1], intData[2], intData[3]);
        }
        return;
    }

    case GET_INDEX(Texture2Ddata): { // do nothing
    }
    }
}
} // namespace

//
//
//

void ShaderCodeParser::parseDefaultUniforms(const std::string& shaderCode,
    std::vector<GLShader::Variable>& uniformVariables)
{
    // std::string removeCommentsCode = removeComments(shaderCode);

    std::regex pattern(R"(uniform\s+(\w+)\s+(\w+)\s*=\s*(.+);)");

    std::unordered_map<std::string, int> uniformNamesSet; // used to stop further code parsing, if all uniforms found
    for (int varIndex = 0; varIndex < uniformVariables.size(); ++varIndex) {
        const auto& v = uniformVariables[varIndex];
        uniformNamesSet.emplace(v.m_name, varIndex);
    }

    auto regex_iterator = std::sregex_iterator(shaderCode.begin(), shaderCode.end(), pattern);
    auto end_iterator = std::sregex_iterator();

    for (std::sregex_iterator iter = regex_iterator; iter != end_iterator; ++iter) {
        // std::string dataType = (*iter)[1].str();
        const std::string& variableName = (*iter)[2].str();
        const std::string& variableValue = (*iter)[3].str();

        auto foundVarIter = uniformNamesSet.find(variableName);
        int varIndex = foundVarIter->second;
        parseDefaultVariable(uniformVariables[varIndex].m_defaultData, variableValue);
        // variableMap[variableName] = variableValue;

        uniformNamesSet.erase(foundVarIter);
        if (uniformNamesSet.empty()) // all variables seems found in shader
            break;
    }
}
