#include "NewsItemWidget.h"
#include <QVBoxLayout>
#include <QDesktopServices>
#include <QUrl>
#include <QFontDatabase>

/**
 * NewsItemWidget constructor
 * \param p The color palette for the UI
 * \param URL The URL to the RSS article
 * \param source The RSS feed the headline came from
 * \param title The headline to be displayed
 * \param parent The parent QWidget
 */
NewsItemWidget::NewsItemWidget(QSettings* p, QString URL, QString source, QString title, QString descriptionString, QWidget* parent) : QWidget(parent)
{

    this->setObjectName("NewsItemWidget");
   // this->setMaximumHeight(200);
    this->palette = p;
    this->setStyleSheet(
                "#imageLabel {"
                "border-top-right-radius: 4px; "
                "border-top-left-radius: 4px; "
                "}"
                "#sourceLabel {"
                "border-bottom-right-radius: 4px; "
                "border-bottom-left-radius: 4px; "
                "}"
                "QLabel {"
                "padding: 3px; "
                "color: " + p->value("Primary/LightText").toString() + ";"
                "}");
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setSpacing(0);

    imageLabel = new QLabel (""); //TODO: Change this to default image
    imageLabel->setObjectName("imageLabel");
    imageLabel->setContentsMargins(5,5,5,5);
    layout->addWidget(imageLabel);

    titleLabel = new QLabel (title);
    QFont titleFont("Gill Sans MT", 12, 5);\
    titleLabel->setFont(titleFont);
    titleLabel->setContentsMargins(0,0,5,5);
    titleLabel->setWordWrap(true);
    layout->addWidget(titleLabel);


    descriptionLabel = new QLabel(descriptionString);
    descriptionLabel->setContentsMargins(0,0,5,5);
    descriptionLabel->setWordWrap(true);
    layout->addWidget(descriptionLabel);

    sourceLabel = new QLabel (source);
    sourceLabel->setContentsMargins(0,0,5,5);
    sourceLabel->setWordWrap(true);
    sourceLabel->setObjectName("sourceLabel");
    layout->addWidget(sourceLabel);

    this->urlString = URL;
    this->descriptionString = descriptionString;

    //Load thumbnail
    manager = new QNetworkAccessManager(this);
    const QUrl url(this->urlString);
    QNetworkRequest* req = new QNetworkRequest(url);
    req->setRawHeader("User-Agent", "Horizon Launcher");
    QNetworkReply* reply = manager->get(*req);
    connect (reply, &QNetworkReply::finished, [=] {
        if(reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt() == 301)
        {
          QString newUrl = reply->attribute(QNetworkRequest::RedirectionTargetAttribute).toString();
          QNetworkRequest* req = new QNetworkRequest(newUrl);
          req->setRawHeader("User-Agent", "Horizon Launcher");
          QNetworkReply* newReply = manager->get(*req);
          connect(newReply, &QNetworkReply::finished, [=] {
              sourceCodeFetchComplete(newReply);
          });
        }

        else
        {
            sourceCodeFetchComplete(reply);
        }
    });
}

void NewsItemWidget::mousePressEvent(QMouseEvent *)
{
    QDesktopServices::openUrl(QUrl(this->urlString));
}

/**
 * Called when the source HTML is fetched for a webpage.
 * Retrieves the thumbnail for the page using the OpenGraph image property, if
 * defined in the source
 * @param reply The network reply containing the HTML source
 */
void NewsItemWidget::sourceCodeFetchComplete(QNetworkReply* reply)
{
    QString htmlSource = reply->readAll();

    //TODO: Get rid of magic numbers here LEARN REGEX
    int startIndex = htmlSource.indexOf("og:image") + 19;
    int endIndex = htmlSource.indexOf("jpg", startIndex);
    QString imgUrl =  htmlSource.mid(startIndex, endIndex - startIndex + 3);

    const QUrl url(imgUrl);
    QNetworkRequest* req = new QNetworkRequest(url);
    req->setRawHeader("User-Agent", "Horizon Launcher");
    QNetworkReply* imageReply = manager->get(*req);
    connect (imageReply, &QNetworkReply::finished, [=]
    {
        QByteArray jpegData = imageReply->readAll();
        QPixmap pixmap;
        pixmap.loadFromData(jpegData);
        if (!pixmap) { return; }
        imageLabel->setPixmap(pixmap.scaledToWidth(imageLabel->width()));
    });
}

void NewsItemWidget::enterEvent(QEvent* event)
{
   titleLabel->setStyleSheet( "background-color: " + this->palette->value("Primary/HoverSelection").toString() + ";");
   descriptionLabel->setStyleSheet("background-color: " + this->palette->value("Primary/HoverSelection").toString() + ";");
   sourceLabel->setStyleSheet("background-color: " + this->palette->value("Primary/HoverSelection").toString() + ";");
   imageLabel->setStyleSheet("background-color: " + this->palette->value("Primary/HoverSelection").toString() + ";");
   this->setCursor(Qt::PointingHandCursor);

}

void NewsItemWidget::leaveEvent(QEvent* event)
{
   titleLabel->setStyleSheet("background-color: " + this->palette->value("Primary/SecondaryBase").toString() + ";");
   descriptionLabel->setStyleSheet("background-color: " + this->palette->value("Primary/SecondaryBase").toString() + ";");
   sourceLabel->setStyleSheet("background-color: " + this->palette->value("Primary/SecondaryBase").toString() + ";");
   imageLabel->setStyleSheet("background-color: " + this->palette->value("Primary/SecondaryBase").toString() + ";");
   this->setCursor(Qt::ArrowCursor);

}
