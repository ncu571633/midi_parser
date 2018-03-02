#include "Music.hpp"

const std::string MusicUtility::pianoKeyBoard[] = {std::string("c")};//{"c","c+d-","d", "d+e-","e","f","f+g-","g","g+a-","a","a+b-","b"};
int MusicUtility::PianoOctaveKeyNumber = sizeof(MusicUtility::pianoKeyBoard) / sizeof(MusicUtility::pianoKeyBoard[0]);   // 12


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
	for (int j=0; j<MusicUtility::NoteArraySize; j++)
	{
        std::string note = MusicUtility::decode(j);
		for (int i=0; i<MusicUtility::PianoOctaveKeyNumber; i++)
		{
			NoteArray[j] = 0;
			if (	(pianoKeyBoard[i][0]==note[1]) 
					&&	(
							(	(pianoKeyBoard[i][1]=='\0') && (note[2]=='\0')	)
						||	(	(pianoKeyBoard[i][0]==note[1]) && (pianoKeyBoard[i][1]==note[2]) && (note[2]!='\0') )
						||	(	(pianoKeyBoard[i][2]==note[1]) && (pianoKeyBoard[i][3]==note[2]) && (note[2]!='\0') )
						)
			)
			{
				NoteArray[j] = (i + (note[0]-'0')*PIANOOCTAVEKEYNUMBER -8);
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
/*	//because "%", the result belongs to [0, PIANOOCTAVEKEYNUMBER-1]
	int index = (PianoKeyNumber+9)%PIANOOCTAVEKEYNUMBER-1;
	n[0] = ((PianoKeyNumber+9) / PIANOOCTAVEKEYNUMBER) + '0';
	if ( index < 0 )
	{
		n[0]--;
		index+=PIANOOCTAVEKEYNUMBER;
	}
	n[1] = pianoKeyBoard[index][0];
	n[2] = pianoKeyBoard[index][1];
	n[3] = '\0';
    */
    return ret;
}
