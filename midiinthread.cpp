/****************************************************************************
**
** Copyright (C) 2018 Robert Vetter.
**
** This file is part of the MagicstompFrenzy - an editor for Yamaha Magicstomp
** effect processor
**
** THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
** ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
** IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
** PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
**
** GNU General Public License Usage
** This file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version . The licenses are
** as published by the Free Software Foundation and appearing in the file LICENSE
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**/
#include "midiinthread.h"

#include "midievent.h"

#ifdef Q_OS_LINUX
#include <alsa/asoundlib.h>
#endif
#include <QApplication>
#include <QDebug>


MidiInThread::MidiInThread(snd_seq_t *handle, QObject *parent)
    :QThread(parent), handle(handle)
{

}

void MidiInThread::run()
{
#ifdef Q_OS_LINUX
    snd_seq_event_t *ev;
    MidiEvent *midisysexevent = nullptr;
    // IMPORTANT snd_seq_event_input blocks even after snd_seq_close has been execuded.
    // It will be  neccessary for this application to send an SND_SEQ_EVENT_CLIENT_EXIT to himself before terminating
    while (snd_seq_event_input(handle, &ev) >= 0)
    {
        if(ev->type==SND_SEQ_EVENT_SYSEX)
        {
            QByteArray arr((char *)ev->data.ext.ptr, ev->data.ext.len);
            if( midisysexevent != nullptr)
            {
                QByteArray *data = midisysexevent->sysExData();
                data->append(arr);
            }
            else
            {
                midisysexevent = new MidiEvent(static_cast<QEvent::Type>(MidiEvent::SysEx));
                quint32 port = ev->source.port;
                port |= static_cast<quint32>(ev->source.client) << 8;
                midisysexevent->setPort(port);
                QByteArray *data = midisysexevent->sysExData();
                *data=arr;
            }
            if(static_cast<unsigned char>(arr.at(arr.size()-1)) == 0xF7)
            {
                QApplication::postEvent(parent(), midisysexevent);
                midisysexevent=nullptr;
            }
        }
        else if(ev->type==SND_SEQ_EVENT_PORT_SUBSCRIBED)
        {
            snd_seq_connect conn = ev->data.connect;
            emit portConnectionStatusChanged( MidiClientPortId(conn.sender.client, conn.sender.port),
                                              MidiClientPortId(conn.dest.client, conn.dest.port),
                                              true);

        }
        else if(ev->type==SND_SEQ_EVENT_PORT_UNSUBSCRIBED)
        {
            snd_seq_connect conn = ev->data.connect;
            emit portConnectionStatusChanged( MidiClientPortId(conn.sender.client, conn.sender.port),
                                              MidiClientPortId(conn.dest.client, conn.dest.port),
                                              false);

        }
        else if(ev->type==SND_SEQ_EVENT_CLIENT_START)
        {
            snd_seq_addr_t addr = ev->data.addr;
            emit portClientPortStatusChanged(MidiClientPortId(addr.client, addr.port), true);
        }
        else if(ev->type==SND_SEQ_EVENT_CLIENT_EXIT)
        {
            snd_seq_addr_t addr = ev->data.addr;
            emit portClientPortStatusChanged(MidiClientPortId(addr.client, addr.port), false);
        }
        else if(ev->type==SND_SEQ_EVENT_SENSING)
        {
            //qDebug("got SND_SEQ_EVENT_SENSING");
        }

        //qDebug("MIDI Event. Type = %d",ev->type);
        snd_seq_free_event(ev);
    }
#endif
}
