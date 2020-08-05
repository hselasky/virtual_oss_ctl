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

enum {
	VOSS_TYPE_RX = 256,
	VOSS_TYPE_TX = 0,
};

class VOSSButton;
class VOSSButton;
class VOSSButtonMap;
class VOSSCompressor;
class VOSSConnect;
class VOSSController;
class VOSSEqualizer;
class VOSSGridLayout;
class VOSSGroupBox;
class VOSSMainWindow;
class VOSSVolume;

#endif		/* _VIRTUAL_OSS_CTL_H_ */
