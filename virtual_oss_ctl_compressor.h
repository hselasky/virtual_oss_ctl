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

#ifndef _VOSS_CTL_COMPRESSOR_H_
#define	_VOSS_CTL_COMPRESSOR_H_

#include "virtual_oss_ctl.h"

struct virtual_oss_compressor;

class VOSSCompressor : public QWidget
{
	Q_OBJECT;
public:
	VOSSCompressor(VOSSMainWindow *_parent, int _type, int _num, int _channel, const char *name);
	~VOSSCompressor();

	void get_values(const virtual_oss_compressor *);
	void get_values(void);
	void gain_update(const virtual_oss_compressor *);
	void gain_update(void);
  	void get_param(virtual_oss_compressor *);

	int type;
	int num;
	int channel;

	struct virtual_oss_compressor out_limit;
	struct virtual_oss_io_limit io_limit;

	QGridLayout *gl;
	VOSSMainWindow *parent;

	QLabel *lbl_gain;
	QSpinBox *spn_enabled;
	QSpinBox *spn_knee;
	QSpinBox *spn_attack;
	QSpinBox *spn_decay;

public slots:
	void handle_copy();
	void handle_paste();
	void handle_update();
};

#endif		/* _VOSS_CTL_COMPRESSOR_H_ */
