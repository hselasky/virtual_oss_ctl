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

#ifndef _VIRTUAL_OSS_CTL_MAINWINDOW_H_
#define	_VIRTUAL_OSS_CTL_MAINWINDOW_H_

#include "virtual_oss_ctl.h"

class VOSSVolumeBar : public QWidget
{
	Q_OBJECT;

public:
	VOSSVolumeBar(VOSSController *parent = 0, int type = 0, int chan = 0, int num = 0);
	~VOSSVolumeBar();

	void drawBar(QPainter &, int, int, int);

	VOSSController *parent;

	struct virtual_oss_io_peak io_peak;
	struct virtual_oss_mon_peak mon_peak;
	struct virtual_oss_master_peak master_peak;

	int type;
	int channel;
	int number;
	int generation;

	void paintEvent(QPaintEvent *);
};

class VOSSAudioDelayLocator : public QGroupBox
{
	Q_OBJECT;

public:
	VOSSAudioDelayLocator(VOSSMainWindow * = 0);
	~VOSSAudioDelayLocator();

	VOSSMainWindow *parent;

	QGridLayout *gl;

	QLabel *lbl_status;

	QPushButton *but_reset;
	QPushButton *but_enable_disable;
	QPushButton *but_signal_up;
	QPushButton *but_signal_down;
	QSpinBox *spn_channel_in;
	QSpinBox *spn_channel_out;

	void read_state();

public slots:
	void handle_reset();
	void handle_signal_up();
	void handle_signal_down();
	void handle_enable_disable();
	void handle_channel_in();
	void handle_channel_out();
};

class VOSSRecordStatus : public QGroupBox
{
	Q_OBJECT;

public:
	VOSSRecordStatus(VOSSMainWindow * = 0);
	~VOSSRecordStatus();

	VOSSMainWindow *parent;

	QGridLayout *gl;

	QPushButton *but_start;
	QPushButton *but_stop;

	void read_state();

public slots:
	void handle_start();
	void handle_stop();
};

class VOSSAddOptions : public QGroupBox
{
	Q_OBJECT;

public:
	VOSSAddOptions(VOSSMainWindow * = 0);
	~VOSSAddOptions();

	VOSSMainWindow *parent;

	QGridLayout *gl;

	QLineEdit *led_config;
	QPushButton *but_add;

	char buffer[VIRTUAL_OSS_OPTIONS_MAX];

public slots:
	void handle_add();
};

class VOSSController : public QGroupBox
{
	Q_OBJECT;

public:
	VOSSController(VOSSMainWindow *parent = 0, int type = 0, int channel = 0, int number = 0);
	~VOSSController();

	void set_desc(const char *);
	void set_rx_amp(int);
	void set_tx_amp(int);

	void get_config(void);

	void watchdog(void);

	VOSSMainWindow *parent;

	QGridLayout *gl;

	QCheckBox *rx_mute;
	QCheckBox *tx_mute;
	QCheckBox *rx_polarity;
	QCheckBox *tx_polarity;
	QLabel *lbl_rx_amp;
	QLabel *lbl_tx_amp;
	QPushButton *rx_amp_up;
	QPushButton *rx_amp_down;
	QPushButton *tx_amp_up;
	QPushButton *tx_amp_down;

	QPushButton *rx_eq_show;
	QPushButton *tx_eq_show;
	VOSSEqualizer *rx_eq;
	VOSSEqualizer *tx_eq;

	QLineEdit *connect_input_label;
	QLineEdit *connect_output_label;

	uint32_t connect_row;

	QSpinBox *spn_group;
	QSpinBox *spn_limit;
	QSpinBox *spn_rx_chn;
	QSpinBox *spn_tx_chn;
	QSpinBox *spn_rx_dly;

	VOSSVolumeBar *peak_vol;

	int type;
	int channel;
	int number;
	int rx_amp;
	int tx_amp;

	struct virtual_oss_io_info io_info;
	struct virtual_oss_mon_info mon_info;
	struct virtual_oss_output_limit out_limit;
	struct virtual_oss_output_chn_grp out_chn_grp;
	struct virtual_oss_io_limit io_limit;

public slots:
	void handle_rx_amp_up(void);
	void handle_rx_amp_down(void);
  	void handle_rx_eq(void);
	void handle_tx_amp_up(void);
	void handle_tx_amp_down(void);
	void handle_tx_eq(void);
	void handle_set_config(void);
	void handle_spn_grp(int);
	void handle_spn_lim(int);
};

class VOSSMainWindow : public QScrollArea
{
	Q_OBJECT;

public:
	VOSSMainWindow(const char *dsp = 0);
	~VOSSMainWindow();

	VOSSEqualizer *eq_copy;

	VOSSGridLayout *gl_ctl;

	VOSSGridLayout *gl_main;

	VOSSController *vb[MAX_VOLUME_BAR];

	VOSSConnect *vconnect;

	VOSSAudioDelayLocator *vaudiodelay;

	VOSSRecordStatus *vrecordstatus;
	VOSSAddOptions *vaddoptions;

	QTimer *watchdog;

	const char *dsp_name;
	int dsp_fd;
	int generation;

public slots:
	void handle_watchdog(void);
};

#endif		/* _VIRTUAL_OSS_CTL_MAINWINDOW_H_ */
