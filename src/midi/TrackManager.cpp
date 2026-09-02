#include "TrackManager.h"

TrackManager trackMgr;

DawTrack::DawTrack(uint8_t armCcNumber)
    : armCc(armCcNumber), volume(0), armed(false) {}

int DawTrack::toggleArm(uint8_t midiChannel) {
    armed = !armed;
    // Send momentary pulse (127 followed by 0)
    usbMIDI.sendControlChange(armCc, 127, midiChannel);
    usbMIDI.sendControlChange(armCc, 0, midiChannel);
    return armed ? volume : 0;
}

int DawTrack::adjustVolume(int incdec, uint8_t midiChannel) {
    if (!armed || incdec == 0) {
        return -1;
    }
    int step = (incdec > 0) ? 1 : -1;
    if ((step > 0 && volume < 127) || (step < 0 && volume > 0)) {
        volume = static_cast<uint8_t>(constrain(volume + step, 0, 127));
        // Volume CC is base arm CC + 3 (e.g. 107 -> 110, 108 -> 111, 109 -> 112)
        usbMIDI.sendControlChange(armCc + MidiCC::TRACK_VOL_OFFSET, volume, midiChannel);
        return volume;
    }
    return -1;
}

TrackManager::TrackManager()
    : tracks{
          DawTrack(MidiCC::TRACK_ARM_BASE + 0),
          DawTrack(MidiCC::TRACK_ARM_BASE + 1),
          DawTrack(MidiCC::TRACK_ARM_BASE + 2)
      },
      currentScene(MidiCC::SCENE_RECORD_START) {}

int TrackManager::toggleTrackArm(uint8_t trackIndex, uint8_t midiChannel) {
    if (trackIndex < 3) {
        return tracks[trackIndex].toggleArm(midiChannel);
    }
    return -1;
}

int TrackManager::adjustArmedTracksVolume(int incdec, uint8_t midiChannel) {
    int lastChangedVolume = -1;
    if (incdec != 0) {
        for (int i = 0; i < 3; i++) {
            int newVol = tracks[i].adjustVolume(incdec, midiChannel);
            if (newVol >= 0) {
                lastChangedVolume = newVol;
            }
        }
    }
    return lastChangedVolume;
}

void TrackManager::sendAllTrackLevels(uint8_t midiChannel) {
    for (int i = 0; i < 3; i++) {
        usbMIDI.sendControlChange(tracks[i].getCcNumber(), tracks[i].getVolume(), midiChannel);
    }
}

bool TrackManager::handleRecording(bool recPressed, bool stopPressed, uint8_t midiChannel) {
    if (recPressed) {
        // Cycle scenes CC 111..119
        currentScene = (currentScene >= MidiCC::SCENE_RECORD_END)
            ? MidiCC::SCENE_RECORD_START
            : currentScene + 1;
        usbMIDI.sendControlChange(currentScene, 127, midiChannel);
        usbMIDI.sendControlChange(currentScene, 0, midiChannel);
    }
    if (stopPressed) {
        // Stop recording and reset cycle to 111
        usbMIDI.sendControlChange(MidiCC::SCENE_STOP, 127, midiChannel);
        usbMIDI.sendControlChange(MidiCC::SCENE_STOP, 0, midiChannel);
        currentScene = MidiCC::SCENE_RECORD_START;
    }
    return (currentScene > MidiCC::SCENE_RECORD_START);
}

bool TrackManager::isTrackArmed(uint8_t trackIndex) const {
    if (trackIndex < 3) {
        return tracks[trackIndex].isArmed();
    }
    return false;
}

uint8_t TrackManager::getTrackVolume(uint8_t trackIndex) const {
    if (trackIndex < 3) {
        return tracks[trackIndex].getVolume();
    }
    return 0;
}
