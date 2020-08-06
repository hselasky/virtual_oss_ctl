/*-
 * Copyright (c) 2020 Hans Petter Selasky. All rights reserved.
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

#include "virtual_oss_ctl_compressor.h"
#include "virtual_oss_ctl_mainwindow.h"

VOSSCompressor :: VOSSCompressor(VOSSMainWindow *_parent,
    int _type, int _num, int _channel, const char *name)
{
	QPushButton *pb;

	parent = _parent;
	type = _type;
	num = _num;
	channel = _channel;

	setWindowTitle(QString("Virtual OSS Compressor for %1").arg(name));
	setWindowIcon(QIcon(QString(":/virtual_oss_ctl.png")));

	gl = new QGridLayout(this);

	gl->addWidget(new QLabel(tr("Current gain")), 0,0,1,1);
	lbl_gain = new QLabel("1.0");
	gl->addWidget(lbl_gain, 0,1,1,1);

	gl->addWidget(new QLabel(tr("Enabled")), 1,0,1,1);
	spn_enabled = new QSpinBox();
	spn_enabled->setRange(0,1);
	gl->addWidget(spn_enabled, 1,1,1,1);

	gl->addWidget(new QLabel(tr("Knee")), 2,0,1,1);
	spn_knee = new QSpinBox();
	spn_knee->setRange(VIRTUAL_OSS_KNEE_MIN, VIRTUAL_OSS_KNEE_MAX);
	gl->addWidget(spn_knee, 2,1,1,1);

	gl->addWidget(new QLabel(tr("Attack")), 3,0,1,1);
	spn_attack = new QSpinBox();
	spn_attack->setRange(VIRTUAL_OSS_ATTACK_MIN, VIRTUAL_OSS_ATTACK_MAX);
	gl->addWidget(spn_attack, 3,1,1,1);

	gl->addWidget(new QLabel(tr("Decay")), 4,0,1,1);
	spn_decay = new QSpinBox();
	spn_decay->setRange(VIRTUAL_OSS_DECAY_MIN, VIRTUAL_OSS_DECAY_MAX);
	gl->addWidget(spn_decay, 4,1,1,1);
	
	pb = new QPushButton(tr("COPY"));
	connect(pb, SIGNAL(released()), this, SLOT(handle_copy()));
	gl->addWidget(pb, 5,0,1,1);

	pb = new QPushButton(tr("PASTE"));
	connect(pb, SIGNAL(released()), this, SLOT(handle_paste()));
	gl->addWidget(pb, 5,1,1,1);

	get_values();

	connect(spn_enabled, SIGNAL(valueChanged(int)), this, SLOT(handle_update()));
	connect(spn_knee, SIGNAL(valueChanged(int)), this, SLOT(handle_update()));
	connect(spn_attack, SIGNAL(valueChanged(int)), this, SLOT(handle_update()));
	connect(spn_decay, SIGNAL(valueChanged(int)), this, SLOT(handle_update()));
}

VOSSCompressor :: ~VOSSCompressor()
{
	if (parent->compressor_copy == this)
		parent->compressor_copy = 0;
}

void
VOSSCompressor :: get_values(const virtual_oss_compressor *ptr)
{
	spn_enabled->setValue(ptr->enabled);
	spn_knee->setValue(ptr->knee);
	spn_attack->setValue(ptr->attack);
	spn_decay->setValue(ptr->decay);
}

void
VOSSCompressor :: get_values(void)
{
	if (parent->dsp_fd < 0)
		return;

	switch (type) {
	int error;
	case VOSS_TYPE_MAIN_OUTPUT:
		memset(&out_limit, 0, sizeof(out_limit));
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_OUTPUT_LIMIT, &out_limit);
		if (error == 0)
			get_values(&out_limit);
		break;
	case VOSS_TYPE_DEVICE:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = num;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_DEV_LIMIT, &io_limit);
		if (error == 0)
			get_values(&io_limit.param);
		break;
	case VOSS_TYPE_LOOPBACK:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = num;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_LOOP_LIMIT, &io_limit);
		if (error == 0)
			get_values(&io_limit.param);
		break;
	default:
		break;
	}
}

void
VOSSCompressor :: handle_copy()
{
	parent->compressor_copy = this;
}

void
VOSSCompressor :: handle_paste()
{
	if (parent->compressor_copy == 0 || parent->compressor_copy == this)
		return;

	spn_enabled->setValue(parent->compressor_copy->spn_enabled->value());
	spn_knee->setValue(parent->compressor_copy->spn_knee->value());
	spn_attack->setValue(parent->compressor_copy->spn_attack->value());
	spn_decay->setValue(parent->compressor_copy->spn_decay->value());
}

void
VOSSCompressor :: gain_update(const virtual_oss_compressor *ptr)
{
	lbl_gain->setText(QString("%1").arg((double)ptr->gain / (double)1000.0));
}

void
VOSSCompressor :: gain_update(void)
{
	if (parent->dsp_fd < 0)
		return;

	switch (type) {
	int error;
	case VOSS_TYPE_MAIN_OUTPUT:
		memset(&out_limit, 0, sizeof(out_limit));
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_OUTPUT_LIMIT, &out_limit);
		if (error == 0)
			gain_update(&out_limit);
		break;
	case VOSS_TYPE_DEVICE:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = num;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_DEV_LIMIT, &io_limit);
		if (error == 0)
			gain_update(&io_limit.param);
		break;
	case VOSS_TYPE_LOOPBACK:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = num;
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_GET_LOOP_LIMIT, &io_limit);
		if (error == 0)
			gain_update(&io_limit.param);
		break;
	default:
		break;
	}
}

void
VOSSCompressor :: get_param(virtual_oss_compressor *ptr)
{
	memset(ptr, 0, sizeof(*ptr));
	ptr->enabled = spn_enabled->value();
	ptr->knee = spn_knee->value();
	ptr->attack = spn_attack->value();
	ptr->decay = spn_decay->value();
}

void
VOSSCompressor :: handle_update()
{
	if (parent->dsp_fd < 0)
		return;

	switch (type) {
	int error;
	case VOSS_TYPE_MAIN_OUTPUT:
		get_param(&out_limit);
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_OUTPUT_LIMIT, &out_limit);
		break;
	case VOSS_TYPE_DEVICE:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = num;
		get_param(&io_limit.param);
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_DEV_LIMIT, &io_limit);
		break;
	case VOSS_TYPE_LOOPBACK:
		memset(&io_limit, 0, sizeof(io_limit));
		io_limit.number = num;
		get_param(&io_limit.param);
		error = ::ioctl(parent->dsp_fd, VIRTUAL_OSS_SET_LOOP_LIMIT, &io_limit);
		break;
	default:
		break;
	}
}
