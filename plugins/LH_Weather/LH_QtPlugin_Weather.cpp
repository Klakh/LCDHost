/**
  \file     LH_QtPlugin_Weather.cpp
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

#include "LH_QtPlugin_Weather.h"

#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QLocale>
#include <QNetworkProxy>
#include <QRegularExpression>
#include <QUrlQuery>

#include <cmath>

LH_PLUGIN(LH_QtPlugin_Weather)

char __lcdhostplugin_xml[] =
"<?xml version=\"1.0\"?>"
"<lcdhostplugin>"
  "<id>Weather</id>"
  "<rev>" STRINGIZE(REVISION) "</rev>"
  "<api>" STRINGIZE(LH_API_MAJOR) "." STRINGIZE(LH_API_MINOR) "</api>"
  "<ver>" STRINGIZE(VERSION) "\nr" STRINGIZE(REVISION) "</ver>"
  "<author>Andy \"Triscopic\" Bridges</author>"
  "<homepageurl><a href=\"https://github.com/LokLakh-s/LCDHost-Revival\">LCDHost Revival</a></homepageurl>"
  "<logourl></logourl>"
  "<shortdesc>"
  "Displays current weather conditions and a five day forecast."
  "</shortdesc>"
  "<longdesc>"
  "<p>The weather plugin gets its data from <a href=\"https://open-meteo.com/\">Open-Meteo</a> "
  "(free, no account needed; data licensed under CC BY 4.0) and makes it available via a text and an image class. "
  "Translation is supported, but it has to be done manually: select your language from the list and unknown words "
  "will be stored in a file in the LCDHost folder. Edit this file to add the missing translations.</p>"
  "<p>To configure the plugin, use the plugin settings panel to the right: enter your location (a city name, optionally "
  "followed by a comma and the region or country, or \"latitude, longitude\") and select the units you want to use. "
  "These settings are stored globally and applied to any layout which displays the weather.</p>"
  "<p>To add weather data to a layout, use one of the included classes:</p>"
  "<p><b>Weather Text</b></p>"
  "<p>Weather Text objects show weather data in a standard text object, such as conditions, temperatures, wind speeds, "
  "etc. To make a layout most flexible never add a static text object for the city or country names - use a weather text "
  "object and set it to display location data. That way other users will be able to load your layouts and have them work "
  "straight away.</p>"
  "<p><b>Weather Image</b></p>"
  "<p>The image class looks up an image based on a weather status code (the codes of the former Yahoo! Weather feed, "
  "0 to 47, and 3200 for \"not available\"). To do this it requires a text file (called the image map) listing all the "
  "codes and matching each one to two images in the same folder (one for day and one for night).</p>"
  "<p><b>Weather Browser Opener</b></p>"
  "<p>This object can be added to a layout allowing a key to be used to open the forecast in the default browser.</p>"
  "</longdesc>"
"</lcdhostplugin>";

namespace {

const char *kGeocodingUrl = "https://geocoding-api.open-meteo.com/v1/search";
const char *kForecastUrl = "https://api.open-meteo.com/v1/forecast";
const int kForecastDays = 5;
const int kUnknownCode = 3200;

// Country names users commonly type that differ from ISO 3166-1 codes.
QString countryAlias(const QString &word)
{
    if (word == "uk") return "gb";
    if (word == "usa" || word == "america") return "us";
    return word;
}

struct Condition
{
    int dayCode;
    int nightCode;
    const char *text;
};

// Maps a WMO weather interpretation code (as returned by Open-Meteo) to the
// Yahoo! Weather condition codes used by image maps, and a description.
Condition wmoCondition(int wmo)
{
    switch (wmo)
    {
    case 0:  return {32, 31, "Clear"};
    case 1:  return {34, 33, "Mainly Clear"};
    case 2:  return {30, 29, "Partly Cloudy"};
    case 3:  return {26, 26, "Cloudy"};
    case 45: return {20, 20, "Fog"};
    case 48: return {20, 20, "Freezing Fog"};
    case 51: return {9, 9, "Light Drizzle"};
    case 53: return {9, 9, "Drizzle"};
    case 55: return {9, 9, "Heavy Drizzle"};
    case 56: return {8, 8, "Light Freezing Drizzle"};
    case 57: return {8, 8, "Freezing Drizzle"};
    case 61: return {11, 11, "Light Rain"};
    case 63: return {12, 12, "Rain"};
    case 65: return {12, 12, "Heavy Rain"};
    case 66: return {10, 10, "Light Freezing Rain"};
    case 67: return {10, 10, "Freezing Rain"};
    case 71: return {14, 14, "Light Snow"};
    case 73: return {16, 16, "Snow"};
    case 75: return {41, 41, "Heavy Snow"};
    case 77: return {16, 16, "Snow Grains"};
    case 80: return {40, 40, "Light Showers"};
    case 81: return {11, 11, "Showers"};
    case 82: return {12, 12, "Heavy Showers"};
    case 85: return {42, 42, "Light Snow Showers"};
    case 86: return {46, 46, "Snow Showers"};
    case 95: return {4, 4, "Thunderstorms"};
    case 96: return {3, 3, "Thunderstorms with Hail"};
    case 99: return {3, 3, "Severe Thunderstorms with Hail"};
    default: return {kUnknownCode, kUnknownCode, "Unknown"};
    }
}

QString rounded(const QJsonValue &value, int decimals = 0)
{
    if (!value.isDouble())
        return QString();
    return QString::number(value.toDouble(), 'f', decimals);
}

QLocale english()
{
    return QLocale(QLocale::English, QLocale::UnitedKingdom);
}

// "2026-09-27T06:52" -> "6:52 am", the format used by the former feed.
QString shortTime(const QString &isoDateTime)
{
    const QDateTime dt = QDateTime::fromString(isoDateTime, Qt::ISODate);
    if (!dt.isValid())
        return QString();
    return english().toString(dt.time(), "h:mm ap").toLower();
}

} // namespace

//------------------------------------------------------------------------------------------------------------------

LH_QtPlugin_Weather::LH_QtPlugin_Weather() : weather_data(), translator("Weather", this)
{}

const char *LH_QtPlugin_Weather::userInit()
{
    if( const char *err = LH_QtPlugin::userInit() ) return err;
    translator.setTargetLanguage("en");

    setup_show_all_languages_ = new LH_Qt_bool(this,"^Show Untranslated Languages", false, LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    connect(setup_show_all_languages_, SIGNAL(changed()), this, SLOT(updateLanguagesList()));
    setup_show_all_languages_->setHelp("<p>Ticking this box will allow you choose a language with no current translation. Unknown items will be added to a translation cache located in the LCDHost directory. Simply edit this cache to complete the translation.</p>");

    setup_languages_ = new LH_Qt_QStringList(this, "Language", QStringList(), LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_languages_->setHelp("<p>Weather descriptions are provided in English and translated using a local translation cache.</p>"
                              "<p>Missing translations can be corrected by editing the translation cache located in the LCDHost directory.</p>");
    connect(setup_languages_, SIGNAL(changed()), this, SLOT(selectLanguage()));

    setup_language_ = new LH_Qt_QString(this, "Language Code", "en", LH_FLAG_HIDDEN | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE | LH_FLAG_BLANKTITLE);
    connect(setup_language_, SIGNAL(changed()), this, SLOT(setLanguage()));

    setup_location_name_ = new LH_Qt_QString(this,"Location",QString("London, UK"), LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_location_name_->setHelp("<p>The location whose weather you want to display: a city name, optionally followed by "
                                  "a comma and the region or country (e.g. \"Paris, France\"), or \"latitude, longitude\".</p>");
    setup_location_name_->setOrder(-5);
    connect( setup_location_name_, SIGNAL(changed()), this, SLOT(lookupLocation()));

    setup_coordinates_ = new LH_Qt_QString(this,"Coordinates",QString(), LH_FLAG_HIDDEN | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_coordinates_->setHelp("Internal use only: latitude and longitude of the location");
    setup_coordinates_->setOrder(-4);

    setup_city_ = new LH_Qt_QString(this,"^City",QString(), LH_FLAG_READONLY | LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_city_->setHelp("<p>The location whose weather is currently being displayed.</p>"
                         "<p>The plugin looks up the place entered in the \"Location\" box, and displays the best match here.</p>");
    setup_city_->setOrder(-4);

    QStringList unitTypes = QStringList();
    unitTypes.append("Metric (Centigrade, Kilometers, etc)");
    unitTypes.append("Imperial (Fahrenheit, Miles, etc)");
    setup_units_type_ = new LH_Qt_QStringList(this, "Units", unitTypes, LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_units_type_->setHelp("Select whether you want metric (European) units or imperial (British Commonwealth & USA)");
    setup_units_type_->setOrder(-1);
    connect( setup_units_type_, SIGNAL(changed()), this, SLOT(fetchForecast()) );

    setup_refresh_ = new LH_Qt_int(this,tr("Refresh (minutes)"),5, LH_FLAG_NOSINK | LH_FLAG_NOSOURCE);
    setup_refresh_->setHelp("How long to wait before checking for an update of the weather data (in minutes)");
    connect( setup_refresh_, SIGNAL(changed()), this, SLOT(requestPolling()) );

    setup_json_weather_ = new LH_Qt_QString(this, "JSON Data", QString(), LH_FLAG_NOSAVE_DATA | LH_FLAG_NOSAVE_LINK | LH_FLAG_NOSINK | LH_FLAG_HIDDEN);
    setup_json_weather_->setPublishPath("/JSON_Weather_Data");
    setup_json_weather_->setMimeType("application/x-weather");

    for (int i = 0; i < kForecastDays; i++)
        setNoForecast(weather_data.forecast[i]);

    updateLanguagesList();

    // Refresh as soon as the first once-a-second notification arrives.
    lastrefresh_ = QDateTime();
    return 0;
}

int LH_QtPlugin_Weather::notify(int code,void* param)
{
    Q_UNUSED(param);
    if( code & LH_NOTE_SECOND )
    {
        const int minutes = qMax(1, setup_refresh_->value());
        if( !lastrefresh_.isValid() || lastrefresh_.addSecs(60*minutes) < QDateTime::currentDateTime() )
        {
            lastrefresh_ = QDateTime::currentDateTime();
            if (setup_coordinates_->value().isEmpty())
                lookupLocation();
            else
                fetchForecast();
        }
    }
    return LH_NOTE_SECOND;
}

bool LH_QtPlugin_Weather::isMetric() const
{
    return setup_units_type_->value() != 1;
}

QNetworkReply *LH_QtPlugin_Weather::get(const QUrl &url)
{
    const QList<QNetworkProxy> proxies = QNetworkProxyFactory::systemProxyForQuery(QNetworkProxyQuery(url));
    nam_.setProxy(proxies.isEmpty() ? QNetworkProxy(QNetworkProxy::NoProxy) : proxies.first());

    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "LCDHost-Weather (https://github.com/LokLakh-s/LCDHost-Revival)");
    request.setTransferTimeout(30000);
    return nam_.get(request);
}

void LH_QtPlugin_Weather::lookupLocation()
{
    const QString location = setup_location_name_->value().trimmed();
    if (location.isEmpty())
        return;

    // "latitude, longitude" needs no lookup.
    static const QRegularExpression coords("^\\s*(-?\\d+(?:\\.\\d+)?)\\s*[,;]\\s*(-?\\d+(?:\\.\\d+)?)\\s*$");
    const QRegularExpressionMatch coordsMatch = coords.match(location);
    if (coordsMatch.hasMatch())
    {
        setup_coordinates_->setValue(QString("%1,%2").arg(coordsMatch.captured(1), coordsMatch.captured(2)));
        weather_data.location = locationData();
        weather_data.location.city = location;
        setup_city_->setValue(location);
        fetchForecast();
        return;
    }

    // "City, Region, Country": search for the city, use the rest to choose
    // between places with the same name.
    QStringList parts = location.split(',', Qt::SkipEmptyParts);
    geocodeHints_.clear();
    for (int i = 1; i < parts.count(); i++)
        geocodeHints_ += parts.at(i).toLower().split(' ', Qt::SkipEmptyParts);
    geocode(parts.first().trimmed());
}

void LH_QtPlugin_Weather::geocode(const QString &name)
{
    if (geocodeReply_)
        geocodeReply_->abort();

    geocodeName_ = name;
    QUrl url(kGeocodingUrl);
    QUrlQuery query;
    query.addQueryItem("name", name);
    query.addQueryItem("count", "10");
    query.addQueryItem("language", "en");
    query.addQueryItem("format", "json");
    url.setQuery(query);

    QNetworkReply *reply = get(url);
    geocodeReply_ = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { geocodeFinished(reply); });
}

void LH_QtPlugin_Weather::geocodeFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply != geocodeReply_)
        return; // superseded by a newer lookup
    if (reply->error() != QNetworkReply::NoError)
    {
        if (reply->error() != QNetworkReply::OperationCanceledError)
            qWarning() << "LH_QtPlugin_Weather: location lookup failed:" << reply->errorString();
        return;
    }

    const QJsonArray results = QJsonDocument::fromJson(reply->readAll()).object().value("results").toArray();
    if (results.isEmpty())
    {
        // "New York City" or "London UK": retry without the last word and use
        // it to pick the right place.
        const int space = geocodeName_.lastIndexOf(' ');
        if (space > 0)
        {
            geocodeHints_.prepend(geocodeName_.mid(space + 1).toLower());
            geocode(geocodeName_.left(space));
            return;
        }
        setup_city_->setValue("Location not recognised");
        setup_coordinates_->setValue(QString());
        return;
    }

    // Results come sorted by relevance; prefer the first one matching the most hints.
    QJsonObject best = results.first().toObject();
    int bestScore = 0;
    for (const QJsonValue &value : results)
    {
        const QJsonObject place = value.toObject();
        const QStringList fields = {
            place.value("country").toString().toLower(),
            place.value("country_code").toString().toLower(),
            place.value("admin1").toString().toLower(),
            place.value("admin2").toString().toLower(),
        };
        int score = 0;
        for (const QString &hint : std::as_const(geocodeHints_))
            for (const QString &field : fields)
                if (!field.isEmpty() && (field == countryAlias(hint) || field.contains(hint)))
                {
                    score++;
                    break;
                }
        if (score > bestScore)
        {
            best = place;
            bestScore = score;
        }
    }

    weather_data.location.city = best.value("name").toString();
    weather_data.location.region = best.value("admin1").toString();
    weather_data.location.country = best.value("country").toString();
    setup_coordinates_->setValue(QString("%1,%2")
                                 .arg(best.value("latitude").toDouble(), 0, 'f', 4)
                                 .arg(best.value("longitude").toDouble(), 0, 'f', 4));

    QStringList cityName(weather_data.location.city);
    if (!weather_data.location.region.isEmpty()) cityName << weather_data.location.region;
    if (!weather_data.location.country.isEmpty()) cityName << weather_data.location.country;
    setup_city_->setValue(cityName.join(", "));

    fetchForecast();
}

void LH_QtPlugin_Weather::fetchForecast()
{
    const QStringList coords = setup_coordinates_->value().split(',');
    if (coords.count() != 2)
        return;
    if (forecastReply_)
        forecastReply_->abort();

    QUrl url(kForecastUrl);
    QUrlQuery query;
    query.addQueryItem("latitude", coords.at(0));
    query.addQueryItem("longitude", coords.at(1));
    query.addQueryItem("current", "temperature_2m,relative_humidity_2m,apparent_temperature,is_day,weather_code,"
                                  "pressure_msl,wind_speed_10m,wind_direction_10m,visibility");
    // The last hours of pressure give the barometric trend.
    query.addQueryItem("hourly", "pressure_msl");
    query.addQueryItem("past_hours", "3");
    query.addQueryItem("forecast_hours", "1");
    query.addQueryItem("daily", "weather_code,temperature_2m_max,temperature_2m_min,sunrise,sunset");
    query.addQueryItem("forecast_days", QString::number(kForecastDays));
    query.addQueryItem("timezone", "auto");
    if (!isMetric())
    {
        query.addQueryItem("temperature_unit", "fahrenheit");
        query.addQueryItem("wind_speed_unit", "mph");
    }
    url.setQuery(query);

    QNetworkReply *reply = get(url);
    forecastReply_ = reply;
    connect(reply, &QNetworkReply::finished, this, [this, reply]() { forecastFinished(reply); });
}

void LH_QtPlugin_Weather::forecastFinished(QNetworkReply *reply)
{
    reply->deleteLater();
    if (reply != forecastReply_)
        return;
    if (reply->error() != QNetworkReply::NoError)
    {
        if (reply->error() != QNetworkReply::OperationCanceledError)
            qWarning() << "LH_QtPlugin_Weather: forecast request failed:" << reply->errorString();
        return;
    }

    QJsonParseError error;
    const QJsonDocument doc = QJsonDocument::fromJson(reply->readAll(), &error);
    if (!doc.isObject())
    {
        qWarning() << "LH_QtPlugin_Weather: invalid forecast data:" << error.errorString();
        return;
    }
    parseForecast(doc.object());
    publish();
    qDebug() << "LH_QtPlugin_Weather: updated" << setup_city_->value() << weather_data.condition.text
             << weather_data.condition.temp << "code" << weather_data.condition.code;
}

void LH_QtPlugin_Weather::parseForecast(const QJsonObject &json)
{
    const bool metric = isMetric();
    const QJsonObject current = json.value("current").toObject();
    const QJsonObject daily = json.value("daily").toObject();

    weather_data.units.temperature = QString(QChar(0x00B0)) + (metric ? "C" : "F");
    weather_data.units.distance = metric ? "km" : "mi";
    weather_data.units.pressure = metric ? "mb" : "in";
    weather_data.units.speed = metric ? "km/h" : "mph";

    weather_data.isNight = current.value("is_day").toInt(1) == 0;

    const Condition condition = wmoCondition(current.value("weather_code").toInt(-1));
    weather_data.condition.code = QString::number(weather_data.isNight ? condition.nightCode : condition.dayCode);
    weather_data.condition.text = condition.text;
    weather_data.condition.temp = rounded(current.value("temperature_2m"));
    const QDateTime observed = QDateTime::fromString(current.value("time").toString(), Qt::ISODate);
    weather_data.condition.date = english().toString(observed, "ddd, d MMM yyyy h:mm ap").replace("AM", "am").replace("PM", "pm")
            + " " + json.value("timezone_abbreviation").toString();

    weather_data.wind.chill = rounded(current.value("apparent_temperature"));
    weather_data.wind.direction = rounded(current.value("wind_direction_10m"));
    weather_data.wind.speed = rounded(current.value("wind_speed_10m"));

    weather_data.atmosphere.humidity = rounded(current.value("relative_humidity_2m"));
    const QJsonValue visibility = current.value("visibility"); // metres
    weather_data.atmosphere.visibility = visibility.isDouble()
            ? QString::number(visibility.toDouble() / (metric ? 1000.0 : 1609.344), 'f', 1) : QString();
    const QJsonValue pressure = current.value("pressure_msl"); // hPa
    weather_data.atmosphere.pressure = pressure.isDouble()
            ? (metric ? QString::number(pressure.toDouble(), 'f', 0)
                      : QString::number(pressure.toDouble() * 0.0295300, 'f', 2)) : QString();

    const QJsonArray pressures = json.value("hourly").toObject().value("pressure_msl").toArray();
    weather_data.atmosphere.barometricReading = QString();
    if (pressures.count() >= 2 && pressures.first().isDouble() && pressures.last().isDouble())
    {
        const double change = pressures.last().toDouble() - pressures.first().toDouble();
        weather_data.atmosphere.barometricReading = change > 1.0 ? "Rising" : (change < -1.0 ? "Falling" : "Steady");
    }

    const QJsonArray days = daily.value("time").toArray();
    weather_data.astronomy.sunrise = shortTime(daily.value("sunrise").toArray().at(0).toString());
    weather_data.astronomy.sunset = shortTime(daily.value("sunset").toArray().at(0).toString());

    weather_data.forecastDays = qMin(int(days.count()), kForecastDays);
    for (int i = 0; i < kForecastDays; i++)
    {
        forecastData &forecast = weather_data.forecast[i];
        const QDate date = QDate::fromString(days.at(i).toString(), Qt::ISODate);
        if (i >= weather_data.forecastDays || !date.isValid())
        {
            setNoForecast(forecast);
            continue;
        }
        const Condition dayCondition = wmoCondition(daily.value("weather_code").toArray().at(i).toInt(-1));
        forecast.day = english().toString(date, "ddd");
        forecast.date = english().toString(date, "d MMM yyyy");
        switch (i)
        {
        case 0:
            forecast.relativeDay = weather_data.isNight ? "Tonight" : "Today";
            break;
        case 1:
            forecast.relativeDay = "Tomorrow";
            break;
        default:
            forecast.relativeDay = translator.fullDateName(forecast.day);
            break;
        }
        forecast.low = rounded(daily.value("temperature_2m_min").toArray().at(i));
        forecast.high = rounded(daily.value("temperature_2m_max").toArray().at(i));
        forecast.text = dayCondition.text;
        forecast.code = QString::number(dayCondition.dayCode);
    }

    const QStringList coords = setup_coordinates_->value().split(',');
    weather_data.url = coords.count() == 2
            ? QString("https://open-meteo.com/en/docs?latitude=%1&longitude=%2").arg(coords.at(0), coords.at(1))
            : QString();
}

void LH_QtPlugin_Weather::setNoForecast(forecastData &forecast)
{
    forecast.day  = "N/A";
    forecast.relativeDay = "N/A";
    forecast.date = "N/A";
    forecast.low  = "?";
    forecast.high = "?";
    forecast.text = "Unknown";
    forecast.code = QString::number(kUnknownCode);
}

void LH_QtPlugin_Weather::publish()
{
    requestTranslation();
    setup_json_weather_->setValue(weather_data.serialize());
}

void LH_QtPlugin_Weather::requestTranslation()
{
    translator.addItem(&weather_data.atmosphere.barometricReading);

    translator.addItem(&weather_data.location.city, ttNoun);
    translator.addItem(&weather_data.location.region, ttNoun);
    translator.addItem(&weather_data.location.country, ttNoun);

    translator.addItem(&weather_data.condition.text);
    translator.addItem(&weather_data.condition.date, ttMonthName);
    translator.addItem(&weather_data.condition.date, ttDayName);

    for(int i=0; i<kForecastDays; i++)
    {
        translator.addItem(&weather_data.forecast[i].day, ttDayName);
        translator.addItem(&weather_data.forecast[i].relativeDay);
        translator.addItem(&weather_data.forecast[i].date, ttMonthName);
        translator.addItem(&weather_data.forecast[i].date, ttDayName);
        translator.addItem(&weather_data.forecast[i].text);
    }

    translator.saveCache();
}

void LH_QtPlugin_Weather::updateLanguagesList()
{
    translator.loadLanguages(setup_show_all_languages_->value());
    setup_languages_->list().clear();
    foreach(QString name, translator.languages.names())
        setup_languages_->list().append(name);
    setup_languages_->refreshList();
    setup_languages_->setValue(translator.languages.codes().indexOf(setup_language_->value()));
}

void LH_QtPlugin_Weather::selectLanguage()
{
    QString code = translator.languages.getCode(setup_languages_->valueText());
    setup_language_->setValue(code);
    translator.setTargetLanguage(code);
    fetchForecast();
}

void LH_QtPlugin_Weather::setLanguage()
{
    translator.setTargetLanguage(setup_language_->value());
    setup_languages_->setValue(translator.languages.codes().indexOf(setup_language_->value()));
    updateLanguagesList();
}
