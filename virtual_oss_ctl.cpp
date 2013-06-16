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

#define	VBAR_HEIGHT 32
#define	VBAR_WIDTH 128

VOssVolumeBar :: VOssVolumeBar(VOssController *_parent, int _type, int _channel, int _number)
  : QWidget(_parent)
{
	parent = _parent;
	type = _type;
	channel = _channel;
	number = _number;

	memset(&dev_peak, 0, sizeof(dev_peak));
	dev_peak.number = _number;
	dev_peak.channel = _channel;

	memset(&mon_peak, 0, sizeof(mon_peak));
	mon_peak.number = _number;

	memset(&master_peak, 0, sizeof(master_peak));
	master_peak.channel = _channel;

	generation = 0;

	if (type == VOSS_TYPE_DEVICE) {
		setMinimumSize(VBAR_WIDTH, VBAR_HEIGHT);
		setMaximumSize(VBAR_WIDTH, VBAR_HEIGHT);
	} else {
		setMinimumSize(VBAR_WIDTH, VBAR_HEIGHT / 2);
		setMaximumSize(VBAR_WIDTH, VBAR_HEIGHT / 2);
	}
}

VOssVolumeBar :: ~VOssVolumeBar()
{

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
			error = ::ioctl(fd, VIRTUAL_OSS_GET_DEV_PEAK, &dev_peak);
			if (error)
				break;
		}
		w = convertPeak(dev_peak.rx_peak_value, dev_peak.bits);
		drawBar(paint, 0, VBAR_HEIGHT / 2, w);

		w = convertPeak(dev_peak.tx_peak_value, dev_peak.bits);
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
  : QGroupBox(_parent)
{
	int x;

	gl = new QGridLayout(this);

	parent = _parent;
	type = _type;
	channel = _channel;
	number = _number;

	memset(&dev_info, 0, sizeof(dev_info));
	dev_info.number = _number;
	dev_info.channel = _channel;

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
	spn_rx_chn->setRange(0, 64);
	spn_tx_chn = new QSpinBox();
	spn_tx_chn->setRange(0, 64);

	get_config();

	connect(rx_mute, SIGNAL(stateChanged(int)), this, SLOT(handle_mute(int)));
	connect(tx_mute, SIGNAL(stateChanged(int)), this, SLOT(handle_mute(int)));
	connect(rx_polarity, SIGNAL(stateChanged(int)), this, SLOT(handle_polarity(int)));
	connect(rx_polarity, SIGNAL(stateChanged(int)), this, SLOT(handle_polarity(int)));
	connect(rx_amp_up, SIGNAL(released()), this, SLOT(handle_rx_amp_up()));
	connect(rx_amp_down, SIGNAL(released()), this, SLOT(handle_rx_amp_down()));
	connect(tx_amp_up, SIGNAL(released()), this, SLOT(handle_tx_amp_up()));
	connect(tx_amp_down, SIGNAL(released()), this, SLOT(handle_tx_amp_down()));
	connect(spn_group, SIGNAL(valueChanged(int)), this, SLOT(handle_spn_grp(int)));
	connect(spn_limit, SIGNAL(valueChanged(int)), this, SLOT(handle_spn_lim(int)));
	connect(spn_rx_chn, SIGNAL(valueChanged(int)), this, SLOT(handle_spn_a(int)));
	connect(spn_tx_chn, SIGNAL(valueChanged(int)), this, SLOT(handle_spn_b(int)));

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
		gl->addWidget(new QLabel(QString("CHAN:")), 0, x, 1, 1, Qt::AlignCenter);
		gl->addWidget(new QLabel(QString("CHAN:")), 1, x, 1, 1, Qt::AlignCenter);
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
		gl->addWidget(new QLabel(QString("I-LIM:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_limit, 0, x, 1, 1, Qt::AlignCenter);
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
		gl->addWidget(new QLabel(QString("SCH:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_rx_chn, 0, x, 1, 1, Qt::AlignCenter);

		x++;
		gl->addWidget(new QLabel(QString("DCH:")), 0, x, 1, 1, Qt::AlignCenter);
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
		gl->addWidget(new QLabel(QString("GRP:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_group, 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(new QLabel(QString("O-LIM:")), 0, x, 1, 1, Qt::AlignCenter);
		x++;
		gl->addWidget(spn_limit, 0, x, 1, 1, Qt::AlignCenter);
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
		case VOSS_TYPE_MASTER_OUTPUT:
			snprintf(buf, sizeof(buf),
			    "%s - channel %d", desc, channel);
			break;
		case VOSS_TYPE_INPUT_MON:
		case VOSS_TYPE_OUTPUT_MON:
			snprintf(buf, sizeof(buf),
			    "%s - number %d", desc, number);
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
VOssController :: set_config(void)
{
	int error;

	switch (type) {
	case VOSS_TYPE_DEVICE:
		dev_info.rx_mute = (rx_mute->checkState() == Qt::Checked);
		dev_info.tx_mute = (tx_mute->checkState() == Qt::Checked);
		dev_info.rx_pol = (rx_polarity->checkState() == Qt::Checked);
		dev_info.tx_pol = (tx_polarity->checkState() == Qt::Checked);
		dev_info.rx_amp = rx_amp;
		dev_info.tx_amp = tx_amp;
		dev_info.rx_chan = spn_rx_chn->value();
		dev_info.tx_chan = spn_tx_chn->value();
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_DEV_INFO, &dev_info);
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
}

void
VOssController :: watchdog(void)
{
	peak_vol->repaint();

	switch (type) {
	int error;
	case VOSS_TYPE_DEVICE:
		memset(&dev_limit, 0, sizeof(dev_limit));
		dev_limit.number = number;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_DEV_LIMIT, &dev_limit);
		if (error != 0)
			break;
		spn_limit->setValue(dev_limit.limit);
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
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_DEV_INFO, &dev_info);
		if (error)
			break;
		rx_mute->setCheckState(dev_info.rx_mute ? Qt::Checked : Qt::Unchecked);
		tx_mute->setCheckState(dev_info.tx_mute ? Qt::Checked : Qt::Unchecked);
		rx_polarity->setCheckState(dev_info.rx_pol ? Qt::Checked : Qt::Unchecked);
		tx_polarity->setCheckState(dev_info.tx_pol ? Qt::Checked : Qt::Unchecked);
		set_rx_amp(dev_info.rx_amp);
		set_tx_amp(dev_info.tx_amp);
		spn_rx_chn->setValue(dev_info.rx_chan);
		spn_tx_chn->setValue(dev_info.tx_chan);
		set_desc(dev_info.name);
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
	default:
		break;
	}
}

void
VOssController :: handle_mute(int val)
{
	set_config();
}

void
VOssController :: handle_polarity(int val)
{
	set_config();
}

void
VOssController :: handle_rx_amp_up(void)
{
	set_rx_amp(rx_amp + 1);
	set_config();
}

void
VOssController :: handle_rx_amp_down(void)
{
	set_rx_amp(rx_amp - 1);
	set_config();
}

void
VOssController :: handle_tx_amp_up(void)
{
	set_tx_amp(tx_amp + 1);
	set_config();
}

void
VOssController :: handle_tx_amp_down(void)
{
	set_tx_amp(tx_amp - 1);
	set_config();
}

void
VOssController :: handle_spn_a(int)
{
	set_config();
}

void
VOssController :: handle_spn_b(int)
{
	set_config();
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
		memset(&dev_limit, 0, sizeof(dev_limit));
		dev_limit.number = number;
		dev_limit.limit = value;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_DEV_LIMIT, &dev_limit);
		break;
	default:
		break;
	}
}

VOssMainWindow :: VOssMainWindow(QWidget *parent, const char *dsp)
  : QWidget(parent)
{
	struct virtual_oss_dev_peak dev_peak;
	struct virtual_oss_mon_peak mon_peak;
	struct virtual_oss_master_peak master_peak;

	int x;
	int type = 0;
	int num = 0;
	int chan = 0;
	int error;

	generation = 0;

	setMinimumSize(128,128);

	dsp_name = dsp;

	dsp_fd = ::open(dsp, O_RDWR);

	gl = new QGridLayout(this);

	for (x = 0; x != MAX_VOLUME_BAR; ) {
		switch (type) {
		case VOSS_TYPE_DEVICE:
			memset(&dev_peak, 0, sizeof(dev_peak));
			dev_peak.number = num;
			dev_peak.channel = chan;
			error = ::ioctl(dsp_fd, VIRTUAL_OSS_GET_DEV_PEAK, &dev_peak);
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
		gl->addWidget(vb[x], x, 0, 1, 1);
		chan++;
		x++;
	}

	for (; x != MAX_VOLUME_BAR; x++)
		vb[x] = 0;

	watchdog = new QTimer(this);
	connect(watchdog, SIGNAL(timeout()), this, SLOT(handle_watchdog()));

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

	VOssMainWindow mw(0, ctldevice);

	QScrollArea *psa = new QScrollArea();

	psa->setWidget(&mw);
	psa->setWindowTitle(QString("Virtual Oss Control"));
	psa->setWindowIcon(QIcon(QString(":/virtual_oss_ctl.png")));
	psa->show();
	psa->setMinimumWidth(mw.width() + psa->verticalScrollBar()->width() + 4);

	return (app.exec());
}
