var baseUrl = "http://www.biblegateway.com/votd/get/?format=plaintext&version=";
var configPageUrl = 'https://dl.dropboxusercontent.com/u/96641345/VOTDConfig.html';
var versionStorageKey = 0;
var verseRefStorageKey = 1;
var verseTextStorageKey = 2;
var updateTimeStorageKey = 3;
var enableLightStorageKey = 4;
var versionString = "esvuk";
var barStart = -1;
var barEnd = -1;
var verseRef;
var verseText;
var updateTime = -1;
var enableLight = true;
var barBoundsChanged = false;
var verseNew = false;
var updateTimeChanged = false;
var enableLightChanged = false;

var xhrRequest = function (url, type, callback) {
  var xhr = new XMLHttpRequest();
  xhr.onload = function () {
    callback(this.responseText);
  };
  xhr.open(type, url);
  xhr.send();
};

var sendToPebble = function() {	
	var dictionary = {};
	var sendMessage = false;
	
	if (verseNew)
	{
		dictionary.KEY_VERSE_REFERENCE = verseRef;
		dictionary.KEY_VERSE_TEXT = verseText;
		sendMessage = true;
	}
	/*
	if (barBoundsChanged)
	{
		dictionary.KEY_BAR_START = barStart;
		dictionary.KEY_BAR_END = barEnd;
		sendMessage = true;
	}
	*/
	if (updateTimeChanged)
	{		
		dictionary.KEY_UPDATE_TIME = updateTime;
		sendMessage = true;
	}
	
	if (enableLightChanged)
	{		
		dictionary.KEY_ENABLE_LIGHT = enableLight;
		sendMessage = true;
	}
	
	if (sendMessage) {
		console.log(JSON.stringify(dictionary));
		
		Pebble.sendAppMessage(dictionary,
		function(e) {
			console.log("Sent to Pebble successfully!");
			verseNew = verseNew ? false : true;
			barBoundsChanged = barBoundsChanged ? false : true;
			updateTimeChanged = updateTimeChanged ? false : true;
			enableLightChanged = enableLightChanged ? false : true;
		},
		function(e) {
			console.log("Error sending to Pebble!");
		});
	}
};

var verseReceivedCallback = function(responseText) {
	var reference = responseText.slice(0, responseText.search('\n'));
			
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
	
	if ( (verseRef != reference) || (verseText != text) )
	{
		verseRef = reference;
		verseText = text;
		verseNew = true;
		sendToPebble();
	}
};

var getVerse = function getVerse()
{	
	var url = baseUrl.concat(versionString);
	//console.log("Getting verse - URL: "+ url);
	xhrRequest(url, 'GET', verseReceivedCallback);
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

	tempVar = localStorage.getItem(updateTimeStorageKey);	
	if (tempVar !== null)
		updateTime = tempVar;

	tempVar = localStorage.getItem(enableLightStorageKey);	
	if (tempVar !== null)
		enableLight = tempVar;
	
	getVerse();
  }
);

// Listen for when an AppMessage is received
Pebble.addEventListener('appmessage',
  function(e) {
	getVerse();
  }
);

Pebble.addEventListener("webviewclosed",
  function(e) {
    var configuration = JSON.parse(decodeURIComponent(e.response));
    console.log("Configuration returned: " + JSON.stringify(configuration));
	
    var gettingVerse = false;
    
    if ( configuration.hasOwnProperty('versionString') && (configuration.versionString != versionString) ) {
      versionString = configuration.versionString;
      localStorage.setItem(versionStorageKey, versionString);
      getVerse();
      gettingVerse = true;
    }
    
    if ( configuration.hasOwnProperty('dayStart') && (configuration.dayStart != barStart.toString()) ) {
      barStart = configuration.dayStart;
      barBoundsChanged = true;
    }
	
    if ( configuration.hasOwnProperty('dayEnd') && (configuration.dayEnd != barEnd.toString()) ) {
      barEnd = configuration.dayEnd;
      barBoundsChanged = true;
    }
	
	if ( configuration.hasOwnProperty('updateTime') && (configuration.updateTime != updateTime.toString()) ) {
      updateTime = configuration.updateTime;
      localStorage.setItem(updateTimeStorageKey, updateTime);
      updateTimeChanged = true;
	}
	
	if ( configuration.hasOwnProperty('enableLight') && (configuration.enableLight != enableLight.toString()) ) {
      enableLight = (configuration.enableLight.toString() === "true");
      localStorage.setItem(enableLightStorageKey, enableLight);
      enableLightChanged = true;
	}
	
	if (!gettingVerse && (barBoundsChanged || updateTimeChanged|| enableLightChanged) ) // if getting verse then all will be sent when the verse is received
      sendToPebble();
  }
);

Pebble.addEventListener('showConfiguration', function(e) {
  var args = '?versionString=' + versionString;
  args += '&updateTime=' + updateTime.toString();
  args +='&enableLight=' + enableLight.toString();

  Pebble.openURL(configPageUrl + args);
});
