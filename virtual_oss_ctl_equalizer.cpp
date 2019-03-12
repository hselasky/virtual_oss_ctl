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

#include "virtual_oss_ctl_buttonmap.h"
#include "virtual_oss_ctl_equalizer.h"
#include "virtual_oss_ctl_groupbox.h"
#include "virtual_oss_ctl_mainwindow.h"
#include "virtual_oss_ctl_volume.h"

#include <math.h>

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

VOSSEQBand :: VOSSEQBand()
{
	gl = new QGridLayout(this);

	vfreq = new VOSSVolume();
	vfreq->setRange(1, EQ_FREQ_MAX, 1);
	connect(vfreq, SIGNAL(valueChanged(int)), this, SLOT(handle_update()));

	vgain = new VOSSVolume();
	vgain->setRange(-EQ_GAIN_MAX, EQ_GAIN_MAX, 0);
	vgain->setValue(0);
	connect(vgain, SIGNAL(valueChanged(int)), this, SLOT(handle_update()));

	vwidth = new VOSSVolume();
	vwidth->setRange(0, EQ_WIDTH_MAX, 0);
	vwidth->setValue(0);
	connect(vwidth, SIGNAL(valueChanged(int)), this, SLOT(handle_update()));

	mtype = new VOSSButtonMap("Type\0" "OFF\0" "LP\0" "HP\0" "BP\0", 4, 2);
	connect(mtype, SIGNAL(selectionChanged(int)), this, SLOT(handle_update()));

	gl->addWidget(new VOSSGroupBoxWrapper("Freq", vfreq), 0,0,1,1);
	gl->addWidget(new VOSSGroupBoxWrapper("Gain", vgain), 1,0,1,1);
	gl->addWidget(new VOSSGroupBoxWrapper("Width", vwidth), 2,0,1,1);
	gl->addWidget(mtype, 3,0,1,1);
}

void
VOSSEQBand :: copy(VOSSEQBand *other)
{
	vfreq->setValue(other->vfreq->value());
	vgain->setValue(other->vgain->value());
	vwidth->setValue(other->vwidth->value());
	mtype->setSelection(other->mtype->currSelection);
}

static void
voss_ctl_basic_filter(double freq, double gain, double *factor,
    unsigned window_size, unsigned sample_rate)
{
	int wh = window_size / 2;
	int x;

	freq /= (double)sample_rate;
	freq *= (double)wh;

	factor[wh] += (2.0 * freq * gain) / (double)wh;

	freq *= (2.0 * M_PI) / (double)wh;

	for (x = -wh+1; x < wh; x++) {
		if (x == 0)
			continue;
		factor[x + wh] += gain * sin(freq * (double)(x)) / (M_PI * (double)(x));
	}
}

static void
voss_ctl_range_check(double &value, const double low, const double high)
{
	if (value < low)
		value = low;
	else if (value > high)
		value = high;
}

static void
voss_ctl_band_pass(double freq_low, double freq_high, double gain,
    double *factor, unsigned window_size, unsigned sample_rate)
{
	const double high = sample_rate / 2.0;

	voss_ctl_range_check(freq_low, 0, high);
	voss_ctl_range_check(freq_high, 0, high);

	/* lowpass */
	voss_ctl_basic_filter(freq_low, gain, factor, window_size, sample_rate);

	/* highpass */
	voss_ctl_basic_filter(-freq_high, gain, factor, window_size, sample_rate);
}

static void
voss_ctl_low_pass(double freq_low, double gain, double *factor,
    unsigned window_size, unsigned sample_rate)
{
  	const double high = sample_rate / 2.0;

	voss_ctl_range_check(freq_low, 0, high);

	/* lowpass */
	voss_ctl_basic_filter(freq_low, gain, factor, window_size, sample_rate);
	voss_ctl_basic_filter(-sample_rate, gain, factor, window_size, sample_rate);
}

static void
voss_ctl_high_pass(double freq_high, double gain, double *factor,
    unsigned window_size, unsigned sample_rate)
{
	const double high = sample_rate / 2.0;

	voss_ctl_range_check(freq_high, 0, high);

	/* highpass */
	voss_ctl_basic_filter(freq_high, gain, factor, window_size, sample_rate);
	voss_ctl_basic_filter(-high, gain, factor, window_size, sample_rate);
}

void
VOSSEQBand :: generate_fir(double *out, int size, int sample_rate)
{
	const double rate = sample_rate / 2.0;
	const double freq = rate * pow(2.0, vfreq->value() / (double)EQ_FREQ_MAX) - rate;
	const double width = (rate * pow(2.0, vwidth->value() / (double)EQ_FREQ_MAX) - rate) / 2.0;
	const double gain = pow(2.0, (16.0 * vgain->value()) / (double)EQ_GAIN_MAX);
	
	switch (mtype->currSelection) {
	case EQ_TYPE_LOW_PASS:
		voss_ctl_low_pass(freq, gain, out, size, sample_rate);
		break;
	case EQ_TYPE_HIGH_PASS:
		voss_ctl_high_pass(freq, gain, out, size, sample_rate);
		break;
	case EQ_TYPE_BAND_PASS:
		voss_ctl_band_pass(freq - width, freq + width, gain, out, size, sample_rate);
		break;
	default:
		break;
	}
}

void
VOSSEQBand :: handle_update()
{
	emit valueChanged();
}

VOSSEQFreqResponse :: VOSSEQFreqResponse(VOSSEqualizer *_parent)
{
	parent = _parent;
}

double
VOSSEQFreqResponse :: get_amplitude(int band)
{
	if (parent->filter_size <= 0 || parent->sample_rate <= 0 ||
	    parent->filter_data == 0)
		return (1);

	double a_x = 0;
	double a_y = 0;
	double phase = 0;
	double rate = parent->sample_rate / 2.0;
	double freq = rate * pow(2.0, band / (double)EQ_FREQ_MAX) - rate;
	double step = M_PI * freq / rate;

	for (int x = 0; x != parent->filter_size; x++) {
		a_x += cos(phase) * parent->filter_data[x];
		a_y += sin(phase) * parent->filter_data[x];
		phase += step;
	}

	return log(sqrt(a_x * a_x + a_y * a_y));
	
}

void
VOSSEQFreqResponse :: paintEvent(QPaintEvent *)
{
	int w = width();
	int h = height();

	double amp[EQ_FREQ_MAX];

  	QPainter paint(this);

	QImage graph(EQ_FREQ_MAX, 256, QImage::Format_ARGB32);

	QColor black(0,0,0);
	QColor grey(192,192,192);
	QColor white(255,255,255);

	graph.fill(white);

	for (int x = 0; x != EQ_FREQ_MAX; x++) {
		amp[x] = get_amplitude(x);
	}

	int y = 0;
	int z = 0;

	for (int x = 0; x != EQ_FREQ_MAX; x++) {
		if (amp[x] > amp[y])
			y = x;
		if (amp[x] < amp[z])
			z = x;
	}

	if (amp[y] != 0.0) {
		for (int x = 0; x != EQ_FREQ_MAX; x++) {
			int a = 255.0 * (amp[x] - amp[z]) / (amp[y] - amp[z]);
			if (a < 0)
				a = 0;
			else if (a > 255)
				a = 255;

			a = 255 - a;

			graph.setPixelColor(x,a,black);
			while (++a < 256)
				graph.setPixelColor(x,a,grey);
		}
	}

	paint.drawImage(QRect(0,0,w,h), graph);
}

VOSSEqualizer :: VOSSEqualizer(VOSSMainWindow *_parent, int _type, int _num, int _channel)
{
  	VOSSGroupBox *gb;
	QPushButton *pb;

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

	gl = new QGridLayout(this);

	freqres = new VOSSEQFreqResponse(this);
	gl->addWidget(freqres, 0,2,1,EQ_BANDS_MAX - 2);

	onoff = new VOSSButtonMap("Equalizer state\0" "OFF\0" "ON\0", 2, 1);
	connect(onoff, SIGNAL(selectionChanged(int)), this, SLOT(handle_update()));
	gl->addWidget(onoff, 0,0,1,1);

	for (int x = 0; x != EQ_BANDS_MAX; x++) {
		band[x] = new VOSSEQBand();
		connect(band[x], SIGNAL(valueChanged()), this, SLOT(handle_update()));
		gl->addWidget(new VOSSGroupBoxWrapper(QString("F%1").arg(x + 1), band[x]), 1,x,1,1);
	}

	gb = new VOSSGroupBox("Control");
	pb = new QPushButton(tr("COPY"));
	connect(pb, SIGNAL(released()), this, SLOT(handle_copy()));
	gb->addWidget(pb, 0,0,1,1);

	pb = new QPushButton(tr("PASTE"));
	connect(pb, SIGNAL(released()), this, SLOT(handle_paste()));
	gb->addWidget(pb, 1,0,1,1);

	gl->addWidget(gb,0,1,1,1);

	gl->setRowStretch(0,1);

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
			onoff->setSelection(0);
		} else {
			onoff->setSelection(1);
		}
	} else {
		onoff->setSelection(0);
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

	for (int x = 0; x != EQ_BANDS_MAX; x++)
		band[x]->copy(parent->eq_copy->band[x]);

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
		if (filter_data == 0)
			filter_data = (double *)malloc(sizeof(double) * filter_size);
		memset(filter_data, 0, sizeof(double) * filter_size);

		for (int x = 0; x != EQ_BANDS_MAX; x++)
			band[x]->generate_fir(filter_data, filter_size, sample_rate);

		double sum = 0;
		for (int x = 0; x != filter_size; x++)
			sum += fabs(filter_data[x]);

		if (sum != 0) {
			for (int x = 0; x != filter_size; x++)
				filter_data[x] /= sum;
		} else {
			filter_data[filter_size / 2] = 1.0;
		}
	}
	voss_set_fir_filter(parent->dsp_fd, type, num, channel, filter_data, filter_size);

	freqres->update();
}
