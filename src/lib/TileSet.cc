/*
 * Copyright (c) 2013-2014, Julien Bernard
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */
#include <tmx/TileSet.h>

namespace tmx {

  const Tile *TileSet::getTile(unsigned id) const noexcept {
    for (auto tile : *this) {
      if (tile->getId() == id) {
        return tile;
      }
    }

    return nullptr;
  }

  Rect TileSet::getCoords(unsigned id, Size size) const noexcept {
    unsigned width = (size.width - 2 * m_margin + m_spacing) / (m_tilewidth + m_spacing); // number of tiles
    unsigned height = (size.height - 2 * m_margin + m_spacing) / (m_tileheight + m_spacing); // number of tiles

    unsigned tu = id % width;
    unsigned tv = id / width;
    assert(tv < height);

    unsigned du = m_margin + tu * m_spacing + m_x;
    unsigned dv = m_margin + tv * m_spacing + m_y;
    assert((tu + 1) * m_tilewidth + du <= size.width);
    assert((tv + 1) * m_tileheight + dv <= size.height);

    return { tu * m_tilewidth + du, tv * m_tileheight + dv, m_tilewidth, m_tileheight };
  }

}
