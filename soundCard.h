#ifndef soundCard_h
#define soundCard_h

#include <Adafruit_VS1053.h>
#include <idlers.h>
#include <timeObj.h>
#include <mapper.h>

// These are the hard wired pins used for the music maker shield
#define SHIELD_RESET	-1		// VS1053 reset pin (unused!)
#define SHIELD_CS		7		// VS1053 chip select pin (output)
#define SHIELD_DC		6		// VS1053 Data/command select pin (output)      
#define SHIELD_SC_CS	4		// Onboard SD card.
#define SHIELD_DREQ	3 


// Possible setups 
#define soundCard_SHIELD   0
#define soundCard_BREAKOUT 1


// how long 'till we load in more sound data.
#define soundCard_SLEEP_MS 40


// This interface has you sending a 0..100% float value to set volume. Internally these
// values are mapped to a byte value for the hardware. These are the limits we use to map
// them.
//
// Tweaked volume values for the byte that is dropped in the hardware.
#define MIN_VOL	115	// Anything bugger than this you can't really hear.
#define MAX_VOL	0		// Pretty much 0 is maxed out.


enum action { play, pause, fullStop, restart };

enum soundCardErr { 
  noErr, badSetup, initErr, badCommand, 
  noFileErr,mallocErr,nullStrErr, unknownErr
  };


class soundCard : public idler, public timeObj {

	public:
				soundCard(byte boardSetup,byte inCsPin ,byte inDrqPin,byte inResetPin=-1);
	virtual	~soundCard(void);

				bool				begin(void);
				bool				setSoundfile(const char* inFilePath);
				bool				command(action inCommand);
				bool				isPlaying(void);
				void				setVolume(float inVolume);
				float				getVolume(void);
				void				setError(soundCardErr inErr);
				soundCardErr	getLastError(void);
				void				playClip(const char* filePath);		// NOTE: This BLOCKS! ONLY for "clicks".

  protected:
  
	virtual	void				idle(void);

				Adafruit_VS1053_FilePlayer* musicPlayer;
				mapper			volMapper;
				soundCardErr	lastErr;
				byte				setupType;
				byte				csPin;
				byte				drqPin;
				byte				resetPin;
				char*				filePath;
				float				volume;
				bool				playing;
				bool				newSong;
};

#endif

