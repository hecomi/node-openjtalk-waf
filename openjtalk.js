// sample

var util     = require('util');
var https    = require('https');
var host     = 'stream.twitter.com';

var OpenJTalk = require('./build/Release/openjtalk').OpenJTalk;
var mei = new OpenJTalk();
mei.init("data/mei_normal", "openjtalk/open_jtalk_dic_utf_8-1.05");

/* 以下のようにパラメタを指定することも出来ます
mei.init("data/mei_normal", "openjtalk/open_jtalk_dic_utf_8-1.05", {
	beta: 0.5,
	uv_threshold: 0.2,
	alpha: 0.2,
});
*/

var request  = https.get({
	host : host,
	port : 443,
	path : encodeURI('/1/statuses/filter.json?track=初音ミク'),
	auth : 'TwitterID:TwitterPass'
}, function(response) {
	console.log('Response: ' + response.statusCode);
}).on('error', function(e){
	console.log(e);
}).on('response', function(response) {
	response.on('data', function(chunk) {
		var tweet = JSON.parse(chunk);
		if ('user' in tweet && 'name' in tweet.user) {
			util.puts('[' + tweet.user['name'] + ']\n' + tweet.text);
			mei.talk(tweet.user.name);
		}
	});
});

process.on('uncaughtException', function (err) {
	console.log('uncaughtException => ' + err);
});
