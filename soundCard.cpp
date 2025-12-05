#include <soundCard.h>
#include <strTools.h>
#include <LC_SPI.h>


soundCard::soundCard(byte boardSetup,byte inCsPin,byte inDrqPin,byte inResetPin) 
  : timeObj(soundCard_SLEEP_MS) { 
  
    setupType = boardSetup;
    csPin = inCsPin;
    drqPin = inDrqPin;
    resetPin = inResetPin;
    filePath = NULL;
    volume = 40;						// Hardcoded default from Adafruit.
    newSong = false;
    volMapper.setValues(0,100,MIN_VOL,MAX_VOL);
    setError(noErr);
  }


soundCard::~soundCard(void) { 
  
    if (musicPlayer) {
      delete musicPlayer;
      musicPlayer = NULL;
    }
    freeStr(&filePath);
  }


boolean soundCard::begin(void) { 

    start();                    // Whatever happens, lets get the timer rolling.
    switch (setupType) {
      case soundCard_SHIELD :
        musicPlayer = new Adafruit_VS1053_FilePlayer(SHIELD_RESET, SHIELD_CS, SHIELD_DC, SHIELD_DREQ);
      break;
      case soundCard_BREAKOUT :
        musicPlayer = new Adafruit_VS1053_FilePlayer(resetPin,csPin,LC_DC,drqPin,0);
      break;
      default : setError(badSetup); return false;
    }
    if (musicPlayer) {
      if (musicPlayer->begin()) {
        hookup();
        return true;
      } else {
        setError(initErr);
      }
    } else {
      setError(mallocErr);
    }
    return false;
  }


boolean soundCard::setSoundfile(const char* inFilePath) {

	File testFile;
	 
	heapStr(&filePath,inFilePath);		// Copy the incoming file path.
	if (filePath) {							// If we got the filePath..
		testFile = SD.open(filePath);		// See if we can open it.
		if(testFile) {							// If we got it open..
			testFile.close();					// Close it.
			newSong = true;					// We got a new song.
			return true;						// And return our success.
		} else {									// No test file?
			freeStr(&filePath);				// Path's no good. Recycle it.
			setError(noFileErr);				// Toss a file error.
		}											//
	} else {										// Didn't get the file path at all!
		setError(mallocErr);					// Toss an error.
	}												//
	return false;								// If we got here, trouble..
}


boolean soundCard::command(action inCommand) {

	boolean success;

	setError(noErr);
	success = false;
	switch(inCommand) {															// Eww, let's see what the user wants.
		case play :																	// User hit play.
			if (newSong) {															// New song loaded?
				musicPlayer->stopPlaying();									// Call stop to close data file!
				success = musicPlayer->startPlayingFile(filePath);		// Try playing the new song.
				if (success) {														// It worked? Cool!
					newSong = false;												// Note it.
				} else {																// Didn't work?
					setError(unknownErr);										// Who knows what broke in there?
				}																		//
			} else if (musicPlayer->paused()) {								// No fresh new song. Been paused.
				musicPlayer->pausePlaying(false);							// Just un-pause.
				success = true;													// That's a success.
			} else if (filePath) {												// Not paused, but there's a file.
				success = musicPlayer->startPlayingFile(filePath);		// Try playing that.
				if (!success) setError(unknownErr);							// No? Again, who knows what broke in there?
			} else {																	// And if we had no file?
				setError(noFileErr);												// Flag it.
			}																			//
		break;																		// See ya!
		case pause :																// User hits pause.
			if (musicPlayer->paused()) {										// We're already paused.
				musicPlayer->pausePlaying(false);							// Then just start up again.
			} else if (musicPlayer->playingMusic) {						// If we're playing?
				musicPlayer->pausePlaying(true);								// Well, then pause!
			}																			//
			success = true;														// Good enough really, call it good.
		break;																		// We're out.
		case fullStop :															// They want us to stop..
			musicPlayer->stopPlaying();										// Well do it.
			success = true;														// Simple enough.
		break;																		// We're off.
		case restart :																// User wants a restart.
			if (filePath) {														// If we have a file.
				musicPlayer->stopPlaying();									// Call stop to close data file!
				success = musicPlayer->startPlayingFile(filePath);		// See if we can start playing this file.
			} else {																	// And if we had no file?
				setError(noFileErr);												// Flag it.
			}																			//
		break;																		// Exit stage right!
		default : setError(badCommand);										// We don't know WHAT the user wants!
	}																					//
	return success;																// Return our success or failure.
}

          
boolean soundCard::isPlaying(void) { return musicPlayer->playingMusic; }


// Now volume is 0..100% Makes more sense to me.
void soundCard::setVolume(float inVolume) { 

	byte volByte;
	
	volume = inVolume;
	volByte = round(volMapper.map(volume));
	musicPlayer->setVolume(volByte,volByte);
}


float soundCard::getVolume(void) { return volume; }


void soundCard::setError(soundCardErr inErr) { lastErr = inErr; }


soundCardErr soundCard::getLastError(void) { return lastErr; }


void soundCard::idle(void) {

	if (musicPlayer->playingMusic && ding()) {
		musicPlayer->feedBuffer();
		start();
	}
}


// A BLOCKING run-'till-you're-done function. The idea is that there are very short
// "beep" & "click" sounds that would be nice for UI Development and it would be OK to
// block for a couple ms.
void soundCard::playClip(const char* filePath) {

	File soundFile;
	
	if (!musicPlayer->playingMusic) {		// Ok, no one is using the hardware.
		musicPlayer->startPlayingFile(filePath);
		while(musicPlayer->playingMusic) {
			if (musicPlayer->readyForData()) {
				musicPlayer->feedBuffer();
			}
		}
	}
}


