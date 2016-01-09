void onNoteOff(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOff(note, 0, channel);
}
void onNoteOn(byte channel, byte note, byte velocity){
  usbMIDI.sendNoteOn(note, velocity, channel);
}

void onPolyPressure(byte channel, byte note, byte pressure){
	usbMIDI.sendPolyPressure(note, pressure, channel);
}

void onControl(byte channel, byte control, byte value){
	usbMIDI.sendControlChange(control, value, channel);
}

void onProgram(byte channel, byte program){
	usbMIDI.sendProgramChange(program, channel);
}

void onAfterTouch(byte channel, byte pressure){
	usbMIDI.sendAfterTouch(pressure, channel);
}

void onPitchBend(byte channel, int bend){
	usbMIDI.sendPitchBend(bend, channel);
}

