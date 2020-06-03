/*
 * DISTRHO Plugin Framework (DPF)
 * Copyright (C) 2012-2019 Filipe Coelho <falktx@falktx.com>
 *
 * Permission to use, copy, modify, and/or distribute this software for any purpose with
 * or without fee is hereby granted, provided that the above copyright notice and this
 * permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES WITH REGARD
 * TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS. IN
 * NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL
 * DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER
 * IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF OR IN
 * CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include "PartialImage.hpp"

START_NAMESPACE_DGL

// -----------------------------------------------------------------------

PartialImage::PartialImage(const char* const rawData, const uint width, const uint height, const GLenum format, const GLenum type)
    : ImageBase(rawData, width, height),
      fFormat(format),
      fType(type),
      fTextureId(0),
      fIsReady(false)
{
    glGenTextures(1, &fTextureId);
}

PartialImage::~PartialImage()
{
    if (fTextureId != 0)
    {
#ifndef DISTRHO_OS_MAC // FIXME
        glDeleteTextures(1, &fTextureId);
#endif
        fTextureId = 0;
    }
}

void PartialImage::_drawAt(const Point<int>& pos)
{
    if (fTextureId == 0 || ! isValid())
        return;

    glEnable(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, fTextureId);

    if (! fIsReady)
    {
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

        static const float trans[] = { 0.0f, 0.0f, 0.0f, 0.0f };
        glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, trans);

        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     static_cast<GLsizei>(fSize.getWidth()), static_cast<GLsizei>(fSize.getHeight()), 0,
                     fFormat, fType, fRawData);

        fIsReady = true;
    }

    unsigned int height = fSize.getHeight() * yScale;
    if (height > 0 && height <= fSize.getHeight()) {
        Rectangle<int>(pos, static_cast<int>(fSize.getWidth()), static_cast<int>(height)).draw();
    }

    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_TEXTURE_2D);
}

// -----------------------------------------------------------------------

END_NAMESPACE_DGL
