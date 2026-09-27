/**
  \file     LH_QtPlugin_Weather.h
  @author   Andy "Triscopic" Bridges <triscopic@codeleap.co.uk>
  Copyright (c) 2010 Andrew Bridges

    Permission is hereby granted, free of charge, to any person obtaining a copy
    of this software and associated documentation files (the "Software"), to deal
    in the Software without restriction, including without limitation the rights
    to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
    copies of the Software, and to permit persons to whom the Software is
    furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included in
    all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
    AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
    OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
    THE SOFTWARE.

  */

#ifndef LH_QTPLUGIN_WEATHER_H
#define LH_QTPLUGIN_WEATHER_H

#include "LH_QtPlugin.h"

#include <QDateTime>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPointer>
#include <QUrl>

#include "LH_Qt_QString.h"
#include "LH_Qt_QStringList.h"
#include "LH_Qt_int.h"
#include "LH_Qt_bool.h"

#include "LH_WeatherData.h"
#include "SimpleTranslator.h"

// Weather data comes from Open-Meteo (https://open-meteo.com), a free service
// that needs no API key. Results are converted to the data model and weather
// codes of the former Yahoo! Weather feed, so that existing layouts and image
// maps keep working.
class LH_QtPlugin_Weather : public LH_QtPlugin
{
    Q_OBJECT

    weatherData weather_data;
    QDateTime lastrefresh_;

    QNetworkAccessManager nam_;
    QPointer<QNetworkReply> geocodeReply_;
    QPointer<QNetworkReply> forecastReply_;

    // Location lookup state: the name being searched and the words used to
    // pick the best match (country, region...).
    QString geocodeName_;
    QStringList geocodeHints_;

    QNetworkReply *get(const QUrl &url);
    void geocode(const QString &name);
    void geocodeFinished(QNetworkReply *reply);
    void forecastFinished(QNetworkReply *reply);
    bool isMetric() const;
    void parseForecast(const QJsonObject &json);
    void setNoForecast(forecastData &forecast);
    void publish();
    void requestTranslation();

protected:
    LH_Qt_QString *setup_location_name_;
    LH_Qt_QString *setup_coordinates_;
    LH_Qt_QString *setup_city_;
    LH_Qt_int *setup_refresh_;
    LH_Qt_QStringList *setup_units_type_;

    LH_Qt_QStringList *setup_languages_;
    LH_Qt_QString *setup_language_;
    LH_Qt_bool *setup_show_all_languages_;

    LH_Qt_QString *setup_json_weather_;

    SimpleTranslator translator;

public:
    LH_QtPlugin_Weather();

    const char *userInit();
    int notify(int code, void *param);

public slots:
    void lookupLocation();
    void fetchForecast();

    void updateLanguagesList();
    void selectLanguage();
    void setLanguage();
};

#endif // LH_QTPLUGIN_WEATHER_H
