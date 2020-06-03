
#ifndef PARTIAL_IMAGE_HPP_INCLUDED
#define PARTIAL_IMAGE_HPP_INCLUDED

#include "../dgl/ImageBase.hpp"
#include "../dgl/OpenGL.hpp"

START_NAMESPACE_DGL

// -----------------------------------------------------------------------

/**
   OpenGL Image class.

   This is an Image class that handles raw image data in pixels.
   You can init the image data on the contructor or later on by calling loadFromMemory().

   To generate raw data useful for this class see the utils/png2rgba.py script.
   Be careful when using a PNG without alpha channel, for those the format is 'GL_BGR'
   instead of the default 'GL_BGRA'.

   Images are drawn on screen via 2D textures.
 */
class PartialImage : public ImageBase
{
public:
   /**
      Constructor for a null Image.
    */
    PartialImage();

   /**
      Constructor using raw image data.
      @note @a rawData must remain valid for the lifetime of this Image.
    */
    PartialImage(const char* const rawData,
          const uint width,
          const uint height,
          const GLenum format = GL_BGRA,
          const GLenum type = GL_UNSIGNED_BYTE);

   /**
      Constructor using raw image data.
      @note @a rawData must remain valid for the lifetime of this Image.
    */
    PartialImage(const char* const rawData,
          const Size<uint>& size,
          const GLenum format = GL_BGRA,
          const GLenum type = GL_UNSIGNED_BYTE);

   /**
      Constructor using another image data.
    */
    PartialImage(const PartialImage& image);

   /**
      Destructor.
    */
    ~PartialImage() override;

   /**
      Load image data from memory.
      @note @a rawData must remain valid for the lifetime of this Image.
    */
    void loadFromMemory(const char* const rawData,
                        const uint width,
                        const uint height,
                        const GLenum format = GL_BGRA,
                        const GLenum type = GL_UNSIGNED_BYTE) noexcept;

   /**
      Load image data from memory.
      @note @a rawData must remain valid for the lifetime of this Image.
    */
    void loadFromMemory(const char* const rawData,
                        const Size<uint>& size,
                        const GLenum format = GL_BGRA,
                        const GLenum type = GL_UNSIGNED_BYTE) noexcept;

   /**
      Get the image format.
    */
    GLenum getFormat() const noexcept;

   /**
      Get the image type.
    */
    GLenum getType() const noexcept;

   /**
      TODO document this.
    */
    PartialImage& operator=(const PartialImage& image) noexcept;

    float xScale, yScale;

protected:
   /** @internal */
    void _drawAt(const Point<int>& pos) override;

private:
    GLenum fFormat;
    GLenum fType;
    GLuint fTextureId;
    bool fIsReady;
};

// -----------------------------------------------------------------------

END_NAMESPACE_DGL

#endif // PARTIAL_IMAGE_HPP_INCLUDED
