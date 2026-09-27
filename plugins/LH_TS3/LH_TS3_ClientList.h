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

#ifndef LH_TS3_CLIENTLIST_H
#define LH_TS3_CLIENTLIST_H

#include <QHash>
#include <QRegularExpression>

struct clientdetail
{
    int clid;
    int cid;
    int dbid;
    QString name;
    int type;
    bool talking;
    bool inputMuted;
    bool outputMuted;
    int inputHardware;
    int outputHardware;
    int talkPower;
    bool isTalker;
    bool isPrioritySpeaker;
    bool isRecording;
    bool isChannelCommander;
};

class clientlist: public QHash<int,clientdetail>
{
    clientdetail parseClientString(QString item)
    {
        QRegularExpression rx("(clid|cid|client_database_id|client_nickname|client_type|client_flag_talking|client_input_muted|client_output_muted|client_input_hardware|client_output_hardware|client_talk_power|client_is_talker|client_is_priority_speaker|client_is_recording|client_is_channel_commander)\\s*=\\s*(\\S+)");
        QRegularExpressionMatch rxMatch;
        clientdetail client;
        QRegularExpressionMatchIterator it = rx.globalMatch(item);
        while(it.hasNext())
        {
            const QRegularExpressionMatch rxMatch = it.next();
            if(rxMatch.captured(1)=="clid")
                client.clid = rxMatch.captured(2).toInt();
            if(rxMatch.captured(1)=="cid")
                client.cid = rxMatch.captured(2).toInt();
            if(rxMatch.captured(1)=="client_database_id")
                client.dbid = rxMatch.captured(2).toInt();
            if(rxMatch.captured(1)=="client_nickname")
                client.name = rxMatch.captured(2).replace("\\s"," ");;
            if(rxMatch.captured(1)=="client_type")
                client.type = rxMatch.captured(2).toInt();
            if(rxMatch.captured(1)=="client_flag_talking")
                client.talking = (rxMatch.captured(2).toInt()!=0);
            if(rxMatch.captured(1)=="client_input_muted")
                client.inputMuted = (rxMatch.captured(2).toInt()!=0);
            if(rxMatch.captured(1)=="client_output_muted")
                client.outputMuted = (rxMatch.captured(2).toInt()!=0);
            if(rxMatch.captured(1)=="client_input_hardware")
                client.inputHardware = rxMatch.captured(2).toInt();
            if(rxMatch.captured(1)=="client_output_hardware")
                client.outputHardware = rxMatch.captured(2).toInt();
            if(rxMatch.captured(1)=="client_talk_power")
                client.talkPower = rxMatch.captured(2).toInt();
            if(rxMatch.captured(1)=="client_is_talker")
                client.isTalker = (rxMatch.captured(2).toInt()!=0);
            if(rxMatch.captured(1)=="client_is_priority_speaker")
                client.isPrioritySpeaker = (rxMatch.captured(2).toInt()!=0);
            if(rxMatch.captured(1)=="client_is_recording")
                client.isRecording = (rxMatch.captured(2).toInt()!=0);
            if(rxMatch.captured(1)=="client_is_channel_commander")
                client.isChannelCommander = (rxMatch.captured(2).toInt()!=0);
        }
        return client;
    }

public:
    bool load(QString responseMsg){
        if(!responseMsg.contains("clid=") || !responseMsg.contains("cid=") )
            return false;
        this->clear();
        QStringList list = responseMsg.split('|');
        foreach(QString item, list)
        {
            clientdetail client = parseClientString(item);
            insert(client.clid, client);
        }
        return true;
    }

    int findclid(QString regexp)
    {
        QRegularExpression rx(QString("%2%1%3").arg(regexp).arg(regexp.startsWith("^")?"":"^").arg(regexp.endsWith("$")?"":"$"));
        QRegularExpressionMatch rxMatch;
        foreach(clientdetail client, this->values())
            if((rxMatch = rx.match(client.name)).hasMatch())
                return client.clid;
        return -1;
    }

    QString toString()
    {
        QString names = "";
        foreach(clientdetail client, this->values())
            names += (names==""?"":", ") + client.name;
        return names;
    }
};

#endif // LH_TS3_CLIENTLIST_H
