#include "object.h"

ObjectGL::ObjectGL()
    : ImageBuffer( nullptr ),
      VAO( 0 ),
      VBO( 0 ),
      IBO( 0 ),
      DrawMode( 0 ),
      VerticesCount( 0 ),
      EmissionColor( 0.0f, 0.0f, 0.0f, 1.0f ),
      AmbientReflectionColor( 0.2f, 0.2f, 0.2f, 1.0f ),
      DiffuseReflectionColor( 0.8f, 0.8f, 0.8f, 1.0f ),
      SpecularReflectionColor( 0.0f, 0.0f, 0.0f, 1.0f ),
      SpecularReflectionExponent( 0.0f ) {}

ObjectGL::~ObjectGL()
{
    if (IBO != 0)
        glDeleteBuffers( 1, &IBO );
    if (VBO != 0)
        glDeleteBuffers( 1, &VBO );
    if (VAO != 0)
        glDeleteVertexArrays( 1, &VAO );
    for (const auto& texture_id : TextureID) {
        if (texture_id != 0)
            glDeleteTextures( 1, &texture_id );
    }
    for (const auto& buffer : CustomBuffers) {
        if (buffer != 0)
            glDeleteBuffers( 1, &buffer );
    }
    delete [] ImageBuffer;
}

bool ObjectGL::prepareTexture2DUsingFreeImage(const std::string& file_path, bool is_grayscale)
{
    const FREE_IMAGE_FORMAT format = FreeImage_GetFileType( file_path.c_str(), 0 );
    FIBITMAP* texture = FreeImage_Load( format, file_path.c_str() );
    if (!texture) return false;

    FIBITMAP* texture_converted;
    const uint n_bits_per_pixel = FreeImage_GetBPP( texture );
    const uint n_bits = is_grayscale ? 8 : 32;
    if (is_grayscale) {
        texture_converted = n_bits_per_pixel == n_bits ? texture : FreeImage_GetChannel( texture, FICC_RED );
    }
    else {
        texture_converted = n_bits_per_pixel == n_bits ? texture : FreeImage_ConvertTo32Bits( texture );
    }

    const auto width = static_cast<GLsizei>(FreeImage_GetWidth( texture_converted ));
    const auto height = static_cast<GLsizei>(FreeImage_GetHeight( texture_converted ));
    const GLvoid* data = FreeImage_GetBits( texture_converted );
    glTextureStorage2D( TextureID.back(), 1, is_grayscale ? GL_R8 : GL_RGBA8, width, height );
    glTextureSubImage2D(
        TextureID.back(), 0, 0, 0,
        width, height,
        is_grayscale ? GL_RED : GL_BGRA,
        GL_UNSIGNED_BYTE,
        data
    );
    TextureIDToSize[TextureID.back()] = glm::ivec2( width, height );

    FreeImage_Unload( texture_converted );
    if (n_bits_per_pixel != n_bits) FreeImage_Unload( texture );
    return true;
}

int ObjectGL::addTexture(const std::string& texture_file_path, bool is_grayscale)
{
    GLuint texture_id = 0;
    glCreateTextures( GL_TEXTURE_2D, 1, &texture_id );
    TextureID.emplace_back( texture_id );
    if (!prepareTexture2DUsingFreeImage( texture_file_path, is_grayscale )) {
        glDeleteTextures( 1, &texture_id );
        TextureID.erase( TextureID.end() - 1 );
        throw std::runtime_error( "Could not read image file " + texture_file_path );
    }

    glTextureParameteri( texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glGenerateTextureMipmap( texture_id );
    return static_cast<int>(TextureID.size() - 1);
}

void ObjectGL::addTexture(int width, int height, bool is_grayscale)
{
    GLuint texture_id = 0;
    glCreateTextures( GL_TEXTURE_2D, 1, &texture_id );
    glTextureStorage2D(
        texture_id,
        1,
        is_grayscale ? GL_R8 : GL_RGBA8,
        width,
        height
    );
    glTextureParameteri( texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_S, GL_REPEAT );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_T, GL_REPEAT );
    glGenerateTextureMipmap( texture_id );
    TextureID.emplace_back( texture_id );
    TextureIDToSize[TextureID.back()] = glm::ivec2( width, height );
}

int ObjectGL::addTexture(const uint8_t* image_buffer, int width, int height, bool is_grayscale)
{
    addTexture( width, height, is_grayscale );
    glTextureSubImage2D(
        TextureID.back(),
        0,
        0,
        0,
        width,
        height,
        is_grayscale ? GL_RED : GL_BGRA,
        GL_UNSIGNED_BYTE,
        image_buffer
    );
    return static_cast<int>(TextureID.size() - 1);
}

void ObjectGL::addCubeTextures(const std::array<uint8_t*, 6>& textures, int width, int height)
{
    GLuint texture_id = 0;
    glCreateTextures( GL_TEXTURE_CUBE_MAP, 1, &texture_id );
    glBindTexture( GL_TEXTURE_CUBE_MAP, texture_id );
    TextureID.emplace_back( texture_id );

    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTextureParameteri( texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_BASE_LEVEL, 0 );
    glTextureParameteri( texture_id, GL_TEXTURE_MAX_LEVEL, 0 );
    glGenerateTextureMipmap( texture_id );

    for (int i = 0; i < 6; ++i) {
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB,
            width, height, 0,
            GL_BGRA, GL_UNSIGNED_BYTE,
            textures[i]
        );
    }
}

void ObjectGL::addCubeTextures(const std::array<std::string, 6>& texture_paths)
{
    GLuint texture_id = 0;
    glCreateTextures( GL_TEXTURE_CUBE_MAP, 1, &texture_id );
    glBindTexture( GL_TEXTURE_CUBE_MAP, texture_id );
    TextureID.emplace_back( texture_id );

    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE );
    glTextureParameteri( texture_id, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE );
    glTextureParameteri( texture_id, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
    glTextureParameteri( texture_id, GL_TEXTURE_BASE_LEVEL, 0 );
    glTextureParameteri( texture_id, GL_TEXTURE_MAX_LEVEL, 0 );
    glGenerateTextureMipmap( texture_id );

    for (int i = 0; i < 6; ++i) {
        const FREE_IMAGE_FORMAT format = FreeImage_GetFileType( texture_paths[i].c_str(), 0 );
        FIBITMAP* texture = FreeImage_Load( format, texture_paths[i].c_str() );
        const uint n_bits_per_pixel = FreeImage_GetBPP( texture );
        FIBITMAP* texture_converted = n_bits_per_pixel == 32 ? texture : FreeImage_ConvertTo32Bits( texture );
        const auto width = static_cast<GLsizei>(FreeImage_GetWidth( texture_converted ));
        const auto height = static_cast<GLsizei>(FreeImage_GetHeight( texture_converted ));
        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, GL_RGB,
            width, height, 0,
            GL_BGRA, GL_UNSIGNED_BYTE,
            FreeImage_GetBits( texture_converted )
        );
        FreeImage_Unload( texture_converted );
        if (n_bits_per_pixel != 32) FreeImage_Unload( texture );
    }
}

void ObjectGL::prepareTexture(bool normals_exist) const
{
    const uint offset = normals_exist ? 6 : 3;
    glVertexArrayAttribFormat( VAO, TextureLocation, 2, GL_FLOAT, GL_FALSE, offset * sizeof( GLfloat ) );
    glEnableVertexArrayAttrib( VAO, TextureLocation );
    glVertexArrayAttribBinding( VAO, TextureLocation, 0 );
}

void ObjectGL::prepareNormal() const
{
    glVertexArrayAttribFormat( VAO, NormalLocation, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ) );
    glEnableVertexArrayAttrib( VAO, NormalLocation );
    glVertexArrayAttribBinding( VAO, NormalLocation, 0 );
}

void ObjectGL::prepareVertexBuffer(int n_bytes_per_vertex)
{
    glCreateBuffers( 1, &VBO );
    glNamedBufferStorage( VBO, sizeof( GLfloat ) * DataBuffer.size(), DataBuffer.data(), GL_DYNAMIC_STORAGE_BIT );

    glCreateVertexArrays( 1, &VAO );
    glVertexArrayVertexBuffer( VAO, 0, VBO, 0, n_bytes_per_vertex );
    glVertexArrayAttribFormat( VAO, VertexLocation, 3, GL_FLOAT, GL_FALSE, 0 );
    glEnableVertexArrayAttrib( VAO, VertexLocation );
    glVertexArrayAttribBinding( VAO, VertexLocation, 0 );
}

void ObjectGL::prepareIndexBuffer(const std::vector<GLuint>& indices)
{
    assert( VAO != 0 );

    if (IBO != 0)
        glDeleteBuffers( 1, &IBO );

    glCreateBuffers( 1, &IBO );
    glNamedBufferStorage( IBO, sizeof( GLuint ) * indices.size(), indices.data(), GL_DYNAMIC_STORAGE_BIT );
    glVertexArrayElementBuffer( VAO, IBO );
}

void ObjectGL::getSquareObject(
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& textures
)
{
    vertices = {
        { 1.0f, 0.0f, 0.0f },
        { 1.0f, 1.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },

        { 1.0f, 0.0f, 0.0f },
        { 0.0f, 1.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f }
    };
    normals = {
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },

        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f },
        { 0.0f, 0.0f, 1.0f }
    };
    textures = {
        { 1.0f, 0.0f },
        { 1.0f, 1.0f },
        { 0.0f, 1.0f },

        { 1.0f, 0.0f },
        { 0.0f, 1.0f },
        { 0.0f, 0.0f }
    };
}

void ObjectGL::setObject(GLenum draw_mode, int vertex_num)
{
    DrawMode = draw_mode;
    VerticesCount = vertex_num;
    DataBuffer.resize( vertex_num * 3 );
    prepareVertexBuffer( 3 * sizeof( GLfloat ) );
    DataBuffer.clear();
}

void ObjectGL::setObject(GLenum draw_mode, const std::vector<glm::vec3>& vertices)
{
    DrawMode = draw_mode;
    VerticesCount = 0;
    for (auto& vertex : vertices) {
        DataBuffer.emplace_back( vertex.x );
        DataBuffer.emplace_back( vertex.y );
        DataBuffer.emplace_back( vertex.z );
        VerticesCount++;
    }
    constexpr int n_bytes_per_vertex = 3 * sizeof( GLfloat );
    prepareVertexBuffer( n_bytes_per_vertex );
    DataBuffer.clear();
}

void ObjectGL::setObject(
    GLenum draw_mode,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals
)
{
    DrawMode = draw_mode;
    VerticesCount = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        DataBuffer.emplace_back( vertices[i].x );
        DataBuffer.emplace_back( vertices[i].y );
        DataBuffer.emplace_back( vertices[i].z );
        DataBuffer.emplace_back( normals[i].x );
        DataBuffer.emplace_back( normals[i].y );
        DataBuffer.emplace_back( normals[i].z );
        VerticesCount++;
    }
    constexpr int n_bytes_per_vertex = 6 * sizeof( GLfloat );
    prepareVertexBuffer( n_bytes_per_vertex );
    prepareNormal();
    DataBuffer.clear();
}

void ObjectGL::setObject(
    GLenum draw_mode,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec2>& textures,
    const std::string& texture_file_path,
    bool is_grayscale
)
{
    DrawMode = draw_mode;
    VerticesCount = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        DataBuffer.emplace_back( vertices[i].x );
        DataBuffer.emplace_back( vertices[i].y );
        DataBuffer.emplace_back( vertices[i].z );
        DataBuffer.emplace_back( textures[i].x );
        DataBuffer.emplace_back( textures[i].y );
        VerticesCount++;
    }
    constexpr int n_bytes_per_vertex = 5 * sizeof( GLfloat );
    prepareVertexBuffer( n_bytes_per_vertex );
    prepareTexture( false );
    addTexture( texture_file_path, is_grayscale );
    DataBuffer.clear();
}

void ObjectGL::setObject(
    GLenum draw_mode,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& textures
)
{
    DrawMode = draw_mode;
    VerticesCount = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        DataBuffer.emplace_back( vertices[i].x );
        DataBuffer.emplace_back( vertices[i].y );
        DataBuffer.emplace_back( vertices[i].z );
        DataBuffer.emplace_back( normals[i].x );
        DataBuffer.emplace_back( normals[i].y );
        DataBuffer.emplace_back( normals[i].z );
        DataBuffer.emplace_back( textures[i].x );
        DataBuffer.emplace_back( textures[i].y );
        VerticesCount++;
    }
    constexpr int n_bytes_per_vertex = 8 * sizeof( GLfloat );
    prepareVertexBuffer( n_bytes_per_vertex );
    prepareNormal();
    prepareTexture( true );
    DataBuffer.clear();
}

void ObjectGL::setObject(
    GLenum draw_mode,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& textures,
    const std::vector<GLuint>& indices
)
{
    DrawMode = draw_mode;
    VerticesCount = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        DataBuffer.emplace_back( vertices[i].x );
        DataBuffer.emplace_back( vertices[i].y );
        DataBuffer.emplace_back( vertices[i].z );
        DataBuffer.emplace_back( normals[i].x );
        DataBuffer.emplace_back( normals[i].y );
        DataBuffer.emplace_back( normals[i].z );
        DataBuffer.emplace_back( textures[i].x );
        DataBuffer.emplace_back( textures[i].y );
        VerticesCount++;
    }
    constexpr int n_bytes_per_vertex = 8 * sizeof( GLfloat );
    prepareVertexBuffer( n_bytes_per_vertex );
    prepareNormal();
    prepareTexture( true );
    prepareIndexBuffer( indices );
    DataBuffer.clear();
}

void ObjectGL::setObject(
    GLenum draw_mode,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& textures,
    const std::string& texture_file_path,
    bool is_grayscale
)
{
    setObject( draw_mode, vertices, normals, textures );
    addTexture( texture_file_path, is_grayscale );
}

void ObjectGL::setObject(
    GLenum draw_mode,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& textures,
    const uint8_t* image_buffer,
    int width,
    int height,
    bool is_grayscale
)
{
    setObject( draw_mode, vertices, normals, textures );
    addTexture( image_buffer, width, height, is_grayscale );
}

void ObjectGL::setObject(
    GLenum draw_mode,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& textures,
    const std::vector<GLuint>& indices,
    const std::string& texture_file_path,
    bool is_grayscale
)
{
    setObject( draw_mode, vertices, normals, textures, indices );
    addTexture( texture_file_path, is_grayscale );
}

void ObjectGL::setSquareObject(GLenum draw_mode, bool use_texture)
{
    std::vector<glm::vec3> square_vertices, square_normals;
    std::vector<glm::vec2> square_textures;
    getSquareObject( square_vertices, square_normals, square_textures );
    if (use_texture) setObject( draw_mode, square_vertices, square_normals, square_textures );
    else setObject( draw_mode, square_vertices, square_normals );
}

void ObjectGL::setSquareObject(GLenum draw_mode, const std::string& texture_file_path, bool is_grayscale)
{
    std::vector<glm::vec3> square_vertices, square_normals;
    std::vector<glm::vec2> square_textures;
    getSquareObject( square_vertices, square_normals, square_textures );
    setObject( draw_mode, square_vertices, square_normals, square_textures, texture_file_path, is_grayscale );
}

void ObjectGL::setSquareObject(GLenum draw_mode, const uint8_t* image_buffer, int width, int height, bool is_grayscale)
{
    std::vector<glm::vec3> square_vertices, square_normals;
    std::vector<glm::vec2> square_textures;
    getSquareObject( square_vertices, square_normals, square_textures );
    setObject( draw_mode, square_vertices, square_normals, square_textures, image_buffer, width, height, is_grayscale );
}

void ObjectGL::updateDataBuffer(const std::vector<glm::vec3>& vertices)
{
    assert( VBO != 0 );

    VerticesCount = 0;
    for (const auto& vertex : vertices) {
        DataBuffer.push_back( vertex.x );
        DataBuffer.push_back( vertex.y );
        DataBuffer.push_back( vertex.z );
        VerticesCount++;
    }
    glNamedBufferSubData( VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * DataBuffer.size()), DataBuffer.data() );
    DataBuffer.clear();
}

void ObjectGL::updateDataBuffer(const std::vector<glm::vec3>& vertices, const std::vector<glm::vec3>& normals)
{
    assert( VBO != 0 );

    VerticesCount = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        DataBuffer.push_back( vertices[i].x );
        DataBuffer.push_back( vertices[i].y );
        DataBuffer.push_back( vertices[i].z );
        DataBuffer.push_back( normals[i].x );
        DataBuffer.push_back( normals[i].y );
        DataBuffer.push_back( normals[i].z );
        VerticesCount++;
    }
    glNamedBufferSubData( VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * DataBuffer.size()), DataBuffer.data() );
    DataBuffer.clear();
}

void ObjectGL::updateDataBuffer(
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec3>& normals,
    const std::vector<glm::vec2>& textures
)
{
    assert( VBO != 0 );

    VerticesCount = 0;
    for (size_t i = 0; i < vertices.size(); ++i) {
        DataBuffer.push_back( vertices[i].x );
        DataBuffer.push_back( vertices[i].y );
        DataBuffer.push_back( vertices[i].z );
        DataBuffer.push_back( normals[i].x );
        DataBuffer.push_back( normals[i].y );
        DataBuffer.push_back( normals[i].z );
        DataBuffer.push_back( textures[i].x );
        DataBuffer.push_back( textures[i].y );
        VerticesCount++;
    }
    glNamedBufferSubData( VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * DataBuffer.size()), DataBuffer.data() );
    DataBuffer.clear();
}

void ObjectGL::updateTexture(const uint8_t* image_buffer, int index, int width, int height, GLenum format) const
{
    glTextureSubImage2D(
        TextureID[index], 0, 0, 0,
        width, height,
        format,
        GL_UNSIGNED_BYTE,
        image_buffer
    );
}

void ObjectGL::updateCubeTextures(const std::array<uint8_t*, 6>& textures, int width, int height)
{
    for (int i = 0; i < 6; ++i) {
        glTexSubImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, 0, 0,
            width, height,
            GL_BGRA,
            GL_UNSIGNED_BYTE,
            textures[i]
        );
    }
}

void ObjectGL::replaceVertices(
    const std::vector<glm::vec3>& vertices,
    bool normals_exist,
    bool textures_exist
)
{
    assert( VBO != 0 );

    VerticesCount = 0;
    int step = 3;
    if (normals_exist) step += 3;
    if (textures_exist) step += 2;
    for (size_t i = 0; i < vertices.size(); ++i) {
        DataBuffer[i * step] = vertices[i].x;
        DataBuffer[i * step + 1] = vertices[i].y;
        DataBuffer[i * step + 2] = vertices[i].z;
        VerticesCount++;
    }
    glNamedBufferSubData(
        VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * VerticesCount * step), DataBuffer.data()
    );
}

void ObjectGL::replaceVertices(
    const std::vector<float>& vertices,
    bool normals_exist,
    bool textures_exist
)
{
    assert( VBO != 0 );

    VerticesCount = 0;
    int step = 3;
    if (normals_exist) step += 3;
    if (textures_exist) step += 2;
    for (size_t i = 0, j = 0; i < vertices.size(); i += 3, ++j) {
        DataBuffer[j * step] = vertices[i];
        DataBuffer[j * step + 1] = vertices[i + 1];
        DataBuffer[j * step + 2] = vertices[i + 2];
        VerticesCount++;
    }
    glNamedBufferSubData(
        VBO, 0, static_cast<GLsizeiptr>(sizeof( GLfloat ) * VerticesCount * step), DataBuffer.data()
    );
}

bool ObjectGL::readObjectFile(std::vector<glm::vec3>& vertices, const std::string& file_path)
{
    std::ifstream file( file_path );
    if (!file.is_open()) {
        std::cout << "The object file is not correct.\n";
        return false;
    }

    std::vector<glm::vec3> vertex_buffer;
    std::vector<int> vertex_indices;
    while (!file.eof()) {
        std::string word;
        file >> word;

        if (word == "v") {
            glm::vec3 vertex;
            file >> vertex.x >> vertex.y >> vertex.z;
            vertex_buffer.emplace_back( vertex );
        }
        else if (word == "f") {
            std::string line;
            std::getline( file, line );

            const std::regex delimiter( "[ /]" );
            const std::sregex_token_iterator it( line.begin() + 1, line.end(), delimiter, -1 );
            const std::vector<std::string> n( it, std::sregex_token_iterator() );

            assert( n.size() == 3 );

            vertex_indices.emplace_back( std::stof( n[0] ) );
            vertex_indices.emplace_back( std::stof( n[1] ) );
            vertex_indices.emplace_back( std::stof( n[2] ) );
        }
        else std::getline( file, word );
    }

    for (const auto i : vertex_indices) {
        vertices.emplace_back( vertex_buffer[i - 1] );
    }
    return true;
}

bool ObjectGL::readObjectFile(
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& textures,
    const std::string& file_path
)
{
    std::ifstream file( file_path );
    if (!file.is_open()) {
        std::cout << "The object file is not correct.\n";
        return false;
    }

    std::vector<glm::vec3> vertex_buffer, normal_buffer;
    std::vector<glm::vec2> texture_buffer;
    std::vector<int> vertex_indices, normal_indices, texture_indices;
    while (!file.eof()) {
        std::string word;
        file >> word;

        if (word == "v") {
            glm::vec3 vertex;
            file >> vertex.x >> vertex.y >> vertex.z;
            vertex_buffer.emplace_back( vertex );
        }
        else if (word == "vt") {
            glm::vec2 uv;
            file >> uv.x >> uv.y;
            texture_buffer.emplace_back( uv );
        }
        else if (word == "vn") {
            glm::vec3 normal;
            file >> normal.x >> normal.y >> normal.z;
            normal_buffer.emplace_back( normal );
        }
        else if (word == "f") {
            std::string line;
            std::getline( file, line );

            const std::regex delimiter( "[ /]" );
            const std::sregex_token_iterator it( line.begin() + 1, line.end(), delimiter, -1 );
            const std::vector<std::string> n( it, std::sregex_token_iterator() );

            assert( n.size() == 9 );

            vertex_indices.emplace_back( std::stof( n[0] ) );
            texture_indices.emplace_back( std::stof( n[1] ) );
            normal_indices.emplace_back( std::stof( n[2] ) );
            vertex_indices.emplace_back( std::stof( n[3] ) );
            texture_indices.emplace_back( std::stof( n[4] ) );
            normal_indices.emplace_back( std::stof( n[5] ) );
            vertex_indices.emplace_back( std::stof( n[6] ) );
            texture_indices.emplace_back( std::stof( n[7] ) );
            normal_indices.emplace_back( std::stof( n[8] ) );
        }
        else std::getline( file, word );
    }

    for (uint i = 0; i < vertex_indices.size(); ++i) {
        vertices.emplace_back( vertex_buffer[vertex_indices[i] - 1] );
        normals.emplace_back( normal_buffer[normal_indices[i] - 1] );
        textures.emplace_back( texture_buffer[texture_indices[i] - 1] );
    }
    return true;
}

bool ObjectGL::readTextFile(
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec3>& normals,
    const std::string& file_path
)
{
    std::ifstream file( file_path );
    if (!file.is_open()) {
        std::cout << "The object file is not correct.\n";
        return false;
    }

    int polygon_num;
    file >> polygon_num;

    const int vertex_num = polygon_num * 3;
    vertices.resize( vertex_num );
    normals.resize( vertex_num );
    for (int i = 0; i < polygon_num; ++i) {
        int triangle_vertex_num;
        file >> triangle_vertex_num;
        for (int v = 0; v < triangle_vertex_num; ++v) {
            const int index = i * triangle_vertex_num + v;
            file >> vertices[index].x >> vertices[index].y >> vertices[index].z;
            file >> normals[index].x >> normals[index].y >> normals[index].z;
        }
    }
    file.close();
    return true;
}

bool ObjectGL::readTextFile(
    std::vector<glm::vec3>& vertices,
    std::vector<glm::vec3>& normals,
    std::vector<glm::vec2>& textures,
    const std::string& file_path
)
{
    std::ifstream file( file_path );
    if (!file.is_open()) {
        std::cout << "The object file is not correct.\n";
        return false;
    }

    int polygon_num;
    file >> polygon_num;

    const int vertex_num = polygon_num * 3;
    vertices.resize( vertex_num );
    normals.resize( vertex_num );
    textures.resize( vertex_num );
    for (int i = 0; i < polygon_num; ++i) {
        int triangle_vertex_num;
        file >> triangle_vertex_num;
        for (int v = 0; v < triangle_vertex_num; ++v) {
            const int index = i * triangle_vertex_num + v;
            file >> vertices[index].x >> vertices[index].y >> vertices[index].z;
            file >> normals[index].x >> normals[index].y >> normals[index].z;
            file >> textures[index].x >> textures[index].y;
        }
    }
    file.close();
    return true;
}

void ObjectGL::calculateTangents(
    std::vector<glm::vec3>& tangents,
    const std::vector<glm::vec3>& vertices,
    const std::vector<glm::vec2>& textures
)
{
    for (size_t i = 0; i < vertices.size(); i += 3) {
        const glm::vec3 edge1 = vertices[i + 1] - vertices[i];
        const glm::vec3 edge2 = vertices[i + 2] - vertices[i];
        const glm::vec2 delta_uv1 = textures[i + 1] - textures[i];
        const glm::vec2 delta_uv2 = textures[i + 2] - textures[i];
        const glm::vec3 tangent = normalize(
            (delta_uv2.y * edge1 - delta_uv1.y * edge2) / (delta_uv1.x * delta_uv2.y - delta_uv1.y * delta_uv2.x)
        );
        tangents.emplace_back( tangent );
        tangents.emplace_back( tangent );
        tangents.emplace_back( tangent );
    }
}