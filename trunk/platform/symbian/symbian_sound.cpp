/*
Portable ZX-Spectrum emulator.
Copyright (C) 2001-2010 SMT, Dexus, Alone Coder, deathsoft, djdron, scor

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "../platform.h"

#ifdef _SYMBIAN

#include <mda/common/audio.h>
#include <mdaaudiooutputstream.h>
#include <e32std.h>

namespace xPlatform
{

class eAudio : public CBase, public MMdaAudioOutputStreamCallback
{
public:
	eAudio() : iSndStream(NULL) {}
	~eAudio() { SAFE_DELETE(iSndStream); }
	TBool UpdatePSndRate()
	{
		TInt sampleRate = TMdaAudioDataSettings::ESampleRate8000Hz;
		if(iSampleRate == 11025)
			sampleRate = TMdaAudioDataSettings::ESampleRate11025Hz;
		else if (iSampleRate == 16000)
			sampleRate = TMdaAudioDataSettings::ESampleRate16000Hz;
		else if (iSampleRate == 22050)
			sampleRate = TMdaAudioDataSettings::ESampleRate22050Hz;
		else if (iSampleRate == 32000)
			sampleRate = TMdaAudioDataSettings::ESampleRate32000Hz;
		else if (iSampleRate == 44100)
			sampleRate = TMdaAudioDataSettings::ESampleRate44100Hz;

		TRAPD(err,iSndStream->SetAudioPropertiesL(sampleRate, iStereo ? TMdaAudioDataSettings::EChannelsStereo : TMdaAudioDataSettings::EChannelsMono));
		return (err == KErrNone);
	}

	void WriteSound(unsigned char* aBuffer, int bytes)
	{
		TPtrC8 ptr(aBuffer, bytes);
		TPtr8 des((unsigned char*) iSound[iCurrentBuf].Ptr()+bytes*iCurrentSeg, bytes);
		des.Copy(ptr);
		iCurrentSeg++;
		if(iCurrentSeg == eSegsCount)
		{
			iSound[iCurrentBuf].SetLength(bytes*eSegsCount);
			iSndStream->WriteL(iSound[iCurrentBuf]);
			iCurrentBuf = 1 - iCurrentBuf;
			iCurrentSeg = 0;
		}
	}

	virtual void MaoscOpenComplete(TInt aError)
	{
		if(aError == KErrNone)
		{
			iSndStream->SetPriority(EPriorityMuchMore, EMdaPriorityPreferenceNone);
			iSndStream->SetVolume((iSndStream->MaxVolume()*Handler()->Volume())/10);
			iVolume = Handler()->Volume();
			if(!UpdatePSndRate())
			{
				iSampleRate = 8000;
				UpdatePSndRate();
			}
		}
		CActiveScheduler::Stop();
	}

	virtual void MaoscBufferCopied(TInt aError, const TDesC8& aBuffer)
	{
		if(iVolume != Handler()->Volume() || aError != KErrNone)
		{
			iSndStream->SetVolume((iSndStream->MaxVolume()*Handler()->Volume())/10);
			iVolume = Handler()->Volume();
		}
	}

	virtual void MaoscPlayComplete(TInt aError)
	{
		if(aError != KErrNone)
		{
			iSndStream->SetVolume((iSndStream->MaxVolume()*Handler()->Volume())/10);
			iVolume = Handler()->Volume();
			UpdatePSndRate();
		}
	}

	void ConstructL()
	{
		iCurrentBuf = 0;
		iCurrentSeg = 0;
		iVolume = 0;
		iSampleRate = 44100;
		iStereo = true;
		iSndStream = CMdaAudioOutputStream::NewL(*this);
		iAudioSettings.Query();
		iAudioSettings.iSampleRate = TMdaAudioDataSettings::ESampleRate8000Hz;
		iAudioSettings.iChannels = TMdaAudioDataSettings::EChannelsMono;
		iAudioSettings.iFlags = 0;
		iAudioSettings.iVolume = iAudioSettings.iMaxVolume/2;
		iSndStream->Open(&iAudioSettings);
		CActiveScheduler::Start();
	}

	void Update()
	{
		using namespace xPlatform;
		for(int i = Handler()->AudioSources(); --i >= 0;)
		{
			dword size = Handler()->AudioDataReady(i);
			bool ui_enabled = Handler()->VideoDataUI() != NULL;
			if(i == Handler()->Sound() && !ui_enabled && !Handler()->FullSpeed())
			{
				WriteSound((byte*)Handler()->AudioData(i), size);
			}
			Handler()->AudioDataUse(i, size);
		}
	}

protected:
	enum { eSegsCount = 5 };
	CMdaAudioOutputStream*	iSndStream;
	TMdaAudioDataSettings	iAudioSettings;
	TBuf8<16384 * eSegsCount> iSound[2];
	TInt					iCurrentBuf;
	TInt					iCurrentSeg;
	TInt					iVolume;
	TInt					iSampleRate;
	TInt					iStereo;
};

static eAudio* audio = NULL;

void InitSound()
{
	audio = new (ELeave) eAudio;
	audio->ConstructL();
}

void DoneSound()
{
	SAFE_DELETE(audio);
}

void OnLoopSound()
{
	audio->Update();
}

}
//namespace xPlatform

#endif//_SYMBIAN
