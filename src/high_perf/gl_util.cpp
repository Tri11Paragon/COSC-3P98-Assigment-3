/*
 * Created by Brett on 31/03/23.
 * Licensed under GNU General Public License V3.0
 * See LICENSE file for license detail
 */
#include <high_perf/gl_util.h>
#include "blt/std/memory.h"
#include <blt/std/loader.h>

/**
 * Note: This is taken from my final project,
 * https://github.com/Tri11Paragon/COSC-3P98-Final-Project/blob/main/include/render/gl.h
 */

static inline std::string removeEmptyFirstLines(const std::string& string){
    auto lines = blt::string::split(string, "\n");
    std::string new_source_string;
    for (const auto& line : lines) {
        if (!line.empty() && !blt::string::contains(line, "\"")) {
            new_source_string += line;
            new_source_string += "\n";
        }
    }
    return new_source_string;
}

unsigned int shader::createShader(const std::string& source, int type) {
    const char* shader_code = source.c_str();
    // creates a Shader
    unsigned int shaderID = glCreateShader(type);
    // loads the shader code for later complication and uploading into the graphics card
    // TODO: defines can be added here by sending them as additional strings. No need to edit the source string
    glShaderSource(shaderID, 1, &shader_code, nullptr);
    // Compile it
    glCompileShader(shaderID);
    
    // make sure there are no errors in the compilation. If there is then print out information pertaining to the error.
    // the actual log is highly dependent on the platform this is being run from, so we cannot make any assumptions about the issue.
    // the TODO: maybe find a way of lexing the output to give suggestions about fixing the error? default error messages can be unhelpful at times.
    GLint success;
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &success);
    if (!success) {
        int log_length = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &log_length);
        
        // scoped buffers will delete their memory when they go out of scope. A benefit of using BLT
        blt::scoped_buffer<GLchar> infoLog{static_cast<unsigned long>(log_length + 1)};
        
        glGetShaderInfoLog(shaderID, log_length + 1, nullptr, infoLog.buffer);
        auto shader_type_str = (type == GL_VERTEX_SHADER ? "Vertex Shader" : type == GL_FRAGMENT_SHADER ? "Fragment Shader" : "Other Shader");
        BLT_ERROR("--- --- --- --- --- --- --- --- ---");
        BLT_ERROR("Unable to compile shader of type %s\nShader source:", shader_type_str);
        BLT_ERROR(source);
        BLT_ERROR("I have an log of %d length", log_length);
        BLT_ERROR(infoLog.buffer);
        BLT_ERROR("--- --- --- --- --- --- --- --- ---");
    }
    return shaderID;
}

shader::shader(const std::string& vertex, const std::string& fragment, const std::string& geometry, bool load_as_string) {
    // load shader sources
    bool load_geometry = !geometry.empty();
    std::string vertex_source = vertex;
    std::string fragment_source = fragment;
    std::string geometry_source = geometry;
    if (!load_as_string){
        // BLT provides a recursive file loader for glsl shaders. It's pretty much just a recursive function looking for include statements.
        vertex_source = blt::fs::loadShaderFile(vertex);
        fragment_source = blt::fs::loadShaderFile(fragment);
        if (load_geometry)
            geometry_source = blt::fs::loadShaderFile(geometry);
    } else {
        vertex_source = removeEmptyFirstLines(vertex_source);
        fragment_source = removeEmptyFirstLines(fragment_source);
        geometry_source = removeEmptyFirstLines(geometry_source);
    }
    
    // create the shaders
    vertexShaderID = createShader(vertex_source, GL_VERTEX_SHADER);
    fragmentShaderID = createShader(fragment_source, GL_FRAGMENT_SHADER);
    if (load_geometry)
        BLT_ERROR("Unable to load geometry shader because webgl doesn't support it!");
    
    // bind them to a program
    programID = glCreateProgram();
    // attach the loaded shaders to the Shader program
    glAttachShader(programID, vertexShaderID);
    glAttachShader(programID, fragmentShaderID);
    if (load_geometry)
        glAttachShader(programID, geometryShaderID);
    // link and make sure that our program is valid.
    glLinkProgram(programID);
    
    GLint success;
    glGetProgramiv(programID, GL_LINK_STATUS, &success);
    if (!success) {
        int log_length = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &log_length);
        
        // scoped buffers will delete their memory when they go out of scope.
        blt::scoped_buffer<GLchar> infoLog{static_cast<unsigned long>(log_length + 1)};
        
        glGetProgramInfoLog(programID, log_length + 1, nullptr, infoLog.buffer);
        BLT_ERROR("--- --- --- --- --- --- --- --- ---");
        BLT_ERROR("Unable to link program of ID: %d", programID);
        BLT_ERROR(vertex_source);
        BLT_ERROR(fragment_source);
        BLT_ERROR(geometry_source);
        BLT_ERROR("I have an log of %d length", log_length);
        BLT_ERROR(infoLog.buffer);
        BLT_ERROR("--- --- --- --- --- --- --- --- ---");
    }
    
    glValidateProgram(programID);
    bind();
    setUniformBlockLocation("StandardMatrices", 0);
    glUseProgram(0);
    
}

void shader::bindAttribute(int attribute, const std::string &name) const {
    bind();
    glBindAttribLocation(programID, attribute, name.c_str());
}

void shader::setUniformBlockLocation(const std::string &name, int location) const {
    bind();
    glUniformBlockBinding(programID, glGetUniformBlockIndex(programID, name.c_str()), location);
}

shader::~shader() {
    glUseProgram(0);
    // shader was moved
    if (programID <= 0)
        return;
    // remove all the shaders from the program
    glDetachShader(programID, vertexShaderID);
    if (geometryShaderID)
        glDetachShader(programID, geometryShaderID);
    if (tessellationShaderID)
        glDetachShader(programID, tessellationShaderID);
    glDetachShader(programID, fragmentShaderID);
    
    // delete the shaders
    glDeleteShader(vertexShaderID);
    if (geometryShaderID)
        glDeleteShader(geometryShaderID);
    if (tessellationShaderID)
        glDeleteShader(tessellationShaderID);
    glDeleteShader(fragmentShaderID);
    
    // delete the Shader program
    glDeleteProgram(programID);
}

shader::shader(shader&& move) noexcept {
    // the move constructor doesn't need to construct a new shader but it does need to ensure all old variables are moved over
    programID = move.programID;
    vertexShaderID = move.vertexShaderID;
    fragmentShaderID = move.fragmentShaderID;
    geometryShaderID = move.geometryShaderID;
    tessellationShaderID = move.tessellationShaderID;
    for (const auto& pair : move.uniformVars)
        uniformVars.insert(pair);
    // by setting the program ID to -1 we tell the shader it has been moved.
    move.programID = -1;
}

/**
 * This part was made for this assignment and will likely be used in future projects
 */

compute_shader::compute_shader(const std::string& shader_source, bool loadAsString) {
    int status;
    std::string source;
    const char* c_source;
    
    if (!loadAsString)
        source = blt::fs::loadShaderFile(shader_source);
    else
        source = removeEmptyFirstLines(shader_source);
    
    c_source = source.c_str();
    
    shaderID = glCreateShader(GL_COMPUTE_SHADER);
    
    glShaderSource(shaderID, 1, &c_source, NULL);
    glCompileShader(shaderID);
    
    glGetShaderiv(shaderID, GL_COMPILE_STATUS, &status);
    if (!status){
        int log_length = 0;
        glGetShaderiv(shaderID, GL_INFO_LOG_LENGTH, &log_length);
        blt::scoped_buffer<GLchar> infoLog{static_cast<unsigned long>(log_length + 1)};
        glGetShaderInfoLog(shaderID, log_length + 1, nullptr, infoLog.buffer);
        BLT_ERROR("Unable to compile compute shader! (%d)", log_length);
        BLT_ERROR(infoLog.buffer);
    }
    
    programID = glCreateProgram();
    glAttachShader(programID, shaderID);
    glLinkProgram(programID);
    
    glGetProgramiv(shaderID, GL_LINK_STATUS, &status);
    if (!status){
        int log_length = 0;
        glGetProgramiv(programID, GL_INFO_LOG_LENGTH, &log_length);
        blt::scoped_buffer<GLchar> infoLog{static_cast<unsigned long>(log_length + 1)};
        glGetProgramInfoLog(programID, log_length + 1, nullptr, infoLog.buffer);
        BLT_ERROR("Unable to link compute shader!");
        BLT_ERROR(infoLog.buffer);
    }
}

compute_shader::~compute_shader() {
    glDeleteShader(shaderID);
}
