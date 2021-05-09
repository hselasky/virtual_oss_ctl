/*-
 * Copyright (c) 2019-2021 Hans Petter Selasky. All rights reserved.
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

#define	EQ_FREQ_MAX 256

class VOSSEQButtons : public QGroupBox
{
	Q_OBJECT;
public:
	VOSSEQButtons(VOSSEqualizer *);

	VOSSEqualizer *parent;

	QGridLayout gl_control;

	QPushButton b_defaults;
	QPushButton b_lowpass;
	QPushButton b_highpass;
	QPushButton b_bandpass;

public slots:
	void handle_defaults();
	void handle_lowpass();
	void handle_highpass();
	void handle_bandpass();
};

class VOSSEQFreqResponse : public QWidget
{
public:
	VOSSEQFreqResponse(VOSSEqualizer *_parent);

	VOSSEqualizer *parent;

	void paintEvent(QPaintEvent *);
	void get_amplitude(int band, double &, double &);
};

class VOSSEQEditor : public QGroupBox
{
public:
	VOSSEQEditor() : gl_spec(this) {
		edit.setTabChangesFocus(true);
		setTitle(tr("Filter specification"));
		b_apply.setText(tr("Apply filter"));
		gl_spec.addWidget(&edit, 0,0,1,2);
		gl_spec.addWidget(&b_apply, 1,1,1,1);
	};
	QGridLayout gl_spec;
	QTextEdit edit;
	QPushButton b_apply;
};

class VOSSEQClipboard : public QGroupBox
{
public:
	VOSSEQClipboard(VOSSEqualizer *_parent) :
	  parent(_parent), gl_clip(this) {

		b_copy.setText(tr("COPY"));
		b_paste.setText(tr("PASTE"));

		setTitle("Clipboard");
		gl_clip.addWidget(&b_copy, 0,0);
		gl_clip.addWidget(&b_paste, 1,0);
	};
	VOSSEqualizer *parent;

	QGridLayout gl_clip;
	QPushButton b_copy;
	QPushButton b_paste;
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

	QGridLayout gl;
	VOSSMainWindow *parent;
	VOSSEQFreqResponse *freqres;
	VOSSButtonMap *onoff;
	VOSSEQButtons *buttons;
	VOSSEQEditor *edit;
	VOSSEQClipboard *clip;

public slots:
	void handle_update();
	void handle_copy();
	void handle_paste();
};

#endif		/* _VOSS_CTL_EQUALIZER_H_ */
