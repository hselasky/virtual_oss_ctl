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

#ifndef _VIRTUAL_OSS_CTL_H_
#define	_VIRTUAL_OSS_CTL_H_

#include <stdint.h>
#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sysexits.h>

#include <errno.h>
#include <err.h>

#include <QApplication>
#include <QDialog>
#include <QPushButton>
#include <QSpinBox>
#include <QGroupBox>
#include <QGridLayout>
#include <QScrollBar>
#include <QScrollArea>
#include <QStackedWidget>
#include <QTabWidget>
#include <QLabel>
#include <QTimer>
#include <QFile>
#include <QFileDialog>
#include <QCheckBox>
#include <QSpacerItem>
#include <QPicture>
#include <QColor>
#include <QPaintEvent>
#include <QPainter>
#include <QFont>
#include <QFileInfo>
#include <QPixmap>
#include <QIcon>
#include <QMessageBox>
#include <QDir>
#include <QLineEdit>

#include "virtual_oss.h"

#define	MAX_VOLUME_BAR 128
#define	MAX_MASTER_CHN 64

enum {
	VOSS_TYPE_DEVICE,
	VOSS_TYPE_LOOPBACK,
	VOSS_TYPE_INPUT_MON,
	VOSS_TYPE_OUTPUT_MON,
	VOSS_TYPE_MASTER_OUTPUT,
	VOSS_TYPE_MASTER_INPUT,
	VOSS_TYPE_MAX,
};

class VOssButton;
class VOssConnect;
class VOssController;
class VOssMainWindow;

class VOssGridLayout : public QWidget, public QGridLayout
{
public:
	VOssGridLayout();
	~VOssGridLayout();
};

class VOssVolumeBar : public QWidget
{
	Q_OBJECT;

public:
	VOssVolumeBar(VOssController *parent = 0, int type = 0, int chan = 0, int num = 0);
	~VOssVolumeBar();

	void drawBar(QPainter &, int, int, int);

	VOssController *parent;

	struct virtual_oss_io_peak io_peak;
	struct virtual_oss_mon_peak mon_peak;
	struct virtual_oss_master_peak master_peak;

	int type;
	int channel;
	int number;
	int generation;

	void paintEvent(QPaintEvent *);
};

class VOssAudioDelayLocator : public QWidget
{
	Q_OBJECT;

public:
	VOssAudioDelayLocator(VOssMainWindow * = 0);
	~VOssAudioDelayLocator();

	VOssMainWindow *parent;

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

class VOssController : public QGroupBox
{
	Q_OBJECT;

public:
	VOssController(VOssMainWindow *parent = 0, int type = 0, int channel = 0, int number = 0);
	~VOssController();

	void set_desc(const char *);
	void set_rx_amp(int);
	void set_tx_amp(int);

	void get_config(void);

	void watchdog(void);

	VOssMainWindow *parent;

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

	QLineEdit *connect_input_label;
	QLineEdit *connect_output_label;

	uint32_t connect_row;

	QSpinBox *spn_group;
	QSpinBox *spn_limit;
	QSpinBox *spn_rx_chn;
	QSpinBox *spn_tx_chn;
	QSpinBox *spn_rx_dly;

	VOssVolumeBar *peak_vol;

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
	void handle_tx_amp_up(void);
	void handle_tx_amp_down(void);
	void handle_set_config(void);
	void handle_spn_grp(int);
	void handle_spn_lim(int);
};

class VOssMainWindow : public QScrollArea
{
	Q_OBJECT;

public:
	VOssMainWindow(const char *dsp = 0);
	~VOssMainWindow();

	VOssGridLayout *gl_ctl;

	VOssGridLayout *gl_main;

	VOssController *vb[MAX_VOLUME_BAR];

	VOssConnect *vconnect;

	VOssAudioDelayLocator *vaudiodelay;

	QTimer *watchdog;

	const char *dsp_name;
	int dsp_fd;
	int generation;

public slots:
	void handle_watchdog(void);
};

#endif		/* _VIRTUAL_OSS_CTL_H_ */
