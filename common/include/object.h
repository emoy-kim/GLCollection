#pragma once

#include "base.h"

class ObjectGL final
{
public:
    enum LayoutLocation { VertexLocation = 0, NormalLocation, TextureLocation };

    ObjectGL();
    ~ObjectGL();

    void setEmissionColor(const glm::vec4& emission_color);
    void setAmbientReflectionColor(const glm::vec4& ambient_reflection_color);
    void setDiffuseReflectionColor(const glm::vec4& diffuse_reflection_color);
    void setSpecularReflectionColor(const glm::vec4& specular_reflection_color);
    void setSpecularReflectionExponent(const float& specular_reflection_exponent);
    void setObject(GLenum draw_mode, int vertex_num);
    void setObject(GLenum draw_mode, const std::vector<glm::vec3>& vertices);
    void setObject(
        GLenum draw_mode,
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec3>& normals
    );
    void setObject(
        GLenum draw_mode,
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec2>& textures,
        const std::string& texture_file_path,
        bool is_grayscale = false
    );
    void setObject(
        GLenum draw_mode,
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& textures
    );
    void setObject(
        GLenum draw_mode,
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& textures,
        const std::string& texture_file_path,
        bool is_grayscale = false
    );
    void setObject(
        GLenum draw_mode,
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& textures,
        const uint8_t* image_buffer,
        int width,
        int height,
        bool is_grayscale = false
    );
    void setSquareObject(GLenum draw_mode, bool use_texture = true);
    void setSquareObject(
        GLenum draw_mode,
        const std::string& texture_file_path,
        bool is_grayscale = false
    );
    void setSquareObject(
        GLenum draw_mode,
        const uint8_t* image_buffer,
        int width,
        int height,
        bool is_grayscale = false
    );
    int addTexture(const std::string& texture_file_path, bool is_grayscale = false);
    void addTexture(int width, int height, bool is_grayscale = false);
    int addTexture(const uint8_t* image_buffer, int width, int height, bool is_grayscale = false);
    void addCubeTextures(const std::array<uint8_t*, 6>& textures, int width, int height);
    void addCubeTextures(const std::array<std::string, 6>& texture_paths);
    void updateDataBuffer(const std::vector<glm::vec3>& vertices);
    void updateDataBuffer(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals);
    void updateDataBuffer(
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec3>& normals,
        const std::vector<glm::vec2>& textures
    );
    void updateTexture(const uint8_t* image_buffer, int index, int width, int height) const;
    static void updateCubeTextures(const std::array<uint8_t*, 6>& textures, int width, int height);
    void replaceVertices(const std::vector<glm::vec3>& vertices, bool normals_exist, bool textures_exist);
    void replaceVertices(const std::vector<float>& vertices, bool normals_exist, bool textures_exist);
    [[nodiscard]] static bool readObjectFile(
        std::vector<glm::vec3>& vertices,
        std::vector<glm::vec3>& normals,
        std::vector<glm::vec2>& textures,
        const std::string& file_path
    );
    [[nodiscard]] GLuint getVAO() const { return VAO; }
    [[nodiscard]] GLenum getDrawMode() const { return DrawMode; }
    [[nodiscard]] GLsizei getVertexNum() const { return VerticesCount; }
    [[nodiscard]] GLuint getTextureID(int index) const { return TextureID[index]; }
    [[nodiscard]] int getTextureNum() const { return static_cast<int>(TextureID.size()); }
    [[nodiscard]] glm::vec4 getEmissionColor() const { return EmissionColor; }
    [[nodiscard]] glm::vec4 getAmbientReflectionColor() const { return AmbientReflectionColor; }
    [[nodiscard]] glm::vec4 getDiffuseReflectionColor() const { return DiffuseReflectionColor; }
    [[nodiscard]] glm::vec4 getSpecularReflectionColor() const { return SpecularReflectionColor; }
    [[nodiscard]] float getSpecularReflectionExponent() const { return SpecularReflectionExponent; }
    [[nodiscard]] glm::ivec2 getTextureSize(GLuint id) { return TextureIDToSize[id]; }

    template<typename T>
    [[nodiscard]] GLuint addCustomBufferObject(int data_size)
    {
        GLuint buffer = 0;
        glCreateBuffers( 1, &buffer );
        glNamedBufferStorage( buffer, sizeof( T ) * data_size, nullptr, GL_DYNAMIC_STORAGE_BIT );
        CustomBuffers.emplace_back( buffer );
        return buffer;
    }

private:
    uint8_t* ImageBuffer;
    GLuint VAO;
    GLuint VBO;
    GLenum DrawMode;
    std::vector<GLfloat> DataBuffer;
    std::vector<GLuint> TextureID;
    std::vector<GLuint> CustomBuffers;
    std::map<GLuint, glm::ivec2> TextureIDToSize;
    GLsizei VerticesCount;
    glm::vec4 EmissionColor;

    // It is usually set to the same color with DiffuseReflectionColor.
    // Otherwise, it should be in balance with DiffuseReflectionColor.
    glm::vec4 AmbientReflectionColor;

    glm::vec4 DiffuseReflectionColor; // The intrinsic color
    glm::vec4 SpecularReflectionColor;
    float SpecularReflectionExponent;

    [[nodiscard]] bool prepareTexture2DUsingFreeImage(const std::string& file_path, bool is_grayscale);
    void prepareTexture(bool normals_exist) const;
    void prepareVertexBuffer(int n_bytes_per_vertex);
    void prepareNormal() const;
    static void getSquareObject(
        std::vector<glm::vec3>& vertices,
        std::vector<glm::vec3>& normals,
        std::vector<glm::vec2>& textures
    );
    static void calculateTangents(
        std::vector<glm::vec3>& tangents,
        const std::vector<glm::vec3>& vertices,
        const std::vector<glm::vec2>& textures
    );
};