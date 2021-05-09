/*-
 * Copyright (c) 2019 Google LLC, written by Richard Kralovic <riso@google.com>
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

#include "virtual_oss_ctl_buttonmap.h"
#include "virtual_oss_ctl_equalizer.h"
#include "virtual_oss_ctl_groupbox.h"
#include "virtual_oss_ctl_mainwindow.h"
#include "virtual_oss_ctl_volume.h"

#include <fftw3.h>
#include <math.h>

static void
hpsjam_skip_space(const char **pp, bool newline)
{
	const char *ptr = *pp;
	while (*ptr == ' ' || *ptr == '\t' || *ptr == '\r' || (*ptr == '\n' && newline))
		ptr++;
	*pp = ptr;
}

static bool
hpsjam_parse_double(const char **pp, double &out)
{
	const char *ptr = *pp;
	bool any = false;

	out = 0;

	hpsjam_skip_space(&ptr, false);

	while (*ptr >= '0' && *ptr <= '9') {
		out *= 10.0;
		out += *ptr - '0';
		any = true;
		ptr++;
	}

	if (*ptr == '.') {
		double k = 1.0 / 10.0;
		ptr ++;
		while (*ptr >= '0' && *ptr <= '9') {
			out += k * (*ptr - '0');
			k /= 10.0;
			any = true;
			ptr++;
		}
	}

	*pp = ptr;
	return (any);
}

struct equalizer {
	double rate;
	size_t block_size;
	bool do_normalize;

	/* (block_size * 2) elements, time domain */
	double *fftw_time;

	/* (block_size * 2) elements, half-complex, freq domain */
	double *fftw_freq;

	fftw_plan forward;
	fftw_plan inverse;

	void init(double _rate, size_t _block_size) {
		rate = _rate;
		block_size = _block_size;

		fftw_time = new double [block_size];
		fftw_freq = new double [block_size];

		forward = fftw_plan_r2r_1d(block_size, fftw_time, fftw_freq, FFTW_R2HC, FFTW_MEASURE);
		inverse = fftw_plan_r2r_1d(block_size, fftw_freq, fftw_time, FFTW_HC2R, FFTW_MEASURE);
	};
	void cleanup() {
		fftw_destroy_plan(forward);
		fftw_destroy_plan(inverse);
		delete [] fftw_time;
		delete [] fftw_freq;
	};
	double get_window(double x) {
		return (0.5 + 0.5 * cos(M_PI * x / (block_size / 2))) / block_size;
	};
	bool load_freq_amps(const char *config) {
		double prev_f = 0.0;
		double prev_amp = 1.0;
		double next_f = 0.0;
		double next_amp = 1.0;
		size_t i;

		if (strncasecmp(config, "normalize", 4) == 0) {
			while (*config != 0) {
				if (*config == '\n') {
					config++;
					break;
				}
				config++;
			}
			do_normalize = true;
		} else {
			do_normalize = false;
		}

		for (i = 0; i <= (block_size / 2); ++i) {
			const double f = (i * rate) / block_size;

			while (f >= next_f) {
				prev_f = next_f;
				prev_amp = next_amp;

				hpsjam_skip_space(&config, true);

				if (*config == 0) {
					next_f = rate;
					next_amp = prev_amp;
				} else {
					if (hpsjam_parse_double(&config, next_f) &&
					    hpsjam_parse_double(&config, next_amp)) {
						if (next_f < prev_f)
							return (true);
					} else {
						return (true);
					}
				}
				if (prev_f == 0.0)
					prev_amp = next_amp;
			}
			fftw_freq[i] = ((f - prev_f) / (next_f - prev_f)) * (next_amp - prev_amp) + prev_amp;
		}
		return (false);
	};
	bool load(const char *config) {
		bool retval;
		size_t i;

		memset(fftw_freq, 0, sizeof(fftw_freq[0]) * block_size);

		retval = load_freq_amps(config);
		if (retval)
			return (retval);

		fftw_execute(inverse);

		/* Multiply by symmetric window and shift */
		for (i = 0; i != (block_size / 2); ++i) {
			double weight = get_window(i);

			fftw_time[block_size / 2 + i] = fftw_time[i] * weight;
		}

		for (i = (block_size / 2); i-- > 1; )
			fftw_time[i] = fftw_time[block_size - i];

		fftw_time[0] = 0;

		fftw_execute(forward);

		for (i = 0; i != block_size; i++)
			fftw_freq[i] /= block_size;

		/* Normalize FIR filter, if any */
		if (do_normalize) {
			double sum = 0;

			for (i = 0; i < block_size; ++i)
				sum += fabs(fftw_time[i]);
			if (sum != 0.0) {
				for (i = 0; i < block_size; ++i)
					fftw_time[i] /= sum;
			}
		}
		return (retval);
	};
};

static int
voss_get_fir_filter(int fd, int type, int num, int channel, double *data, int size)
{
	struct virtual_oss_fir_filter fir = {};
	int error;

	fir.number = num;
	fir.channel = channel;
	fir.filter_size = size;
	fir.filter_data = data;

	switch (type) {
	case VOSS_TYPE_DEVICE | VOSS_TYPE_RX:
		error = ::ioctl(fd, VIRTUAL_OSS_GET_RX_DEV_FIR_FILTER, &fir);
		break;
	case VOSS_TYPE_LOOPBACK | VOSS_TYPE_RX:
		error = ::ioctl(fd, VIRTUAL_OSS_GET_RX_LOOP_FIR_FILTER, &fir);
		break;
	case VOSS_TYPE_DEVICE | VOSS_TYPE_TX:
		error = ::ioctl(fd, VIRTUAL_OSS_GET_TX_DEV_FIR_FILTER, &fir);
		break;
	case VOSS_TYPE_LOOPBACK | VOSS_TYPE_TX:
		error = ::ioctl(fd, VIRTUAL_OSS_GET_TX_LOOP_FIR_FILTER, &fir);
		break;
	default:
		error = -1;
		break;
	}
	if (error != 0 || fir.filter_size < 0 || fir.filter_size > VIRTUAL_OSS_FILTER_MAX)
		return (0);
	else
		return (fir.filter_size);
}

static void
voss_set_fir_filter(int fd, int type, int num, int channel, double *data, int size)
{
	struct virtual_oss_fir_filter fir = {};
	int error;

	fir.number = num;
	fir.channel = channel;
	fir.filter_size = size;
	fir.filter_data = data;

	switch (type) {
	case VOSS_TYPE_DEVICE | VOSS_TYPE_RX:
		error = ::ioctl(fd, VIRTUAL_OSS_SET_RX_DEV_FIR_FILTER, &fir);
		break;
	case VOSS_TYPE_LOOPBACK | VOSS_TYPE_RX:
		error = ::ioctl(fd, VIRTUAL_OSS_SET_RX_LOOP_FIR_FILTER, &fir);
		break;
	case VOSS_TYPE_DEVICE | VOSS_TYPE_TX:
		error = ::ioctl(fd, VIRTUAL_OSS_SET_TX_DEV_FIR_FILTER, &fir);
		break;
	case VOSS_TYPE_LOOPBACK | VOSS_TYPE_TX:
		error = ::ioctl(fd, VIRTUAL_OSS_SET_TX_LOOP_FIR_FILTER, &fir);
		break;
	default:
		error = -1;
		break;
	}
}

VOSSEQButtons :: VOSSEQButtons(VOSSEqualizer *_parent) :
    parent(_parent), gl_control(this)
{
	setTitle(tr("Presets"));

	b_defaults.setText(tr("Defa&ults"));
	b_lowpass.setText(tr("&LowPass"));
	b_highpass.setText(tr("&HighPass"));
	b_bandpass.setText(tr("&BandPass"));

	gl_control.addWidget(&b_defaults, 0,0);
	gl_control.addWidget(&b_lowpass, 0,1);
	gl_control.addWidget(&b_highpass, 1,0);
	gl_control.addWidget(&b_bandpass, 1,1);

	connect(&b_defaults, SIGNAL(released()), this, SLOT(handle_defaults()));
	connect(&b_lowpass, SIGNAL(released()), this, SLOT(handle_lowpass()));
	connect(&b_highpass, SIGNAL(released()), this, SLOT(handle_highpass()));
	connect(&b_bandpass, SIGNAL(released()), this, SLOT(handle_bandpass()));
}

void
VOSSEQButtons :: handle_defaults()
{
	parent->edit->edit.setText(QString(
	    "norm\n"
	    "20 1\n"
	    "25 1\n"
	    "31.5 1\n"
	    "40 1\n"
	    "50 1\n"
	    "63 1\n"
	    "80 1\n"
	    "100 1\n"
	    "125 1\n"
	    "160 1\n"
	    "200 1\n"
	    "250 1\n"
	    "315 1\n"
	    "400 1\n"
	    "500 1\n"
	    "630 1\n"
	    "800 1\n"
	    "1000 1\n"
	    "1250 1\n"
	    "1600 1\n"
	    "2000 1\n"
	    "2500 1\n"
	    "3150 1\n"
	    "4000 1\n"
	    "5000 1\n"
	    "6300 1\n"
	    "8000 1\n"
	    "10000 1\n"
	    "12500 1\n"
	    "16000 1\n"
	    "20000 1\n"
     ));
}

void
VOSSEQButtons :: handle_lowpass()
{
	parent->edit->edit.setText(QString(
	    "norm\n"
	    "150 1\n"
	    "1000 0\n"
	));
}

void
VOSSEQButtons :: handle_highpass()
{
	parent->edit->edit.setText(QString(
	    "norm\n"
	    "150 0\n"
	    "1000 1\n"
	));
}

void
VOSSEQButtons :: handle_bandpass()
{
	parent->edit->edit.setText(QString(
	    "norm\n"
	    "250 0\n"
	    "500 1\n"
	    "1000 1\n"
	    "2000 0\n"
	));
};

VOSSEQFreqResponse :: VOSSEQFreqResponse(VOSSEqualizer *_parent)
{
	parent = _parent;
	setFixedSize(EQ_FREQ_MAX, 128);
}

void
VOSSEQFreqResponse :: get_amplitude(int band, double &freq, double &ret)
{
	if (parent->sample_rate <= 0) {
		freq = ret = 0;
		return;
	}

	double maxfreq = (parent->sample_rate >= 16000.0) ? 8000.0 : (parent->sample_rate / 2.0);

	freq = maxfreq * pow(2.0, band / (double)EQ_FREQ_MAX) - maxfreq;

	if (parent->filter_size <= 0 || parent->filter_data == 0) {
		ret = 0;
		return;
	}

	double phase = 0;
	double a_x = 0;
	double a_y = 0;
	double step = 2.0 * M_PI * freq / (double)parent->sample_rate;

	for (int x = 0; x != parent->filter_size; x++) {
		a_x += cos(phase) * parent->filter_data[x];
		a_y += sin(phase) * parent->filter_data[x];
		phase += step;
	}

	ret = sqrt(a_x * a_x + a_y * a_y);
}

void
VOSSEQFreqResponse :: paintEvent(QPaintEvent *)
{
	int w = width();
	int h = height();

	double amp[EQ_FREQ_MAX];
	double freq[EQ_FREQ_MAX];

  	QPainter paint(this);

	QFont fnt = paint.font();
	fnt.setPixelSize(8);
	paint.setFont(fnt);

	QImage graph(EQ_FREQ_MAX, 128, QImage::Format_ARGB32);

	QColor black(0,0,0);
	QColor grey(192,192,192);
	QColor white(255,255,255);

	graph.fill(white);

	for (int x = 0; x != EQ_FREQ_MAX; x++)
		get_amplitude(x, freq[x], amp[x]);

	int y = 0;
	int z = 0;

	for (int x = 0; x != EQ_FREQ_MAX; x++) {
		if (amp[x] > amp[y])
			y = x;
		if (amp[x] < amp[z])
			z = x;
	}

	for (int x = 0; x != EQ_FREQ_MAX; x++) {
		int a = 127.0 * (amp[x] - amp[z]) / (1.0 + amp[y] - amp[z]);
		if (a < 0)
			a = 0;
		else if (a > 127)
			a = 127;

		a = 127 - a;

		graph.setPixelColor(x,a,black);
		while (++a < 128)
			graph.setPixelColor(x,a,grey);
	}

	paint.drawImage(QRect(0,0,w,h), graph);
	paint.setPen(black);

	for (int x = EQ_FREQ_MAX / 8; x != EQ_FREQ_MAX; x += EQ_FREQ_MAX / 8) {
		if (freq[x] < 2000.0) {
			paint.drawText(QPoint((w * x) / EQ_FREQ_MAX, h - 1),
			       QString("%1").arg(100 * (int)(freq[x] / 100.0)));
		} else {
			paint.drawText(QPoint((w * x) / EQ_FREQ_MAX, h - 1),
			       QString("%1k").arg((int)(freq[x] / 1000.0)));
		}
	}
}

VOSSEqualizer :: VOSSEqualizer(VOSSMainWindow *_parent, int _type, int _num, int _channel) : gl(this)
{
	parent = _parent;
	type = _type;
	num = _num;
	channel = _channel;

	filter_data = 0;
	filter_size = 0;
	sample_rate = 0;

	setWindowTitle(QString("Virtual OSS Equalizer %1 Ch%2")
	    .arg((_type & VOSS_TYPE_RX) ? "RX" : "TX").arg(channel));
	setWindowIcon(QIcon(QString(":/virtual_oss_ctl.png")));

	onoff = new VOSSButtonMap("Equalizer state\0" "OFF\0" "ON\0", 2, 1);
	connect(onoff, SIGNAL(selectionChanged(int)), this, SLOT(handle_update()));
	gl.addWidget(onoff, 0,0,1,1);

	clip = new VOSSEQClipboard(this);
	connect(&clip->b_copy, SIGNAL(released()), this, SLOT(handle_copy()));
	connect(&clip->b_paste, SIGNAL(released()), this, SLOT(handle_paste()));

	gl.addWidget(clip, 0,1,1,1);

	buttons = new VOSSEQButtons(this);
	gl.addWidget(buttons, 0,2,1,1);

	freqres = new VOSSEQFreqResponse(this);
	gl.addWidget(freqres, 0,3,1,1);

	edit = new VOSSEQEditor();
	connect(&edit->b_apply, SIGNAL(released()), this, SLOT(handle_update()));
	gl.addWidget(edit, 1,0,1,4);

	gl.setRowStretch(2,1);
	gl.setColumnStretch(4,1);

	get_filter();
}

VOSSEqualizer :: ~VOSSEqualizer()
{
	if (parent->eq_copy == this)
		parent->eq_copy = 0;
}

void
VOSSEqualizer :: get_filter()
{
	free(filter_data);
	filter_data = 0;
	sample_rate = 0;
	filter_size = 0;

	if (parent->dsp_fd < 0)
		return;

	::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_SAMPLE_RATE, &sample_rate);

	filter_size = voss_get_fir_filter(parent->dsp_fd, type, num, channel, 0, 0);

	if (filter_size != 0) {
		filter_data = (double *)malloc(sizeof(double) * filter_size);

		if (voss_get_fir_filter(parent->dsp_fd, type, num, channel,
		    filter_data, filter_size) != filter_size) {
			free(filter_data);
			filter_data = 0;
			VOSS_BLOCKED(onoff,setSelection(0));
		} else {
			VOSS_BLOCKED(onoff,setSelection(1));
		}
	} else {
		VOSS_BLOCKED(onoff,setSelection(0));
	}
}

void
VOSSEqualizer :: handle_copy()
{
	parent->eq_copy = this;
}

void
VOSSEqualizer :: handle_paste()
{
	if (parent->eq_copy == 0 || parent->eq_copy == this)
		return;

	edit->edit.setText(parent->eq_copy->edit->edit.toPlainText());
	onoff->setSelection(parent->eq_copy->onoff->currSelection);
}

void
VOSSEqualizer :: handle_update()
{
	if (filter_size <= 0 || sample_rate <= 0 || parent->dsp_fd < 0)
		return;

	if (onoff->currSelection == 0) {
		free(filter_data);
		filter_data = 0;
	} else {
		bool filter_alloc = (filter_data == 0);

		if (filter_alloc) {
			filter_data = (double *)malloc(sizeof(double) * filter_size);
			memset(filter_data, 0, sizeof(filter_data[0]) * filter_size);
		}

		QString str = edit->edit.toPlainText().trimmed();

		if (!str.isEmpty()) {
			QByteArray ba = str.toUtf8();
			equalizer eq = {};
			eq.init(sample_rate, filter_size);

			if (eq.load(ba.data())) {
				eq.cleanup();
				if (filter_alloc) {
					free(filter_data);
					filter_data = 0;
				}
				QMessageBox::information(this, "Virtual OSS Control",
				    tr("Invalid EQ filter specification"));
				return;
			} else {
				for (int x = 0; x != filter_size; x++)
					filter_data[x] = eq.fftw_time[x];
			}
			eq.cleanup();
		} else {
			if (filter_alloc) {
				free(filter_data);
				filter_data = 0;
			}
			QMessageBox::information(this, "Virtual OSS Control",
				    tr("EQ filter specification is empty"));
			return;
		}
	}

	voss_set_fir_filter(parent->dsp_fd, type, num, channel, filter_data, filter_size);
	freqres->update();
}
