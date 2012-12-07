#include <iostream>
#include <fstream>
#include <string>
#include <GL/glew.h>

namespace imac2gl3 {

static const char* readFile(const char* filePath) {
    std::ifstream file(filePath);

    if (!file) {
        return 0;
    }

    file.seekg(0, std::ios_base::end);
    size_t length = file.tellg();
    file.seekg(0);

    char* src = new char[length + 1];
    file.read(src, length);
    src[length] = '\0';
    
    return src;
}

GLuint loadProgram(const char* vertexShaderFile, const char* fragmentShaderFile) {
    const char* vertexShaderSource = readFile(vertexShaderFile);
    if(!vertexShaderSource) {
        std::cerr << "Unable to load " << vertexShaderFile << std::endl;
        return 0;
    }
    
    const char* fragmentShaderSource = readFile(fragmentShaderFile);
    if(!fragmentShaderSource) {
        std::cerr << "Unable to load " << fragmentShaderFile << std::endl;
        return 0;
    }
    
    // Creation d'un Vertex Shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    
    // Spécification du code source
    glShaderSource(vertexShader, 1, &vertexShaderSource, 0);
    
    // Compilation du shader
    glCompileShader(vertexShader);
    
    /// Vérification que la compilation a bien fonctionnée (très important !)
    GLint compileStatus;
    
    // Récupération du status de compilation
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE) {
        // Si echec, récupération de la taille du log de compilation
        GLint logLength;
        glGetShaderiv(vertexShader, GL_INFO_LOG_LENGTH, &logLength);
        
        // Allocation d'une chaine de caractère suffisement grande pour contenir le log
        char* log = new char[logLength];
        
        glGetShaderInfoLog(vertexShader, logLength, 0, log);
        std::cerr << "Vertex Shader error:" << log << std::endl;
        std::cerr << vertexShaderSource << std::endl;
        
        delete [] log;
        return 0;
    }
    
    // Creation d'un Fragment Shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    
    // Spécification du code source
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, 0);
    
    // Compilation du shader
    glCompileShader(fragmentShader);
    
    /// Vérification que la compilation a bien fonctionnée (très important !)
    
    // Récupération du status de compilation
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &compileStatus);
    if(compileStatus == GL_FALSE) {
        // Si echec, récupération de la taille du log de compilation
        GLint logLength;
        glGetShaderiv(fragmentShader, GL_INFO_LOG_LENGTH, &logLength);
        
        // Allocation d'une chaine de caractère suffisement grande pour contenir le log
        char* log = new char[logLength];
        
        glGetShaderInfoLog(fragmentShader, logLength, 0, log);
        std::cerr << "Fragment Shader error:" << log << std::endl;
        std::cerr << fragmentShaderSource << std::endl;
        
        delete [] log;
        return 0;
    }
    
    GLuint program;
    
    // Creation d'un programme
    program = glCreateProgram();
    
    // Attachement des shaders au programme
    glAttachShader(program, vertexShader);
    glAttachShader(program, fragmentShader);
    
    // Désallocation des shaders: ils ne seront réellement supprimés que lorsque le programme sera supprimé
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    // Edition de lien
    glLinkProgram(program);
    
    /// Vérification que l'édition de liens a bien fonctionnée (très important aussi !)
    GLint linkStatus;
    glGetProgramiv(program, GL_LINK_STATUS, &linkStatus);
    if(linkStatus == GL_FALSE) {
        // Si echec, récupération de la taille du log de link
        GLint logLength;
        glGetProgramiv(program, GL_INFO_LOG_LENGTH, &logLength);
        
        // Allocation d'une chaine de caractère suffisement grande pour contenir le log
        char* log = new char[logLength];
        
        glGetProgramInfoLog(program, logLength, 0, log);
        std::cerr << "Program link error:" << log << std::endl;
        
        delete [] log;
        return 0;
    }
    
    delete [] vertexShaderSource;
    delete [] fragmentShaderSource;
    
    return program;
}

}
