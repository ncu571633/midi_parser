#include "Music.hpp"

const std::string MusicUtility::scale[] = {"c","c+d-","d", "d+e-","e","f","f+g-","g","g+a-","a","a+b-","b"};
int MusicUtility::scaleSize = sizeof(MusicUtility::scale) / sizeof(MusicUtility::scale[0]);   // 12
std::vector<int> MusicUtility::NoteArray;

int MusicUtility::code(const std::string& note)
{
	return ((note[0]-'0')<<5) + ((note[1]-'a')<<2) + ( note[2]?(((~(note[2]-'+'))&0b11)-1):(0b01) );
}

std::string MusicUtility::decode(int number)
{
    std::string ret;
    ret.push_back(( (number & 0b11100000) >>5 ) + '0');
    ret.push_back(( (number & 0b00011100) >>2 ) + 'a');
	ret.push_back(( (number&0b11) != 0b01) ? ((~number&0b11)+'+'-1) : '\0');
    return ret;
}

void MusicUtility::initNoteArray()
{
    MusicUtility::NoteArray.resize(MusicUtility::NoteArraySize);

	for (int j=0; j<MusicUtility::NoteArraySize; j++)
	{
        std::string note = MusicUtility::decode(j);
		for (int i=0; i<MusicUtility::scaleSize; i++)
		{
			NoteArray[j] = 0;
			if (	(scale[i][0]==note[1]) 
					&&	(
							(	(MusicUtility::scale[i][1]=='\0') && (note[2]=='\0')	)
						||	(	(MusicUtility::scale[i][0]==note[1]) && (MusicUtility::scale[i][1]==note[2]) && (note[2]!='\0') )
						||	(	(MusicUtility::scale[i][2]==note[1]) && (MusicUtility::scale[i][3]==note[2]) && (note[2]!='\0') )
						)
			)
			{
				MusicUtility::NoteArray[j] = (i + (note[0]-'0')*MusicUtility::scaleSize-8);
				break;
			}
		}
	//	printf("%s, %d, %d\n", note, NoteArray[j], j);
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
	return MusicUtility::NoteArray[i];
}

std::string MusicUtility::KeyNumber2Note(int PianoKeyNumber)
{
    std::string ret;
/*	//because "%", the result belongs to [0, scaleSize-1]
	int index = (PianoKeyNumber+9)%scaleSize-1;
	n[0] = ((PianoKeyNumber+9) / scaleSize) + '0';
	if ( index < 0 )
	{
		n[0]--;
		index+=scaleSize;
	}
	n[1] = scale[index][0];
	n[2] = scale[index][1];
	n[3] = '\0';
    */
    return ret;
}

int MusicUtility::Note2MidiNumber(std::string& note)
{
    return MusicUtility::KeyNumber2Midi(MusicUtility::Note2KeyNumber(note));
}

std::string MusicUtility::MidiNumber2Note(int PianoKeyNumber)
{
    return MusicUtility::KeyNumber2Note(MusicUtility::Midi2KeyNumber(PianoKeyNumber));
}
