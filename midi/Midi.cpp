#include "Midi.hpp"

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>

TrackChunk::~TrackChunk()
{
    for(Event* event: Events)
    {
        delete event;
    }
}

MidiFile::~MidiFile()
{
    for(TrackChunk* t: trackChunks)
    {
        delete(t);
    }
}

int MidiFile::getNBitsNumber(const std::string &midistr, size_t& offset, int bits)
{
    int number = 0;
    while (bits--) {
        number = (number<<8) | midistr[offset++];
    }
    return number;
}

size_t MidiFile::getDWord(const std::string &midistr, size_t& offset)
{
    size_t number = midistr[offset++];
    if ( number & 0x80 )
    {
        number = number & 0x7f;
        do {
            number = (number<<7) + (midistr[offset]&0x7f);
        } while (midistr[offset++]&0x80);
    }
    return number;
}

std::string MidiFile::getDWordContent(const std::string &midistr, size_t& offset, int type)
{
    std::string str;
    size_t len = getDWord(midistr, offset);
    for (size_t i=0; i<len; i++)
    {
        if (type) { // print the ASCII content
            str += midistr[offset++];
        }
        else {//system sysex event "%x "
            // std::cout << std::hex << midistr[offset++];
        }
    }
    return str;
}

//ff -> meta event
MetaEvent* MidiFile::importMetaEvent(const std::string& midistr, size_t& offset)
{
    MetaEvent* metaEvent = new MetaEvent();

    metaEvent->type = midistr[offset++]; 
    switch (metaEvent->type)
    {
        case 0x00: // SequenceNumber
            metaEvent->size = getDWord(midistr, offset);
            if (metaEvent->size == 2)
            {
                metaEvent->v1 = midistr[offset++];   //MSB
                metaEvent->v2 = midistr[offset++];   //LSB
                return metaEvent;
            }

        case 0x32:  // 20 ChannelPrefix
            metaEvent->size = getDWord(midistr, offset);
            if (metaEvent->size == 1) {
                metaEvent->v1 = midistr[offset++];   // channel number
                return metaEvent;
            }
        case 0x2f: //47 EndOfTrack
            metaEvent->size = getDWord(midistr, offset);
            if(metaEvent->size == 0) {
                return metaEvent;
            }
        case 0x51: //81 SetTempo
            metaEvent->size = getDWord(midistr, offset);
            if (metaEvent->size == 3)
            {
                metaEvent->v1 = getNBitsNumber(midistr, offset, 3);
                //BPM = 60,000,000/(tt tt tt)
                //BPM(Beats Per Minute): a tempo of 100 BPM means that 100 quarter notes per minute
                //MicrosecondsPerQuarterNote=itemp BPM=60000000/itemp
                return metaEvent;
            }
        case 0x54: //84 SMPTE Offset
            metaEvent->size = getDWord(midistr, offset);
            if (metaEvent->size == 5) {
                metaEvent->v1 = midistr[offset++];   // hour
                metaEvent->v2 = midistr[offset++];   // min
                metaEvent->v3 = midistr[offset++];   // sec
                metaEvent->v4 = midistr[offset++];   // fr
                metaEvent->v5 = midistr[offset++];   // subfr
                return metaEvent;
            }
        case 0x58: //88 TimeSignature
            metaEvent->size = getDWord(midistr, offset);
            if (metaEvent->size == 4) {
                metaEvent->v1 = midistr[offset++];   // Numerator
                metaEvent->v2 = midistr[offset++];   // LogDenominator
                metaEvent->v3 = midistr[offset++];   // MIDIClocksPerMetronomeClick
                metaEvent->v4 = midistr[offset++];   // ThirtySecondsPer24Clocks
                return metaEvent;
            }
        case 0x59: // KeySignature
            metaEvent->size = getDWord(midistr, offset);
            if(metaEvent->size == 2)
            {
                // Fifths:
                // -7: 7flats
                // -1: 1flat
                // 0; keyofC
                // 1: 1sharp
                // 7: 7sharps
                metaEvent->v1 = midistr[offset++];
                // Mode:
                // 0: Major Key
                // 1: Minor Key
                metaEvent->v2 = midistr[offset++];   // Mode
                return metaEvent;
            }
            //0x01~0x07 string contents
        case 0x01:  // TextEvent
        case 0x02:  // CopyRight
        case 0x03:  // SequenceOrTrackName 
        case 0x04:  // InstrumentName
        case 0x05:  // Lyric
        case 0x06:  // Marker
        case 0x07:  // CuePoint
        case 0x7f: // SequencerSpecificInformation
        default:
            metaEvent->content = getDWordContent(midistr, offset, 1);
            metaEvent->size = metaEvent->content.size();
            return metaEvent;

    }
    std::cerr << "MetaEvent wrong size of " << metaEvent->type << metaEvent->size;
    return metaEvent;
}

//8-E
MidiEvent* MidiFile::importMidiEvent(const std::string& midistr, size_t& offset, unsigned char head)
{
    MidiEvent* e = new MidiEvent();
    // head&0xf : channelNumber
    e->type = (head>>4);
    switch (e->type)
    {
        case 0x8://note off: pitch, velocity
        case 0x9://note on: pitch, velocity
        case 0xA:// key after touch: pitch, amount
        case 0xB:// Control Change: control, value
        case 0xE:// PitchWheelChange:  BottomValue, TopValue
            e->v1 = midistr[offset++];
            e->v2 = midistr[offset++];
            e->size = 2;
            //KeyNumberToNote(MidiToKeyNumber(c1), n); n: note
            break;
        case 0xC:// Program Change: New program Number
        case 0xD:// ChannelAfterTouch:  ChannelNumber
            e->v1 = midistr[offset++];
            e->size = 1;
            break;
        case 0xF: // SysxMessage with itemp octets
            e->content = getDWord(midistr, offset);
            e->size = e->content.size();
            break;
        default:
            throw std::runtime_error("Unsupported MidiEvent.");
    }
    return e;
}

Event* MidiFile::importEvent(const std::string& midistr, size_t& offset)
{
    Event* e = nullptr;

    size_t deltaTime = getDWord(midistr, offset);
    unsigned char baseType = midistr[offset++];
    if ( baseType && 0x80 )
    {
        // Meta Event
        if (baseType == 0xff) {
            e = importMetaEvent(midistr, offset);
        }
        else if ((baseType>>4)==0xf) { // SysexEvent
            e = new Event();
            e->content = getDWordContent(midistr, offset, 0);
            e->size = e->content.size();
        }
        else if (baseType>>4) { // MidiEvent
            e = importMidiEvent(midistr, offset, baseType);
        }
        else {
            throw std::runtime_error("Unsupported Event.");
        }
    } else {
        std::cerr << "Useless data: " << baseType;
    }
    e->deltaTime = deltaTime;
    e->baseType = baseType;
    return e;
}

void MidiFile::importHeaderChunk(const std::string& midistr, size_t& offset)
{
    //check header chunk head("MThd") and header chunk size(6)
    if (midistr.size()<offset+4 || midistr.substr(offset, 4)!="MThd"
            || !(headerChunk.chunkSize=getNBitsNumber(midistr, offset=offset+4, 4)) || headerChunk.chunkSize!=6 )
    {
        throw std::runtime_error("HeaderChunk Wrong file head.\n");
    }

    headerChunk.chunkID = "MThd";
    headerChunk.format = getNBitsNumber(midistr, offset, 2);
    headerChunk.tracksNumber = getNBitsNumber(midistr, offset, 2);
    headerChunk.deltaTimeTicks = getNBitsNumber(midistr, offset, 2);
}

void MidiFile::importTrackChunks(const std::string& midistr, size_t& offset)
{
    TrackChunk *trackChunk = new TrackChunk();

    //check track chunk head("MTrk") and track chunk size(shold not be 0)
    if (midistr.size()<offset+4 || midistr.substr(offset, 4)!="MTrk"
            || !(trackChunk->chunkSize=getNBitsNumber(midistr, offset=offset+4, 4)) )
    {
        throw std::runtime_error("TrackChunk Wrong file head.\n");
    }

    trackChunk->chunkID = "MTrk";

    //read event
    while (true)
    {
        Event* e = importEvent(midistr, offset);
        trackChunk->Events.push_back(e);
        if(e->size==0)
            break;
    }

    trackChunks.push_back(trackChunk);
}

void MidiFile::importMidiFile(const std::string& fileName)
{
    try {
        std::string midistr;
        {
            std::ifstream midifp(fileName.c_str(), std::ios::in | std::ios::binary);
            if(!midifp)
            {
                throw std::runtime_error(std::strerror(errno));
            }
            midistr = std::string((std::istreambuf_iterator<char>(midifp)), std::istreambuf_iterator<char>());
        }

        size_t offset = 0;
        //read midi file head chunk
        importHeaderChunk(midistr, offset);

        //read midi file track chunks
        for (size_t i=0; i<headerChunk.tracksNumber; i++)
        {
            importTrackChunks(midistr, offset);
        }

    } catch (const std::exception& e) {
        std::cerr << fileName << " " << e.what();
    }
}
