/*
  Copyright (c) 2009-2016 Johan Lindh <johan@linkdata.se>

  This file is part of LCDHost.

  LCDHost is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  LCDHost is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with LCDHost.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef LH_DATAVIEWERDATA_H
#define LH_DATAVIEWERDATA_H

#include "LH_Qt_QString.h"

#include <QStringList>
#include <QDateTime>
#include <QDebug>
#include <QRegularExpression>

// #include <stdio.h>
// #include <windows.h>

#include "LH_DataViewerDataTypes.h"
extern dataNode* rootNode;
extern sharedCollection* sharedData;


class LH_DataViewerData
{
    LH_QtObject *parent_;

    QString lastupdate_;
    QString lasttemplate_;
    bool lastexpired_;

public:
    bool expired;

    LH_DataViewerData(LH_QtObject *parent)
    {
        parent_ = parent;
    }

    ~LH_DataViewerData(){
    }

    bool open()
    {
        if(sharedData)
        {
            if(sharedData->expiresAt.trimmed()!="N/A")
            {
                QDateTime expiryTime = QDateTime::fromString(sharedData->expiresAt, "yyyyMMddHHmmss.zzz");
                expired = (QDateTime::currentDateTime().secsTo(expiryTime)<=0);
            } else
                expired = false;
            return true;
        }
        else
        {
            expired = false;
            return false;
        }
    }

    bool valid(QString currTemplate)
    {
        if( sharedData->valid && (lastupdate_ != sharedData->lastUpdated || lasttemplate_ != currTemplate || lastexpired_ != expired))
        {
            lastupdate_ = sharedData->lastUpdated;
            lasttemplate_ = currTemplate;
            lastexpired_ = expired;
            return true;
        } else
            return false;
    }

    QString populateLookupCode(QString lookupTemplate, bool getNames = false, bool preventEmpty = false)
    {
        QString templateResult = QString(lookupTemplate);

        // look for the old index-style references, e.g. {0} {2} {34}
        QRegularExpression rx = QRegularExpression("\\{([0-9]*)\\}");
        QRegularExpressionMatch rxMatch;
        bool blankValues = true;
        int hitCount = templateResult.count(rx);
        while ((rxMatch = rx.match(templateResult)).hasMatch())
        {
            QString strVal = getValueText(rxMatch.captured(1).toInt(), getNames);
            if (strVal!="") blankValues = false;
            templateResult.replace(rxMatch.captured(0), strVal);
        }

        // look for new address-based references, e.g. {player.name}
        rx = QRegularExpression("\\{([a-zA-Z0-9.[\\]]*(?:@[a-zA-Z0-9]+)?)\\}");
        hitCount += templateResult.count(rx);
        while ((rxMatch = rx.match(templateResult)).hasMatch())
        {
            QString strVal = getValueText(rxMatch.captured(1), getNames);
            if (strVal!="") blankValues = false;
            templateResult.replace(rxMatch.captured(0), strVal);
        }

        if(!getNames)
        {
            // look for formatting commands, e.g. {=<something>:%.2f}
            rx = QRegularExpression("\\{=([^}=:]*)(?:\\:(.*))?\\}");
            hitCount += templateResult.count(rx);
            while ((rxMatch = rx.match(templateResult)).hasMatch())
            {
                QString strVal = parseMath(rxMatch.captured(1),0,rxMatch.captured(2));
                templateResult.replace(rxMatch.captured(0), strVal);
            }
        }

        if (blankValues && hitCount!=0) templateResult = "";
        if (preventEmpty && templateResult == "") templateResult = " ";

        return templateResult;
    }

    QString getValueText(int valueIndex, bool getName = false)
    {
        if (valueIndex>=sharedData->count() && sharedData->count()!=0)
            valueIndex = sharedData->count()-1;

        QString resultText = "";
        if (valueIndex < sharedData->count())
        {
            if(getName)
                resultText = sharedData->item(valueIndex).name;
            else if(expired)
                resultText = "";
            else
                resultText = sharedData->item(valueIndex).value;
        }

        return resultText;
    }

    QString getValueText(QString valueAddress, bool getName = false)
    {
        QString resultText = "";

        if(getName)
            return valueAddress;
        else
        {
            QString attrName = "";
            QStringList path = valueAddress.split(".");
            dataNode* curNode = rootNode;
            QRegularExpression rx("^(.*)\\[([0-9]+)\\]$");
            QRegularExpressionMatch rxMatch;
            while (path.length()!=0)
            {
                QString nodeName = path.first();
                path.removeFirst();
                if(!nodeName.contains("@"))
                    attrName = "";
                else
                {
                    attrName = nodeName.split('@')[1];
                    nodeName = nodeName.split('@')[0];
                }

                int nodeIndex = 0;
                if((rxMatch = rx.match(nodeName)).hasMatch())
                {
                    nodeName = rxMatch.captured(1);
                    nodeIndex = rxMatch.captured(2).toInt();
                }

                if(!curNode->contains(nodeName))
                    return valueAddress;
                if(nodeIndex>=curNode->child(nodeName).count())
                    return valueAddress;
                curNode = curNode->child(nodeName)[nodeIndex];
            }
            while(curNode->defaultItem()!="")
                curNode = curNode->child(curNode->defaultItem())[0];

            if(attrName == "")
                resultText = curNode->value();
            else
                resultText = curNode->attributes.value(attrName);
        }

        return resultText;
    }

    QString parseMath(QString mathString, int depth, QString formatting = "")
    {
        if (depth>=50)
        {
            qWarning() << "Parser Depth >50: Aborted parse. " << mathString;
            return "0";
        }
        //qDebug() << "Parse Math: in: " << mathString;

        QRegularExpression rx = QRegularExpression("\\(([^)=]*)\\)");
        QRegularExpressionMatch rxMatch;
        qreal fltVal = 0;
        int loopCount = 0;
        while ((rxMatch = rx.match(mathString)).hasMatch())
        {
            QString strVal = parseMath(rxMatch.captured(1), depth + 1);
            mathString.replace(rxMatch.captured(0), strVal);
            if (++loopCount >=100)
            {
                qWarning() << "Parser Loops >100: Aborted parse. " << mathString;
                return "0";
            }
        }

        loopCount = 0;
        rx = QRegularExpression("(-?[0-9]*(?:\\.[0-9]*)?)\\s*(\\*|/)\\s*(-?[0-9]*(?:\\.[0-9]*)?)");
        while ((rxMatch = rx.match(mathString)).hasMatch())
        {
            //qDebug() << "Parse Math: action: [" << rxMatch.captured(1) << "] [" << rxMatch.captured(2) << "] [" << rxMatch.captured(3) << "]";
            if (rxMatch.captured(2)=="*")
                fltVal = rxMatch.captured(1).toFloat() * rxMatch.captured(3).toFloat(); else
            if (rxMatch.captured(2)=="/" && rxMatch.captured(3).toFloat()!=0)
                fltVal = rxMatch.captured(1).toFloat() / rxMatch.captured(3).toFloat(); else
            if (rxMatch.captured(2)=="/" && rxMatch.captured(3).toFloat()==0)
                fltVal = 0;

            mathString.replace(rxMatch.captured(0), QString::number(fltVal));
            if (++loopCount >=100)
            {
                qWarning() << "Parser Loops >100: Aborted parse. " << mathString;
                return "0";
            }
        }

        loopCount = 0;
        rx = QRegularExpression("(-?(?:[0-9]*\\.)?[0-9]{1,})\\s*(\\+|\\-)\\s*(-?(?:[0-9]*\\.)?[0-9]{1,})");
        while ((rxMatch = rx.match(mathString)).hasMatch())
        {
            //qDebug() << "Parse Math: action: [" << rxMatch.captured(1) << "] [" << rxMatch.captured(2) << "] [" << rxMatch.captured(3) << "]";
            if (rxMatch.captured(2)=="+")
                fltVal = rxMatch.captured(1).toFloat() + rxMatch.captured(3).toFloat();
            if (rxMatch.captured(2)=="-")
                fltVal = rxMatch.captured(1).toFloat() - rxMatch.captured(3).toFloat();

            mathString.replace(rxMatch.captured(0), QString::number(fltVal));
            if (++loopCount >=100)
            {
                qWarning() << "Parser Loops >100: Aborted parse. " << mathString;
                return "0";
            }
        }


        if (formatting!="")
        {
            mathString = QString::asprintf(formatting.toLatin1().constData(), mathString.toFloat());
        }

        //qDebug() << "Parse Math: out: " << mathString;

        return mathString;
    }
};



#endif // LH_DATAVIEWERDATA_H
