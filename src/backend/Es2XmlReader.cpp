#include "Es2XmlReader.h"

#include "Es2GamelistReader.h"
#include "Model.h"
#include "Utils.h"

#include <QDir>
#include <QDebug>


QXmlStreamReader Es2XmlReader::xml;

bool Es2XmlReader::read(QList<Model::Platform*>& platform_list)
{
    QVector<Model::Platform*> platforms = readSystemsFile();
    if (xml.error()) {
        qWarning().noquote() << xml.errorString();
        return false;
    }

    for (auto& platform : platforms) {
        QVector<Model::Game*> games = Es2GamelistReader::read(platform);
        if (xml.error()) {
            qWarning().noquote() << xml.errorString();
            continue;
        }

        for (Model::Game* game : games) {
            Es2GamelistReader::findGameAssets(platform, game);
            platform->m_games.append(game);
        }

        if (platform->m_games.count() > 0)
            platform_list.append(platform);
    }

    return true;
}

QVector<Model::Platform*> Es2XmlReader::readSystemsFile()
{
    QString systemscfg_path = findSystemsCfg();
    if (systemscfg_path.isEmpty()) {
        qWarning().noquote() << QObject::tr("ES2 system config not found");
        return {};
    }

    QFile systemscfg(systemscfg_path);
    if (!systemscfg.open(QIODevice::ReadOnly)) {
        xml.raiseError(QObject::tr("Could not open `%1`").arg(systemscfg_path));
        return {};
    }

    QVector<Model::Platform*> platforms;

    xml.setDevice(&systemscfg);
    if (xml.readNextStartElement()) {
        if (xml.name() != "systemList")
            xml.raiseError(QObject::tr("`%1` does not start with a `<systemList>` node!"));
        else {
            while (xml.readNextStartElement()) {
                if (xml.name() == "system") {
                    Model::Platform* platform = readSystem();
                    if (!platform->m_short_name.isEmpty())
                        platforms.push_back(platform);
                }
                else
                    xml.skipCurrentElement();
            }
        }
    }

    return platforms;
}

QString Es2XmlReader::findSystemsCfg()
{
    static const QString FOUND_MSG = "Found `%1`";
    static const QString FALLBACK_MSG = "`%1` not found, trying next fallback";

    static const QVector<QString> possible_paths = {
        QDir::homePath() + "/.config/emulationstation/es_systems.cfg",
        QDir::homePath() + "/.emulationstation/es_systems.cfg",
        "/etc/emulationstation/es_systems.cfg",
    };

    for (const auto& path : possible_paths) {
        if (validFile(path)) {
            qInfo().noquote() << FOUND_MSG.arg(path);
            return path;
        }
        // qDebug() << FALLBACK_MSG.arg(path);
    }

    return QString();
}

Model::Platform* Es2XmlReader::readSystem()
{
    Q_ASSERT(xml.isStartElement() && xml.name() == "system");

    Model::Platform* platform(new Model::Platform());

    while (xml.readNextStartElement()) {
        if (xml.name() == "name")
            parseSystemShortName(platform);
        else if (xml.name() == "path")
            parseSystemRomDirPath(platform);
        else if (xml.name() == "command")
            parseSystemRunCmd(platform);
        else
            xml.skipCurrentElement();
    }

    return platform;
}

void Es2XmlReader::parseSystemShortName(Model::Platform* platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "name");
    platform->m_short_name = xml.readElementText();
}

void Es2XmlReader::parseSystemRomDirPath(Model::Platform* platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "path");
    platform->m_rom_dir_path = xml.readElementText()
        .replace("\\", "/")
        .replace("~", QDir::homePath());
}

void Es2XmlReader::parseSystemRunCmd(Model::Platform* platform) {
    Q_ASSERT(xml.isStartElement() && xml.name() == "command");
    platform->m_launch_cmd = xml.readElementText();
}
