// 
// GlShaderMgr.h - Version 0.1, April 9, 2018
//
// Author: Sam Buss
//
// Software accompanying POSSIBLE SECOND EDITION TO the book
// 3D Computer Graphics : A Mathematical Introduction with OpenGL,
// by S.Buss, Cambridge University Press, 2003.
//
// Software is "as-is" and carries no warranty.It may be used without
// restriction, but if you modify it, please change the filenames to
// prevent confusion between different versions.
// Bug reports : Sam Buss, sbuss@ucsd.edu.
// Web page : http://math.ucsd.edu/~sbuss/MathCG2
//

//
// Routines for 
//   A. Reading shader source code from files.
//   B. Compiling and linking shader programs
//

#define GLEW_STATIC
#include <GL/glew.h> 
#include <GLFW/glfw3.h> 

#include "GlShaderMgr.h"

#include <fstream>
#include <string>
#include <iostream>
#include <sstream>
#include <algorithm>

// ****
// FILE INPUT
// Load shader source code from multiple files. 
// Files can contain multiple shaders.
// Each shader MUST be prefixed with a line of the form
//     #begincode <shadertype> <codeblockname>
// Ever block of code must end with a line of the form
//     #endcode
// Any line not between a #beginglsl and #endglsl is IGNORED!
// In #beginglsl, the argument <shadertype> must be one of:
//     vertexshader
//     fragmentshader
//     geometryshader
//     codeblock    (a part of a shader)
//  (Other types to be supported in the future.)
//  <codeblockname> must a unique name for the shader (or block of code).
// ****

// Names are not case sensitive
std::vector<std::string> GlShaderMgr::shaderTypeName = {
    "vertexshader", "fragmentshader", "geometryshader", "codeblock" };

std::vector<unsigned int> GlShaderMgr::openGLtypes = {
    GL_VERTEX_SHADER, GL_FRAGMENT_SHADER, GL_GEOMETRY_SHADER };

// Information about all the code blocks,
//   plus information about the individual compiled shader programs.
std::vector<GlShaderMgr::ShaderInfo> GlShaderMgr::shdrInfo;

// List of all shader program OpenGL handles.
std::vector<unsigned int> GlShaderMgr::shdrPrograms;

bool GlShaderMgr::LoadShaderSource(int numFiles, const char* filenamePtr[])
{
    bool ret = true;
    for (int i = 0; i < numFiles; i++) {
        ret = ret && LoadShaderSource(filenamePtr[i]);
    }
    return ret;
}

// Load shader source code from multiple files.
bool GlShaderMgr::LoadShaderSource(const char* filename)
{
    std::ifstream inFile;
    inFile.open(filename);
    if (inFile.fail()) {
        std::cerr << "GlShaderMgr::LoadShaderSource: Failed to open shader source file " << filename << "." << std::endl;
        return false;
    }
    int shdrIdx = -1;
    int beforeCount = shdrInfo.size();
    int lineNumber = 1;
    for (std::string inLine; std::getline(inFile, inLine); lineNumber++) {
        std::stringstream instream(inLine);
        std::string w1, w2, w3;
        instream >> w1;
        if (w1.substr(0,10).compare("#beginglsl") == 0) {
            if (shdrIdx != -1) {
                std::cerr << "GlShaderMgr::LoadShaderSource: Unexpected #beginglsl while reading source code." << std::endl;
                shdrIdx = -2;
                break;
            }
            shdrIdx = shdrInfo.size();
            instream >> w2 >> w3;
            bool ok = AllocateShdrInfo(w2, w3);
            if (!ok) {
                shdrIdx = -2;
                break;
            }
         }
        else if (w1.substr(0, 8).compare("#endglsl") == 0) {
            if (shdrIdx == -1) {
                std::cerr << "GlShaderMgr::LoadShaderSource: Unexpected #endglsl encountered." << std::endl;
                shdrIdx = -2;
                break;
            }
            shdrIdx = -1;           // Done with loading the shader code block
        }
        else {
            if (shdrIdx == -1) {
                continue;               // Ignore code not between #beginglsl and #endglsl
            }
            std::string& code = shdrInfo.back().shaderCodeArray;
            code += inLine;
            code += '\n';
        }
    }
    if (shdrIdx >= 0) {
        std::cerr << "GlShaderMgr::LoadShaderSource: Unexpected EOF encountered, missing #endglsl." << std::endl;
        shdrIdx = -2;
    }
    if (shdrIdx != -2 && shdrInfo.size() == beforeCount) {
        std::cerr << "GlShaderMgr::LoadShaderSource: File contained no #beginglsl line!" << std::endl;
        shdrIdx = -2;
    }
    if (shdrIdx == -2) {
        std::cerr << "     Error on line " << lineNumber << " of " << filename << "." << std::endl;
        return false;
    }
    return true;
}

// The next two routines are more "lagacy" kind of routines.
// They load a single shader source code block from either
//    a file or a string

// Load a single shader from a file (with no #glslbegin or $glslend commands).
bool GlShaderMgr::LoadSingleShaderFile(const char* filename, const char* shaderType, const char* shaderCodeName)
{
    std::ifstream inFile;
    inFile.open(filename);
    if (inFile.fail()) {
        std::cerr << "GlShaderMgr::LoadSingleShaderFile: Failed to open shader source file " << filename << "." << std::endl;
        return false;
    }
    std::string w2(shaderType);
    std::string w3(shaderCodeName);
    if (!AllocateShdrInfo(w2, w3)) {
        return false;
    }
    std::stringstream buffer;
    buffer << inFile.rdbuf();
    shdrInfo.back().shaderCodeArray = buffer.str();
    return true;
}

//  Load a single shader's source code from a C++ string.
bool GlShaderMgr::LoadSingleShaderString(const char* shaderSource, const char* shaderType, const char* shaderCodeName)
{
    std::string w2(shaderType);
    std::string w3(shaderCodeName);
    if (!AllocateShdrInfo(w2, w3)) {
        return false;
    }
    shdrInfo.back().shaderCodeArray = shaderSource;
    return true;
}

// Helper routine for loading shader source code block
bool GlShaderMgr::AllocateShdrInfo(std::string& shaderType, std::string& shaderCodeName)
{
    std::transform(shaderType.begin(), shaderType.end(), shaderType.begin(), tolower);
    auto it = find(shaderTypeName.begin(), shaderTypeName.end(), shaderType);
    if (it == shaderTypeName.end()) {
        std::cerr << "GlShaderMgr::AllocateShdrInfo: Invalid shader type name `" << shaderType << "'." << std::endl;
        return false;
    }
    auto it2 = findCodeName(shaderCodeName);
    if (it2 != shdrInfo.end()) {
        std::cerr << "GlShaderMgr::AllocateShdrInfo: Duplicated shader code block name `" << shaderCodeName << "'." << std::endl;
        return false;
    }
    ShaderInfo newInfo;                                             // Does not yet contain any code.
    newInfo.shaderType = (ShaderType)(it - shaderTypeName.begin()); // Shader code block type
    newInfo.shaderCodeName = shaderCodeName;                        // Shader code name
    newInfo.shaderOpenGLhandle = 0;                                 // Not (yet) compiled into a shader program
    shdrInfo.push_back(newInfo);
    return true;
}

// Clean up all intermediate compiled shaders
//    Remove source code, delete no-longer needed compiled shaders
void GlShaderMgr::FinalizeCompileAndLink()
{
    for (auto si : shdrInfo) {
        si.shaderCodeArray.clear();
        if (glIsShader(si.shaderOpenGLhandle)) {
            glDeleteShader(si.shaderOpenGLhandle);
            si.shaderOpenGLhandle = 0;
        }
    }
}



// ***** 
// Internal utilities for looking up code name's or shader handles
//   in the shdrInfo table.
// *****

std::vector<GlShaderMgr::ShaderInfo>::iterator GlShaderMgr::findCodeName(std::string theName)
{
    auto it = std::find_if(shdrInfo.begin(), shdrInfo.end(), 
                 [theName](const ShaderInfo& si) {return(si.shaderCodeName.compare(theName) == 0); });
    return it;
}

std::vector<GlShaderMgr::ShaderInfo>::iterator GlShaderMgr::findOpenGLhandle(unsigned int theHandle)
{
    auto it = std::find_if(shdrInfo.begin(), shdrInfo.end(),
        [theHandle](const ShaderInfo& si) {return(si.shaderOpenGLhandle == theHandle); });
    return it;
}

// ****
// Compiling and linking shaders

unsigned int GlShaderMgr::CompileShader(const char* shaderCodeName)
{
    return CompileShader(1, &shaderCodeName);;
}

unsigned int GlShaderMgr::CompileShader(int numcodeBlocks, const char* shaderCodeNames[])
{
    ShaderType typeSoFar = code_block;
    int* stringLengths = new int[numcodeBlocks];
    char** codeBlockPtrs = new char*[numcodeBlocks];
    int lastIdx = -1;
    for (int i = 0; i < numcodeBlocks; i++) {
        std::string nameStr(shaderCodeNames[i]);
        auto it = findCodeName(nameStr);
        if (it == shdrInfo.end()) {
            std::cerr << "GlShaderMgr::CompileShader: No shader with name '" << nameStr << "." << std::endl;
            return 0;
        }
        if (it->shaderType != code_block) {
            if (typeSoFar != code_block) {
                std::cerr << "GlShaderMgr::CompileShader: Found two code blocks specifying shader type: should be exactly one!" << std::endl;
                return 0;
            }
            typeSoFar = it->shaderType;
        }
        stringLengths[i] = it->shaderCodeArray.size();
        codeBlockPtrs[i] = &(it->shaderCodeArray[0]);
        lastIdx = it - shdrInfo.begin();
    }
    if (typeSoFar == code_block) {
        std::cerr << "GlShaderMgr::CompileShader: No code block specifies the shader type. Unable to compile!" << std::endl;
        return 0;
    }

    if (numcodeBlocks == 1) {
        unsigned int oldShader = shdrInfo[lastIdx].shaderOpenGLhandle;
        if (oldShader != 0) {
            return oldShader;       // Already compiled
        }
    }

    unsigned int newShader = glCreateShader(openGLtypes[typeSoFar]);
    glShaderSource(newShader, numcodeBlocks, codeBlockPtrs, stringLengths);
    glCompileShader(newShader);
    
    unsigned int ok = check_compilation_shader(newShader);
    if (!ok) {
        printf("   Above errors from compiling: ");
        for (int i = 0; i < numcodeBlocks; i++) {
            if (i != 0) {
                printf(", ");
            }
            printf("%s",shaderCodeNames[i]);
        }
        printf(".\n");
        return 0;
    }

    if (numcodeBlocks == 1) {
        shdrInfo[lastIdx].shaderOpenGLhandle = newShader;
    }
    else {
        ShaderInfo si;
        si.shaderType = typeSoFar;
        si.shaderOpenGLhandle = newShader;
        shdrInfo.push_back(si);
    }
     
    return newShader;
}

// Link a list of already compiled shaders -- specified by their OpenGL handles,
//     as returned by CompileShader().
unsigned int GlShaderMgr::LinkShaderProgram(int numShaders, const unsigned int shaderList[])
{
    unsigned int shaderProgram = glCreateProgram();
    for (int i = 0; i < numShaders; i++) {
        glAttachShader(shaderProgram, shaderList[i]);
    }
    glLinkProgram(shaderProgram);

    int ok = check_link_status(shaderProgram);
    if (ok == 0) {
        return 0;               // Link error occured.
    }

    shdrPrograms.push_back(shaderProgram);
    return shaderProgram;
}

// The next three "convenience" routines allow compiling and linking shaders
//    with a little less code

unsigned int GlShaderMgr::CompileAndLinkProgram(const char* shaderName1, const char* shaderName2)
{
    unsigned int shader1 = GlShaderMgr::CompileShader(shaderName1);
    unsigned int shader2 = GlShaderMgr::CompileShader(shaderName2);
    if (shader1 == 0 || shader2 == 0) {
        return 0;
    }
    unsigned int shaders[2] = { shader1, shader2 };
    return GlShaderMgr::LinkShaderProgram(2, shaders);
}

unsigned int GlShaderMgr::CompileAndLinkProgram(const char* shaderName1, const char* shaderName2, const char* shaderName3)
{
    unsigned int shader1 = GlShaderMgr::CompileShader(shaderName1);
    unsigned int shader2 = GlShaderMgr::CompileShader(shaderName2);
    unsigned int shader3 = GlShaderMgr::CompileShader(shaderName3);
    if (shader1 == 0 || shader2 == 0 || shader3==0) {
        return 0;
    }
    unsigned int shaders[3] = { shader1, shader2, shader3 };
    return GlShaderMgr::LinkShaderProgram(3, shaders);
}

// Call this only if all the shaders are to be combined into a shader program.
unsigned int GlShaderMgr::CompileAndLinkAll()
{
    std::vector<unsigned int> shaderList;
    for (ShaderInfo& si : shdrInfo) {
        if (si.shaderOpenGLhandle == 0) {
            if (si.shaderType == code_block) {
                std::cerr << "GlShaderMgr::CompileAndLinkAll: Cannot compile a code block." << std::endl;
                return 0;
            }
            unsigned int newShader = glCreateShader(openGLtypes[si.shaderType]);
            int len = si.shaderCodeArray.size();
            char* code = &(si.shaderCodeArray[0]);
            glShaderSource(newShader, 1, &code, &len);
            glCompileShader(newShader);
            unsigned int ok = check_compilation_shader(newShader);
            if (!ok) {
                return 0;
            }
            shaderList.push_back(newShader);
        }
    }
    unsigned int shaderProgram = LinkShaderProgram(shaderList.size(), &shaderList[0]);
    FinalizeCompileAndLink();
    return shaderProgram;
}


// ****
// Check for compile errors for a shader.
// Parameters:
//    - shader. The shader identifier (an unsigned integer)
// Returns:
//    shader (the same value as was passed in) if compile succeeded.  Or,
//    0 if an error occured in compilation or if not a valid shader.

 unsigned int GlShaderMgr::check_compilation_shader(unsigned int shader) {
    if (!glIsShader(shader)) {
        printf("ERROR: Not a shader! Possibly an allocation error.\n");
        return 0;
    }

    int success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (success) {
        return shader;	// Compilation was successful
    }

    // Compilation failed
    int infoLogLength;
    glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);
    char* infoLog = new char[infoLogLength];
    glGetShaderInfoLog(shader, infoLogLength, NULL, infoLog);
    printf("ERROR::Shader compilation failed!\n%s\n", infoLog);
    delete[] infoLog;
    return 0;
}

/*
* Check for link errors for a program.
* Parameters:
*    - program. The program identifier (an unsigned integer)
*          A "program" is a combination of one or more shaders.
* Returns:
*    program (the same value as was passed in) if there are no link errors.  Or,
*    0 if there is a link error or if not a valid program identifier.
*/
unsigned int GlShaderMgr::check_link_status(unsigned int program) {
    if (!glIsProgram(program)) {
        printf("ERROR: Not a shader program! Possibly an allocation error.\n");
        return 0;
    }

    int success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (success) {
        return program;		// Linkage was successful
    }

    int infoLogLength;
    glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infoLogLength);
    char* infoLog = new char[infoLogLength];
    glGetProgramInfoLog(program, infoLogLength, NULL, infoLog);
    printf("ERROR::Shader program link failed!\n%s\n", infoLog);
    return 0;
}



