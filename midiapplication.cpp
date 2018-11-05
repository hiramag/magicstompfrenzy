#include "midiapplication.h"

#ifdef Q_OS_LINUX
#include <alsa/asoundlib.h>
#endif
#ifdef Q_OS_MACOS
#include <AudioToolbox/AudioToolbox.h>
#endif

#include "midievent.h"
#include "midiinthread.h"
#include "midisender.h"
#include "midiportmodel.h"

#ifdef Q_OS_MAC
void MIDIEngineNotifyProc(const MIDINotification *message, void *refCon)
{
    printf("MIDI Notify, messageId=%d,", message->messageID);
}

void MIDIEngineReadProc(const MIDIPacketList *pktlist, void *arg,void *connRefCon)
{
    /*std::map<MIDIEvent, MIDIEventCallback> *MIDIEventCallackMap = static_cast<std::map<MIDIEvent, MIDIEventCallback> *>(arg);
    MIDIPacket *packet = (MIDIPacket *)pktlist->packet;
    for (unsigned int i=0; i < pktlist->numPackets; i++)
    {
        MIDIEvent mevent;
        mevent.status=packet->data[0];
        mevent.data1=packet->data[1];
        mevent.data2=packet->data[2];

        std::map<MIDIEvent, MIDIEventCallback>::iterator it;

        it=MIDIEventCallackMap->find(mevent);

        if(it != MIDIEventCallackMap->end())
            (*it).second(mevent);

        fprintf(stderr,"MIDI Read, Channel=%d, Command=%X, data1=%d, data2=%d\n", (mevent.status & 0x0F) +1 , mevent.status >> 4, mevent.data1, mevent.data2);
    }*/
}

MIDISysexSendRequest sysexReq;
void sysexCompletionProc(MIDISysexSendRequest *req)
{

}

#endif

MidiApplication::MidiApplication(int &argc, char **argv)
    :QApplication( argc, argv)
{
    int  midiEventType;
    midiEventType = QEvent::registerEventType(MidiEvent::Common);
    Q_ASSERT(midiEventType==MidiEvent::Common);
    midiEventType = QEvent::registerEventType(MidiEvent::SysEx);
    Q_ASSERT(midiEventType==MidiEvent::SysEx);

    midiSystemInit();

#ifdef Q_OS_LINUX
    midiInThread = new MidiInThread(handle, this);
    midiInThread->start();

    midiSender = new MidiSender(handle, thisOutPort);

    midiOutThread = new QThread(this);
    midiSender->moveToThread(midiOutThread);
    midiOutThread->start();
#endif
#ifdef Q_OS_MACOS
    MIDIInputPortCreate(handle, CFSTR("In Port"), MIDIEngineReadProc, nullptr, &inPort);
    MIDIOutputPortCreate(handle, CFSTR("Out Port"),  &outPort);
#endif

    readablePortsModel = new MidiPortModel(handle, MidiPortModel::ReadablePorts, this);
    readablePortsModel->scan();
    writablePortsModel = new MidiPortModel(handle, MidiPortModel::WritablePorts, this);
    writablePortsModel->scan();

    connect(this, SIGNAL(aboutToQuit()), this, SLOT(isQuitting()));

#if defined(QT_DEBUG) || defined(Q_OS_LINUX)
    if(argc > 2)
    {
        bool okclient, okport;
        int client = QString(argv[1]).toInt( &okclient);
        int port = QString(argv[2]).toInt( &okport);
        if( okclient && okport  )
            connectToReadablePort( MidiClientPortId (client, port));
    }
    if(argc > 4)
    {
        bool okclient, okport;
        int client = QString(argv[3]).toInt( &okclient);
        int port = QString(argv[4]).toInt( &okport);
        if( okclient && okport  )
            connectToWritablePort(MidiClientPortId (client, port));
    }
#endif
}

bool MidiApplication::event(QEvent *e)
{
    MidiEvent *me = dynamic_cast<MidiEvent *>(e);
    if(me)
    {
        emit midiEventReceived(me);
    }
    return QApplication::event(e);
}

void  MidiApplication::midiSystemInit()
{
#ifdef Q_OS_LINUX

    int err;
    err = snd_seq_open(&handle, "default", SND_SEQ_OPEN_DUPLEX, 0);

    snd_seq_set_client_name(handle, "MagicstompFrenzy");

    thisInPort = snd_seq_create_simple_port(handle, "MagicstompFrenzy IN",
                                            SND_SEQ_PORT_CAP_WRITE|SND_SEQ_PORT_CAP_SUBS_WRITE, SND_SEQ_PORT_TYPE_MIDI_GENERIC | SND_SEQ_PORT_TYPE_APPLICATION);
    thisOutPort = snd_seq_create_simple_port(handle, "MagicstompFrenzy OUT",
                                             SND_SEQ_PORT_CAP_READ|SND_SEQ_PORT_CAP_SUBS_READ, SND_SEQ_PORT_TYPE_MIDI_GENERIC );

    // Subscribe to the announce port.
    snd_seq_port_subscribe_t* subs;
    snd_seq_port_subscribe_alloca(&subs);
    snd_seq_addr_t announce_sender;
    snd_seq_addr_t announce_dest;
    announce_sender.client = SND_SEQ_CLIENT_SYSTEM;
    announce_sender.port = SND_SEQ_PORT_SYSTEM_ANNOUNCE;
    announce_dest.client = snd_seq_client_id(handle);
    announce_dest.port = thisInPort;
    snd_seq_port_subscribe_set_sender(subs, &announce_sender);
    snd_seq_port_subscribe_set_dest(subs, &announce_dest);
    err = snd_seq_subscribe_port(handle, subs);
    if (err != 0)
    {
        puts ("snd_seq_subscribe_port on the announce port fails: ");
    }
#endif
#ifdef Q_OS_MACOS
    MIDIClientCreate(CFSTR("MagicstompFrenzy"), MIDIEngineNotifyProc, 0, &handle);
#endif
}

bool MidiApplication::connectToReadablePort( MidiClientPortId mcpId)
{
#ifdef Q_OS_LINUX
    snd_seq_addr_t sender, dest;
    snd_seq_port_subscribe_t* subs;
    snd_seq_port_subscribe_alloca(&subs);

    dest.client = snd_seq_client_id(handle);
    dest.port = thisInPort;
    sender.client = mcpId.clientId();
    sender.port = mcpId.portId();

    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
    return snd_seq_subscribe_port(handle, subs);
#endif
#ifdef Q_OS_MACOS
    return MIDIPortConnectSource(inPort, portId, nullptr);
#endif
}

bool MidiApplication::connectToWritablePort( MidiClientPortId mcpId)
{
#ifdef Q_OS_LINUX
    snd_seq_addr_t sender, dest;
    snd_seq_port_subscribe_t* subs;
    snd_seq_port_subscribe_alloca(&subs);

    sender.client = snd_seq_client_id(handle);
    sender.port = thisOutPort;
    dest.client = mcpId.clientId();
    dest.port = mcpId.portId();

    snd_seq_port_subscribe_set_sender(subs, &sender);
    snd_seq_port_subscribe_set_dest(subs, &dest);
    return snd_seq_subscribe_port(handle, subs);
#endif
#ifdef Q_OS_MACOS
    outDest = portId;
    return true;
    //return MIDIPortConnectSource(outPort, portId, nullptr);
#endif
}
void MidiApplication::sendMidiEvent(MidiEvent *ev)
{
#ifdef Q_OS_LINUX
    postEvent( midiSender, ev);
#endif
#ifdef Q_OS_MACOS
    sysexReq.bytesToSend = ev->sysExData()->size();
    sysexReq.complete = false;
    sysexReq.completionProc = sysexCompletionProc;
    sysexReq.completionRefCon = nullptr;
    sysexReq.data = reinterpret_cast<const Byte *>(ev->sysExData()->constData());
    sysexReq.destination = outDest;
    MIDISendSysex( & sysexReq);
    MIDIFlushOutput( outDest);
#endif
}

void MidiApplication::isQuitting()
{
#ifdef Q_OS_LINUX
    midiInThread->terminate();
    midiInThread->wait();

    midiOutThread->quit();
    midiOutThread->wait();

    snd_seq_close(handle);
#endif
#ifdef Q_OS_MACOS
    MIDIPortDispose(inPort);
    MIDIPortDispose(outPort);
#endif
}
