# Dogwatch

this is a collection of bash scripts I use to keep an eye and ear on my (anxious) dog while I'm out of the house. Does a super-low-framerate webcam update and records sounds looking for volume peaks. Generates an HTML page and line graph of volume vs time.

Audio and video recordings may be performed simultaneously or on their own.


## Slowcam
### Usage
1. Connect video source to computer
1. Edit `slowcam` if your video source is not on /dev/video0, or you want to replace mplayer as the snapshot program
1. `./slowcam`
1. Point webserver to dogwatch/00000002.jpg for refreshable webcam


### Dependencies
 - **mplayer**: grabs screenshots from a webcam every second or so

Can be replaced by something like ffmpeg or anything else that can take a video feed and output a still frame


## Bark Watch
### Usage
1. Connect audio source to computer
1. Before leaving the house, run `./record_bark`
1. Return, kill the recording

To view the stats:

1. Run `./process_bark` to calculate audio peaks, will exit when finished.
1. `make` (adds header and footer to bark.txt)
1. point a local webserver at the folder
1. load `index.html` through localhost and view graph

Is the webserver a little unnecessary? sure. If you're really against it and want to spend the effort, you could add a step to `make` that puts the contents of `bark.txt` into `index.html` so it doesn't have to load over ajax


### Dependencies
 - **arecord**: part of ALSA. records quick bits of sound as wavs
 - **sox**: generates quick audio peaks of wav files

Also replaceable by anything that records sound, and can process peaks.
