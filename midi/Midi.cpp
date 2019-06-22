#include "Midi.hpp"
#include "Music.hpp"

#include <cerrno>
#include <cstring> // strerror
#include <iostream>
#include <cmath> // log
#include <sstream>

namespace Midi
{
    int MidiUtility::getNBitsNumber(const std::string &midistr, size_t& offset, int bits)
    {
        int number = 0;
        while (bits--) {
            number = (number<<8) | (unsigned char)midistr[offset++];
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
            if (type) {
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

    template<class T>
        typename std::enable_if<std::is_fundamental<T>::value, std::string>::type
        MidiUtility::addXMLAttribute(const std::string& attribute, T& value)
        {
            return std::string(" " + attribute + "=\"" + std::to_string(value) + "\" ");
        }

    template<class T>
        typename std::enable_if<!std::is_fundamental<T>::value, std::string>::type
        MidiUtility::addXMLAttribute(const std::string& attribute, T& value)
        {
            return std::string(" " + attribute + "=\"" + value + "\" ");
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

    bool Event::compare(EventType t, int vv1, int vv2, int vv3, int vv4, int vv5)
    {
        if(t!=eventType || v1!=vv1 || v2!=vv2)
            return false;
        return true;
    }

    bool Event::edit(EventType t, int vv1, int vv2, int vv3, int vv4, int vv5)
    {
        if(eventType != t)
            return false;
        v1 = vv1, v2 = vv2;
        return true;
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
                    return;
                }
            case 0x32:  // 20 ChannelPrefix
                size = MidiUtility::getDWord(midistr, offset);
                if (size == 1) {
                    v1 = midistr[offset++];   // channel number
                    return;
                }
            case 0x2f: //47 EndOfTrack
                size = MidiUtility::getDWord(midistr, offset);
                if(size == 0) {
                    return;
                }
            case 0x51: //81 SetTempo
                size = MidiUtility::getDWord(midistr, offset);
                if (size == 3)
                {
                    v1 = MidiUtility::getNBitsNumber(midistr, offset, 3);
                    //BPM = 60,000,000/(tt tt tt)
                    //BPM(Beats Per Minute): a tempo of 100 BPM means that 100 quarter notes per minute
                    //MicrosecondsPerQuarterNote=itemp BPM=60000000/itemp
                    return;
                }
            case 0x54: //84 SMPTE Offset
                size = MidiUtility::getDWord(midistr, offset);
                if (size == 5) {
                    v1 = midistr[offset++];   // hour
                    v2 = midistr[offset++];   // min
                    v3 = midistr[offset++];   // sec
                    v4 = midistr[offset++];   // fr
                    v5 = midistr[offset++];   // subfr
                    return;
                }
            case 0x58: //88 TimeSignature
                size = MidiUtility::getDWord(midistr, offset);
                if (size == 4) {
                    v1 = midistr[offset++];   // Numerator
                    v2 = midistr[offset++];   // LogDenominator
                    v3 = midistr[offset++];   // MIDIClocksPerMetronomeClick = Numberator * interval /(16*4);
                    v4 = midistr[offset++];   // ThirtySecondsPer24Clocks = interval / (16*3);
                    if (v4*3 != v3*4/v1) {
                        std::cerr << "Wrong Meta TimeSignature event value\n";
                    }
                    return;
                }
            case 0x59: // KeySignature
                size = MidiUtility::getDWord(midistr, offset);
                if(size == 2)
                {
                    // Fifths(-7: 7flats, -1: 1flat, 0: keyofC, 1: 1sharp, 7: 7sharps)
                    v1 = midistr[offset++];
                    // Mode(0: Major Key, 1: Minor Key)
                    v2 = midistr[offset++];   // Mode
                    return;
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
                return;
        }
        std::cerr << "MetaEvent wrong size of " << type << size;
        return ;
    }

    void MetaEvent::exportEvent(std::string& midistr)
    {
        MidiUtility::writeDWord(midistr, 0); 	//delta time = 0
        MidiUtility::writeNBitsNumber(midistr, 0xff, 1);
        MidiUtility::writeNBitsNumber(midistr, type, 1);

        // Special case: type 0x51 tempo
        if (type == 0x51) {
            MidiUtility::writeNBitsNumber(midistr, 3, 1);   // size
            MidiUtility::writeNBitsNumber(midistr, v1, 3);
            return ;
        }

        // string
        std::string str;
        if(!content.empty()) 
        {
            str = content;
        } else {
            if(v1!=-1) {
                MidiUtility::writeNBitsNumber(str, v1, 1);
            }
            if(v2!=-1) {
                MidiUtility::writeNBitsNumber(str, v2, 1);
            }
            if(v3!=-1) {
                MidiUtility::writeNBitsNumber(str, v3, 1);
            }
            if(v4!=-1) {
                MidiUtility::writeNBitsNumber(str, v4, 1);
            }
            if(v5!=-1) {
                MidiUtility::writeNBitsNumber(str, v5, 1);
            }
        }

        MidiUtility::writeNBitsNumber(midistr, str.size(), 1);	//length
        MidiUtility::writeString(midistr, str);
    }

    void MetaEvent::exportEvent2XML(std::ofstream& midifp)
    {
        midifp << "\t<MetaEvent" << MidiUtility::addXMLAttribute("deltaTime", deltaTime);

        switch (type)
        {
            case 0x00:
                midifp << MidiUtility::addXMLAttribute("Name", "SequenceNumber")
                    << MidiUtility::addXMLAttribute("MSB", v1)
                    << MidiUtility::addXMLAttribute("LSB", v2);
                break;
            case 0x32:  // 20 ChannelPrefix
                midifp << MidiUtility::addXMLAttribute("Name", "ChannelPrefix")
                    << MidiUtility::addXMLAttribute("ChannelNumber", v1);
                break;
            case 0x2f: //47 EndOfTrack
                midifp << MidiUtility::addXMLAttribute("Name", "EndOfTrack");
                break;
            case 0x51: //81 SetTempo
                {
                    size_t beatsPerMinute = 60000000/v1;
                    midifp << MidiUtility::addXMLAttribute("Name", "SetTempo")
                        << MidiUtility::addXMLAttribute("MicrosecondsPerQuarterNote", v1)
                        << MidiUtility::addXMLAttribute("BeatsPerMinute", beatsPerMinute);
                    break;
                }
            case 0x54: //84 SMPTE Offset
                midifp << MidiUtility::addXMLAttribute("Name", "SMPTE Offset")
                    << MidiUtility::addXMLAttribute("hour", v1)
                    << MidiUtility::addXMLAttribute("min", v2)
                    << MidiUtility::addXMLAttribute("sec", v3)
                    << MidiUtility::addXMLAttribute("fr", v4)
                    << MidiUtility::addXMLAttribute("subfr", v5);
                break;
            case 0x58: //88 TimeSignature
                midifp << MidiUtility::addXMLAttribute("Name", "TimeSignature")
                    << MidiUtility::addXMLAttribute("Numerator", v1)
                    << MidiUtility::addXMLAttribute("LogDenominator", v2)
                    << MidiUtility::addXMLAttribute("MIDIClocksPerMetronomeClick", v3)
                    << MidiUtility::addXMLAttribute("ThirtySecondsPer24Clocks", v4);
                break;
            case 0x59: // KeySignature
                midifp << MidiUtility::addXMLAttribute("Name", "KeySignature");
                if (-7<=v1 && v1<0) {
                    std::string str = std::to_string(-v1)+"Flats";
                    midifp << MidiUtility::addXMLAttribute("Fifths", str);
                } else if(v1==0){
                    midifp << MidiUtility::addXMLAttribute("Fifths", "KeyOfC");
                } else if(0<v1 && v1<=7) {
                    std::string str = std::to_string(v1)+"Sharp";
                    midifp << MidiUtility::addXMLAttribute("Fifths", str);
                } else {
                    std::cerr << "MetaEvent 0x59 KeySignature: Invalid Fifths " << v1 << "\n";
                }
                switch (v2) {
                    case 0:
                        midifp << MidiUtility::addXMLAttribute("Mode", "MajorKey");
                        break;
                    case 1:
                        midifp << MidiUtility::addXMLAttribute("Mode", "MinorKey");
                        break;
                    default:
                        std::cerr << "MetaEvent 0x59 KeySignature: Invalid mode " << v2 << "\n";
                        break;
                }
                break;
            case 0x01: 
                midifp << MidiUtility::addXMLAttribute("TextEvent", content);
                break;
            case 0x02:
                midifp << MidiUtility::addXMLAttribute("CopyRight", content);
                break;
            case 0x03:
                midifp << MidiUtility::addXMLAttribute("SequenceOrTrackName", content);
                break;
            case 0x04:
                midifp << MidiUtility::addXMLAttribute("InstrumentName", content);
                break;
            case 0x05:
                midifp << MidiUtility::addXMLAttribute("Lyric", content);
                break;
            case 0x06:
                midifp << MidiUtility::addXMLAttribute("Marker", content);
                break;
            case 0x07:
                midifp << MidiUtility::addXMLAttribute("CuePoint", content);
                break;
            case 0x7f:
                midifp << MidiUtility::addXMLAttribute("SequencerSpecificInformation", content);
                break;
        }
        midifp << "></MetaEvent>\n";
    }

    void MetaEvent::setContent(unsigned char type, std::string c) // 0x51
    {
        type = type;
        content = c;
        size = content.size();
    }

    void MetaEvent::setTempo(int tempo) // 0x51
    {
        type = 0x51;
        v1 = (int) (60.0 * 1000000 / tempo); //convert second to microsecond
        size = 3;
    }

    void MetaEvent::setSMPTEOffset(int hour, int minute, int second, int frame, int subframe) // 0x54
    {
        type = 0x54;
        v1 = hour, v2 = minute, v3 = second, v4 = frame, v5 = subframe;
        size = 5;
    }

    void MetaEvent::setTimeSignature(int numer, int denom, int interval) // 0x58
    {
        type = 0x58;
        v1 = numer;
        v2 = (int)(log(denom) / log(2) + 0.5);
        v3 = numer * interval /(16*4);
        v4 = interval / (16*3);
        size = 4;
    }

    void MetaEvent::setKeySignature(int sf, int mi) // 0x59
    {
        type = 0x59;
        v1 = sf, v2 = mi;
        size = 2;
    }

    bool MetaEvent::compare(EventType t, int vv1, int vv2, int vv3, int vv4, int vv5)
    {
        if(!Event::compare(t, vv1, vv2) || v3!=vv3 || v4!=vv4 || v5!=vv5)
            return false;
        return true;
    }

    bool MetaEvent::edit(EventType type, int vv1, int vv2, int vv3, int vv4, int vv5)
    {
        if(!Event::edit(type, vv1, vv2, vv3, vv4, vv4))
            return false;
        v3 = vv3, v4 = vv4, v5 = vv5;
        return true;
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
                throw std::runtime_error("Unsupported MidiEvent.\n");
        }
        throw std::runtime_error("Unsupported MidiEvent.\n");
    }

    void MidiEvent::exportEvent(std::string& midistr)
    {
        MidiUtility::writeDWord(midistr, deltaTime); 	//delta time
        MidiUtility::writeNBitsNumber(midistr, baseType, 1); // type
        switch(type)
        {
            case 0x8://note off: pitch, velocity
            case 0x9://note on: pitch, velocity
            case 0xA://key after touch: pitch, amount
            case 0xB://Control Change: control, value
            case 0xE://PitchWheelChange:  BottomValue, TopValue
                MidiUtility::writeNBitsNumber(midistr, v1, 1);
                MidiUtility::writeNBitsNumber(midistr, v2, 1);
                return ;
            case 0xC:// Program Change: New program Number
            case 0xD:// ChannelAfterTouch:  ChannelNumber
                MidiUtility::writeNBitsNumber(midistr, v1, 1);
                return ;
            default:
                break;
        }
    }

    void MidiEvent::exportEvent2XML(std::ofstream& midifp)
    {
        int channelNumber = baseType&0x0f;
        midifp << "\t<MidiEvent" << MidiUtility::addXMLAttribute("deltaTime", deltaTime)
            << MidiUtility::addXMLAttribute("ChannelNumber", channelNumber);

        switch(type)
        {
            case 0x8:
                {
                    std::string note = MusicUtility::MidiNumber2Note(v1);
                    midifp << MidiUtility::addXMLAttribute("Name", "Note Off")
                        << MidiUtility::addXMLAttribute("pitch", v1)
                        << MidiUtility::addXMLAttribute("note", note)
                        << MidiUtility::addXMLAttribute("velocity", v2);
                    break;
                }
            case 0x9:
                {
                    std::string note = MusicUtility::MidiNumber2Note(v1);
                    midifp << MidiUtility::addXMLAttribute("Name", "Note On")
                        << MidiUtility::addXMLAttribute("pitch", v1)
                        << MidiUtility::addXMLAttribute("note", note)
                        << MidiUtility::addXMLAttribute("velocity", v2);
                    break;
                }
            case 0xA:
                midifp << MidiUtility::addXMLAttribute("Name", "Key after touch")
                    << MidiUtility::addXMLAttribute("pitch", v1)
                    << MidiUtility::addXMLAttribute("amount", v2);
                break;
            case 0xB:
                midifp << MidiUtility::addXMLAttribute("Name", "Control Change")
                    << MidiUtility::addXMLAttribute("control", v1)
                    << MidiUtility::addXMLAttribute("value", v2);
                break;
            case 0xE:
                midifp << MidiUtility::addXMLAttribute("Name", "PitchWheelChange")
                    << MidiUtility::addXMLAttribute("BottomValue", v1)
                    << MidiUtility::addXMLAttribute("TopValue", v2);
                break;
            case 0xC:
                midifp << MidiUtility::addXMLAttribute("Name", "Program Change")
                    << MidiUtility::addXMLAttribute("NewProgramNumber", v1);
                break;
            case 0xD:
                midifp << MidiUtility::addXMLAttribute("Name", "ChannelAfterTouch")
                    << MidiUtility::addXMLAttribute("ChannelNumber", v1);
                break;
            default:
                break;
        }

        midifp << "></MidiEvent>\n";
    }

    void MidiEvent::importEventFromTXT(const std::string& midistr, int noteChannel, int& lastTime)
    {
        std::istringstream ls(midistr);
        std::string time, noteType, noteNumber, noteVelocity;
        ls >> time >> noteType >> noteNumber >> noteVelocity;
        if (stoi(noteType)) { // 1 on
            setNoteOn(stoi(time)-lastTime, noteChannel, stoi(noteNumber, nullptr, 16), stoi(noteVelocity, nullptr, 16)); // 9
        } else {
            setNoteOff(stoi(time)-lastTime, noteChannel, stoi(noteNumber, nullptr, 16), stoi(noteVelocity, nullptr, 16)); // 8 
        }
        lastTime = stoi(time);
    }

    void MidiEvent::exportEvent2TXT(std::ofstream& midifp, size_t& time)
    {
        if (type == 0x8 || type == 0x9) {
            time += deltaTime;
            midifp << std::dec << time << " " << type-8 << " "
                << std::hex << v1 << " " << v2 << "\n";
        }
    }

    void MidiEvent::setEvent(unsigned char Type, size_t DeltaTime, int noteChannel, int V1, int V2)
    {
        baseType = (Type<<4) + noteChannel;
        type = Type;
        deltaTime = DeltaTime;
        v1 = V1, v2 = V2;
        switch(type)
        {
            case 8:
            case 9:
            case 0xA:
            case 0xB:
            case 0xE:
                size += 3;
                break;
            case 0xC:
            case 0xD:
                size += 2;
                break;
            default:
                std::cerr << "Wrong midi type.\n";
        }
    }

    void MidiEvent::setNoteOff(size_t deltaTime, int noteChannel, int noteNumber, int velocity) { // 8
        setEvent(8, deltaTime, noteChannel, noteNumber, velocity);
    }

    void MidiEvent::setNoteOn(size_t deltaTime, int noteChannel, int noteNumber, int velocity) { // 9
        setEvent(9, deltaTime, noteChannel, noteNumber, velocity);
    }

    void MidiEvent::setKeyAfterTouch(int noteChannel, int noteNumber, int velocity) { // A
        setEvent(0xA, 0, noteChannel, noteNumber, velocity);
    }

    void MidiEvent::setControlChange(int noteChannel, int controllerNumber, int newValue) { // B
        setEvent(0xB, 0, noteChannel, controllerNumber, newValue);
    }

    void MidiEvent::setProgramChange(int noteChannel, int newProgramNumber) { // C
        setEvent(0xC, 0, noteChannel, newProgramNumber, 0);
    }

    void MidiEvent::setChannelAfterTouch(int noteChannel, int channelNumber) { // D
        setEvent(0xD, 0, noteChannel, channelNumber, 0);
    }

    void MidiEvent::setPitchWheel(int NoteChannel, int pitch) //E
    {
        if (pitch>8191 || pitch<-8192) {
            std::cerr << "Wrong Pitch" << pitch << size;
        }
        pitch = pitch - (-8192);
        setEvent(0xE, 0, NoteChannel, pitch%128, pitch/128);
    }

    //volume range: 0 - 0xFFFF
    void MidiEvent::setSoundVolume(int noteChannel, int volume) {
        setControlChange(noteChannel, 7, volume>>8);
        setControlChange(noteChannel, 39, volume&0xff);
    }

    void SysexEvent::importEvent(const std::string& midistr, size_t& offset)
    {
        baseType = midistr[offset++];
        content = MidiUtility::getString(midistr, offset, 0);
        size = content.size();
        return;
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
                throw std::runtime_error("Unsupported Event.\n");
            }
        } else {
            throw std::runtime_error("Useless data.\n");
        }
        e->importEvent(midistr, offset);
        e->setDeltaTime(deltaTime);
        return e;
    }

    bool TrackChunk::deleteEvent(EventType t, int v1, int v2, int v3, int v4, int v5)
    {
        for(auto it=Events.begin(); it!=Events.end(); it++)
        {
            if((*it)->compare(t, v1, v2, v3, v4, v5)) 
            {
                Events.erase(it);
                return true;
            }
        }
        return false;
    }

    bool TrackChunk::editEvent(size_t eventIndex, EventType type, int v1, int v2, int v3, int v4, int v5)
    {
        if(eventIndex < 0 || Events.size() < eventIndex)
            return false;
        return Events[eventIndex]->edit(type, v1, v2, v3, v4, v5);
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

    void HeaderChunk::exportChunk2XML(std::ofstream& midifp, size_t trackNumber)
    {
        midifp << "<HeaderChunk>\n"
            << "Format Type: " << format << "\n"
            << "Track Number: " << tracksNumber << "\n"
            << "DeltaTime: " << deltaTimeTicks << "\n"
            << "</HeaderChunk>\n";
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

    //write midi track to the midi file
    void TrackChunk::exportChunk(std::string& midistr)
    {
        MidiUtility::writeString(midistr, chunkID);		//midi head: MTrk

        std::string str;
        for (size_t i=0; i<Events.size(); i++) {
            Events[i]->exportEvent(str);
        }
        MidiUtility::writeNBitsNumber(midistr, str.size(), 4);	//chunk size
        MidiUtility::writeString(midistr, str);	//chunk size
    }

    void TrackChunk::exportChunk2XML(std::ofstream& midifp, size_t trackNumber)
    {
        midifp << "<TrackChunk" << MidiUtility::addXMLAttribute("TrackNumber", trackNumber) << ">\n";
        for (size_t i=0; i<Events.size(); i++) {
            Events[i]->exportEvent2XML(midifp);
        }
        midifp << "</TrackChunk>\n";
    }

    void TrackChunk::importChunkFromTXT(const std::string& txtName)
    {
        std::ifstream midifp(txtName, std::ifstream::in);
        if (!midifp) {
            throw std::runtime_error("Could not open " + txtName + "for writing.\n");
        }

        // set up meta event
        int deltaTimeTicks = 500000;
        MetaEvent *e1 = new MetaEvent();
        e1->setTempo(deltaTimeTicks);
        Events.push_back(e1);

        e1 = new MetaEvent();
        e1->setTimeSignature(4, 4, deltaTimeTicks); // 4/4 C Major
        Events.push_back(e1);

        e1 = new MetaEvent();
        e1->setKeySignature(3, 1);
        Events.push_back(e1);

        MidiEvent *e2 = new MidiEvent();
        e2->setProgramChange(0, 0x21);
        Events.push_back(e2);

        e2 = new MidiEvent();
        e2->setControlChange(0, 0x7, 0x7f);
        //e2->setSoundVolume(0, 0);
        Events.push_back(e2);

        // import midi event data from txt file
        int lastTime = 0;
        int noteChannel = 0;
        std::string line;
        while (std::getline(midifp,line)) {
            MidiEvent* e = new MidiEvent();
            e->importEventFromTXT(line, noteChannel, lastTime);
            Events.push_back(e);
        }
    }

    void TrackChunk::exportChunk2TXT(const std::string& txtName)
    {
        std::ofstream midifp(txtName);
        if (!midifp) {
            throw std::runtime_error("Could not open " + txtName + " for writing.\n");
        }

        for(size_t i=0, time = 0; i<Events.size(); i++) {
            Events[i]->exportEvent2TXT(midifp, time);
        }
    }

    void MidiFile::importMidiFile(const std::string& fileName)
    {
        try {
            if(trackChunks.size())
            {
                throw std::runtime_error("Create another MidiFile object for importing.\n");
            }

            std::string midistr;
            {
                std::ifstream midifp(fileName.c_str(), std::ios::in | std::ios::binary);
                if(!midifp) {
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
        for (size_t i=0; i<headerChunk.getTracksNumber(); i++) {
            trackChunks[i]->exportChunk(midistr);
        }

        try {
            std::ofstream midifp(fileName.c_str(), std::ios::binary);
            if (!midifp) {
                throw std::runtime_error("Could not open " + fileName + "for writing.\n");
            }
            std::copy(midistr.begin(), midistr.end(), std::ostreambuf_iterator<char>(midifp));
        } catch (const std::exception& e) {
            std::cerr << fileName << " " << e.what();
        }
    }

    void MidiFile::exportXMLFile(const std::string& fileName)
    {
        try {
            std::ofstream midifp(fileName.c_str());
            if (!midifp) {
                throw std::runtime_error("Could not open " + fileName + "for writing.\n");
            }
            midifp << "<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>\n";
            midifp << "<!DOCTYPE MIDIFile PUBLIC\n";
            midifp << "  \"-//Recordare//DTD MusicXML 0.9 MIDI//EN\"\n";
            midifp << "  \"http://www.musicxml.org/dtds/midixml.dtd\">\n";
            midifp << "<MIDIFile>\n";

            headerChunk.exportChunk2XML(midifp);
            for (size_t i=0; i<headerChunk.getTracksNumber(); i++) {
                trackChunks[i]->exportChunk2XML(midifp, i);
            }

            midifp << "</MIDIFile>\n";
            midifp.close();
        } catch (const std::exception& e) {
            std::cerr << fileName << " " << e.what();
        }
    }

    void MidiFile::importMidiTXT(const std::string& midiTxt)
    {
        try {
            if(trackChunks.size()) {
                throw std::runtime_error("Create another MidiFile object for importing.\n");
            }

            //HeaderChunk: default setting

            //TrackChunk
            TrackChunk *trackChunk = new TrackChunk();
            trackChunk->importChunkFromTXT(midiTxt);
            trackChunks.push_back(trackChunk);

        } catch (const std::exception& e) {
            std::cerr << midiTxt << " " << e.what();
        }
    }

    void MidiFile::exportMidiTXT(const std::string& txtName)
    {
        try {
            // empty MidiFile
            if (!trackChunks.size()) {
                throw std::runtime_error("Export empty midiFile\n");
            }

            trackChunks[0]->exportChunk2TXT(txtName);
        } catch (const std::exception& e) {
            std::cerr << txtName << " " << e.what();
        }
    }

    bool MidiFile::deleteTrackEvent(size_t trackIndex, EventType t, int v1, int v2, int v3, int v4, int v5)
    {
        return trackChunks[trackIndex]->deleteEvent(t, v1, v2, v3, v4, v5);
    }

    bool MidiFile::editTrackEvent(size_t trackIndex, size_t eventIndex, EventType t, int v1, int v2, int v3, int v4, int v5)
    {
        return trackChunks[trackIndex]->editEvent(eventIndex, t, v1, v2, v3, v4, v5);
    }
}
