/*
 * Define synths control functions.
 * Basic inputs are midi commands
 */

// Call this once at setup
void setupControl();

void noteOn(byte channel, byte pitch, byte velocity);
void noteOff(byte channel, byte pitch, byte velocity);
void controlChange(byte channel, byte control, byte value);

// Must be call periodically on REFRESH_RATE
void updateChannels();