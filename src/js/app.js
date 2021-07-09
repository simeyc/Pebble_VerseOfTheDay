var votdBaseUrl = 'http://www.biblegateway.com/votd/get/?format=plaintext&version=';
var favVerseBaseUrl = 'http://www.esvapi.org/v2/rest/passageQuery?key=IP&output-format=plain-text&include-first-verse-numbers=0&include-verse-numbers=0&include-footnotes=0&include-passage-horizontal-lines=0&include-heading-horizontal-lines=0&include-headings=0&include-subheadings=0&include-selahs=0&include-short-copyright=0&passage=';
var configPageUrl = 'https://dl.dropboxusercontent.com/u/96641345/VOTDConfigV1_5.html';
var versionStorageKey = 0;
var verseRefStorageKey = 1;
var verseTextStorageKey = 2;
var updateTimeStorageKey = 3;
var enableLightStorageKey = 4;
var btVibeStorageKey = 5;
var invertColoursStorageKey = 6;
var dateFormatStorageKey = 7;
var showAmPmStorageKey = 8;
var verseFontStorageKey = 9;
var scrollSpeedStorageKey = 10;
var favVerseEnableStorageKey = 11;
var favVerseStorageKey = 12;
var versionString = "esv";
var verseRef = "No Verse";
var verseText = "No Verse";
var dateFormat = 5;
var updateTime = 360;
var enableLight = true;
var btVibe = true;
var invertColours = false;
var showAmPm = true;
var verseFont = 1;
var scrollSpeed = 1;
var favVerseEnable = false;
var favVerse = 'joh3:16';

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var sendVerse = function() {
	var dictionary = {};
	
	dictionary.KEY_VERSE_REFERENCE = verseRef;
	dictionary.KEY_VERSE_TEXT = verseText;
	
	console.log("Sending to pebble: " + JSON.stringify(dictionary));
		
	Pebble.sendAppMessage(dictionary,
	function(e) {
		console.log("Sent verse to Pebble successfully!");
	},
	function(e) {
		console.log("Error sending verse to Pebble!");
	});
};

var saveAndSendVerse = function(reference, text) {	
	verseRef = reference;
	verseText = text;
    
	// persist store verse and ref
    localStorage.setItem(verseRefStorageKey, verseRef);
    localStorage.setItem(verseTextStorageKey, verseText);
	
	sendVerse();
};

var votdReceivedCallback = function(responseText) {
	var reference = responseText.slice(0, responseText.search('\n')).trim();
			
	var text = responseText.slice(reference.length+1); // up to the first '='
	text = text.slice(text.indexOf('\n'));
	text = text.replace(/\n/g, ' ');
	
	var squareBracketOpen = text.search(/\[ /);
	var squareBracketClose = text.search(/ \] /);
	
	while ( (squareBracketOpen >= 0) && (squareBracketClose >= 0) ) {
		text = text.replace(text.slice(squareBracketOpen, squareBracketClose+2), '');
		
		squareBracketOpen = text.search(/\[ /);
		squareBracketClose = text.search(/ \] /);
	}
	text = text.trim() + ' (' + versionString.toUpperCase() + ')';
	
	saveAndSendVerse(reference, text);
};

var favVerseReceivedCallback = function(responseText) {
	var reference = responseText.slice(0, responseText.search('\n')).trim();
	var text = '';
	
	if (reference.search('ERROR') >= 0)
	{
		reference = 'Invalid Verse';
		text = ':-(\n\nPlease check Favourite Verse settings';
	}
	else
	{
		text = responseText.slice(reference.length);
		text = text.replace(/\n|[ ]{2,}/g, ' ');
		text = text.trim() + ' (ESV)';
	}
	
	saveAndSendVerse(reference, text);
};

var getVerse = function() {
	var url;
	var callback;
			
	if (favVerseEnable) {
		url = favVerseBaseUrl.concat(favVerse);
		callback = favVerseReceivedCallback;
	}
	else {
		url = votdBaseUrl.concat(versionString);
		callback = votdReceivedCallback;
	}
	
	console.log("Getting verse - URL: "+ url);
	xhrRequest(url, 'GET', callback);
};

var sendUpdates = function(dateFormatChanged, updateTimeChanged, enableLightChanged, btVibeChanged,
							invertColoursChanged, showAmPmChanged, verseFontChanged, scrollSpeedChanged, needVerse) {
	var dictionary = {};
	var settingsChanged = false;
	
	if (dateFormatChanged) {		
		dictionary.KEY_DATE_FORMAT = dateFormat;
		settingsChanged = true;
	}
	
	if (updateTimeChanged) {		
		dictionary.KEY_UPDATE_TIME = updateTime;
		settingsChanged = true;
	}
	
	if (enableLightChanged) {		
		dictionary.KEY_ENABLE_LIGHT = enableLight?1:0;
		settingsChanged = true;
	}
	
	if (btVibeChanged) {		
		dictionary.KEY_BT_VIBE = btVibe?1:0;
		settingsChanged = true;
	}
	
	if (invertColoursChanged) {		
		dictionary.KEY_INVERT_COLOURS = invertColours?1:0;
		settingsChanged = true;
	}
	
	if (showAmPmChanged) {		
		dictionary.KEY_SHOW_AMPM = showAmPm?1:0;
		settingsChanged = true;
	}
	
	if (verseFontChanged) {		
		dictionary.KEY_VERSE_FONT = verseFont;
		settingsChanged = true;
	}
	
	if (scrollSpeedChanged) {		
		dictionary.KEY_SCROLL_SPEED = scrollSpeed;
		settingsChanged = true;
	}
	
	if (settingsChanged) {
		console.log("Sending to pebble: " + JSON.stringify(dictionary));
		
		Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log("Sent settings to Pebble successfully!");
			if (needVerse)
				getVerse();
		},
		function(e) {
			console.log("Error sending settings to Pebble!");
		});
	}
	else if (needVerse)
		getVerse();
};

// Listen for when the watchface is opened
Pebble.addEventListener('ready', 
  function(e) {	
	// Persist read a key's value. May be null!
	var tempVar = localStorage.getItem(versionStorageKey);	
	if (tempVar !== null)
		versionString = tempVar;
	
	tempVar = localStorage.getItem(verseRefStorageKey);	
	if (tempVar !== null)
		verseRef = tempVar;
	
	tempVar = localStorage.getItem(verseTextStorageKey);	
	if (tempVar !== null)
		verseText = tempVar;

	tempVar = localStorage.getItem(dateFormatStorageKey);	
	if (tempVar !== null)
		dateFormat = parseInt(tempVar);

	tempVar = localStorage.getItem(updateTimeStorageKey);	
	if (tempVar !== null)
		updateTime = parseInt(tempVar);

	tempVar = localStorage.getItem(enableLightStorageKey);	
	if (tempVar !== null)
		enableLight = (tempVar === 'true');

	tempVar = localStorage.getItem(btVibeStorageKey);	
	if (tempVar !== null)
		btVibe = (tempVar === 'true');

	tempVar = localStorage.getItem(invertColoursStorageKey);	
	if (tempVar !== null)
		invertColours = (tempVar === 'true');

	tempVar = localStorage.getItem(showAmPmStorageKey);	
	if (tempVar !== null)
		showAmPm = (tempVar === 'true');

	tempVar = localStorage.getItem(verseFontStorageKey);	
	if (tempVar !== null)
		verseFont = tempVar;

	tempVar = localStorage.getItem(scrollSpeedStorageKey);	
	if (tempVar !== null)
		scrollSpeed = tempVar;

	tempVar = localStorage.getItem(favVerseEnableStorageKey);	
	if (tempVar !== null)
		favVerseEnable = (tempVar === 'true');
	
	tempVar = localStorage.getItem(favVerseStorageKey);	
	if (tempVar !== null)
		favVerse = tempVar;
	
	//if (verseRef === "No Verse")
	//{
	//	console.log("verseRef == NoVerse: " + verseRef);
		getVerse();
	//}
	//else
	//{
	//	console.log("verseRef != NoVerse: " + verseRef);
	//	sendVerse();
	//}
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
    console.log("Message Received: " + JSON.stringify(e.payload));
	
	if (e.payload.hasOwnProperty("KEY_REQUEST_SETTINGS"))
		sendUpdates(true, true, true, true, true, true, true, true, true); // will cause verse to be got after all settings are sent
	else
		getVerse();
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration returned: " + JSON.stringify(configuration));
	
	var dateFormatChanged = false;
    var updateTimeChanged = false;
    var enableLightChanged = false;
    var btVibeChanged = false;
    var invertColoursChanged = false;
    var showAmPmChanged = false;
    var verseFontChanged = false;
    var scrollSpeedChanged = false;
    var needVerse = false;
    
    if ( configuration.hasOwnProperty('versionString') && (configuration.versionString != versionString) ) {
      versionString = configuration.versionString;
      localStorage.setItem(versionStorageKey, versionString);
      needVerse = true;
    }
    
    if ( configuration.hasOwnProperty('dateFormat') && (configuration.dateFormat != dateFormat.toString()) ) {
      dateFormat = parseInt(configuration.dateFormat);
      localStorage.setItem(dateFormatStorageKey, dateFormat);
      dateFormatChanged = true;
    }
	
	if ( configuration.hasOwnProperty('updateTime') && (configuration.updateTime != updateTime.toString()) ) {
      updateTime = parseInt(configuration.updateTime);
      localStorage.setItem(updateTimeStorageKey, updateTime);
      updateTimeChanged = true;
	}
	
	if ( configuration.hasOwnProperty('enableLight') && (configuration.enableLight != enableLight.toString()) ) {
      enableLight = (configuration.enableLight.toString() === "true");
      localStorage.setItem(enableLightStorageKey, enableLight);
      enableLightChanged = true;
	}
	
	if ( configuration.hasOwnProperty('btVibe') && (configuration.btVibe != btVibe.toString()) ) {
      btVibe = (configuration.btVibe.toString() === "true");
      localStorage.setItem(btVibeStorageKey, btVibe);
      btVibeChanged = true;
	}
	
	if ( configuration.hasOwnProperty('invertColours') && (configuration.invertColours != invertColours.toString()) ) {
      invertColours = (configuration.invertColours.toString() === "true");
      localStorage.setItem(invertColoursStorageKey, invertColours);
      invertColoursChanged = true;
	}
	
	if ( configuration.hasOwnProperty('showAmPm') && (configuration.showAmPm != showAmPm.toString()) ) {
      showAmPm = (configuration.showAmPm.toString() === "true");
      localStorage.setItem(showAmPmStorageKey, showAmPm);
      showAmPmChanged = true;
	}
	
	if ( configuration.hasOwnProperty('verseFont') && (configuration.verseFont != verseFont.toString()) ) {
      verseFont = parseInt(configuration.verseFont);
      localStorage.setItem(verseFontStorageKey, verseFont);
      verseFontChanged = true;
	}
	
	if ( configuration.hasOwnProperty('scrollSpeed') && (configuration.scrollSpeed != scrollSpeed.toString()) ) {
      scrollSpeed = parseInt(configuration.scrollSpeed);
      localStorage.setItem(scrollSpeedStorageKey, scrollSpeed);
      scrollSpeedChanged = true;
	}
	
	if ( configuration.hasOwnProperty('favVerseEnable') && (configuration.favVerseEnable != favVerseEnable.toString()) ) {
      favVerseEnable = (configuration.favVerseEnable.toString() === "true");
      localStorage.setItem(favVerseEnableStorageKey, favVerseEnable);
      needVerse = true;
	}
	
	if ( configuration.hasOwnProperty('favVerse') && (configuration.favVerse != favVerse) ) {
      favVerse = configuration.favVerse;
      localStorage.setItem(favVerseStorageKey, favVerse);
      needVerse = true;
	}
	
    sendUpdates(dateFormatChanged, updateTimeChanged, enableLightChanged, btVibeChanged,
		invertColoursChanged, showAmPmChanged, verseFontChanged, scrollSpeedChanged, needVerse);
  }
);

Pebble.addEventListener('showConfiguration',
  function(e) {
  var args = '?versionString=' + versionString;
  args += '&dateFormat=' + dateFormat.toString();
  args += '&updateTime=' + updateTime.toString();
  args +='&enableLight=' + enableLight.toString();
  args +='&btVibe=' + btVibe.toString();
  args +='&invertColours=' + invertColours.toString();
  args +='&showAmPm=' + showAmPm.toString();
  args +='&verseFont=' + verseFont.toString();
  args +='&scrollSpeed=' + scrollSpeed.toString();
  args +='&favVerseEnable=' + favVerseEnable.toString();
  args +='&favVerse=' + favVerse.toString();

  Pebble.openURL(configPageUrl + args);
}
);
