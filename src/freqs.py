import json

def get_midi_freqs():
	return [
		(0,  "C-1", 8.18),
		(1,  "Cs-1", 8.66),
		(2,  "D-1", 9.18),
		(3,  "Ds-1", 9.72),
		(4,  "E-1", 10.30),
		(5,  "F-1", 10.91),
		(6,  "Fs-1", 11.56),
		(7,  "G-1", 12.25),
		(8,  "Gs-1", 12.98),
		(9,  "A-1", 13.75),
		(10, "As-1", 14.57),
		(11, "B-1", 15.43),
		(12, "C0", 16.35),
		(13, "Cs0", 17.32),
		(14, "D0", 18.35),
		(15, "Ds0", 19.45),
		(16, "E0", 20.60),
		(17, "F0", 21.83),
		(18, "Fs0", 23.12),
		(19, "G0", 24.50),
		(20, "Gs0", 25.96),
		(21, "A0", 27.50),
		(22, "As0", 29.14),
		(23, "B0", 30.87),
		(24, "C1", 32.70),
		(25, "Cs1", 34.65),
		(26, "D1", 36.71),
		(27, "Ds1", 38.89),
		(28, "E1", 41.20),
		(29, "F1", 43.65),
		(30, "Fs1", 46.25),
		(31, "G1", 49.00),
		(32, "Gs1", 51.91),
		(33, "A1", 55.00),
		(34, "As1", 58.27),
		(35, "B1", 61.74),
		(36, "C2", 65.41),
		(37, "Cs2", 69.30),
		(38, "D2", 73.42),
		(39, "Ds2", 77.78),
		(40, "E2", 82.41),
		(41, "F2", 87.31),
		(42, "Fs2", 92.50),
		(43, "G2", 98.00),
		(44, "Gs2", 103.83),
		(45, "A2", 110.00),
		(46, "As2", 116.54),
		(47, "B2", 123.47),
		(48, "C3", 130.81),
		(49, "Cs3", 138.59),
		(50, "D3", 146.83),
		(51, "Ds3", 155.56),
		(52, "E3", 164.81),
		(53, "F3", 174.61),
		(54, "Fs3", 185.00),
		(55, "G3", 196.00),
		(56, "Gs3", 207.65),
		(57, "A3", 220.00),
		(58, "As3", 233.08),
		(59, "B3", 246.94),
		(60, "C4", 261.63),
		(61, "Cs4", 277.18),
		(62, "D4", 293.66),
		(63, "Ds4", 311.13),
		(64, "E4", 329.63),
		(65, "F4", 349.23),
		(66, "Fs4", 369.99),
		(67, "G4", 392.00),
		(68, "Gs4", 415.30),
		(69, "A4", 440.00),
		(70, "As4", 466.16),
		(71, "B4", 493.88),
		(72, "C5", 523.25),
		(73, "Cs5", 554.37),
		(74, "D5", 587.33),
		(75, "Ds5", 622.25),
		(76, "E5", 659.26),
		(77, "F5", 698.46),
		(78, "Fs5", 739.99),
		(79, "G5", 783.99),
		(80, "Gs5", 830.61),
		(81, "A5", 880.00),
		(82, "As5", 932.33),
		(83, "B5", 987.77),
		(84, "C6", 1046.50),
		(85, "Cs6", 1108.73),
		(86, "D6", 1174.66),
		(87, "Ds6", 1244.51),
		(88, "E6", 1318.51),
		(89, "F6", 1396.91),
		(90, "Fs6", 1479.98),
		(91, "G6", 1567.98),
		(92, "Gs6", 1661.22),
		(93, "A6", 1760.00),
		(94, "As6", 1864.66),
		(95, "B6", 1975.53),
		(96, "C7", 2093.00),
		(97, "Cs7", 2217.46),
		(98, "D7", 2349.32),
		(99, "Ds7", 2489.02),
		(100, "E7", 2637.02),
		(101, "F7", 2793.83),
		(102, "Fs7", 2959.96),
		(103, "G7", 3135.96),
		(104, "Gs7", 3322.44),
		(105, "A7", 3520.00),
		(106, "As7", 3729.31),
		(107, "B7", 3951.07),
		(108, "C8", 4186.01),
		(109, "Cs8", 4434.92),
		(110, "D8", 4698.64),
		(111, "Ds8", 4978.03),
		(112, "E8", 5274.04),
		(113, "F8", 5587.65),
		(114, "Fs8", 5919.91),
		(115, "G8", 6271.93),
		(116, "Gs8", 6644.88),
		(117, "A8", 7040.00),
		(118, "As8", 7458.62),
		(119, "B8", 7902.13),
		(120, "C9", 8372.02),
		(121, "Cs9", 8869.84),
		(122, "D9", 9397.27),
		(123, "Ds9", 9956.06),
		(124, "E9", 10548.08),
		(125, "F9", 11175.30),
		(126, "Fs9", 11839.82),
		(127, "G9", 12543.85),
	]

# Settings
clock_stepper = 5

# How many steps to trigger flipflop, which must be triggered twice for clock signal
clock_divider = (clock_stepper + 1) * 2

clock_frequency = 16e6

output_freq = clock_frequency / clock_divider

midi_freqs = get_midi_freqs()
dividers = [(i, n, f, output_freq / (32 * f)) for (i, n, f) in midi_freqs]
dividers = [(i, n, f, d, round(min(d, (1<<10) - 1))) for (i, n, f, d) in dividers]
dividers = [(i, n, f, d, int_d, abs(int_d - d) * 100 / d) for (i, n, f, d, int_d) in dividers]

print("const unsigned int NOTES[128] PROGMEM = {")
for (index, name, freq, divider, int_divider, err) in dividers:
	print("    {} /* {}: {} freq={:.2f}, divider={:.2f}, err={:.2f}% */,".format(int_divider, index, name, freq, divider, err))
print("};")
