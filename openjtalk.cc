#include <node.h>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>
#include "v8.hpp"
#include "text_to_speech.hpp"

using namespace v8;

/* ------------------------------------------------------------------------- */
//  JavaScript へ Export する : TextToSpeech
/* ------------------------------------------------------------------------- */
class TextToSpeechJS : public hecomi::V8::ExportToJSIF
{
private:
	//! TextToSpeech で喋らせるクラス
	boost::shared_ptr<TextToSpeech> tts_;

	//! 初期化したかどうか
	bool if_initialized_;

	//! 初期化したかどうかを返す
	bool initialized() {
		if (!if_initialized_) {
			std::cerr << "Error! TextToSpeech has not been initialized yet." << std::endl;
			return false;
		}
		return true;
	}

public:
	//! コンストラクタ
	TextToSpeechJS() : if_initialized_(false) {}

	//! JavaScript へエクスポートする関数
	boost::any func(const std::string& func_name, const v8::Arguments& args)
	{
		// 初期化
		if (func_name == "init") {
			if (args.Length() < 2) {
				std::cout << "Error! TextToSpeech.init must have 2 or 3 arguments." << std::endl;
				return false;
			}
			OpenJTalkParams params;
			v8::String::Utf8Value voice_dir(args[0]);
			v8::String::Utf8Value dic_dir(args[1]);
			if (args[2]->IsObject()) {
				v8::Local<v8::Object> obj = v8::Local<v8::Object>::Cast(args[2]);
				if (obj->Get(v8::String::New("sampling_rate"))->IsInt32())
					params.sampling_rate = obj->Get(v8::String::New("sampling_rate"))->Int32Value();
				if (obj->Get(v8::String::New("stage"))->IsInt32())
					params.stage = obj->Get(v8::String::New("stage"))->Int32Value();
				if (obj->Get(v8::String::New("audio_buff_size"))->IsInt32())
					params.audio_buff_size = obj->Get(v8::String::New("audio_buff_size"))->Int32Value();
				if (obj->Get(v8::String::New("alpha"))->IsNumber())
					params.alpha = obj->Get(v8::String::New("alpha"))->NumberValue();
				if (obj->Get(v8::String::New("beta"))->IsNumber())
					params.beta = obj->Get(v8::String::New("beta"))->NumberValue();
				if (obj->Get(v8::String::New("uv_threshold"))->IsNumber())
					params.uv_threshold = obj->Get(v8::String::New("uv_threshold"))->NumberValue();
				if (obj->Get(v8::String::New("gv_weight_mgc"))->IsNumber())
					params.gv_weight_mgc = obj->Get(v8::String::New("gv_weight_mgc"))->NumberValue();
				if (obj->Get(v8::String::New("gv_weight_lf0"))->IsNumber())
					params.gv_weight_lf0 = obj->Get(v8::String::New("gv_weight_lf0"))->NumberValue();
				if (obj->Get(v8::String::New("gv_weight_lpf"))->IsNumber())
					params.gv_weight_lpf = obj->Get(v8::String::New("gv_weight_lpf"))->NumberValue();
			}
			tts_ = boost::make_shared<TextToSpeech>(*voice_dir, *dic_dir, params);
			if_initialized_ = true;
			return true;
		}
		// 指定した言葉を喋る
		else if (func_name == "talk") {
			if (!initialized()) return false;
			std::string str = *(v8::String::Utf8Value(args[0]));
			int fperiod = 220; // default
			if (args[1]->IsInt32()) fperiod = args[1]->Int32Value();
			tts_->talk(str, fperiod);
			return true;
		}
		// 喋りを止める
		else if (func_name == "stop") {
			if (!initialized()) return false;
			tts_->stop();
			return true;
		}
		return 0;
	}
};

void init(Handle<Object> target) {
	hecomi::V8::ExportToJS<TextToSpeechJS> openjtalk("OpenJTalk");
	openjtalk.add_func<bool>("init");
	openjtalk.add_func<bool>("talk");
	openjtalk.add_func<bool>("stop");
	target->Set(
		String::NewSymbol(openjtalk.get_class_name().c_str()),
		openjtalk.get_class()->GetFunction()
	);
}

NODE_MODULE(openjtalk, init)

