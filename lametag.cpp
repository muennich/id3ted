/* id3ted: lametag.cpp
 * Copyright (c) 2011 Bert Muennich <be.muennich at googlemail.com>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

/* LAME tag specification:
 *   http://gabriel.mp3-tech.org/mp3infotag.html
 *
 * other open source implementations:
 *   - LameTag by phwip, http://phwip.wordpress.com/home/audio/
 *   - MAD, http://www.underbit.com/products/mad/ (madplay/tag.c)
 */

#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sstream>

#include "lametag.h"
#include "fileio.h"

unsigned short LameTag::crc16Table[] = {
	0x0000, 0xc0c1, 0xc181, 0x0140, 0xc301, 0x03c0, 0x0280, 0xc241,
	0xc601, 0x06c0, 0x0780, 0xc741, 0x0500, 0xc5c1, 0xc481, 0x0440,
	0xcc01, 0x0cc0, 0x0d80, 0xcd41, 0x0f00, 0xcfc1, 0xce81, 0x0e40,
	0x0a00, 0xcac1, 0xcb81, 0x0b40, 0xc901, 0x09c0, 0x0880, 0xc841,
	0xd801, 0x18c0, 0x1980, 0xd941, 0x1b00, 0xdbc1, 0xda81, 0x1a40,
	0x1e00, 0xdec1, 0xdf81, 0x1f40, 0xdd01, 0x1dc0, 0x1c80, 0xdc41,
	0x1400, 0xd4c1, 0xd581, 0x1540, 0xd701, 0x17c0, 0x1680, 0xd641,
	0xd201, 0x12c0, 0x1380, 0xd341, 0x1100, 0xd1c1, 0xd081, 0x1040,
	0xf001, 0x30c0, 0x3180, 0xf141, 0x3300, 0xf3c1, 0xf281, 0x3240,
	0x3600, 0xf6c1, 0xf781, 0x3740, 0xf501, 0x35c0, 0x3480, 0xf441,
	0x3c00, 0xfcc1, 0xfd81, 0x3d40, 0xff01, 0x3fc0, 0x3e80, 0xfe41,
	0xfa01, 0x3ac0, 0x3b80, 0xfb41, 0x3900, 0xf9c1, 0xf881, 0x3840,
	0x2800, 0xe8c1, 0xe981, 0x2940, 0xeb01, 0x2bc0, 0x2a80, 0xea41,
	0xee01, 0x2ec0, 0x2f80, 0xef41, 0x2d00, 0xedc1, 0xec81, 0x2c40,
	0xe401, 0x24c0, 0x2580, 0xe541, 0x2700, 0xe7c1, 0xe681, 0x2640,
	0x2200, 0xe2c1, 0xe381, 0x2340, 0xe101, 0x21c0, 0x2080, 0xe041,
	0xa001, 0x60c0, 0x6180, 0xa141, 0x6300, 0xa3c1, 0xa281, 0x6240,
	0x6600, 0xa6c1, 0xa781, 0x6740, 0xa501, 0x65c0, 0x6480, 0xa441,
	0x6c00, 0xacc1, 0xad81, 0x6d40, 0xaf01, 0x6fc0, 0x6e80, 0xae41,
	0xaa01, 0x6ac0, 0x6b80, 0xab41, 0x6900, 0xa9c1, 0xa881, 0x6840,
	0x7800, 0xb8c1, 0xb981, 0x7940, 0xbb01, 0x7bc0, 0x7a80, 0xba41,
	0xbe01, 0x7ec0, 0x7f80, 0xbf41, 0x7d00, 0xbdc1, 0xbc81, 0x7c40,
	0xb401, 0x74c0, 0x7580, 0xb541, 0x7700, 0xb7c1, 0xb681, 0x7640,
	0x7200, 0xb2c1, 0xb381, 0x7340, 0xb101, 0x71c0, 0x7080, 0xb041,
	0x5000, 0x90c1, 0x9181, 0x5140, 0x9301, 0x53c0, 0x5280, 0x9241,
	0x9601, 0x56c0, 0x5780, 0x9741, 0x5500, 0x95c1, 0x9481, 0x5440,
	0x9c01, 0x5cc0, 0x5d80, 0x9d41, 0x5f00, 0x9fc1, 0x9e81, 0x5e40,
	0x5a00, 0x9ac1, 0x9b81, 0x5b40, 0x9901, 0x59c0, 0x5880, 0x9841,
	0x8801, 0x48c0, 0x4980, 0x8941, 0x4b00, 0x8bc1, 0x8a81, 0x4a40,
	0x4e00, 0x8ec1, 0x8f81, 0x4f40, 0x8d01, 0x4dc0, 0x4c80, 0x8c41,
	0x4400, 0x84c1, 0x8581, 0x4540, 0x8701, 0x47c0, 0x4680, 0x8641,
	0x8201, 0x42c0, 0x4380, 0x8341, 0x4100, 0x81c1, 0x8081, 0x4040
};

LameTag::LameTag(const char *_filename, long _frameOffset, long _frameLength) :
		valid(false), filename(_filename),
		frameOffset(_frameOffset), frameLength(_frameLength) {
	long oldPos, bytesRead, xingOffset, lameOffset;
	bool oldVersion = false;

	if (filename == NULL)
		return;
	IFile file(filename);
	if (!file.isOpen())
		return;

	oldPos = file.tell();
	file.seek(frameOffset);
	frame.resize(frameLength);
	bytesRead = file.read(frame.data(), frameLength);
	file.seek(oldPos);
	if (bytesRead != frameLength)
		return;

	xingOffset = frame.find("Xing");
	if (xingOffset == -1)
		xingOffset = frame.find("Info");
	if (xingOffset == -1)
		return;

	lameOffset = xingOffset + 0x78;
	if (lameOffset + 0x24 >= frameLength)
		return;
	
	ByteVector lameTag = frame.mid(lameOffset, 0x24);
	if (!lameTag.startsWith("LAME"))
		return;
	
	encoder = lameTag.mid(0, 9);
	tagRevision = lameTag[9] >> 4;
	encodingMethod = lameTag[9] & 0x0F;
	quality = frame[xingOffset + 0x77];
	stereoMode = lameTag[24] >> 2 & 0x07;
	sourceRate = lameTag[24] >> 6 & 0x03;
	bitrate = lameTag[20] & 0xFF;
	musicLength = lameTag.mid(28, 4).toUInt();
	lowpassFilter = lameTag[10] & 0xFF;
	mp3Gain = (lameTag[25] & 0x7F) * 1.5;
	if (lameTag[25] & 0x80)
		mp3Gain *= -1;
	athType = lameTag[19] & 0x0F;
	encodingFlags = lameTag[19] & 0xF0;
	encodingDelay = (lameTag[21] << 4 & 0xFF0) | (lameTag[22] >> 4 & 0x0F);
	padding = (lameTag[22] << 4 & 0xF00) | (lameTag[23] & 0xFF);
	noiseShaping = lameTag[24] >> 6;
	unwiseSettings = lameTag[24] & 0x20;
	tagCRC = (unsigned short) lameTag.mid(34, 2).toUInt();
	musicCRC = (unsigned short) lameTag.mid(32, 2).toUInt();
	peakSignal = (float) (lameTag.mid(11, 4).toUInt() << 5);
	if (!(encoder < "LAME3.94b"))
		peakSignal = (peakSignal - 0.5) / 8388608.0;
	if (encoder < "LAME3.95")
		oldVersion = true;
	trackGain = replayGain(lameTag.mid(15, 2), oldVersion);
	albumGain = replayGain(lameTag.mid(17, 2), oldVersion);

	valid = true;
}

void LameTag::print(bool checkCRC) {
	if (!valid)
		return;

	ostringstream tmp;
	tmp.setf(ios::fixed);
	tmp.precision(1);

	cout << encoder << " tag (revision " << tagRevision << "):" << endl;

	printf("%-16s: ", "encoding method");
	switch (encodingMethod) {
		case 8:
			tmp << "2-pass ";
		case 1:
			tmp << "CBR";
			break;
		case 9:
			tmp << "2-pass ";
		case 2:
			tmp << "ABR";
			break;
		case 3:
		case 4:
		case 5:
		case 6:
			if      (encodingMethod == 3) tmp << "old/re ";
			else if (encodingMethod == 4) tmp << "new/mtrh ";
			else if (encodingMethod == 5) tmp << "new/mt ";
			tmp << "VBR";
			break;
		default:
			tmp << "unknown";
			break;
	}
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "quality");
	if (quality > 0 && quality <= 100)
		printf("V%d/q%d", (100 - quality) / 10, (100 - quality) % 10);
	else
		cout << "unknown";
	cout << endl;

	printf("%-16s: ", "stereo mode");
	switch (stereoMode) {
		case 0:
			tmp << "mono";
			break;
		case 1:
			tmp << "stereo";
			break;
		case 2:
			tmp << "dual";
			break;
		case 3:
			tmp << "joint";
			break;
		case 4:
			tmp << "force";
			break;
		case 5:
			tmp << "auto";
			break;
		case 6:
			tmp << "intensity";
			break;
		default:
			tmp << "undefined";
			break;
	}
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "source rate");
	switch (sourceRate) {
		case 0:
			cout << "<= 32";
			break;
		case 1:
			cout << "44.1";
			break;
		case 2:
			cout << "48";
			break;
		default:
			cout << "> 48";
			break;
	}
	cout << " kHz" << endl;

	if (encodingMethod == 2 || encodingMethod == 9)
		tmp << "average ";
	else if (encodingMethod > 2 && encodingMethod < 7)
		tmp << "minimal ";
	tmp << "bitrate";
	printf("%-16s: ", tmp.str().c_str());
	tmp.str("");
	if (bitrate == 0xFF)
		tmp << ">= ";
	tmp << bitrate << " kBit/s";
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "music length");
	cout << FileIO::sizeHumanReadable(musicLength) << endl;

	printf("%-16s: ", "lowpass");
	if (lowpassFilter == 0) {
		tmp << "unknown";
	} else {
		tmp << lowpassFilter << "00 Hz";
	}
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "mp3gain");
	if (mp3Gain != 0.0)
		printf("%+.f dB", mp3Gain);
	else
		cout << "none" << endl;

	printf("%-16s: ", "ATH type");
	tmp << athType;
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "encoding flags");
	bool flag = false;
	if (encodingFlags & 0x10) {
		cout << "nspsytune ";
		flag = true;
	}
	if (encodingFlags & 0x20) {
		cout << "nssafejoint ";
		flag = true;
	}
	if (encodingFlags & 0xC0) {
		cout << "nogap";
		flag = true;
		if (encodingFlags & 0x80)
			cout << "<";
		if (encodingFlags & 0x40)
			cout << ">";
	}
	if (!flag)
		cout << "none";
	cout << endl;

	printf("%-16s: ", "encoding delay");
	tmp << encodingDelay << " samples";
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "padding");
	cout << padding << " samples" << endl;

	printf("%-16s: ", "noise shaping");
	tmp << noiseShaping;
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "unwise settings");
	cout << (unwiseSettings ? "yes" : "no") << endl;

	printf("%-16s: %04X ", "info tag CRC", tagCRC);
	if (checkCRC) {
		int size = frameLength < 190 ? frameLength : 190;
		unsigned short crc = 0;
		crc16Checksum(&crc, frame.data(), size);
		tmp << "(" << (crc != tagCRC ? "invalid" : "correct") << ")";
	}
	printf("%-10s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: %04X", "music CRC", musicCRC);
	if (checkCRC) {
		unsigned short crc = 0;
		size_t size = musicLength - frameLength;
		char *buffer = new char[FILE_BUF_SIZE];
		IFile file(filename);

		file.seek(frameOffset + frameLength);
		while (size > 0 && !file.eof() && !file.error()) {
			size_t blockSize = size < FILE_BUF_SIZE ? size : FILE_BUF_SIZE;
			file.read(buffer, blockSize);
			if (file.error()) {
				warn("%s: Could not read file", filename);
				return;
			}
			size -= blockSize;
			if (size > 0) {
				crc16Block(&crc, buffer, blockSize);
			} else {
				crc16LastBlock(&crc, buffer, blockSize);
			}
		}
		delete [] buffer;
		cout << " (" << (crc != musicCRC ? "invalid" : "correct") << ")";
	}
	cout << endl;

	printf("%-16s: ", "ReplayGain: peak");
	cout << (int) peakSignal * 100 << endl;

	printf("%-16s: ", "track gain");
	tmp << (trackGain > 0.0 ? "+" : "") << trackGain << " dB";
	printf("%-15s", tmp.str().c_str());
	tmp.str("");

	printf("%-15s: ", "album gain");
	cout << (albumGain > 0.0 ? "+" : "") << albumGain << " dB" << endl;
}

double LameTag::replayGain(const ByteVector &gainData, bool oldVersion) {
	double value = (((gainData[0] << 8) & 0x100) | (gainData[1] & 0xFF)) / 10.0;

	if (gainData[0] & 0x02)
		value *= -1;
	if (oldVersion)
		value += 6;

	return value;
}

void LameTag::crc16Block(unsigned short *crc, const char *data, size_t size) {
	for (; size >= 8; size -= 8) {
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
		*crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	}
}

void LameTag::crc16LastBlock(unsigned short *crc, const char *data, size_t size) {
	crc16Block(crc, data, size);
	data += size - (size % 8);
	switch (size % 8) {
	  case 7: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 6: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 5: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 4: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 3: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 2: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 1: *crc = crc16Table[(*crc ^ *data++) & 0xff] ^ (*crc >> 8);
	  case 0: break;
	}
}

void LameTag::crc16Checksum(unsigned short *crc, const char *data, int size) {
	crc16LastBlock(crc, data, size);
}
