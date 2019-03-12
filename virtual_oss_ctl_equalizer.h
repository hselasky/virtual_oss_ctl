/*-
 * Copyright (c) 2019 Hans Petter Selasky. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#ifndef _VOSS_CTL_EQUALIZER_H_
#define	_VOSS_CTL_EQUALIZER_H_

#include "virtual_oss_ctl.h"

#define	EQ_BANDS_MAX	5
#define	EQ_FREQ_MAX	256
#define	EQ_GAIN_MAX	256
#define	EQ_WIDTH_MAX	256

enum {
      EQ_TYPE_LOW_OFF,
      EQ_TYPE_LOW_PASS,
      EQ_TYPE_HIGH_PASS,
      EQ_TYPE_BAND_PASS,
};

class VOSSEQBand : public QWidget
{
	Q_OBJECT;
public:
	VOSSEQBand();
	~VOSSEQBand() {};

	void generate_fir(double *out, int size, int rate);
	void copy(VOSSEQBand *other);

	QGridLayout *gl;

	VOSSVolume *vfreq;
	VOSSVolume *vgain;
	VOSSVolume *vwidth;
	VOSSButtonMap *mtype;

signals:
	void valueChanged();

public slots:
	void handle_update();
};

class VOSSEQFreqResponse : public QWidget
{
public:
	VOSSEQFreqResponse(VOSSEqualizer *_parent);
	~VOSSEQFreqResponse() {}

	VOSSEqualizer *parent;

	void paintEvent(QPaintEvent *);
	double get_amplitude(int band);
};

class VOSSEqualizer : public QWidget
{
	Q_OBJECT;
public:
	VOSSEqualizer(VOSSMainWindow *_parent, int _type, int _num, int _channel);
	~VOSSEqualizer();

	void get_filter();

	int type;
	int num;
	int channel;
	int sample_rate;

	int filter_size;
	double *filter_data;

	QGridLayout *gl;
	VOSSMainWindow *parent;
	VOSSEQFreqResponse *freqres;
	VOSSButtonMap *onoff;
	VOSSEQBand *band[EQ_BANDS_MAX];

public slots:
	void handle_update();
	void handle_copy();
	void handle_paste();
};

#endif		/* _VOSS_CTL_EQUALIZER_H_ */
