#include "Midi.hpp"

#include <iostream>
#include <fstream>
#include <cerrno>
#include <cstring>

int MidiUtility::getNBitsNumber(const std::string &midistr, size_t& offset, int bits)
{
    int number = 0;
    while (bits--) {
        number = (number<<8) | midistr[offset++];
    }
    return number;
}

size_t MidiUtility::writeNBitsNumberHelper(int bits)
{
	size_t a = 0;
	while (bits--)
		a = (a<<8) + 0xff;
	return a;
}

void MidiUtility::writeNBitsNumber(std::string &midistr, size_t value, int bits)
{
	while ((bits--)>1)
	{
		midistr.push_back(value >> (bits*8));
        value = ( value & writeNBitsNumberHelper(bits) );

	}
    midistr.push_back(value);
}

size_t MidiUtility::getDWord(const std::string &midistr, size_t& offset)
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

void MidiUtility::writeDWord(std::string &midistr, size_t value)
{
    size_t buffer = value & 0x7f;
    while ( (value >>= 7) )
    {
        buffer <<= 8;
        buffer |= ((value & 0x7f) | 0x80);
    }
    while (true)
    {
        midistr.push_back((unsigned char)buffer);
        if (buffer & 0x80)
            buffer >>= 8;
        else
            break;
    }
}

std::string MidiUtility::getString(const std::string &midistr, size_t& offset, int type)
{
    std::string str;
    size_t len = MidiUtility::getDWord(midistr, offset);
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

void MidiUtility::writeString(std::string &midistr, const std::string& str)
{
    midistr += str;
}

TrackChunk::~TrackChunk()
{
    for(Event* event: Events) {
        delete event;
    }
}

MidiFile::~MidiFile()
{
    for(TrackChunk* t: trackChunks) {
        delete(t);
    }
}
//ff -> meta event
void MetaEvent::importEvent(const std::string& midistr, size_t& offset)
{
    baseType=midistr[offset++];
    type = midistr[offset++]; 
    switch (type)
    {
        case 0x00: // SequenceNumber
            size = MidiUtility::getDWord(midistr, offset);
            if (size == 2)
            {
                v1 = midistr[offset++];   //MSB
                v2 = midistr[offset++];   //LSB
                return ;
            }

        case 0x32:  // 20 ChannelPrefix
            size = MidiUtility::getDWord(midistr, offset);
            if (size == 1) {
                v1 = midistr[offset++];   // channel number
                return ;
            }
        case 0x2f: //47 EndOfTrack
            size = MidiUtility::getDWord(midistr, offset);
            if(size == 0) {
                return ;
            }
        case 0x51: //81 SetTempo
            size = MidiUtility::getDWord(midistr, offset);
            if (size == 3)
            {
                v1 = MidiUtility::getNBitsNumber(midistr, offset, 3);
                //BPM = 60,000,000/(tt tt tt)
                //BPM(Beats Per Minute): a tempo of 100 BPM means that 100 quarter notes per minute
                //MicrosecondsPerQuarterNote=itemp BPM=60000000/itemp
                return ;
            }
        case 0x54: //84 SMPTE Offset
            size = MidiUtility::getDWord(midistr, offset);
            if (size == 5) {
                v1 = midistr[offset++];   // hour
                v2 = midistr[offset++];   // min
                v3 = midistr[offset++];   // sec
                v4 = midistr[offset++];   // fr
                v5 = midistr[offset++];   // subfr
                return ;
            }
        case 0x58: //88 TimeSignature
            size = MidiUtility::getDWord(midistr, offset);
            if (size == 4) {
                v1 = midistr[offset++];   // Numerator
                v2 = midistr[offset++];   // LogDenominator
                v3 = midistr[offset++];   // MIDIClocksPerMetronomeClick
                v4 = midistr[offset++];   // ThirtySecondsPer24Clocks
                return ;
            }
        case 0x59: // KeySignature
            size = MidiUtility::getDWord(midistr, offset);
            if(size == 2)
            {
                // Fifths:
                // -7: 7flats
                // -1: 1flat
                // 0; keyofC
                // 1: 1sharp
                // 7: 7sharps
                v1 = midistr[offset++];
                // Mode:
                // 0: Major Key
                // 1: Minor Key
                v2 = midistr[offset++];   // Mode
                return ;
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
            content = MidiUtility::getString(midistr, offset, 1);
            size = content.size();
            return ;

    }
    std::cerr << "MetaEvent wrong size of " << type << size;
}

//8-E
void MidiEvent::importEvent(const std::string& midistr, size_t& offset)
{
    baseType = midistr[offset++];
    // head&0xf : channelNumber
    type = (baseType>>4);
    switch (type)
    {
        case 0x8://note off: pitch, velocity
        case 0x9://note on: pitch, velocity
        case 0xA:// key after touch: pitch, amount
        case 0xB:// Control Change: control, value
        case 0xE:// PitchWheelChange:  BottomValue, TopValue
            v1 = midistr[offset++];
            v2 = midistr[offset++];
            size = 2;
            //KeyNumberToNote(MidiToKeyNumber(c1), n); n: note
            return ;
        case 0xC:// Program Change: New program Number
        case 0xD:// ChannelAfterTouch:  ChannelNumber
            v1 = midistr[offset++];
            size = 1;
            return ;
        case 0xF: // SysxMessage with itemp octets
            content = MidiUtility::getDWord(midistr, offset);
            size = content.size();
            return ;
        default:
            break;
    }
    throw std::runtime_error("Unsupported MidiEvent.");
}

void SysexEvent::importEvent(const std::string& midistr, size_t& offset)
{
    baseType = midistr[offset++];
    content = MidiUtility::getString(midistr, offset, 0);
    size = content.size();
}

Event* TrackChunk::importEvent(const std::string& midistr, size_t& offset)
{
    Event* e = nullptr;

    size_t deltaTime = MidiUtility::getDWord(midistr, offset);
    unsigned char baseType = midistr[offset];
    if ( baseType && 0x80 )
    {
        if (baseType == 0xff) { // Meta Event
            e = new MetaEvent();
        }
        else if ((baseType>>4)==0xf) { // SysexEvent
            e = new SysexEvent();
        }
        else if (baseType>>4) { // MidiEvent
            e = new MidiEvent();
        }
        else {
            throw std::runtime_error("Unsupported Event.");
        }
    } else {
        throw std::runtime_error("Useless data.");
    }
    e->importEvent(midistr, offset);
    e->setDeltaTime(deltaTime);
    return e;
}

void HeaderChunk::importChunk(const std::string& midistr, size_t& offset)
{
    //check header chunk head("MThd") and header chunk size(6)
    if (midistr.size()<offset+4 || midistr.substr(offset, 4)!="MThd"
            || !(chunkSize=MidiUtility::getNBitsNumber(midistr, offset=offset+4, 4)) || chunkSize!=6 )
    {
        throw std::runtime_error("HeaderChunk Wrong file head.\n");
    }

    chunkID = "MThd";
    format = MidiUtility::getNBitsNumber(midistr, offset, 2);
    tracksNumber = MidiUtility::getNBitsNumber(midistr, offset, 2);
    deltaTimeTicks = MidiUtility::getNBitsNumber(midistr, offset, 2);
}

void HeaderChunk::exportChunk(std::string& midistr)
{
    MidiUtility::writeString(midistr, chunkID);		//midi head: MThd
    MidiUtility::writeNBitsNumber(midistr, chunkSize, 4);	//chunk size: 6
    MidiUtility::writeNBitsNumber(midistr, format, 2);		//Format type
    MidiUtility::writeNBitsNumber(midistr, tracksNumber, 2);	//tracks number
    MidiUtility::writeNBitsNumber(midistr, deltaTimeTicks, 2);	//Delta-time ticks per quarter note
}

void TrackChunk::importChunk(const std::string& midistr, size_t& offset)
{
    //check track chunk head("MTrk") and track chunk size(shold not be 0)
    if (midistr.size()<offset+4 || midistr.substr(offset, 4)!="MTrk"
            || !(chunkSize=MidiUtility::getNBitsNumber(midistr, offset=offset+4, 4)) )
    {
        throw std::runtime_error("TrackChunk Wrong file head.\n");
    }

    chunkID = "MTrk";

    //read event
    while (true)
    {
        Event* e = importEvent(midistr, offset);
        Events.push_back(e);
        if(!(e->getSize()))
            break;
    }
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
        headerChunk.importChunk(midistr, offset);

        //read midi file track chunks
        for (size_t i=0; i<headerChunk.getTracksNumber(); i++)
        {
            TrackChunk *trackChunk = new TrackChunk();
            trackChunk->importChunk(midistr, offset);
            trackChunks.push_back(trackChunk);
        }

    } catch (const std::exception& e) {
        std::cerr << fileName << " " << e.what();
    }
}

void MidiFile::exportMidiFile(const std::string& fileName)
{
    std::string midistr;
    
    //write midi file head chunk
    headerChunk.exportChunk(midistr);

    //write midi file track chunks
    for (size_t i=0; i<headerChunk.getTracksNumber(); i++)
    {
        trackChunks.exportChunk(midistr);
    }

    try {
        std::ofstream midifp(fileName.c_str(), std::ios::binary);
        if (!midifp) {
            throw std::runtime_error("Could not open " + fileName + "for writing");
        }
        std::copy(midistr.begin(), midistr.end(), std::ostreambuf_iterator<char>(midifp));
    } catch (const std::exception& e) {
        std::cerr << fileName << " " << e.what();
    }
}
