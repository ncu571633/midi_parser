#ifndef __MIDIHELPER_CPP__
#define __MIDIHELPER_CPP__

//read the notes information from a text file and write the midi file

#if 0
//get the note from the Queue
void writeMidiQueue(char *MidiFileName, LinkQueue *QChord)
{
    //midi event
    int	deltaTimeTicks	=  interval;
    //how many pulses(clock) per quarter note

    int	deltaTime	= 0;
    int	channelNumber	= 0;
    int	noteVelocity	= 0x50;	//volume of the notes.

    int	ControllerNumber = 0;	//BX
    int	NewValue = 0;
    int	ProgramNumber = 0;		//CX
    int	ChannelNumber = 0;		//DX
    int	WheelPitch = 0;			//EX

    //Meta event
    int	SequenceNumber	= -1;		//00
    char	*TextEvent 	= NULL;		//01
    char	*CopyRight 	= NULL;		//02
    char	*TrackName 	= NULL;		//03
    char	*InstrumentName	= NULL;	//04
    char	*Lyric 		= NULL;		//05
    char	*Marker 	= NULL;		//06
    char	*CuePoint 	= NULL;		//07

    double tempo 	= interval;	//51:
    //in setTempo function:
    //m->tempo 	= (int) (60.0 * 1000000 / tempo); //convert second to microsecond
    //tempo: micro second per quarter note

    int	hour 		= -1;	//54: SMPTE
    int	minute		= -1;
    int	second		= -1;
    int	frame		= -1;
    int	subframe	= -1;

    int	numer		= 4;	//58	time signature
    int	denom		= 4;	//key: 4/4
    int	metro		= 0x18;
    int nds			= 8;

    //59:	key signature
    int	sf		= 3;		//	sharps/ flats
    int mi		= 1;		//1: major/ minor

    /**********************************/
    //head chunk
    /**********************************/
    MidiHead *mh = NULL;
    initMidiHead(&mh, deltaTimeTicks);

    /**********************************/
    //track chunk: head
    /**********************************/
    MidiTrack *mt = NULL;
    initTrackChunk(&mt, mh, noteVelocity);

    /**********************************/
    //Track chunk: Meta Event
    /**********************************/
    //time signature // 0X58 4 2 18 8
    numer	= 4;	//top
    denom	= 4;	//bottom
    setTimeSignature(mt, numer, denom);
    if (!debug) printf("denom = %d\n", mt->denom);

    tempo 	= interval;	//51:
    setTempo(mt, tempo);
    if (!debug) printf("tempo = %x\n", mt->tempo);
    if (!debug) printf("chunk size: %lx.\n", mt->chunkSize);

    sf = 3;
    mi = 1;
    setKeySignature(mt, sf, mi);

    /**********************************/
    //Track chunk: Midi Event
    /**********************************/
    //0xC
    ProgramNumber = 0x21;
    setProgramChange(mt, deltaTime, channelNumber, ProgramNumber);

    //0xb
    int volume	= 0xffff;
    setSoundVolume(mt, 0, volume);

    if (!debug) printf("chunk size: %lx.\n", mt->chunkSize);

    //set midi event queue from QChord queue
    readDataQueue(QChord, mt);

    if (!debug) printf("chunk size: %lx.\n", mt->chunkSize);
    if (!debug) traverseMidiEvent(*mt);

    //write the tracks into file
    FILE *mfp = fopen(MidiFileName, "wb+");
    if ( mfp == NULL)
        printErrorMsg(ERROR_openFile, "Can't create midi file\n");
    writeHeadChunk(*mh, mfp);
    writeTrackChunk(*mt, mfp);

    destroyHeadChunk(&mh);
    destroyTrackChunk(&mt);

    fclose(mfp);
}

void writeMidiFile(char *MidiFileName, MidiFile *midifile)
{
    //write midi header chunk;
    writeHeadChunk(*mh, mfp);

    //write midi link list
    QueuePtr pTrack = NULL;
    for (pTrack=midifile->QMTrackChunk->next; pTrack!=NULL; pTrack=pTrack->next)
    {
        MidiTrackChunk *m = (MidiTrackChunk)(pTrack->data.v);
        //write meta events

        //write midi events
        writeTrackChunk(*mt, mfp);
    }
}

#endif


//get the note from the data file
void writeMidiTXTFile(char *inputDataFile, char *MidiFileName, ARG *arg)
{
    //midi event
	MidiFile *m = NULL;
	initMidiFile(&m);

	//set midi head
	m->MHeader->deltaTimeTicks = arg->deltaTimeTicks;
	
	//set midi track
    MidiTrackChunk *mt = NULL;
    initMidiTrackChunk(&mt, m->MHeader);

	//insert the track into the MidiFile track chunk queue
	QElemType e;
	e.v = mt;
	EnQueue(m->QMTrackChunk, e);
	
    /**********************************/
    //Track chunk: Meta Event
    /**********************************/
    setMetaTempo(mt, arg->deltaTimeTicks);	//0X51

    //time signature // 0X58 4 4 18 8
    int numer	= 4;
    int denom	= 4;	// 4/4 C Major
    setMetaTimeSignature(mt, numer, denom, arg->deltaTimeTicks);

    //0x59
    int sf = 3;
    int mi = 1;
    setMetaKeySignature(mt, sf, mi);

    /*	hour 	= 0x40;	// 0x8
    	minute 	= 0;
    	second	= 0;
    	frame	= 0;
    	subframe= 0;
    	setSMPTEOffset(mt, hour, minute, second, frame, subframe);
    */
    if (!debug) printf("chunk size: %lx.\n", mt->chunkSize);

    /**********************************/
    //Track chunk: Midi Event
    /**********************************/
	//0xC
    int ProgramNumber = 0x21;
	//delta time, channal number, program number
    setMidiProgramChange(mt, 0, 0, ProgramNumber);

    //0xb
    int ControllerNumber= 0x7;
    int NewValue	= 0x7f;
    setMidiControlChange(mt, 0, 0, ControllerNumber, NewValue);
    
	if (!debug) printf("chunk size: %lx.\n", mt->chunkSize);

    //set midi event
    readMidiTXTFile(inputDataFile, mt);

    if (!debug) printf("chunk size: %lx.\n", mt->chunkSize);

    //write the tracks into file
	writeMidiFile(m, MidiFileName);

	destroyMidiFile(&m);
}


#endif
