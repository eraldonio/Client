#include "drmsetupwizard.h"
#include "../../steam/steamhelper.h"
#include "../stack/library.h"
#include "../../database.h"
#include "../../defines.h"

#include <QDir>

/** DRMSetupWizard constructor
 */
DRMSetupWizard::DRMSetupWizard(QWidget* parent, Qt::WindowFlags flags)
{
    //addPage(new StartPage());
    addPage(new DRMsFoundPage());
    addPage(new GamesFoundPage());

    setWindowTitle(tr("DRM Setup Wizard"));
}

StartPage::StartPage(QWidget* parent)
{
    setTitle(tr("Welcome"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* label = new QLabel(tr("This wizard will help you add all of your games to Horizon."));
    label->setWordWrap(true);
    layout->addWidget(label);
}

DRMsFoundPage::DRMsFoundPage(QWidget* parent)
{
    setTitle(tr("DRMs Found"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QString steamFoundText;
    if (SteamHelper::doesExist())
    {
        steamFoundText = tr("Steam was found.");
    }
    else
    {
        steamFoundText = tr("Steam was not found.");
    }
    QLabel* label = new QLabel(steamFoundText);
    label->setWordWrap(true);
    layout->addWidget(label);
}

GamesFoundPage::GamesFoundPage(QWidget* parent)
{
    setTitle(tr("Games Found"));

    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* infoLabel = new QLabel(tr("The following games will be added:"));
    infoLabel->setStyleSheet("font-size: 14px; font-weight: 600;");
    layout->addWidget(infoLabel);

    QWidget* gamesWidget = new QWidget();
    gamesLayout = new QVBoxLayout(gamesWidget);

    QScrollArea* scrollArea = new QScrollArea();
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setWidgetResizable(true);
    scrollArea->setWidget(gamesWidget);
    scrollArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    layout->addWidget(scrollArea);
}

void GamesFoundPage::initializePage()
{
    QMap<QString, QString> gamesList = SteamHelper::getGames();

    for (QString game : gamesList.keys())
    {
        if (Library::db.isExistant(game).first) { continue; }
        addGames[game] = gamesList[game];
        gamesLayout->addWidget(new QLabel(game));
    }
}

bool GamesFoundPage::validatePage()
{
    for (QString game : addGames.keys())
    {
        #if defined(__linux__)
            QString openWith = "xdg-open";
        #elif defined(_WIN32) || defined(_WIN64)
            QString openWith = "explorer.exe";
        #elif defined(__APPLE__)
            QString openWith = "open";
        #endif
        Game gameObj = Game {0,
                             game,
                             ".",
                             openWith,
                             "steam://run/" + addGames[game],
                             1};
        Library::db.addGame(gameObj);

        QString bannerPath = CONFIG_FOLDER + QDir::separator() + "image" + QDir::separator() + "steam_" + addGames[game] + "_banner.png";
        SteamMetadataHelper* helper = new SteamMetadataHelper(addGames[game]);
        connect(helper, &SteamMetadataHelper::metadataRecieved, [=] (SteamMetadata metadata)
        {
            onMetadataRecieved(game, metadata);
        });
        connect(helper, &SteamMetadataHelper::headerDownloadCompleted, [=]
        {
            onHeaderDLCompleted(game, bannerPath);
        });
        helper->getMetadata();
        helper->downloadHeader(bannerPath);
    }
}

void GamesFoundPage::onMetadataRecieved(QString gameName, SteamMetadata metadata)
{
    Game game = Library::db.getGameByName(gameName);
    game.developer = metadata.developer;
    game.publisher = metadata.publisher;
    game.releaseDate = metadata.releaseDate;
    game.genre = metadata.genres;
    Library::db.updateGame(game);
}

void GamesFoundPage::onHeaderDLCompleted(QString gameName, QString bannerPath)
{
    Game game = Library::db.getGameByName(gameName);
    game.bannerPath = bannerPath;
    Library::db.updateGame(game);
}
