#include "canvas.h"

void CanvasGL::deleteAllTextures()
{
    if (COLOR0TextureID != 0) {
        glDeleteTextures( 1, &COLOR0TextureID );
        COLOR0TextureID = 0;
    }
    if (COLOR1TextureID != 0) {
        glDeleteTextures( 1, &COLOR1TextureID );
        COLOR1TextureID = 0;
    }
    if (StencilTextureID != 0) {
        glDeleteTextures( 1, &StencilTextureID );
        StencilTextureID = 0;
    }
    if (CanvasID != 0) {
        glDeleteFramebuffers( 1, &CanvasID );
        CanvasID = 0;
    }
}

void CanvasGL::setCanvas(int width, int height, GLenum format, bool use_stencil)
{
    deleteAllTextures();

    glCreateTextures( GL_TEXTURE_2D, 1, &COLOR0TextureID );
    glTextureStorage2D( COLOR0TextureID, 1, format, width, height );
    glTextureParameteri( COLOR0TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTextureParameteri( COLOR0TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glGenerateTextureMipmap( COLOR0TextureID );

    glCreateFramebuffers( 1, &CanvasID );
    glNamedFramebufferTexture( CanvasID, GL_COLOR_ATTACHMENT0, COLOR0TextureID, 0 );

    if (use_stencil) {
        glCreateTextures( GL_TEXTURE_2D, 1, &StencilTextureID );
        glTextureStorage2D( StencilTextureID, 1, GL_STENCIL_INDEX8, width, height );
        glTextureParameteri( StencilTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTextureParameteri( StencilTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glGenerateTextureMipmap( StencilTextureID );

        glNamedFramebufferTexture( CanvasID, GL_STENCIL_ATTACHMENT, StencilTextureID, 0 );
    }

    glCheckNamedFramebufferStatus( CanvasID, GL_FRAMEBUFFER );
}

void CanvasGL::setCanvasWithDoubleDrawBuffers(int width, int height, GLenum format, bool use_stencil)
{
    deleteAllTextures();

    glCreateTextures( GL_TEXTURE_2D, 1, &COLOR0TextureID );
    glTextureStorage2D( COLOR0TextureID, 1, format, width, height );
    glTextureParameteri( COLOR0TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTextureParameteri( COLOR0TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glGenerateTextureMipmap( COLOR0TextureID );

    glCreateTextures( GL_TEXTURE_2D, 1, &COLOR1TextureID );
    glTextureStorage2D( COLOR1TextureID, 1, format, width, height );
    glTextureParameteri( COLOR1TextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
    glTextureParameteri( COLOR1TextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
    glGenerateTextureMipmap( COLOR1TextureID );

    glCreateFramebuffers( 1, &CanvasID );
    glNamedFramebufferTexture( CanvasID, GL_COLOR_ATTACHMENT0, COLOR0TextureID, 0 );
    glNamedFramebufferTexture( CanvasID, GL_COLOR_ATTACHMENT1, COLOR1TextureID, 0 );
    constexpr GLenum draw_buffers[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
    glNamedFramebufferDrawBuffers( CanvasID, 2, draw_buffers );

    if (use_stencil) {
        glCreateTextures( GL_TEXTURE_2D, 1, &StencilTextureID );
        glTextureStorage2D( StencilTextureID, 1, GL_STENCIL_INDEX8, width, height );
        glTextureParameteri( StencilTextureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR );
        glTextureParameteri( StencilTextureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR );
        glGenerateTextureMipmap( StencilTextureID );

        glNamedFramebufferTexture( CanvasID, GL_STENCIL_ATTACHMENT, StencilTextureID, 0 );
    }

    glCheckNamedFramebufferStatus( CanvasID, GL_FRAMEBUFFER );
}

void CanvasGL::setMultiSampledCanvas(int width, int height, int sample_num, GLenum format, bool use_stencil)
{
    deleteAllTextures();

    glCreateTextures( GL_TEXTURE_2D_MULTISAMPLE, 1, &COLOR0TextureID );
    glTextureStorage2DMultisample( COLOR0TextureID, sample_num, format, width, height, GL_TRUE );

    glCreateFramebuffers( 1, &CanvasID );
    glNamedFramebufferTexture( CanvasID, GL_COLOR_ATTACHMENT0, COLOR0TextureID, 0 );

    if (use_stencil) {
        glCreateTextures( GL_TEXTURE_2D_MULTISAMPLE, 1, &StencilTextureID );
        glTextureStorage2DMultisample(
            StencilTextureID,
            sample_num,
            GL_STENCIL_INDEX8,
            width,
            height,
            GL_TRUE
        );

        glNamedFramebufferTexture( CanvasID, GL_STENCIL_ATTACHMENT, StencilTextureID, 0 );
    }

    glCheckNamedFramebufferStatus( CanvasID, GL_FRAMEBUFFER );
}