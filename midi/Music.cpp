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
	for (int j=0; j<NoteArraySize; j++)
	{
		NoteArray[j] = 0;
        std::string note = decode(j);

		for (int i=0; i<scaleSize; i++)
		{
            if((note.size()==2 && note.substr(1)==scale[i])
            || (note.size()==3 && scale[i].find(note.substr(1))!=std::string::npos))
            {
				NoteArray[j] = (i + (note[0]-'0')*scaleSize-8);
				break;
            }
		}
	}
    
    validate();
}

void MusicUtility::validate()
{
    for(int i=0; i<88; i++)
    {
        std::string note = KeyNumber2Note(i);
        assert(decode(code(note)) == note);
        assert(i == Note2KeyNumber(note));
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
	int i = code(note);
	if ( i<0 || i>=NoteArraySize)
	{
        return -1;
	}
    if (NoteArray[0] == -1)
    {
	    initNoteArray();
    }
	return NoteArray[i];
}

std::string MusicUtility::KeyNumber2Note(int k)
{
    std::string ret(1, ((k+9)/scaleSize) + '0');
	int i = (k+9) % scaleSize -1;
	if (i < 0)
	{
		ret[0]--;
		i += scaleSize;
	}
    return ret + scale[i].substr(0, 2); 
}

int MusicUtility::Note2MidiNumber(std::string& note)
{
    return KeyNumber2Midi(Note2KeyNumber(note));
}

std::string MusicUtility::MidiNumber2Note(int PianoKeyNumber)
{
    return KeyNumber2Note(Midi2KeyNumber(PianoKeyNumber));
}
