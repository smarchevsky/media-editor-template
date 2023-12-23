#ifndef SHADERCODEPARSER_H
#define SHADERCODEPARSER_H

#include "graphics/gl_shader.h"

class ShaderCodeParser {
public:
    static void parseDefaultUniforms(const std::string& shaderCode,
        std::vector<GLShader::Variable>& uniformVariables);
};

#endif // SHADERCODEPARSER_H
