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

#ifndef INCLUDE_OPENJTALK_HPP
#define INCLUDE_OPENJTALK_HPP

#include <string>
#include <vector>

/* openjtalk header */
#include "mecab.h"
#include "njd.h"
#include "jpcommon.h"
#include "HTS_engine.h"

/* openal header */
#include <AL/alut.h>

/* Default Parameters */
struct OpenJTalkParams
{
	int    sampling_rate, stage, audio_buff_size;
	double alpha, beta, uv_threshold, gv_weight_mgc, gv_weight_lf0, gv_weight_lpf;
	OpenJTalkParams() {
		sampling_rate   = 48000;
		alpha           = 0.5;
		stage           = 0;
		beta            = 0.8;
		audio_buff_size = 48000;
		uv_threshold    = 0.5;
		gv_weight_mgc   = 1.0;
		gv_weight_lf0   = 1.0;
		gv_weight_lpf   = 1.0;
	}
};

/**
 * Open JTalkでTTSするクラス
 */
class TextToSpeech
{
public:
	/**
	 * OpenJTalkに渡すパラメータを生成
	 * @param[in] voice_dir	音素などが入ったディレクトリ
	 * @param[in] dic_dir	辞書入ったディレクトリ
	 */
	TextToSpeech(const std::string& voice_dir, const std::string& dic_dir, OpenJTalkParams params);

	/**
	 * デストラクタ
	 */
	~TextToSpeech();

	/**
	 * 引数の言葉を喋らせる
	 * @param[in] str	喋らせる文章
	 * @param[in] fperiod	ピッチ（default = 240）
	 */
	void talk(const std::string& str, const int fperiod = 240);

	/**
	 *  前回の言葉をもう一度喋る
	 */
	void retalk();

	/**
	 *  再生中の wav をストップする
	 */
	void stop();

private:
	/**
	 * Open JTalk に必要な情報をまとめておく構造体
	 */
	struct OpenJTalk {
		Mecab mecab;
		NJD njd;
		JPCommon jpcommon;
		HTS_Engine engine;
	} open_jtalk_;

	//! OpenJTalk を指定されたパラメタで初期化する（open_jtalk.c より抜粋）
	void initialize(
		int sampling_rate, int fperiod, double alpha, int stage, double beta, int audio_buff_size,
		double uv_threshold, HTS_Boolean use_log_gain, double gv_weight_mgc,
		double gv_weight_lf0, double gv_weight_lpf);

	//! 必要なファイル群をロードする（open_jtalk.c より抜粋）
	void load(
		char *dn_mecab, char *fn_ms_dur, char *fn_ts_dur,
		char *fn_ms_mgc, char *fn_ts_mgc, char **fn_ws_mgc, int num_ws_mgc,
		char *fn_ms_lf0, char *fn_ts_lf0, char **fn_ws_lf0, int num_ws_lf0,
		char *fn_ms_lpf, char *fn_ts_lpf, char **fn_ws_lpf, int num_ws_lpf,
		char *fn_ms_gvm, char *fn_ts_gvm, char *fn_ms_gvl, char *fn_ts_gvl,
		char *fn_ms_gvf, char *fn_ts_gvf, char *fn_gv_switch);

	//! 指定したテキストを喋る wav ファイルを作成する
	void synthesis(char *txt, FILE * wavfp);

	/**
	 * パラメタなどを整形して synthesis を実行する
	 * @param[in] sentence	喋らせる文章
	 * @param[in] fperiod
	 */
	//!
	void make_wav(const std::string& sentence, const int fperiod);

	//! wav ファイルを再生する
	void play_wav();

	//! wav ファイルを削除する
	void remove_wav() const;

	//! パラメータ
	OpenJTalkParams params_;

	//! 音素などが入ったディレクトリ
	std::string voice_dir_;

	//! 辞書が入ったディレクトリ
	std::string dic_dir_;

	//! 出力wavファイル名
	const std::string wav_filename_;

	//! 再生中の wav
	ALuint wav_src_;

	//! 前回喋った時の言葉
	std::string str_;

	//! 前回喋った時の速度
	int fperiod_;
};

#endif // INCLUDE_OPENJTALK_HPP

