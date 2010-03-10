var it = null;
window.addEventListener('DOMContentLoaded', function(ev){
  it = document.getElementById('itunes_object');
}, false);

function currentTrack(){
  var track = it.currentTrack();
  if(track){
    return {
      "artist": track.artist,
      "album": track.album,
      "name": track.name,
      "genre": track.genre,
    };
  } else {
    return null;
  }
}

var requestsTable = {
  'currentTrack': function(req, sender, func){
    var track = currentTrack();
    if(track){
      func({
        success: true,
        content: track
      });
    } else {
      // not selected
      func({
        success: false
      });
    }
    return true;
  }
};


chrome.extension.onRequestExternal.addListener(function(req, sender, func){
  if(!(req.action &&
       req.action in requestsTable &&
       requestsTable[req.action](req, sender, func))){
    func({
      success: false,
      response: 'invalid request'
    });
  }
});

