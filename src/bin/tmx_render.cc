#include <cstdio>
#include <cstdlib>

#include <boost/filesystem.hpp>

#include <QPainter>
#include <QCache>

#include <tmx/Map.h>
#include <tmx/TileLayer.h>
#include <tmx/ObjectLayer.h>

namespace fs = boost::filesystem;

class LayerRenderer : public tmx::LayerVisitor {
public:
  void renderMap(const fs::path& mapPath) {
    auto map = tmx::Map::parseFile(mapPath);

    if (!map) {
      return;
    }

    if (map->getOrientation() != tmx::Orientation::ORTHOGONAL) {
      std::printf("Can render only orthogonal maps. Exiting.\n");
      return;
    }

    auto tilewidth = map->getTileWidth();
    assert(tilewidth);

    auto tileheight = map->getTileHeight();
    assert(tileheight);

    auto width = map->getWidth();
    assert(width);

    auto height = map->getHeight();
    assert(height);

    // create surface

    QImage image(width * tilewidth, height * tileheight, QImage::Format_ARGB32);
    painter.begin(&image);
    map->visitLayers(*this);
    painter.end();

    std::printf("Saving image...\n");
    image.save("map.png");
  }

private:
  QPainter painter;
  QCache<QString, QImage> cache;

  const QImage getTexture(const fs::path& path) {
    QString str(path.string().c_str());
    QImage *img = cache.object(str);

    if (img != nullptr) {
      return *img;
    }

    img = new QImage(str);
    assert(!img->isNull());

    cache.insert(str, img);
    return *img;
  }

  enum class Alignment {
    TOP_LEFT,
    BOTTOM_LEFT,
  };

  void drawGID(const tmx::Map& map, const QPoint& origin, unsigned gid, Alignment align) {
    auto tileset = map.getTileSetFromGID(gid);
    assert(tileset);
    gid = gid - tileset->getFirstGID();

    if (tileset->hasImage()) {

      auto image = tileset->getImage();
      assert(image);

      const QImage texture = getTexture(image->getSource());

      tmx::Size size;

      if (image->hasSize()) {
        size = image->getSize();
      } else {
        QSize textureSize = texture.size();
        assert(textureSize.width() >= 0);
        assert(textureSize.height() >= 0);
        size.width = textureSize.width();
        size.height = textureSize.height();
      }

      tmx::Rect rect = tileset->getCoords(gid, size);
      QPoint offset;

      if (align == Alignment::BOTTOM_LEFT) {
        offset.ry() -= rect.height;
      }

      painter.drawImage(origin + offset, texture, QRect(rect.x, rect.y, rect.width, rect.height));

    } else {

      auto tile = tileset->getTile(gid);
      assert(tile);
      assert(tile->hasImage());

      auto image = tile->getImage();
      assert(image);

      const QImage texture = getTexture(image->getSource());
      painter.drawImage(origin, texture);

    }
  }

public:
  virtual void visitTileLayer(const tmx::Map& map, const tmx::TileLayer& layer) override {
    if (!layer.isVisible()) {
      return;
    }

    std::printf("Rendering tile layer '%s'.\n", layer.getName().c_str());

    unsigned k = 0;
    for (auto cell : layer) {
      unsigned i = k % map.getWidth();
      unsigned j = k / map.getWidth();
      assert(j < map.getHeight());

      QPoint origin(i * map.getTileWidth(), j * map.getTileHeight());

      unsigned gid = cell.getGID();

      if (gid != 0) {
        drawGID(map, origin, gid, Alignment::TOP_LEFT);
      }

      k++;
    }

  }

  virtual void visitObjectLayer(const tmx::Map& map, const tmx::ObjectLayer &layer) override {
    if (!layer.isVisible()) {
      return;
    }

    std::printf("Rendering object layer '%s'.\n", layer.getName().c_str());

    for (auto obj : layer) {
      if (!obj->isTile()) {
        continue;
      }

      auto tile = static_cast<const tmx::TileObject *>(obj);

      QPoint origin(tile->getX(), tile->getY());

      unsigned gid = tile->getGID();
      assert(gid != 0);

      drawGID(map, origin, gid, Alignment::BOTTOM_LEFT);
    }

  }


};

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::printf("Usage: tmx_render <file.tmx>\n");
    return 1;
  }

  fs::path mapPath(argv[1]);

  LayerRenderer renderer;
  renderer.renderMap(mapPath);

  return 0;
}
