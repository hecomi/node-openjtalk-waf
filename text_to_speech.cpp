/* ----------------------------------------------------------------- */
/*           The Japanese TTS System "Open JTalk"                    */
/*           developed by HTS Working Group                          */
/*           http://open-jtalk.sourceforge.net/                      */
/* ----------------------------------------------------------------- */
/*                                                                   */
/*  Copyright (c) 2008-2011  Nagoya Institute of Technology          */
/*                           Department of Computer Science          */
/*                                                                   */
/* All rights reserved.                                              */
/*                                                                   */
/* Redistribution and use in source and binary forms, with or        */
/* without modification, are permitted provided that the following   */
/* conditions are met:                                               */
/*                                                                   */
/* - Redistributions of source code must retain the above copyright  */
/*   notice, this list of conditions and the following disclaimer.   */
/* - Redistributions in binary form must reproduce the above         */
/*   copyright notice, this list of conditions and the following     */
/*   disclaimer in the documentation and/or other materials provided */
/*   with the distribution.                                          */
/* - Neither the name of the HTS working group nor the names of its  */
/*   contributors may be used to endorse or promote products derived */
/*   from this software without specific prior written permission.   */
/*                                                                   */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND            */
/* CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES,       */
/* INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF          */
/* MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE          */
/* DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS */
/* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,          */
/* EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED   */
/* TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,     */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON */
/* ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,   */
/* OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY    */
/* OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE           */
/* POSSIBILITY OF SUCH DAMAGE.                                       */
/* ----------------------------------------------------------------- */

/* class header */
#include "text_to_speech.hpp"

/* openjtalk headers */
#include "text2mecab.h"
#include "mecab2njd.h"
#include "njd_set_pronunciation.h"
#include "njd_set_digit.h"
#include "njd_set_accent_phrase.h"
#include "njd_set_accent_type.h"
#include "njd_set_unvoiced_vowel.h"
#include "njd_set_long_vowel.h"
#include "njd2jpcommon.h"

/* c headers */
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <cstring>
#include <cmath>

/* c++ header */
#include <sstream>
#include <iostream>

//! buffer size for text2mecab
const size_t MAXBUFLEN = 1024;

TextToSpeech::TextToSpeech(const std::string& voice_dir_, const std::string& dic_dir, OpenJTalkParams params)
:	voice_dir_(voice_dir_),
	dic_dir_(dic_dir),
	wav_filename_("__tmp__.wav"),
	params_(params)
{
}

TextToSpeech::~TextToSpeech()
{
	Mecab_clear(&open_jtalk_.mecab);
	NJD_clear(&open_jtalk_.njd);
	JPCommon_clear(&open_jtalk_.jpcommon);
	HTS_Engine_clear(&open_jtalk_.engine);
}

void TextToSpeech::initialize(
	int sampling_rate, int fperiod, double alpha, int stage, double beta, int audio_buff_size,
	double uv_threshold, HTS_Boolean use_log_gain, double gv_weight_mgc,
	double gv_weight_lf0, double gv_weight_lpf)
{
	Mecab_initialize(&open_jtalk_.mecab);
	NJD_initialize(&open_jtalk_.njd);
	JPCommon_initialize(&open_jtalk_.jpcommon);
	HTS_Engine_initialize(&open_jtalk_.engine, 2);
	HTS_Engine_set_sampling_rate(&open_jtalk_.engine, sampling_rate);
	HTS_Engine_set_fperiod(&open_jtalk_.engine, fperiod);
	HTS_Engine_set_alpha(&open_jtalk_.engine, alpha);
	HTS_Engine_set_gamma(&open_jtalk_.engine, stage);
	HTS_Engine_set_log_gain(&open_jtalk_.engine, use_log_gain);
	HTS_Engine_set_beta(&open_jtalk_.engine, beta);
	HTS_Engine_set_audio_buff_size(&open_jtalk_.engine, audio_buff_size);
	HTS_Engine_set_msd_threshold(&open_jtalk_.engine, 1, uv_threshold);
	HTS_Engine_set_gv_weight(&open_jtalk_.engine, 0, gv_weight_mgc);
	HTS_Engine_set_gv_weight(&open_jtalk_.engine, 1, gv_weight_lf0);
}

void TextToSpeech::load(
	char *dn_mecab, char *fn_ms_dur, char *fn_ts_dur,
	char *fn_ms_mgc, char *fn_ts_mgc, char **fn_ws_mgc, int num_ws_mgc,
	char *fn_ms_lf0, char *fn_ts_lf0, char **fn_ws_lf0, int num_ws_lf0,
	char *fn_ms_lpf, char *fn_ts_lpf, char **fn_ws_lpf, int num_ws_lpf,
	char *fn_ms_gvm, char *fn_ts_gvm, char *fn_ms_gvl, char *fn_ts_gvl,
	char *fn_ms_gvf, char *fn_ts_gvf, char *fn_gv_switch)
{
	Mecab_load(&open_jtalk_.mecab, dn_mecab);
	HTS_Engine_load_duration_from_fn(&open_jtalk_.engine, &fn_ms_dur, &fn_ts_dur, 1);
	HTS_Engine_load_parameter_from_fn(&open_jtalk_.engine, &fn_ms_mgc, &fn_ts_mgc, fn_ws_mgc, 0, FALSE, num_ws_mgc, 1);
	HTS_Engine_load_parameter_from_fn(&open_jtalk_.engine, &fn_ms_lf0, &fn_ts_lf0, fn_ws_lf0, 1, TRUE, num_ws_lf0, 1);
	if (HTS_Engine_get_nstream(&open_jtalk_.engine) == 3)
		HTS_Engine_load_parameter_from_fn(&open_jtalk_.engine, &fn_ms_lpf, &fn_ts_lpf, fn_ws_lpf, 2, FALSE, num_ws_lpf, 1);
	if (fn_ms_gvm != NULL) {
		if (fn_ts_gvm != NULL)
			HTS_Engine_load_gv_from_fn(&open_jtalk_.engine, &fn_ms_gvm, &fn_ts_gvm, 0, 1);
		else
			HTS_Engine_load_gv_from_fn(&open_jtalk_.engine, &fn_ms_gvm, NULL, 0, 1);
	}
	if (fn_ms_gvl != NULL) {
		if (fn_ts_gvl != NULL)
			HTS_Engine_load_gv_from_fn(&open_jtalk_.engine, &fn_ms_gvl, &fn_ts_gvl, 1, 1);
		else
			HTS_Engine_load_gv_from_fn(&open_jtalk_.engine, &fn_ms_gvl, NULL, 1, 1);
	}
	if (HTS_Engine_get_nstream(&open_jtalk_.engine) == 3 && fn_ms_gvf != NULL) {
		if (fn_ts_gvf != NULL)
			HTS_Engine_load_gv_from_fn(&open_jtalk_.engine, &fn_ms_gvf, &fn_ts_gvf, 2, 1);
		else
			HTS_Engine_load_gv_from_fn(&open_jtalk_.engine, &fn_ms_gvf, NULL, 2, 1);
	}
	if (fn_gv_switch != NULL)
		HTS_Engine_load_gv_switch_from_fn(&open_jtalk_.engine, fn_gv_switch);
}

void TextToSpeech::synthesis(char *txt, FILE * wavfp)
{
	char buff[MAXBUFLEN];

	text2mecab(buff, txt);
	Mecab_analysis(&open_jtalk_.mecab, buff);
	mecab2njd(&open_jtalk_.njd, Mecab_get_feature(&open_jtalk_.mecab), Mecab_get_size(&open_jtalk_.mecab));
	njd_set_pronunciation(&open_jtalk_.njd);
	njd_set_digit(&open_jtalk_.njd);
	njd_set_accent_phrase(&open_jtalk_.njd);
	njd_set_accent_type(&open_jtalk_.njd);
	njd_set_unvoiced_vowel(&open_jtalk_.njd);
	njd_set_long_vowel(&open_jtalk_.njd);
	njd2jpcommon(&open_jtalk_.jpcommon, &open_jtalk_.njd);
	JPCommon_make_label(&open_jtalk_.jpcommon);
	if (JPCommon_get_label_size(&open_jtalk_.jpcommon) > 2) {
		HTS_Engine_load_label_from_string_list(
			&open_jtalk_.engine,
			JPCommon_get_label_feature(&open_jtalk_.jpcommon),
			JPCommon_get_label_size(&open_jtalk_.jpcommon)
		);
		HTS_Engine_create_sstream(&open_jtalk_.engine);
		HTS_Engine_create_pstream(&open_jtalk_.engine);
		HTS_Engine_create_gstream(&open_jtalk_.engine);
		if (wavfp != NULL)
			HTS_Engine_save_riff(&open_jtalk_.engine, wavfp);
		HTS_Engine_refresh(&open_jtalk_.engine);
	}
	JPCommon_refresh(&open_jtalk_.jpcommon);
	NJD_refresh(&open_jtalk_.njd);
	Mecab_refresh(&open_jtalk_.mecab);
}

void TextToSpeech::make_wav(const std::string& sentence, int fperiod)
{
	/* file name buffer size */
	const size_t FILE_NAME_BUF_SIZE = 128;

	/* output wav file */
	FILE *wavfp = fopen(wav_filename_.c_str(), "wb");
	if (wavfp == NULL) {
		fprintf(stderr, "ERROR: Getfp() in open_jtalk.c: Cannot open %s.\n", wav_filename_.c_str());
		return;
	}

	/* sentence */
	char talk_str[FILE_NAME_BUF_SIZE]; strcpy(talk_str, sentence.c_str());

	/* directory name of dictionary */
	char dn_mecab[FILE_NAME_BUF_SIZE]; strcpy(dn_mecab, dic_dir_.c_str());

	/* file names of models */
	char fn_ms_dur[FILE_NAME_BUF_SIZE]; strcpy(fn_ms_dur, (voice_dir_ + "/dur.pdf").c_str());
	char fn_ms_mgc[FILE_NAME_BUF_SIZE]; strcpy(fn_ms_mgc, (voice_dir_ + "/mgc.pdf").c_str());
	char fn_ms_lf0[FILE_NAME_BUF_SIZE]; strcpy(fn_ms_lf0, (voice_dir_ + "/lf0.pdf").c_str());
	char *fn_ms_lpf = NULL;

	/* file names of trees */
	char fn_ts_dur[FILE_NAME_BUF_SIZE]; strcpy(fn_ts_dur, (voice_dir_ + "/tree-dur.inf").c_str());
	char fn_ts_mgc[FILE_NAME_BUF_SIZE]; strcpy(fn_ts_mgc, (voice_dir_ + "/tree-mgc.inf").c_str());
	char fn_ts_lf0[FILE_NAME_BUF_SIZE]; strcpy(fn_ts_lf0, (voice_dir_ + "/tree-lf0.inf").c_str());
	char *fn_ts_lpf = NULL;

	/* file names of windows */
	const int FN_WS_BUF_SIZE = 3;
	char **fn_ws_mgc = new char*[FN_WS_BUF_SIZE];
	char **fn_ws_lf0 = new char*[FN_WS_BUF_SIZE];
	for (int i = 0; i < FN_WS_BUF_SIZE; ++i) {
		fn_ws_mgc[i] = new char[FILE_NAME_BUF_SIZE];
		fn_ws_lf0[i] = new char[FILE_NAME_BUF_SIZE];
		sprintf(fn_ws_mgc[i], (voice_dir_ + "/mgc.win%d").c_str(), i+1);
		sprintf(fn_ws_lf0[i], (voice_dir_ + "/lf0.win%d").c_str(), i+1);
	}
	char **fn_ws_lpf = NULL;
	int num_ws_mgc = FN_WS_BUF_SIZE, num_ws_lf0 = FN_WS_BUF_SIZE, num_ws_lpf = 0;

	/* file names of global variance */
	char fn_ms_gvm[FILE_NAME_BUF_SIZE]; strcpy(fn_ms_gvm, (voice_dir_ + "/gv-mgc.pdf").c_str());
	char fn_ms_gvf[FILE_NAME_BUF_SIZE]; strcpy(fn_ms_gvf, (voice_dir_ + "/gv-lf0.pdf").c_str());
	char *fn_ms_gvl = NULL;

	/* file names of global variance trees */
	char fn_ts_gvm[FILE_NAME_BUF_SIZE]; strcpy(fn_ts_gvm, (voice_dir_ + "/tree-gv-mgc.inf").c_str());
	char fn_ts_gvf[FILE_NAME_BUF_SIZE]; strcpy(fn_ts_gvf, (voice_dir_ + "/tree-gv-lf0.inf").c_str());
	char *fn_ts_gvl = NULL;

	/* file names of global variance switch */
	char fn_gv_switch[FILE_NAME_BUF_SIZE]; strcpy(fn_gv_switch, (voice_dir_ + "/gv-switch.inf").c_str());

	/* global parameter */
	int    sampling_rate   = params_.sampling_rate;
	double alpha           = params_.alpha;
	int    stage           = params_.stage;
	double beta            = params_.beta;
	int    audio_buff_size = params_.audio_buff_size;
	double uv_threshold    = params_.uv_threshold;
	double gv_weight_mgc   = params_.gv_weight_mgc;
	double gv_weight_lf0   = params_.gv_weight_lf0;
	double gv_weight_lpf   = params_.gv_weight_lpf;
	HTS_Boolean use_log_gain = FALSE;

	/* initialize and load */
	initialize(sampling_rate, fperiod, alpha, stage, beta,
		audio_buff_size, uv_threshold, use_log_gain, gv_weight_mgc,
		gv_weight_lf0, gv_weight_lpf);
	load(dn_mecab, fn_ms_dur, fn_ts_dur, fn_ms_mgc, fn_ts_mgc,
		fn_ws_mgc, num_ws_mgc, fn_ms_lf0, fn_ts_lf0, fn_ws_lf0, num_ws_lf0,
		fn_ms_lpf, fn_ts_lpf, fn_ws_lpf, num_ws_lpf, fn_ms_gvm, fn_ts_gvm,
		fn_ms_gvl, fn_ts_gvl, fn_ms_gvf, fn_ts_gvf, fn_gv_switch);

	/* synthesis */
	synthesis(talk_str, wavfp);

	/* free */
	fclose(wavfp);
}

void TextToSpeech::play_wav()
{
	// alutの初期化
	int alut_argc = 0;
	char* alut_argv[] = {};
	alutInit(&alut_argc, alut_argv);

	// ソースの用意
	ALuint buf;
	ALenum state;
	buf = alutCreateBufferFromFile(wav_filename_.c_str());
	alGenSources(1, &wav_src_);
	alSourcei(wav_src_, AL_BUFFER, buf);

	alSourcePlay(wav_src_);

	alGetSourcei(wav_src_, AL_SOURCE_STATE, &state);
	while (state == AL_PLAYING) {
		alGetSourcei(wav_src_, AL_SOURCE_STATE, &state);
	}

	// 後片付け
	alDeleteSources(1, &wav_src_);
	alDeleteBuffers(1, &buf);
	alutExit();
}

void TextToSpeech::stop()
{
	alSourceStop(wav_src_);
}

void TextToSpeech::remove_wav() const
{
	remove( wav_filename_.c_str() );
}

void TextToSpeech::talk(const std::string& str, int fperiod)
{
	// retalk の為に覚えておく
	str_ = str; fperiod_ = fperiod;

	std::cout << str << std::endl;
	make_wav(str, fperiod);
	play_wav();
	remove_wav();
}

void TextToSpeech::retalk()
{
	std::cout << str_ << std::endl;
	make_wav(str_, fperiod_);
	play_wav();
	remove_wav();
}
