var it = null;
window.addEventListener('DOMContentLoaded', function(ev){
  it = document.getElementById('itunes_object');
}, false);

function current(){
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

