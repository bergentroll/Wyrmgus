//       _________ __                 __
//      /   _____//  |_____________ _/  |______     ____  __ __  ______
//      \_____  \\   __\_  __ \__  \\   __\__  \   / ___\|  |  \/  ___/
//      /        \|  |  |  | \// __ \|  |  / __ \_/ /_/  >  |  /\___ |
//     /_______  /|__|  |__|  (____  /__| (____  /\___  /|____//____  >
//             \/                  \/          \//_____/            \/
//  ______________________                           ______________________
//                        T H E   W A R   B E G I N S
//         Stratagus - A free fantasy real time strategy game engine
//
/**@name sound_server.cpp - The sound server (hardware layer and so on) */
//
//      (c) Copyright 1998-2006 by Lutz Sammer, Fabrice Rossi, and
//                                 Jimmy Salmon
//
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; only version 2 of the License.
//
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA
//      02111-1307, USA.
//


/*----------------------------------------------------------------------------
--  Includes
----------------------------------------------------------------------------*/

#include "stratagus.h"

#include "sound/sound_server.h"

#include "civilization.h"
#include "faction.h"
#include "iocompat.h"
#include "iolib.h"
#include "unit/unit.h"
//Wyrmgus start
#include "unit/unit_manager.h"
//Wyrmgus end
#include "wyrmgus.h"

#include "SDL.h"

#include <oamlGodotModule/oamlGodotModule.h>

/*----------------------------------------------------------------------------
--  Variables
----------------------------------------------------------------------------*/

static bool SoundInitialized;    /// is sound initialized
static bool MusicPlaying;        /// flag true if playing music

static int EffectsVolume = 128;  /// effects sound volume
static int MusicVolume = 128;    /// music volume

static bool MusicEnabled = true;
static bool EffectsEnabled = true;

/// Channels for sound effects and unit speech
struct SoundChannel {
	CSample *Sample;       /// sample to play
	Origin *Unit;          /// pointer to unit, who plays the sound, if any
	unsigned char Volume;  /// Volume of this channel
	signed char Stereo;    /// stereo location of sound (-128 left, 0 center, 127 right)
	//Wyrmgus start
	int Voice;  /// Voice group of this channel (for identifying voice types)
	//Wyrmgus end

	bool Playing;          /// channel is currently playing
	int Point;             /// point in sample if playing or next free channel

	void (*FinishedCallback)(int channel); /// Callback for when a sample finishes playing
};

#define MaxChannels 64     /// How many channels are supported

static SoundChannel Channels[MaxChannels];
static int NextFreeChannel;

static void ChannelFinished(int channel);

static struct {
	SDL_AudioSpec Format;
	SDL_mutex *Lock;
	SDL_cond *Cond;
	SDL_Thread *Thread;

	int *MixerBuffer;
	Uint8 *Buffer;
	bool Running;
} Audio;

#ifndef SDL_AUDIO_BITSIZE
#define SDL_AUDIO_BITSIZE(x) (x&0xFF)
#endif

/*----------------------------------------------------------------------------
--  Mixers
----------------------------------------------------------------------------*/

/**
**  Convert RAW sound data to 44100 hz, Stereo, 16 bits per channel
**
**  @param src        Source buffer
**  @param dest       Destination buffer
**  @param frequency  Frequency of source
**  @param chansize   Bitrate in bytes per channel of source
**  @param channels   Number of channels of source
**  @param bytes      Number of compressed bytes to read
**
**  @return           Number of bytes written in 'dest'
*/
static int ConvertToStereo32(const char *src, char *dest, int frequency,
							 int chansize, int channels, int bytes)
{
	SDL_AudioCVT acvt;
	Uint16 format;

	if (chansize == 1) {
		format = AUDIO_U8;
	} else {
		format = AUDIO_S16SYS;
	}
	SDL_BuildAudioCVT(&acvt, format, channels, frequency, AUDIO_S16SYS, 2, 44100);

	acvt.buf = (Uint8 *)dest;
	memcpy(dest, src, bytes);
	acvt.len = bytes;

	SDL_ConvertAudio(&acvt);

	return acvt.len_mult * bytes;
}

/**
**  Mix sample to buffer.
**
**  The input samples are adjusted by the local volume and resampled
**  to the output frequence.
**
**  @param sample  Input sample
**  @param index   Position into input sample
**  @param volume  Volume of the input sample
**  @param stereo  Stereo (left/right) position of sample
**  @param buffer  Output buffer
**  @param size    Size of output buffer (in samples per channel)
**
**  @return        The number of bytes used to fill buffer
**
**  @todo          Can mix faster if signed 8 bit buffers are used.
*/
static int MixSampleToStereo32(CSample *sample, int index, unsigned char volume,
							   char stereo, int *buffer, int size)
{
	static int buf[SOUND_BUFFER_SIZE / 2];
	unsigned char left;
	unsigned char right;

	int div = 176400 / (sample->Frequency * (sample->SampleSize / 8) * sample->Channels);
	int local_volume = (int)volume * EffectsVolume / MaxVolume;

	if (stereo < 0) {
		left = 128;
		right = 128 + stereo;
	} else {
		left = 128 - stereo;
		right = 128;
	}

	Assert(!(index & 1));

	size = std::min((sample->Len - index) * div / 2, size);

	size = ConvertToStereo32((char *)(sample->Buffer + index), (char *)buf, sample->Frequency,
							 sample->SampleSize / 8, sample->Channels,
							 size * 2 / div);

	size /= 2;
	for (int i = 0; i < size; i += 2) {
		// FIXME: why taking out '/ 2' leads to distortion
		buffer[i] += ((short *)buf)[i] * local_volume * left / 128 / MaxVolume / 2;
		buffer[i + 1] += ((short *)buf)[i + 1] * local_volume * right / 128 / MaxVolume / 2;
	}

	return 2 * size / div;
}

/**
**  Mix channels to stereo 32 bit.
**
**  @param buffer  Buffer for mixed samples.
**  @param size    Number of samples that fits into buffer.
**
**  @return        How many channels become free after mixing them.
*/
static int MixChannelsToStereo32(int *buffer, int size)
{
	int new_free_channels = 0;

	for (int channel = 0; channel < MaxChannels; ++channel) {
		if (Channels[channel].Playing && Channels[channel].Sample) {
			//Wyrmgus start
			if ((Channels[channel].Point & 1)) {
				fprintf(stderr, "Sound effect error; Index: %d, Voice: %d, Origin: \"%s\", Sample Length: %d, Sample Filename: \"%s\"\n", Channels[channel].Point, Channels[channel].Voice, (Channels[channel].Unit && Channels[channel].Unit->Base) ? UnitManager.GetSlotUnit(Channels[channel].Unit->Id).Type->Ident.c_str() : "", Channels[channel].Sample->Len, Channels[channel].Sample->File.c_str());
			}
			//Wyrmgus end
			int i = MixSampleToStereo32(Channels[channel].Sample,
										Channels[channel].Point, Channels[channel].Volume,
										Channels[channel].Stereo, buffer, size);
			Channels[channel].Point += i;
			Assert(Channels[channel].Point <= Channels[channel].Sample->Len);

			if (Channels[channel].Point == Channels[channel].Sample->Len) {
				ChannelFinished(channel);
				++new_free_channels;
			}
		}
	}
	return new_free_channels;
}

/**
**  Clip mix to output stereo 16 signed bit.
**
**  @param mix     signed 32 bit input.
**  @param size    number of samples in input.
**  @param output  clipped 16 signed bit output buffer.
*/
static void ClipMixToStereo16(const int *mix, int size, short *output)
{
	const int *end = mix + size;

	while (mix < end) {
		int s = (*mix++);
		clamp(&s, SHRT_MIN, SHRT_MAX);
		*output++ = s;
	}
}

/**
**  Mix into buffer.
**
**  @param buffer   Buffer to be filled with samples. Buffer must be big enough.
**  @param samples  Number of samples.
*/
static void MixIntoBuffer(void *buffer, int samples)
{
	// FIXME: can save the memset here, if first channel sets the values
	memset(Audio.MixerBuffer, 0, samples * sizeof(*Audio.MixerBuffer));

	if (EffectsEnabled) {
		// Add channels to mixer buffer
		MixChannelsToStereo32(Audio.MixerBuffer, samples);
	}
	ClipMixToStereo16(Audio.MixerBuffer, samples, (short *)buffer);
}

/**
**  Fill buffer for the sound card.
**
**  @see SDL_OpenAudio
**
**  @param udata   the pointer stored in userdata field of SDL_AudioSpec.
**  @param stream  pointer to buffer you want to fill with information.
**  @param len     is length of audio buffer in bytes.
*/
static void FillAudio(void *, Uint8 *stream, int len)
{
	Assert((len/2) != Audio.Format.size);

	if (Audio.Running == false)
		return;

	SDL_memset(stream, 0, len);

	SDL_LockMutex(Audio.Lock);
	SDL_MixAudio(stream, Audio.Buffer, len, SDL_MIX_MAXVOLUME);

	// Signal our FillThread, we can fill the Audio.Buffer again
	SDL_CondSignal(Audio.Cond);
	SDL_UnlockMutex(Audio.Lock);
}

/**
**  Fill audio thread.
*/
static int FillThread(void *)
{
	while (Audio.Running == true) {
		int status = SDL_LockMutex(Audio.Lock);
#ifdef _WIN32
		if (SDL_CondWaitTimeout(Audio.Cond, Audio.Lock, 1000) == 0) {
#else
		if (SDL_CondWaitTimeout(Audio.Cond, Audio.Lock, 100) == 0) {
#endif
			MixIntoBuffer(Audio.Buffer, Audio.Format.samples * Audio.Format.channels);
		}
		SDL_UnlockMutex(Audio.Lock);
	}

	return 0;
}

/*----------------------------------------------------------------------------
--  Effects
----------------------------------------------------------------------------*/

/**
**  Check if this sound is already playing
*/
bool SampleIsPlaying(CSample *sample)
{
	for (int i = 0; i < MaxChannels; ++i) {
		if (Channels[i].Sample == sample && Channels[i].Playing) {
			return true;
		}
	}
	return false;
}

bool UnitSoundIsPlaying(Origin *origin)
{
	for (int i = 0; i < MaxChannels; ++i) {
		//Wyrmgus start
//		if (origin && Channels[i].Unit && origin->Id && Channels[i].Unit->Id
//			&& origin->Id == Channels[i].Unit->Id && Channels[i].Playing) {
		if (
			origin && Channels[i].Playing
			&& Channels[i].Voice != -1
			&& Channels[i].Voice != VoiceHit && Channels[i].Voice != VoiceMiss && Channels[i].Voice != VoiceFireMissile && Channels[i].Voice != VoiceStep
			&& Channels[i].Unit && origin->Id && Channels[i].Unit->Id
			&& origin->Id == Channels[i].Unit->Id
		) {
		//Wyrmgus end
			return true;
		}
	}
	return false;
}

/**
**  A channel is finished playing
*/
static void ChannelFinished(int channel)
{
	if (Channels[channel].FinishedCallback) {
		Channels[channel].FinishedCallback(channel);
	}

	//Wyrmgus start
//	delete Channels[channel].Unit;
	if (Channels[channel].Unit) {
		delete Channels[channel].Unit;
	}
	//Wyrmgus end
	Channels[channel].Unit = nullptr;
	
	//Wyrmgus start
	Channels[channel].Voice = -1;
	//Wyrmgus end
	Channels[channel].Playing = false;
	Channels[channel].Point = NextFreeChannel;
	NextFreeChannel = channel;
}

/**
**  Put a sound request in the next free channel.
*/
static int FillChannel(CSample *sample, unsigned char volume, char stereo, Origin *origin)
{
	Assert(NextFreeChannel < MaxChannels);

	int old_free = NextFreeChannel;
	int next_free = Channels[NextFreeChannel].Point;

	Channels[NextFreeChannel].Volume = volume;
	Channels[NextFreeChannel].Point = 0;
	//Wyrmgus start
	Channels[NextFreeChannel].Voice = -1;
	//Wyrmgus end
	Channels[NextFreeChannel].Playing = true;
	Channels[NextFreeChannel].Sample = sample;
	Channels[NextFreeChannel].Stereo = stereo;
	Channels[NextFreeChannel].FinishedCallback = nullptr;
	//Wyrmgus start
	Channels[NextFreeChannel].Unit = nullptr;
	//Wyrmgus end
	if (origin && origin->Base) {
		Origin *source = new Origin;
		source->Base = origin->Base;
		source->Id = origin->Id;
		Channels[NextFreeChannel].Unit = source;
	}
	NextFreeChannel = next_free;

	return old_free;
}

/**
**  Set the channel volume
**
**  @param channel  Channel to set
**  @param volume   New volume, <0 will not set the volume
**
**  @return         Current volume of the channel, -1 for error
*/
int SetChannelVolume(int channel, int volume)
{
	if (channel < 0 || channel >= MaxChannels) {
		return -1;
	}

	if (volume < 0) {
		volume = Channels[channel].Volume;
	} else {
		SDL_LockMutex(Audio.Lock);

		volume = std::min(MaxVolume, volume);
		Channels[channel].Volume = volume;

		SDL_UnlockMutex(Audio.Lock);
	}
	return volume;
}

/**
**  Set the channel stereo
**
**  @param channel  Channel to set
**  @param stereo   -128 to 127, out of range will not set the stereo
**
**  @return         Current stereo of the channel, -1 for error
*/
int SetChannelStereo(int channel, int stereo)
{
	if (Preference.StereoSound == false) {
		stereo = 0;
	}
	if (channel < 0 || channel >= MaxChannels) {
		return -1;
	}

	if (stereo < -128 || stereo > 127) {
		stereo = Channels[channel].Stereo;
	} else {
		SDL_LockMutex(Audio.Lock);
		Channels[channel].Stereo = stereo;
		SDL_UnlockMutex(Audio.Lock);
	}
	return stereo;
}

//Wyrmgus start
/**
**  Set the channel voice group
**
**  @param channel  Channel to set
**
**  @return         Current stereo of the channel, -1 for error
*/
int SetChannelVoiceGroup(int channel, UnitVoiceGroup voice)
{
	if (channel < 0 || channel >= MaxChannels) {
		return -1;
	}

	SDL_LockMutex(Audio.Lock);
	Channels[channel].Voice = voice;
	SDL_UnlockMutex(Audio.Lock);
	
	return voice;
}
//Wyrmgus end

/**
**  Set the channel's callback for when a sound finishes playing
**
**  @param channel   Channel to set
**  @param callback  Callback to call when the sound finishes
*/
void SetChannelFinishedCallback(int channel, void (*callback)(int channel))
{
	if (channel < 0 || channel >= MaxChannels) {
		return;
	}
	Channels[channel].FinishedCallback = callback;
}

/**
**  Get the sample playing on a channel
*/
CSample *GetChannelSample(int channel)
{
	if (channel < 0 || channel >= MaxChannels) {
		return nullptr;
	}
	return Channels[channel].Sample;
}

/**
**  Stop a channel
**
**  @param channel  Channel to stop
*/
void StopChannel(int channel)
{
	SDL_LockMutex(Audio.Lock);
	if (channel >= 0 && channel < MaxChannels) {
		if (Channels[channel].Playing) {
			ChannelFinished(channel);
		}
	}
	SDL_UnlockMutex(Audio.Lock);
}

/**
**  Stop all channels
*/
void StopAllChannels()
{
	SDL_LockMutex(Audio.Lock);
	for (int i = 0; i < MaxChannels; ++i) {
		if (Channels[i].Playing) {
			ChannelFinished(i);
		}
	}
	SDL_UnlockMutex(Audio.Lock);
}

static CSample *LoadSample(const char *name, enum _play_audio_flags_ flag)
{
	CSample *sampleWav = LoadWav(name, flag);

	if (sampleWav) {
		return sampleWav;
	}
	CSample *sampleVorbis = LoadVorbis(name, flag);
	if (sampleVorbis) {
		return sampleVorbis;
	}
	return nullptr;
}


/**
**  Load a sample
**
**  @param name  File name of sample (short version).
**
**  @return      General sample loaded from file into memory.
**
**  @todo  Add streaming, caching support.
*/
CSample *LoadSample(const std::string &name)
{
	const std::string filename = LibraryFileName(name.c_str());
	CSample *sample = LoadSample(filename.c_str(), PlayAudioLoadInMemory);

	if (sample == nullptr) {
		fprintf(stderr, "Can't load the sound '%s'\n", name.c_str());
	}
	return sample;
}

/**
**  Play a sound sample
**
**  @param sample  Sample to play
**
**  @return        Channel number, -1 for error
*/
int PlaySample(CSample *sample, Origin *origin)
{
	int channel = -1;

	SDL_LockMutex(Audio.Lock);
	if (SoundEnabled() && EffectsEnabled && sample && NextFreeChannel != MaxChannels) {
		channel = FillChannel(sample, EffectsVolume, 0, origin);
	}
	SDL_UnlockMutex(Audio.Lock);
	return channel;
}

/**
**  Play a sound file
**
**  @param name  Filename of a sound to play
**
**  @return      Channel number the sound is playing on, -1 for error
*/
int PlaySoundFile(const std::string &name)
{
	CSample *sample = LoadSample(name);
	if (sample) {
		return PlaySample(sample);
	}
	return -1;
}

/**
**  Set the global sound volume.
**
**  @param volume  the sound volume 0-255
*/
void SetEffectsVolume(int volume)
{
	clamp(&volume, 0, MaxVolume);
	EffectsVolume = volume;
}

/**
**  Get effects volume
*/
int GetEffectsVolume()
{
	return EffectsVolume;
}

/**
**  Set effects enabled
*/
void SetEffectsEnabled(bool enabled)
{
	EffectsEnabled = enabled;
}

/**
**  Check if effects are enabled
*/
bool IsEffectsEnabled()
{
	return EffectsEnabled;
}

/*----------------------------------------------------------------------------
--  Music
----------------------------------------------------------------------------*/

void PlayMusicName(const std::string &name) {
	if (!IsMusicEnabled()) {
		return;
	}
	
	if (Wyrmgus::GetInstance()->GetOamlModule() == nullptr) {
		return;
	}

	Wyrmgus::GetInstance()->GetOamlModule()->PlayTrack(name.c_str());
}

void PlayMusicByGroupRandom(const std::string &group) {
	if (!IsMusicEnabled()) {
		return;
	}
	
	if (Wyrmgus::GetInstance()->GetOamlModule() == nullptr) {
		return;
	}

	Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupRandom(group.c_str());
}

void PlayMusicByGroupAndSubgroupRandom(const std::string &group, const std::string &subgroup) {
	if (!IsMusicEnabled()) {
		return;
	}
	
	if (Wyrmgus::GetInstance()->GetOamlModule() == nullptr) {
		return;
	}

	if (Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupAndSubgroupRandom(group.c_str(), subgroup.c_str()) != OAML_OK) {
		Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupRandom(group.c_str());
	}
}

void PlayMusicByGroupAndFactionRandom(const std::string &group, const std::string &civilization_ident, const std::string &faction_ident)
{
	if (!IsMusicEnabled()) {
		return;
	}

	if (Wyrmgus::GetInstance()->GetOamlModule() == nullptr) {
		return;
	}

	if (Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupAndSubgroupRandom(group.c_str(), faction_ident.c_str()) != OAML_OK) {
		CCivilization *civilization = CCivilization::Get(civilization_ident);
		const CFaction *faction = CFaction::Get(faction_ident);
		const CFaction *parent_faction = nullptr;
		bool found_music = false;
		if (faction != nullptr) {
			while (true) {
				parent_faction = faction->ParentFaction;
				if (parent_faction == nullptr) {
					break;
				}
				faction = parent_faction;
				
				if (Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupAndSubgroupRandom(group.c_str(), faction->GetIdent().utf8().get_data()) == OAML_OK) {
					found_music = true;
					break;
				}
			}
		}
		if (!found_music && Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupAndSubgroupRandom(group.c_str(), civilization_ident.c_str()) != OAML_OK) {
			CCivilization *parent_civilization = nullptr;
			if (civilization) {
				while (true) {
					parent_civilization = civilization->ParentCivilization;
					if (!parent_civilization) {
						break;
					}
					civilization = parent_civilization;
					
					if (Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupAndSubgroupRandom(group.c_str(), civilization->GetIdent().utf8().get_data()) == OAML_OK) {
						found_music = true;
						break;
					}
				}
			}
			if (!found_music) {
				Wyrmgus::GetInstance()->GetOamlModule()->PlayTrackByGroupRandom(group.c_str());
			}
		}
	}
}

void SetMusicCondition(int id, int value) {
	if (Wyrmgus::GetInstance()->GetOamlModule() == nullptr) {
		return;
	}

	Wyrmgus::GetInstance()->GetOamlModule()->SetCondition(id, value);
}

void SetMusicLayerGain(const std::string &layer, float gain) {
	if (Wyrmgus::GetInstance()->GetOamlModule() == nullptr) {
		return;
	}

	Wyrmgus::GetInstance()->GetOamlModule()->SetLayerGain(layer.c_str(), gain);
}

/**
**  Stop the current playing music.
*/
void StopMusic()
{
	if (Wyrmgus::GetInstance()->GetOamlModule() != nullptr) {
		Wyrmgus::GetInstance()->GetOamlModule()->StopPlaying();
	}
}

/**
**  Set the music volume.
**
**  @param volume  the music volume 0-255
*/
void SetMusicVolume(int volume)
{
	clamp(&volume, 0, MaxVolume);
	MusicVolume = volume;

	if (Wyrmgus::GetInstance()->GetOamlModule() != nullptr) {
		Wyrmgus::GetInstance()->GetOamlModule()->SetVolume(MusicVolume / 255.f);
	}
}

/**
**  Get music volume
*/
int GetMusicVolume()
{
	return MusicVolume;
}

/**
**  Set music enabled
*/
void SetMusicEnabled(bool enabled)
{
	if (enabled) {
		MusicEnabled = true;
	} else {
		MusicEnabled = false;
		StopMusic();
	}
}

/**
**  Check if music is enabled
*/
bool IsMusicEnabled()
{
	return MusicEnabled;
}

/**
**  Check if music is playing
*/
bool IsMusicPlaying()
{
	if (Wyrmgus::GetInstance()->GetOamlModule() != nullptr) {
		if (Wyrmgus::GetInstance()->GetOamlModule()->IsPlaying()) {
			return true;
		}
	}

	return MusicPlaying;
}

/**
**  Add tension to music
*/
void AddMusicTension(int value)
{
	if (Wyrmgus::GetInstance()->GetOamlModule() == nullptr) {
		return;
	}

	Wyrmgus::GetInstance()->GetOamlModule()->AddTension(value);
}


/*----------------------------------------------------------------------------
--  Init
----------------------------------------------------------------------------*/

/**
**  Check if sound is enabled
*/
bool SoundEnabled()
{
	return SoundInitialized;
}

/**
**  Initialize sound card hardware part with SDL.
**
**  @param freq  Sample frequency (44100,22050,11025 hz).
**  @param size  Sample size (8bit, 16bit)
**
**  @return      True if failure, false if everything ok.
*/
static int InitSdlSound(int freq, int size)
{
	SDL_AudioSpec wanted;

	wanted.freq = freq;
	if (size == 8) {
		wanted.format = AUDIO_U8;
	} else if (size == 16) {
		wanted.format = AUDIO_S16SYS;
	} else {
		DebugPrint("Unexpected sample size %d\n" _C_ size);
		wanted.format = AUDIO_S16SYS;
	}
	wanted.channels = 2;
	wanted.samples = 4096;
	wanted.callback = FillAudio;
	wanted.userdata = nullptr;

	//  Open the audio device, forcing the desired format
	if (SDL_OpenAudio(&wanted, &Audio.Format) < 0) {
		fprintf(stderr, "Couldn't open audio: %s\n", SDL_GetError());
		return -1;
	}
	SDL_PauseAudio(0);
	return 0;
}

/**
**  Initialize sound card.
**
**  @return  True if failure, false if everything ok.
*/
bool InitSound()
{
	//
	// Open sound device, 8bit samples, stereo.
	//
	if (InitSdlSound(44100, 16)) {
		SoundInitialized = false;
		return true;
	}
	SoundInitialized = true;

	// ARI: The following must be done here to allow sound to work in
	// pre-start menus!
	// initialize channels
	for (int i = 0; i < MaxChannels; ++i) {
		Channels[i].Point = i + 1;
		//Wyrmgus start
		Channels[i].Sample = nullptr;
		Channels[i].Unit = nullptr;
		Channels[i].Volume = 0;
		Channels[i].Stereo = 0;
		Channels[i].Voice = -1;
		Channels[i].Playing = false;
		//Wyrmgus end
	}

	// Create mutex and cond for FillThread
	Audio.MixerBuffer = new int[Audio.Format.samples * Audio.Format.channels];
	memset(Audio.MixerBuffer, 0, Audio.Format.samples * Audio.Format.channels * sizeof(int));
	Audio.Buffer = new Uint8[Audio.Format.size];
	memset(Audio.Buffer, 0, Audio.Format.size);
	Audio.Lock = SDL_CreateMutex();
	Audio.Cond = SDL_CreateCond();
	Audio.Running = true;

	// Create thread to fill sdl audio buffer
	Audio.Thread = SDL_CreateThread(FillThread, nullptr);
	return false;
}

/**
**  Cleanup sound server.
*/
void QuitSound()
{
	Audio.Running = false;
	SDL_WaitThread(Audio.Thread, nullptr);

	SDL_DestroyCond(Audio.Cond);
	SDL_DestroyMutex(Audio.Lock);

	// Mustn't call SDL_CloseAudio here, it'll be called again from SDL_Quit
	SoundInitialized = false;
	delete[] Audio.MixerBuffer;
	Audio.MixerBuffer = nullptr;
	delete[] Audio.Buffer;
	Audio.Buffer = nullptr;
}
