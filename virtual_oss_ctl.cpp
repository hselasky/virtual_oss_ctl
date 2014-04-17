/*-
 * Copyright (c) 2012-2013 Hans Petter Selasky. All rights reserved.
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

#include "virtual_oss_ctl.h"
#include "virtual_oss_ctl_connect.h"

#define	VBAR_HEIGHT 32
#define	VBAR_WIDTH 128

VOssGridLayout :: VOssGridLayout() : QGridLayout(this)
{
}

VOssGridLayout :: ~VOssGridLayout()
{
}

VOssVolumeBar :: VOssVolumeBar(VOssController *_parent, int _type, int _channel, int _number)
  : QWidget(_parent)
{
	parent = _parent;
	type = _type;
	channel = _channel;
	number = _number;

	memset(&io_peak, 0, sizeof(io_peak));
	io_peak.number = _number;
	io_peak.channel = _channel;

	memset(&mon_peak, 0, sizeof(mon_peak));
	mon_peak.number = _number;

	memset(&master_peak, 0, sizeof(master_peak));
	master_peak.channel = _channel;

	generation = 0;

	if (type == VOSS_TYPE_DEVICE) {
		/* duplex */
		setMinimumSize(VBAR_WIDTH, VBAR_HEIGHT);
		setMaximumSize(VBAR_WIDTH, VBAR_HEIGHT);
	} else {
		/* simplex */
		setMinimumSize(VBAR_WIDTH, VBAR_HEIGHT / 2);
		setMaximumSize(VBAR_WIDTH, VBAR_HEIGHT / 2);
	}
}

VOssVolumeBar :: ~VOssVolumeBar()
{

}

VOssAudioDelayLocator :: VOssAudioDelayLocator(VOssMainWindow *_parent)
  : QGroupBox(_parent)
{
	struct virtual_oss_audio_delay_locator ad;
	int fd = _parent->dsp_fd;
	int error;

	parent = _parent;

	gl = new QGridLayout(this);

	setTitle(tr("Audio Delay Locator"));

	lbl_status = new QLabel();
	but_reset = new QPushButton(tr("Reset"));
	but_enable_disable = new QPushButton(tr("ON/OFF"));
	but_signal_up = new QPushButton(tr("VOL+"));
	but_signal_down = new QPushButton(tr("VOL-"));

	spn_channel_in = new QSpinBox();
	spn_channel_in->setPrefix(QString("InCh "));

	spn_channel_out = new QSpinBox();
	spn_channel_out->setPrefix(QString("OutCh "));

	error = ::ioctl(fd, VIRTUAL_OSS_GET_AUDIO_DELAY_LOCATOR, &ad);
	if (error == 0) {
		spn_channel_in->setRange(0, ad.channel_last);
		spn_channel_out->setRange(0, ad.channel_last);
	}

	gl->addWidget(but_enable_disable,0,0,1,1);
	gl->addWidget(but_reset,0,1,1,1);
	gl->addWidget(but_signal_up,0,2,1,1);
	gl->addWidget(but_signal_down,0,3,1,1);
	gl->addWidget(spn_channel_in,0,4,1,1);
	gl->addWidget(spn_channel_out,0,5,1,1);
	gl->addWidget(lbl_status,1,0,1,6);

	read_state();

	connect(but_reset, SIGNAL(released()), this, SLOT(handle_reset()));
	connect(but_enable_disable, SIGNAL(released()), this, SLOT(handle_enable_disable()));
	connect(but_signal_up, SIGNAL(released()), this, SLOT(handle_signal_up()));
	connect(but_signal_down, SIGNAL(released()), this, SLOT(handle_signal_down()));
	connect(spn_channel_in, SIGNAL(valueChanged(int)), this, SLOT(handle_channel_in()));
	connect(spn_channel_out, SIGNAL(valueChanged(int)), this, SLOT(handle_channel_out()));
}

VOssAudioDelayLocator :: ~VOssAudioDelayLocator()
{

}

void
VOssAudioDelayLocator :: read_state()
{
	struct virtual_oss_audio_delay_locator ad;
	int fd = parent->dsp_fd;
	int error;
	char status[128];

	error = ::ioctl(fd, VIRTUAL_OSS_GET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;

	spn_channel_in->setValue(ad.channel_input);
	spn_channel_out->setValue(ad.channel_output);

	snprintf(status, sizeof(status),
	    "Delay locator is %s. Output volume level is %d. Measured audio delay is %d samples.",
	    ad.locator_enabled ? "enabled" : "disabled",
	    (int)ad.signal_output_level,
	    (int)ad.signal_input_delay);

	lbl_status->setText(QString(status));
}

void
VOssAudioDelayLocator :: handle_reset()
{
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_RST_AUDIO_DELAY_LOCATOR);
	if (error)
		return;
}

void
VOssAudioDelayLocator :: handle_signal_up()
{
	struct virtual_oss_audio_delay_locator ad;
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_GET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;

	ad.signal_output_level++;

	error = ::ioctl(fd, VIRTUAL_OSS_SET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;
}

void
VOssAudioDelayLocator :: handle_signal_down()
{
	struct virtual_oss_audio_delay_locator ad;
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_GET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;

	ad.signal_output_level--;

	error = ::ioctl(fd, VIRTUAL_OSS_SET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;
}

void
VOssAudioDelayLocator :: handle_channel_in()
{
	struct virtual_oss_audio_delay_locator ad;
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_GET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;

	ad.channel_input = spn_channel_in->value();

	error = ::ioctl(fd, VIRTUAL_OSS_SET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;
}

void
VOssAudioDelayLocator :: handle_channel_out()
{
	struct virtual_oss_audio_delay_locator ad;
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_GET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;

	ad.channel_output = spn_channel_out->value();

	error = ::ioctl(fd, VIRTUAL_OSS_SET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;
}

void
VOssAudioDelayLocator :: handle_enable_disable()
{
	struct virtual_oss_audio_delay_locator ad;
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_GET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;

	ad.locator_enabled = ad.locator_enabled ? 0 : 1;

	error = ::ioctl(fd, VIRTUAL_OSS_SET_AUDIO_DELAY_LOCATOR, &ad);
	if (error)
		return;
}

VOssRecordStatus :: VOssRecordStatus(VOssMainWindow *_parent)
  : QGroupBox(_parent)
{
	parent = _parent;

	gl = new QGridLayout(this);

	setTitle(tr("Recording status"));

	but_start = new QPushButton(tr("START"));
	but_stop = new QPushButton(tr("STOP"));

	gl->addWidget(but_start,0,0,1,1);
	gl->addWidget(but_stop,0,1,1,1);

	read_state();

	connect(but_start, SIGNAL(released()), this, SLOT(handle_start()));
	connect(but_stop, SIGNAL(released()), this, SLOT(handle_stop()));
}

VOssRecordStatus :: ~VOssRecordStatus()
{

}

void
VOssRecordStatus :: read_state()
{
	int fd = parent->dsp_fd;
	int value;
	int error;
	
	error = ::ioctl(fd, VIRTUAL_OSS_GET_RECORDING, &value);
	if (error)
		return;

	if (value) {
		but_start->setEnabled(0);
		but_stop->setEnabled(1);
	} else {
		but_start->setEnabled(1);
		but_stop->setEnabled(0);
	}
}

void
VOssRecordStatus :: handle_start()
{
	int fd = parent->dsp_fd;
	int value;
	int error;

	value = 1;
	error = ::ioctl(fd, VIRTUAL_OSS_SET_RECORDING, &value);
	if (error)
		return;

	read_state();
}

void
VOssRecordStatus :: handle_stop()
{
	int fd = parent->dsp_fd;
	int value;
	int error;

	value = 0;
	error = ::ioctl(fd, VIRTUAL_OSS_SET_RECORDING, &value);
	if (error)
		return;

	read_state();
}

static int
convertPeak(long long x, uint8_t bits)
{
	if (bits <= 32)
		x <<= 32 - bits;
	else
		x >>= bits - 32; 

	x = ((x * (long long)VBAR_WIDTH) >> 31);

	if (x < 0 || x >= VBAR_WIDTH)
		x = VBAR_WIDTH;

	return (x);
}

void
VOssVolumeBar :: drawBar(QPainter &paint, int y, int h, int level)
{
	const int d = (VBAR_WIDTH / 8);
	int x;

	for (x = 0; level > 0; level -= d, x += d) {
		QColor bar(96 + x, 192 - x, 96);
		paint.fillRect(x, y, (level >= d) ? d : level, h, bar);
	}
}

void
VOssVolumeBar :: paintEvent(QPaintEvent *event)
{
	int fd = parent->parent->dsp_fd;
	int doit;
	int error;
	int w;
	int x;

	QPainter paint(this);

	QColor black(0,0,0);
	QColor split(192,192,0);

	doit = (generation != parent->parent->generation);
	generation = parent->parent->generation;

	switch (type) {
	case VOSS_TYPE_DEVICE:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT,black);

		if (doit) {
			error = ::ioctl(fd, VIRTUAL_OSS_GET_DEV_PEAK, &io_peak);
			if (error)
				break;
		}
		w = convertPeak(io_peak.rx_peak_value, io_peak.bits);
		drawBar(paint, 0, VBAR_HEIGHT / 2, w);

		w = convertPeak(io_peak.tx_peak_value, io_peak.bits);
		drawBar(paint, VBAR_HEIGHT / 2, VBAR_HEIGHT / 2, w);

		for (x = 1; x != 8; x++) {
			QColor white(192,192,192 - x * 16);
			w = (x * VBAR_WIDTH) / 8;
			paint.fillRect(w,0,1,VBAR_HEIGHT,white);
		}
		paint.fillRect(0,VBAR_HEIGHT/2,VBAR_WIDTH,1,split);
		break;
	case VOSS_TYPE_LOOPBACK:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT / 2,black);

		if (doit) {
			error = ::ioctl(fd, VIRTUAL_OSS_GET_LOOP_PEAK, &io_peak);
			if (error)
				break;
		}
		w = convertPeak(io_peak.rx_peak_value, io_peak.bits);
		drawBar(paint, 0, VBAR_HEIGHT / 2, w);

		for (x = 1; x != 8; x++) {
			QColor white(192,192,192 - x * 16);
			w = (x * VBAR_WIDTH) / 8;
			paint.fillRect(w,0,1,VBAR_HEIGHT / 2,white);
		}
		break;
	case VOSS_TYPE_INPUT_MON:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT / 2,black);

		if (doit) {
			error = ::ioctl(fd, VIRTUAL_OSS_GET_INPUT_MON_PEAK, &mon_peak);
			if (error)
				break;
		}
		w = convertPeak(mon_peak.peak_value, mon_peak.bits);
		drawBar(paint, 0, VBAR_HEIGHT / 2, w);

		for (x = 1; x != 8; x++) {
			QColor white(192,192,192 - x * 16);
			w = (x * VBAR_WIDTH) / 8;
			paint.fillRect(w,0,1,VBAR_HEIGHT / 2,white);
		}
		break;
	case VOSS_TYPE_OUTPUT_MON:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT / 2,black);

		if (doit) {
			error = ::ioctl(fd, VIRTUAL_OSS_GET_OUTPUT_MON_PEAK, &mon_peak);
			if (error)
				break;
		}
		w = convertPeak(mon_peak.peak_value, mon_peak.bits);
		drawBar(paint, 0, VBAR_HEIGHT / 2, w);

		for (x = 1; x != 8; x++) {
			QColor white(192,192,192 - x * 16);
			w = (x * VBAR_WIDTH) / 8;
			paint.fillRect(w,0,1,VBAR_HEIGHT / 2,white);
		}
		break;
	case VOSS_TYPE_MASTER_OUTPUT:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT / 2,black);

		if (doit) {
			error = ::ioctl(fd, VIRTUAL_OSS_GET_OUTPUT_PEAK, &master_peak);
			if (error)
				break;
		}
		w = convertPeak(master_peak.peak_value, master_peak.bits);
		drawBar(paint, 0, VBAR_HEIGHT / 2, w);

		for (x = 1; x != 8; x++) {
			QColor white(192,192,192 - x * 16);
			w = (x * VBAR_WIDTH) / 8;
			paint.fillRect(w,0,1,VBAR_HEIGHT / 2,white);
		}
		break;
	case VOSS_TYPE_MASTER_INPUT:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT / 2,black);

		if (doit) {
			error = ::ioctl(fd, VIRTUAL_OSS_GET_INPUT_PEAK, &master_peak);
			if (error)
				break;
		}
		w = convertPeak(master_peak.peak_value, master_peak.bits);
		drawBar(paint, 0, VBAR_HEIGHT / 2, w);

		for (x = 1; x != 8; x++) {
			QColor white(192,192,192 - x * 16);
			w = (x * VBAR_WIDTH) / 8;
			paint.fillRect(w,0,1,VBAR_HEIGHT / 2,white);
		}
		break;
	default:
		break;
	}
}

VOssController :: VOssController(VOssMainWindow *_parent, int _type, int _channel, int _number)
  : QGroupBox(_parent), connect_input_label(0), connect_output_label(0), connect_row(0)
{
	int x;

	gl = new QGridLayout(this);

	parent = _parent;
	type = _type;
	channel = _channel;
	number = _number;

	memset(&io_info, 0, sizeof(io_info));
	io_info.number = _number;
	io_info.channel = _channel;

	memset(&mon_info, 0, sizeof(mon_info));
	mon_info.number = _number;

	rx_amp = 0;
	tx_amp = 0;

	peak_vol = new VOssVolumeBar(this, _type, _channel, _number);

	rx_mute = new QCheckBox();

	tx_mute = new QCheckBox();

	rx_polarity = new QCheckBox();

	tx_polarity = new QCheckBox();

	lbl_rx_amp = new QLabel();
	lbl_tx_amp = new QLabel();

	rx_amp_up = new QPushButton(QString("+"));

	rx_amp_down = new QPushButton(QString("-"));

	tx_amp_up = new QPushButton(QString("+"));

	tx_amp_down = new QPushButton(QString("-"));

	spn_group = new QSpinBox();
	spn_group->setRange(0, 63);
	spn_limit = new QSpinBox();
	spn_limit->setRange(0, 63);
	spn_rx_chn = new QSpinBox();
	spn_rx_chn->setRange(0, 63);
	spn_rx_chn->setPrefix(tr("SrcCh "));
	spn_tx_chn = new QSpinBox();
	spn_tx_chn->setRange(0, 63);
	spn_tx_chn->setPrefix(tr("DstCh "));
	spn_rx_dly = new QSpinBox();

	get_config();

	spn_rx_dly->setRange(0, io_info.rx_delay_limit);

	connect(rx_mute, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(tx_mute, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(rx_polarity, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(rx_polarity, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(rx_amp_up, SIGNAL(released()), this, SLOT(handle_rx_amp_up()));
	connect(rx_amp_down, SIGNAL(released()), this, SLOT(handle_rx_amp_down()));
	connect(tx_amp_up, SIGNAL(released()), this, SLOT(handle_tx_amp_up()));
	connect(tx_amp_down, SIGNAL(released()), this, SLOT(handle_tx_amp_down()));
	connect(spn_group, SIGNAL(valueChanged(int)), this, SLOT(handle_spn_grp(int)));
	connect(spn_limit, SIGNAL(valueChanged(int)), this, SLOT(handle_spn_lim(int)));
	connect(spn_rx_chn, SIGNAL(valueChanged(int)), this, SLOT(handle_set_config()));
	connect(spn_tx_chn, SIGNAL(valueChanged(int)), this, SLOT(handle_set_config()));
	connect(spn_rx_dly, SIGNAL(valueChanged(int)), this, SLOT(handle_set_config()));

	switch (type) {
	case VOSS_TYPE_DEVICE:
		x = 0;
		gl->addWidget(new QLabel(QString("RX")), 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(new QLabel(QString("TX")), 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(peak_vol, 0, x, 2, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("MUTE:")), 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(new QLabel(QString("MUTE:")), 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_mute, 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(tx_mute, 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("POL:")), 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(new QLabel(QString("POL:")), 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_polarity, 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(tx_polarity, 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_rx_chn, 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(spn_tx_chn, 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_amp_up, 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(tx_amp_up, 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(lbl_rx_amp, 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(lbl_tx_amp, 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_amp_down, 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(tx_amp_down, 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("IN-LIM:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_limit, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("DELAY:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_rx_dly, 0, x, 1, 1, Qt::AlignCenter);
		break;
	case VOSS_TYPE_LOOPBACK:
		x = 0;
		gl->addWidget(new QLabel(QString("RX")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(peak_vol, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("MUTE:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_mute, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("POL:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_polarity, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_rx_chn, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_amp_up, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(lbl_rx_amp, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_amp_down, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("IN-LIM:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_limit, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("DELAY:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_rx_dly, 0, x, 1, 1, Qt::AlignCenter);
		break;
	case VOSS_TYPE_INPUT_MON:
	case VOSS_TYPE_OUTPUT_MON:
		x = 0;
		gl->addWidget(peak_vol, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("MUTE:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_mute, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("POL:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_polarity, 0, x, 1, 1, Qt::AlignCenter);

		x++;
		gl->addWidget(spn_rx_chn, 0, x, 1, 1, Qt::AlignCenter);

		x++;
		gl->addWidget(spn_tx_chn, 0, x, 1, 1, Qt::AlignCenter);

		x++;
		gl->addWidget(rx_amp_up, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(lbl_rx_amp, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_amp_down, 0, x, 1, 1, Qt::AlignCenter);
		break;

	case VOSS_TYPE_MASTER_OUTPUT:
		x = 0;
		gl->addWidget(peak_vol, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("GROUP:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_group, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("OUT-LIM:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_limit, 0, x, 1, 1, Qt::AlignCenter);
		break;

	case VOSS_TYPE_MASTER_INPUT:
		x = 0;
		gl->addWidget(peak_vol, 0, x, 1, 1, Qt::AlignLeft);
		break;

	default:
		break;
	}
}

VOssController :: ~VOssController()
{

}

void
VOssController :: set_desc(const char *desc)
{
	char buf[64];

	if (desc != NULL && desc[0]) {
		switch (type) {
		case VOSS_TYPE_DEVICE:
		case VOSS_TYPE_LOOPBACK:
		case VOSS_TYPE_MASTER_OUTPUT:
		case VOSS_TYPE_MASTER_INPUT:
			snprintf(buf, sizeof(buf),
			    "%s - Ch%d", desc, channel);
			break;
		case VOSS_TYPE_INPUT_MON:
		case VOSS_TYPE_OUTPUT_MON:
			snprintf(buf, sizeof(buf),
			    "%s - Ch%d", desc, number);
			break;
		default:
			snprintf(buf, sizeof(buf),
			    "Channel %d.%d", number, channel);
			break;
		}
	} else {
		snprintf(buf, sizeof(buf),
		    "Channel %d.%d", number, channel);
	}

	setTitle(QString(buf));
}

void
VOssController :: set_rx_amp(int value)
{
	char buf[16];

	if (value < -31)
		value = -31;
	else if (value > 31)
		value = 31;

	rx_amp = value;

	snprintf(buf, sizeof(buf), "%d", value);

	lbl_rx_amp->setText(QString(buf));
}

void
VOssController :: set_tx_amp(int value)
{
	char buf[16];

	if (value < -31)
		value = -31;
	else if (value > 31)
		value = 31;

	tx_amp = value;

	snprintf(buf, sizeof(buf), "%d", value);

	lbl_tx_amp->setText(QString(buf));
}

void
VOssController :: handle_set_config(void)
{
	int error;

	switch (type) {
	case VOSS_TYPE_DEVICE:
		io_info.rx_mute = (rx_mute->checkState() == Qt::Checked);
		io_info.tx_mute = (tx_mute->checkState() == Qt::Checked);
		io_info.rx_pol = (rx_polarity->checkState() == Qt::Checked);
		io_info.tx_pol = (tx_polarity->checkState() == Qt::Checked);
		io_info.rx_amp = rx_amp;
		io_info.tx_amp = tx_amp;
		io_info.rx_chan = spn_rx_chn->value();
		io_info.tx_chan = spn_tx_chn->value();
		io_info.rx_delay = spn_rx_dly->value();
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_DEV_INFO, &io_info);
		break;
	case VOSS_TYPE_LOOPBACK:
		io_info.rx_mute = (rx_mute->checkState() == Qt::Checked);
		io_info.rx_pol = (rx_polarity->checkState() == Qt::Checked);
		io_info.rx_amp = rx_amp;
		io_info.rx_chan = spn_rx_chn->value();
		io_info.rx_delay = spn_rx_dly->value();
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_LOOP_INFO, &io_info);
		break;
	case VOSS_TYPE_INPUT_MON:
		mon_info.mute = (rx_mute->checkState() == Qt::Checked);
		mon_info.pol = (rx_polarity->checkState() == Qt::Checked);
		mon_info.amp = rx_amp;
		mon_info.src_chan = spn_rx_chn->value();
		mon_info.dst_chan = spn_tx_chn->value();
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_INPUT_MON_INFO, &mon_info);
		break;
	case VOSS_TYPE_OUTPUT_MON:
		mon_info.mute = (rx_mute->checkState() == Qt::Checked);
		mon_info.pol = (rx_polarity->checkState() == Qt::Checked);
		mon_info.amp = rx_amp;
		mon_info.src_chan = spn_rx_chn->value();
		mon_info.dst_chan = spn_tx_chn->value();
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_OUTPUT_MON_INFO, &mon_info);
		break;
	default:
		error = EINVAL;
		break;
	}

	parent->vconnect->update();
}

void
VOssController :: watchdog(void)
{
	peak_vol->repaint();

	switch (type) {
	int error;
	case VOSS_TYPE_DEVICE:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = number;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_DEV_LIMIT, &io_limit);
		if (error != 0)
			break;
		spn_limit->setValue(io_limit.limit);
		break;
	case VOSS_TYPE_LOOPBACK:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = number;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_LOOP_LIMIT, &io_limit);
		if (error != 0)
			break;
		spn_limit->setValue(io_limit.limit);
		break;
	case VOSS_TYPE_MASTER_OUTPUT:
		memset(&out_chn_grp, 0, sizeof(out_chn_grp));
		out_chn_grp.channel = channel;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_OUTPUT_CHN_GRP, &out_chn_grp);
		if (error != 0)
			break;
		spn_group->setValue(out_chn_grp.group);

		memset(&out_limit, 0, sizeof(out_limit));
		out_limit.group = out_chn_grp.group;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_OUTPUT_LIMIT, &out_limit);
		if (error != 0)
			break;
		spn_limit->setValue(out_limit.limit);
		break;
	default:
		break;
	}
}

void
VOssController :: get_config(void)
{
	int error;

	switch (type) {
	case VOSS_TYPE_DEVICE:
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_DEV_INFO, &io_info);
		if (error)
			break;
		rx_mute->setCheckState(io_info.rx_mute ? Qt::Checked : Qt::Unchecked);
		tx_mute->setCheckState(io_info.tx_mute ? Qt::Checked : Qt::Unchecked);
		rx_polarity->setCheckState(io_info.rx_pol ? Qt::Checked : Qt::Unchecked);
		tx_polarity->setCheckState(io_info.tx_pol ? Qt::Checked : Qt::Unchecked);
		set_rx_amp(io_info.rx_amp);
		set_tx_amp(io_info.tx_amp);
		spn_rx_chn->setValue(io_info.rx_chan);
		spn_tx_chn->setValue(io_info.tx_chan);
		spn_rx_dly->setValue(io_info.rx_delay);
		set_desc(io_info.name);
		break;
	case VOSS_TYPE_LOOPBACK:
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_LOOP_INFO, &io_info);
		if (error)
			break;
		rx_mute->setCheckState(io_info.rx_mute ? Qt::Checked : Qt::Unchecked);
		rx_polarity->setCheckState(io_info.rx_pol ? Qt::Checked : Qt::Unchecked);
		set_rx_amp(io_info.rx_amp);
		spn_rx_chn->setValue(io_info.rx_chan);
		spn_rx_dly->setValue(io_info.rx_delay);
		set_desc(io_info.name);
		break;
	case VOSS_TYPE_INPUT_MON:
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_INPUT_MON_INFO, &mon_info);
		if (error)
			break;
		rx_mute->setCheckState(mon_info.mute ? Qt::Checked : Qt::Unchecked);
		rx_polarity->setCheckState(mon_info.pol ? Qt::Checked : Qt::Unchecked);
		set_rx_amp(mon_info.amp);
		spn_rx_chn->setValue(mon_info.src_chan);
		spn_tx_chn->setValue(mon_info.dst_chan);
		set_desc("Input Monitor");
		break;
	case VOSS_TYPE_OUTPUT_MON:
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_OUTPUT_MON_INFO, &mon_info);
		if (error)
			break;
		rx_mute->setCheckState(mon_info.mute ? Qt::Checked : Qt::Unchecked);
		rx_polarity->setCheckState(mon_info.pol ? Qt::Checked : Qt::Unchecked);
		set_rx_amp(mon_info.amp);
		spn_rx_chn->setValue(mon_info.src_chan);
		spn_tx_chn->setValue(mon_info.dst_chan);
		set_desc("Output Monitor");
		break;
	case VOSS_TYPE_MASTER_OUTPUT:
		set_desc("Master Output");
		break;
	case VOSS_TYPE_MASTER_INPUT:
		set_desc("Master Input");
		break;
	default:
		break;
	}
}

void
VOssController :: handle_rx_amp_up(void)
{
	set_rx_amp(rx_amp + 1);
	handle_set_config();
}

void
VOssController :: handle_rx_amp_down(void)
{
	set_rx_amp(rx_amp - 1);
	handle_set_config();
}

void
VOssController :: handle_tx_amp_up(void)
{
	set_tx_amp(tx_amp + 1);
	handle_set_config();
}

void
VOssController :: handle_tx_amp_down(void)
{
	set_tx_amp(tx_amp - 1);
	handle_set_config();
}

void
VOssController :: handle_spn_grp(int value)
{
	switch (type) {
	int error;
	case VOSS_TYPE_MASTER_OUTPUT:
		memset(&out_chn_grp, 0, sizeof(out_chn_grp));
		out_chn_grp.channel = channel;
		out_chn_grp.group = value;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_OUTPUT_CHN_GRP, &out_chn_grp);
		break;
	default:
		break;
	}
}

void
VOssController :: handle_spn_lim(int value)
{
	switch (type) {
	int error;
	case VOSS_TYPE_MASTER_OUTPUT:
		memset(&out_limit, 0, sizeof(out_limit));
		out_limit.group = spn_group->value();
		out_limit.limit = value;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_OUTPUT_LIMIT, &out_limit);
		break;
	case VOSS_TYPE_DEVICE:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = number;
		io_limit.limit = value;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_DEV_LIMIT, &io_limit);
		break;
	case VOSS_TYPE_LOOPBACK:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = number;
		io_limit.limit = value;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_LOOP_LIMIT, &io_limit);
		break;
	default:
		break;
	}
}

VOssMainWindow :: VOssMainWindow(const char *dsp)
{
	struct virtual_oss_io_peak io_peak;
	struct virtual_oss_mon_peak mon_peak;
	struct virtual_oss_master_peak master_peak;

	int x;
	int type = 0;
	int num = 0;
	int chan = 0;
	int error;

	generation = 0;

	dsp_name = dsp;

	dsp_fd = ::open(dsp, O_RDWR);

	gl_ctl = new VOssGridLayout();

	for (x = 0; x != MAX_VOLUME_BAR; ) {
		switch (type) {
		case VOSS_TYPE_DEVICE:
			memset(&io_peak, 0, sizeof(io_peak));
			io_peak.number = num;
			io_peak.channel = chan;
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_DEV_PEAK, &io_peak);
			break;
		case VOSS_TYPE_LOOPBACK:
			memset(&io_peak, 0, sizeof(io_peak));
			io_peak.number = num;
			io_peak.channel = chan;
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_LOOP_PEAK, &io_peak);
			break;
		case VOSS_TYPE_INPUT_MON:
			memset(&mon_peak, 0, sizeof(mon_peak));
			mon_peak.number = num;
			if (chan != 0) {
				error = EINVAL;
				break;
			}
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_INPUT_MON_PEAK, &mon_peak);
			break;
		case VOSS_TYPE_OUTPUT_MON:
			memset(&mon_peak, 0, sizeof(mon_peak));
			mon_peak.number = num;
			if (chan != 0) {
				error = EINVAL;
				break;
			}
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_OUTPUT_MON_PEAK, &mon_peak);
			break;
		case VOSS_TYPE_MASTER_OUTPUT:
			memset(&master_peak, 0, sizeof(master_peak));
			master_peak.channel = chan;
			if (num != 0) {
				error = EINVAL;
				break;
			}
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_OUTPUT_PEAK, &master_peak);
			break;
		case VOSS_TYPE_MASTER_INPUT:
			memset(&master_peak, 0, sizeof(master_peak));
			master_peak.channel = chan;
			if (num != 0) {
				error = EINVAL;
				break;
			}
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_INPUT_PEAK, &master_peak);
			break;
		default:
			error = EINVAL;
			break;
		}
		if (error) {
			if (chan == 0) {
				num = chan = 0;
				type++;
				if (type >= VOSS_TYPE_MAX)
					break;
			} else {
				num++;
				chan = 0;
			}
			continue;
		}
		vb[x] = new VOssController(this, type, chan, num);
		gl_ctl->addWidget(vb[x], x, 0, 1, 1);
		chan++;
		x++;
	}

	for (; x != MAX_VOLUME_BAR; x++)
		vb[x] = 0;

	vconnect = new VOssConnect(this);
	vaudiodelay = new VOssAudioDelayLocator(this);
	vrecordstatus = new VOssRecordStatus(this);

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	gl_main = new VOssGridLayout();
	gl_main->addWidget(vconnect,0,0,1,1);
	gl_main->addWidget(gl_ctl,1,0,1,1);
	gl_main->addWidget(vaudiodelay,2,0,1,1);
	gl_main->addWidget(vrecordstatus,3,0,1,1);

	setWindowTitle(QString("Virtual Oss Control"));
	setWindowIcon(QIcon(QString(":/virtual_oss_ctl.png")));
	setWidget(gl_main);

	watchdog->start(100);
}

VOssMainWindow :: ~VOssMainWindow()
{

}

void
VOssMainWindow :: handle_watchdog(void)
{
	int x;
	int error;

	if (dsp_fd < 0)
		dsp_fd = ::open(dsp_name, O_RDWR);

	if (dsp_fd < 0)
		return;

	error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_VERSION, &x);
	if (error != 0) {
		::close(dsp_fd);
		dsp_fd = -1;
		return;
	}

	generation ++;

	for (x = 0; x != MAX_VOLUME_BAR; x++) {
		if (vb[x] == NULL)
			continue;
		vb[x]->watchdog();
	}

	vaudiodelay->read_state();
}

static void
usage(void)
{
	fprintf(stderr, "usage: virtual_oss_ctl -f /dev/vdsp.ctl\n");
	exit(EX_USAGE);
}

int
main(int argc, char **argv)
{
	QApplication app(argc, argv);
	const char *optstring = "f:h?";
	const char *ctldevice = NULL;
	int c;

	while ((c = getopt(argc, argv, optstring)) != -1) {
		switch (c) {
		case 'f':
			ctldevice = optarg;
			break;
		default:
			usage();
			break;
		}
	}

	if (ctldevice == NULL)
		usage();

	VOssMainWindow mw(ctldevice);

	mw.show();

	return (app.exec());
}
