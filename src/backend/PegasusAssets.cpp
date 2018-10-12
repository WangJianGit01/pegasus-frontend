// Pegasus Frontend
// Copyright (C) 2017  Mátyás Mustoha
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program. If not, see <http://www.gnu.org/licenses/>.


#include "PegasusAssets.h"

#include "model/gaming/Game.h"

#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QStringBuilder>
#include <QUrl>


namespace {

static const HashMap<QString, AssetType> TYPE_BY_EXT = {
    { QStringLiteral("png"), AssetType::BOX_FRONT },
    { QStringLiteral("jpg"), AssetType::BOX_FRONT },
    { QStringLiteral("webm"), AssetType::VIDEOS },
    { QStringLiteral("mp4"), AssetType::VIDEOS },
    { QStringLiteral("avi"), AssetType::VIDEOS },
    { QStringLiteral("mp3"), AssetType::MUSIC },
    { QStringLiteral("ogg"), AssetType::MUSIC },
    { QStringLiteral("wav"), AssetType::MUSIC },
};

} // namespace


namespace pegasus_assets {

AssetType str_to_type(const QString& str)
{
    static const HashMap<QString, const AssetType> map {
        { QStringLiteral("boxfront"), AssetType::BOX_FRONT },
        { QStringLiteral("boxFront"), AssetType::BOX_FRONT },
        { QStringLiteral("box_front"), AssetType::BOX_FRONT },
        { QStringLiteral("boxart2D"), AssetType::BOX_FRONT },
        { QStringLiteral("boxart2d"), AssetType::BOX_FRONT },

        { QStringLiteral("boxback"), AssetType::BOX_BACK },
        { QStringLiteral("boxBack"), AssetType::BOX_BACK },
        { QStringLiteral("box_back"), AssetType::BOX_BACK },

        { QStringLiteral("boxspine"), AssetType::BOX_SPINE },
        { QStringLiteral("boxSpine"), AssetType::BOX_SPINE },
        { QStringLiteral("box_spine"), AssetType::BOX_SPINE },

        { QStringLiteral("boxside"), AssetType::BOX_SPINE },
        { QStringLiteral("boxSide"), AssetType::BOX_SPINE },
        { QStringLiteral("box_side"), AssetType::BOX_SPINE },

        { QStringLiteral("boxfull"), AssetType::BOX_FULL },
        { QStringLiteral("boxFull"), AssetType::BOX_FULL },
        { QStringLiteral("box_full"), AssetType::BOX_FULL },
        { QStringLiteral("box"), AssetType::BOX_FULL },

        { QStringLiteral("cartridge"), AssetType::CARTRIDGE },
        { QStringLiteral("disc"), AssetType::CARTRIDGE },
        { QStringLiteral("cart"), AssetType::CARTRIDGE },
        { QStringLiteral("logo"), AssetType::LOGO },
        { QStringLiteral("wheel"), AssetType::LOGO },
        { QStringLiteral("marquee"), AssetType::ARCADE_MARQUEE },
        { QStringLiteral("bezel"), AssetType::ARCADE_BEZEL },
        { QStringLiteral("screenmarquee"), AssetType::ARCADE_BEZEL },
        { QStringLiteral("border"), AssetType::ARCADE_BEZEL },
        { QStringLiteral("panel"), AssetType::ARCADE_PANEL },

        { QStringLiteral("cabinetleft"), AssetType::ARCADE_CABINET_L },
        { QStringLiteral("cabinetLeft"), AssetType::ARCADE_CABINET_L },
        { QStringLiteral("cabinet_left"), AssetType::ARCADE_CABINET_L },

        { QStringLiteral("cabinetright"), AssetType::ARCADE_CABINET_R },
        { QStringLiteral("cabinetRight"), AssetType::ARCADE_CABINET_R },
        { QStringLiteral("cabinet_right"), AssetType::ARCADE_CABINET_R },

        { QStringLiteral("tile"), AssetType::UI_TILE },
        { QStringLiteral("banner"), AssetType::UI_BANNER },
        { QStringLiteral("steam"), AssetType::UI_STEAMGRID },
        { QStringLiteral("steamgrid"), AssetType::UI_STEAMGRID },
        { QStringLiteral("grid"), AssetType::UI_STEAMGRID },
        { QStringLiteral("poster"), AssetType::POSTER },
        { QStringLiteral("flyer"), AssetType::POSTER },
        { QStringLiteral("background"), AssetType::BACKGROUND },
        { QStringLiteral("music"), AssetType::MUSIC },
        { QStringLiteral("screenshot"), AssetType::SCREENSHOTS },
        { QStringLiteral("video"), AssetType::VIDEOS },
    };

    const auto it = map.find(str);
    if (it != map.cend())
        return it->second;

    return AssetType::UNKNOWN;
}

const QStringList& allowed_asset_exts(AssetType type)
{
    static const QStringList empty_list({});
    static const QStringList image_exts = { "png", "jpg" };
    static const QStringList video_exts = { "webm", "mp4", "avi" };
    static const QStringList audio_exts = { "mp3", "ogg", "wav" };

    switch (type) {
        case AssetType::UNKNOWN:
            return empty_list;
        case AssetType::VIDEOS:
            return video_exts;
        case AssetType::MUSIC:
            return audio_exts;
        default:
            return image_exts;
    }
}

bool AssetCheckResult::isValid() const
{
    return asset_type != AssetType::UNKNOWN && !basename.isEmpty();
}

AssetCheckResult checkFile(const QFileInfo& file)
{
    const QString basename = file.completeBaseName();
    const int last_dash = basename.lastIndexOf(QChar('-'));
    const QString suffix = (last_dash == -1)
        ? QString()
        : basename.mid(last_dash + 1);

    const AssetType type = str_to_type(suffix);
    if (type == AssetType::UNKNOWN) {
        // missing/unknown suffix -> guess by extension
        return {
            basename,
            TYPE_BY_EXT.count(file.suffix())
                ? TYPE_BY_EXT.at(file.suffix())
                : AssetType::UNKNOWN,
        };
    }

    const QString game_basename = basename.left(last_dash);
    if (!allowed_asset_exts(type).contains(file.suffix())) {
        // known suffix but wrong extension -> invalid
        return {
            game_basename,
            AssetType::UNKNOWN,
        };
    }

    // known suffix and valid extension
    return {
        game_basename,
        type,
    };
}

void add_asset_to(modeldata::GameAssets& gameassets, AssetType asset_type, const QString& file_path)
{
    const bool is_single = asset_is_single(asset_type);
    QString url = QUrl::fromLocalFile(file_path).toString();

    if (is_single && gameassets.single(asset_type).isEmpty()) {
        gameassets.setSingle(asset_type, std::move(url));
    }
    else if (!is_single && !gameassets.multi(asset_type).contains(url)) {
        gameassets.appendMulti(asset_type, std::move(url));
    }
}

} // namespace pegasus_assets
