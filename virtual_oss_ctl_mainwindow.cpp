/*-
 * Copyright (c) 2012-2022 Hans Petter Selasky
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

#include "virtual_oss_ctl_connect.h"
#include "virtual_oss_ctl_compressor.h"
#include "virtual_oss_ctl_equalizer.h"
#include "virtual_oss_ctl_gridlayout.h"
#include "virtual_oss_ctl_mainwindow.h"

#define	VBAR_HEIGHT 32
#define	VBAR_WIDTH 128

VOSSVolumeBar :: VOSSVolumeBar(VOSSController *_parent, int _type, int _channel, int _number)
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

	setMinimumSize(VBAR_WIDTH, VBAR_HEIGHT);
	setMaximumSize(VBAR_WIDTH, VBAR_HEIGHT);
}

VOSSVolumeBar :: ~VOSSVolumeBar()
{

}

VOSSAudioDelayLocator :: VOSSAudioDelayLocator(VOSSMainWindow *_parent)
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

VOSSAudioDelayLocator :: ~VOSSAudioDelayLocator()
{

}

void
VOSSAudioDelayLocator :: read_state()
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
	    "Delay locator is %s. Output volume level is %d. Measured audio delay is %d samples or %f ms.",
	    ad.locator_enabled ? "enabled" : "disabled",
	    (int)ad.signal_output_level,
	    (int)ad.signal_input_delay,
	    (float)1000.0 * (float)ad.signal_input_delay / (float)ad.signal_delay_hz);

	lbl_status->setText(QString(status));
}

void
VOSSAudioDelayLocator :: handle_reset()
{
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_RST_AUDIO_DELAY_LOCATOR);
	if (error)
		return;
}

void
VOSSAudioDelayLocator :: handle_signal_up()
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
VOSSAudioDelayLocator :: handle_signal_down()
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
VOSSAudioDelayLocator :: handle_channel_in()
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
VOSSAudioDelayLocator :: handle_channel_out()
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
VOSSAudioDelayLocator :: handle_enable_disable()
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

VOSSRecordStatus :: VOSSRecordStatus(VOSSMainWindow *_parent)
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

VOSSRecordStatus :: ~VOSSRecordStatus()
{

}

VOSSAddOptions :: VOSSAddOptions(VOSSMainWindow *_parent)
{
	parent = _parent;

	memset(buffer, 0, sizeof(buffer));

	gl = new QGridLayout(this);

	setTitle(tr("Add Options"));

	but_add = new QPushButton(tr("ADD"));
	led_config = new QLineEdit();

	gl->addWidget(led_config,0,0,1,1);
	gl->addWidget(but_add,0,1,1,1);

	connect(but_add, SIGNAL(released()), this, SLOT(handle_add()));
	connect(led_config, SIGNAL(returnPressed()), this, SLOT(handle_add()));
}

VOSSAddOptions :: ~VOSSAddOptions()
{

}

void
VOSSAddOptions :: handle_add()
{
	int fd = parent->dsp_fd;
	int error;

	strlcpy(buffer, led_config->text().toLatin1().data(), sizeof(buffer));

	error = ::ioctl(fd, VIRTUAL_OSS_ADD_OPTIONS, buffer);
	if (error)
		return;

	led_config->setText(QString(buffer));

	parent->vsysinfo->updateInfo();
}


VOSSSysInfoOptions :: VOSSSysInfoOptions(VOSSMainWindow *_parent)
{
	parent = _parent;

	gl = new QGridLayout(this);

	setTitle(tr("System information"));

	gl->addWidget(&lbl_status, 0,0,1,1);

	updateInfo();
}

VOSSSysInfoOptions :: ~VOSSSysInfoOptions()
{

}

void
VOSSSysInfoOptions :: updateInfo()
{
	struct virtual_oss_system_info info;
	int fd = parent->dsp_fd;
	int error;

	error = ::ioctl(fd, VIRTUAL_OSS_GET_SYSTEM_INFO, &info);
	if (error)
		return;

	lbl_status.setText(QString("%1 Hz, %2 bits, %3 channels, Input: %4, Output: %5")
			   .arg(info.sample_rate)
			   .arg(info.sample_bits)
			   .arg(info.sample_channels)
			   .arg(info.rx_device_name)
			   .arg(info.tx_device_name));
}

void
VOSSRecordStatus :: read_state()
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
VOSSRecordStatus :: handle_start()
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
VOSSRecordStatus :: handle_stop()
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
VOSSVolumeBar :: drawBar(QPainter &paint, int y, int h, int level)
{
	const int d = (VBAR_WIDTH / 8);
	int x;

	for (x = 0; level > 0; level -= d, x += d) {
		QColor bar(96 + x, 192 - x, 96);
		paint.fillRect(x, y, (level >= d) ? d : level, h, bar);
	}
}

void
VOSSVolumeBar :: paintEvent(QPaintEvent *event)
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
	case VOSS_TYPE_LOOPBACK:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT,black);

		if (doit) {
			error = ::ioctl(fd, (type == VOSS_TYPE_DEVICE) ?
			    VIRTUAL_OSS_GET_DEV_PEAK : VIRTUAL_OSS_GET_LOOP_PEAK, &io_peak);
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

	case VOSS_TYPE_LOCAL_MON:
		paint.fillRect(0,0,VBAR_WIDTH,VBAR_HEIGHT / 2,black);

		if (doit) {
			error = ::ioctl(fd, VIRTUAL_OSS_GET_LOCAL_MON_PEAK, &mon_peak);
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

	case VOSS_TYPE_MAIN_OUTPUT:
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

	case VOSS_TYPE_MAIN_INPUT:
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

VOSSController :: VOSSController(VOSSMainWindow *_parent, int _type, int _channel, int _number)
  : connect_input_label(0), connect_output_label(0), connect_row(0)
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

	peak_vol = new VOSSVolumeBar(this, _type, _channel, _number);

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

	switch (type) {
	case VOSS_TYPE_DEVICE:
	case VOSS_TYPE_LOOPBACK:
		rx_eq_show = new QPushButton(QString("EQ"));
		tx_eq_show = new QPushButton(QString("EQ"));
		rx_eq = new VOSSEqualizer(parent, _type | VOSS_TYPE_RX, number, channel);
		tx_eq = new VOSSEqualizer(parent, _type | VOSS_TYPE_TX, number, channel);
		if (rx_eq->filter_size == 0)
			rx_eq_show->setDisabled(1);
		if (tx_eq->filter_size == 0)
			tx_eq_show->setDisabled(1);
		connect(rx_eq_show, SIGNAL(released()), this, SLOT(handle_rx_eq()));
		connect(tx_eq_show, SIGNAL(released()), this, SLOT(handle_tx_eq()));

		spn_rx_dly = new QSpinBox();
		spn_rx_dly->setSuffix(QString(" samples"));
		connect(spn_rx_dly, SIGNAL(valueChanged(int)), this, SLOT(handle_set_config()));
		break;
	default:
		rx_eq_show = 0;
		tx_eq_show = 0;
		rx_eq = 0;
		tx_eq = 0;
		spn_rx_dly = 0;
		break;
	}

	spn_rx_chn = new QSpinBox();
	spn_rx_chn->setRange(0, 63);
	spn_rx_chn->setPrefix(tr("SrcCh "));
	spn_tx_chn = new QSpinBox();
	spn_tx_chn->setRange(0, 63);
	spn_tx_chn->setPrefix(tr("DstCh "));

	get_config();

	switch (type) {
	case VOSS_TYPE_DEVICE:
	case VOSS_TYPE_LOOPBACK:
	case VOSS_TYPE_MAIN_OUTPUT:
		compressor = new QPushButton(tr("Compressor"));
		connect(compressor, SIGNAL(released()), this, SLOT(handle_compressor()));

		if (channel == 0) {
			compressor_edit = new VOSSCompressor(parent,
			    _type, number, channel,
			    (_type == VOSS_TYPE_MAIN_OUTPUT) ?
			    "Main Output" : io_info.name);
		} else {
			compressor_edit = 0;
		}
		break;
	default:
		compressor = 0;
		compressor_edit = 0;
		break;
	}

	connect(rx_mute, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(tx_mute, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(rx_polarity, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(tx_polarity, SIGNAL(stateChanged(int)), this, SLOT(handle_set_config()));
	connect(rx_amp_up, SIGNAL(released()), this, SLOT(handle_rx_amp_up()));
	connect(rx_amp_down, SIGNAL(released()), this, SLOT(handle_rx_amp_down()));
	connect(tx_amp_up, SIGNAL(released()), this, SLOT(handle_tx_amp_up()));
	connect(tx_amp_down, SIGNAL(released()), this, SLOT(handle_tx_amp_down()));
	connect(spn_rx_chn, SIGNAL(valueChanged(int)), this, SLOT(handle_set_config()));
	connect(spn_tx_chn, SIGNAL(valueChanged(int)), this, SLOT(handle_set_config()));

	switch (type) {
	case VOSS_TYPE_DEVICE:
	case VOSS_TYPE_LOOPBACK:
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
		gl->addWidget(compressor, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(rx_eq_show, 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(tx_eq_show, 1, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("DELAY:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_rx_dly, 0, x, 1, 1, Qt::AlignCenter);
		break;

	case VOSS_TYPE_INPUT_MON:
	case VOSS_TYPE_OUTPUT_MON:
	case VOSS_TYPE_LOCAL_MON:
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

	case VOSS_TYPE_MAIN_OUTPUT:
		x = 0;
		gl->addWidget(peak_vol, 0, x, 1, 1, Qt::AlignLeft);
		x++;
		gl->addWidget(compressor, 0, x, 1, 1, Qt::AlignCenter);
		break;

	case VOSS_TYPE_MAIN_INPUT:
		x = 0;
		gl->addWidget(peak_vol, 0, x, 1, 1, Qt::AlignLeft);
		break;

	default:
		break;
	}
}

VOSSController :: ~VOSSController()
{
	delete rx_eq;
	delete tx_eq;
}

void
VOSSController :: set_desc(const char *desc)
{
	char buf[64];

	if (desc != NULL && desc[0]) {
		switch (type) {
		case VOSS_TYPE_DEVICE:
		case VOSS_TYPE_LOOPBACK:
		case VOSS_TYPE_MAIN_OUTPUT:
		case VOSS_TYPE_MAIN_INPUT:
			snprintf(buf, sizeof(buf),
			    "%s - Ch%d", desc, channel);
			break;
		case VOSS_TYPE_INPUT_MON:
		case VOSS_TYPE_OUTPUT_MON:
		case VOSS_TYPE_LOCAL_MON:
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
VOSSController :: set_rx_amp(int value)
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
VOSSController :: set_tx_amp(int value)
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
VOSSController :: handle_set_config(void)
{
	int error;

	switch (type) {
	case VOSS_TYPE_DEVICE:
	case VOSS_TYPE_LOOPBACK:
		io_info.rx_mute = (rx_mute->checkState() == Qt::Checked);
		io_info.tx_mute = (tx_mute->checkState() == Qt::Checked);
		io_info.rx_pol = (rx_polarity->checkState() == Qt::Checked);
		io_info.tx_pol = (tx_polarity->checkState() == Qt::Checked);
		io_info.rx_amp = rx_amp;
		io_info.tx_amp = tx_amp;
		io_info.rx_chan = spn_rx_chn->value();
		io_info.tx_chan = spn_tx_chn->value();
		io_info.rx_delay = spn_rx_dly->value();
		error = ::ioctl(parent->dsp_fd, (type == VOSS_TYPE_DEVICE) ?
		    VIRTUAL_OSS_SET_DEV_INFO : VIRTUAL_OSS_SET_LOOP_INFO, &io_info);
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
	case VOSS_TYPE_LOCAL_MON:
		mon_info.mute = (rx_mute->checkState() == Qt::Checked);
		mon_info.pol = (rx_polarity->checkState() == Qt::Checked);
		mon_info.amp = rx_amp;
		mon_info.src_chan = spn_rx_chn->value();
		mon_info.dst_chan = spn_tx_chn->value();
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_LOCAL_MON_INFO, &mon_info);
		break;
	default:
		error = EINVAL;
		break;
	}

	parent->vconnect->update();
}

void
VOSSController :: watchdog(void)
{
	peak_vol->repaint();

	if (compressor_edit != 0)
		compressor_edit->gain_update();
}

void
VOSSController :: get_config(void)
{
	int error;

	switch (type) {
	case VOSS_TYPE_DEVICE:
	case VOSS_TYPE_LOOPBACK:
		error = ::ioctl(parent->dsp_fd, (type == VOSS_TYPE_DEVICE) ?
		    VIRTUAL_OSS_GET_DEV_INFO : VIRTUAL_OSS_GET_LOOP_INFO, &io_info);
		if (error)
			break;
		VOSS_BLOCKED(rx_mute,setCheckState(io_info.rx_mute ? Qt::Checked : Qt::Unchecked));
		VOSS_BLOCKED(tx_mute,setCheckState(io_info.tx_mute ? Qt::Checked : Qt::Unchecked));
		VOSS_BLOCKED(rx_polarity,setCheckState(io_info.rx_pol ? Qt::Checked : Qt::Unchecked));
		VOSS_BLOCKED(tx_polarity,setCheckState(io_info.tx_pol ? Qt::Checked : Qt::Unchecked));
		set_rx_amp(io_info.rx_amp);
		set_tx_amp(io_info.tx_amp);
		VOSS_BLOCKED(spn_rx_chn,setValue(io_info.rx_chan));
		VOSS_BLOCKED(spn_tx_chn,setValue(io_info.tx_chan));
		spn_rx_dly->setRange(0, io_info.rx_delay_limit);
		VOSS_BLOCKED(spn_rx_dly,setValue(io_info.rx_delay));
		set_desc(io_info.name);
		break;
	case VOSS_TYPE_INPUT_MON:
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_INPUT_MON_INFO, &mon_info);
		if (error)
			break;
		rx_mute->setCheckState(mon_info.mute ? Qt::Checked : Qt::Unchecked);
		rx_polarity->setCheckState(mon_info.pol ? Qt::Checked : Qt::Unchecked);
		set_rx_amp(mon_info.amp);
		VOSS_BLOCKED(spn_rx_chn,setValue(mon_info.src_chan));
		VOSS_BLOCKED(spn_tx_chn,setValue(mon_info.dst_chan));
		set_desc("Input Monitor");
		break;
	case VOSS_TYPE_OUTPUT_MON:
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_OUTPUT_MON_INFO, &mon_info);
		if (error)
			break;
		VOSS_BLOCKED(rx_mute,setCheckState(mon_info.mute ? Qt::Checked : Qt::Unchecked));
		VOSS_BLOCKED(rx_polarity,setCheckState(mon_info.pol ? Qt::Checked : Qt::Unchecked));
		set_rx_amp(mon_info.amp);
		VOSS_BLOCKED(spn_rx_chn,setValue(mon_info.src_chan));
		VOSS_BLOCKED(spn_tx_chn,setValue(mon_info.dst_chan));
		set_desc("Output Monitor");
		break;
	case VOSS_TYPE_LOCAL_MON:
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_LOCAL_MON_INFO, &mon_info);
		if (error)
			break;
		rx_mute->setCheckState(mon_info.mute ? Qt::Checked : Qt::Unchecked);
		rx_polarity->setCheckState(mon_info.pol ? Qt::Checked : Qt::Unchecked);
		set_rx_amp(mon_info.amp);
		VOSS_BLOCKED(spn_rx_chn,setValue(mon_info.src_chan));
		VOSS_BLOCKED(spn_tx_chn,setValue(mon_info.dst_chan));
		set_desc("Local Monitor");
		break;
	case VOSS_TYPE_MAIN_OUTPUT:
		set_desc("Main Output");
		break;
	case VOSS_TYPE_MAIN_INPUT:
		set_desc("Main Input");
		break;
	default:
		break;
	}
}

void
VOSSController :: handle_rx_amp_up(void)
{
	set_rx_amp(rx_amp + 1);
	handle_set_config();
}

void
VOSSController :: handle_rx_amp_down(void)
{
	set_rx_amp(rx_amp - 1);
	handle_set_config();
}

void
VOSSController :: handle_rx_eq(void)
{
	rx_eq->show();
}

void
VOSSController :: handle_tx_amp_up(void)
{
	set_tx_amp(tx_amp + 1);
	handle_set_config();
}

void
VOSSController :: handle_tx_amp_down(void)
{
	set_tx_amp(tx_amp - 1);
	handle_set_config();
}

void
VOSSController :: handle_tx_eq(void)
{
	tx_eq->show();
}

void
VOSSController :: handle_compressor(void)
{
	VOSSController **vb = parent->vb;
	int x;

	for (x = 0; x != MAX_VOLUME_BAR; x++) {
		if (vb[x] == NULL)
			continue;
		if (vb[x]->type == type &&
		    vb[x]->number == number &&
		    vb[x]->channel == 0)
			vb[x]->compressor_edit->show();
	}
}

VOSSMainWindow :: VOSSMainWindow(const char *dsp)
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

	gl_ctl = new VOSSGridLayout();

	eq_copy = 0;
	compressor_copy = 0;

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
		case VOSS_TYPE_LOCAL_MON:
			memset(&mon_peak, 0, sizeof(mon_peak));
			mon_peak.number = num;
			if (chan != 0) {
				error = EINVAL;
				break;
			}
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_LOCAL_MON_PEAK, &mon_peak);
			break;
		case VOSS_TYPE_MAIN_OUTPUT:
			memset(&master_peak, 0, sizeof(master_peak));
			master_peak.channel = chan;
			if (num != 0) {
				error = EINVAL;
				break;
			}
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_OUTPUT_PEAK, &master_peak);
			break;
		case VOSS_TYPE_MAIN_INPUT:
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
		vb[x] = new VOSSController(this, type, chan, num);
		gl_ctl->addWidget(vb[x], x, 0, 1, 1);
		chan++;
		x++;
	}

	for (; x != MAX_VOLUME_BAR; x++)
		vb[x] = 0;

	vconnect = new VOSSConnect(this);
	vaudiodelay = new VOSSAudioDelayLocator(this);
	vrecordstatus = new VOSSRecordStatus(this);
	vaddoptions = new VOSSAddOptions(this);
	vsysinfo = new VOSSSysInfoOptions(this);

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

	gl_main = new VOSSGridLayout();
	gl_main->addWidget(vconnect,0,0,1,1);
	gl_main->addWidget(gl_ctl,1,0,1,1);
	gl_main->addWidget(vaudiodelay,2,0,1,1);
	gl_main->addWidget(vrecordstatus,3,0,1,1);
	gl_main->addWidget(vaddoptions,4,0,1,1);
	gl_main->addWidget(vsysinfo,5,0,1,1);

	setWindowTitle(QString("Virtual OSS Control"));
	setWindowIcon(QIcon(QString(":/virtual_oss_ctl.png")));
	setWidget(gl_main);

	watchdog->start(100);
}

VOSSMainWindow :: ~VOSSMainWindow()
{

}

void
VOSSMainWindow :: handle_watchdog(void)
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
