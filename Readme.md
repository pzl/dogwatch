# Dogwatch

Dogwatch is a program designed to pet-sit and monitor your dog while you're out of the house. The program is able to count how many times your dog barked or whined while you were away.

Dogwatch started as a need to keep an eye and ear on my anxious dog while I was out of the house. She frequently tried to break out of her kennel, necessitating a remote video feed to watch whether she's busted out or not yet. The feed was presented as a jpeg image on a web server, refreshing the image every second or so. With a hefty case of separation anxiety, my dog also barked and I worried for the ears of my neighbors. I started the bark counter portion to measure whether training was effective at reducing my pup's anxiety. It's hard to know how much your dog barked while you were out to dinner, without asking your neighbors.

This program is intended to work cross-platform (Linux, OS X 10.6+, Windows?). 

## Installation

### Precompiled  

See the Releases tab and download the latest release for your platform

### From Source

Required libraries: Cairo, PortAudio

clone this repository and 
```
make
```


## Setup

### Microphone

Dogwatch includes a microphone testing viewer. It allows you to view the live microphone input provided to the program. Launch this view by typing
```
./dogwatch r
```

in a terminal, and your window should become a live waveform viewer. If you talk or make noises into your microphone, you should see significant change in the waveform viewer. If it remains a flat line, you may need to check your system configuration for the default microphone.



## Usage

Place your laptop, or computer microphone close to your dog. If you keep your dog in their kennel or fenced off area, then close to that. If you leave your dog free in the house while you're away, place the microphone near to the area of the house your dog frequently goes to to bark (e.g. window, door).

Before you leave, begin the recording program with
```
./dogwatch
```

The recording will start, and you can let the program run while away. When you return, exit the program with <kbd>Ctrl-C</kbd>. The program will print the rough number of times your pet barked.

### Video Monitor
1. Connect video source to computer
1. Edit `slowcam` if your video source is not on /dev/video0, or you want to replace mplayer as the snapshot program
1. `./slowcam`
1. Point webserver to dogwatch/00000002.jpg for refreshable webcam


### Dependencies
 - **mplayer**: grabs screenshots from a webcam every second or so

Can be replaced by something like ffmpeg or anything else that can take a video feed and output a still frame



## Contributing

If you are interested in contributing to this project, please send me a message! Development is still very early and not everything needed is documented through issues yet.



## Roadmap

Planned big changes:

- cross-platform GUI (GTK,QT,etc)
    + live waveform canvas and reviewer
- Advanced bark categorization through frequency analysis and similar speech recognition methods
- Machine learning to help identify individual dogs and environments
- user feedback on bark (mis-)categorization