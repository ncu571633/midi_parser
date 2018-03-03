#include <cassert>
#include "Music.hpp"

const std::string MusicUtility::scale[] = {"c","c+d-","d", "d+e-","e","f","f+g-","g","g+a-","a","a+b-","b"};
int MusicUtility::scaleSize = sizeof(MusicUtility::scale) / sizeof(MusicUtility::scale[0]);   // 12
std::vector<int> MusicUtility::NoteArray(MusicUtility::NoteArraySize, -1);

int MusicUtility::code(const std::string& note)
{
	return ((note[0]-'0')<<5) + ((note[1]-'a')<<2) + ( note.size()==3?(((~(note[2]-'+'))&0b11)-1):(0b01) );
}

std::string MusicUtility::decode(int number)
{
    std::string ret;
    ret.push_back(( (number & 0b11100000) >>5 ) + '0');
    ret.push_back(( (number & 0b00011100) >>2 ) + 'a');
    if((number&0b11) != 0b01) {
        ret.push_back(((~number&0b11)+'+'-1));
    }
    return ret;
}

void MusicUtility::initNoteArray()
{
	for (int j=0; j<MusicUtility::NoteArraySize; j++)
	{
		MusicUtility::NoteArray[j] = 0; 
        std::string note = MusicUtility::decode(j);
		for (int i=0; i<MusicUtility::scaleSize; i++)
		{
            if(note.substr(1)==MusicUtility::scale[i])
            {
				MusicUtility::NoteArray[j] = (i + (note[0]-'0')*MusicUtility::scaleSize-8);
				break;
            }
		}
	}
    
    MusicUtility::validate();
}

void MusicUtility::validate()
{
    for(int i=0; i<128; i++)
    {
        std::string note = MusicUtility::KeyNumber2Note(i);
        assert(i == MusicUtility::Note2KeyNumber(note));
    }
}

int MusicUtility::Midi2KeyNumber(int MidiNumber)
{
	return MidiNumber - 8;
}

int MusicUtility::KeyNumber2Midi(int KeyNumber)
{
	return KeyNumber + 8;
}

int MusicUtility::Note2KeyNumber(std::string& note)
{
	int i = MusicUtility::code(note);
	if ( i<0 || i>=MusicUtility::NoteArraySize)
	{
        return -1;
	}
    if (MusicUtility::NoteArray[0] == -1)
    {
	    MusicUtility::initNoteArray();
    }
	return MusicUtility::NoteArray[i];
}

std::string MusicUtility::KeyNumber2Note(int k)
{
    return std::string(1, ((k+9)/scaleSize) + '0') + scale[(k+8)%scaleSize];
}

int MusicUtility::Note2MidiNumber(std::string& note)
{
    return MusicUtility::KeyNumber2Midi(MusicUtility::Note2KeyNumber(note));
}

std::string MusicUtility::MidiNumber2Note(int PianoKeyNumber)
{
    return MusicUtility::KeyNumber2Note(MusicUtility::Midi2KeyNumber(PianoKeyNumber));
}
