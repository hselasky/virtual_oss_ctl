/*-
 * Copyright (c) 2013 Hans Petter Selasky. All rights reserved.
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

#ifndef _VIRTUAL_OSS_CTL_CONNECT_H_
#define	_VIRTUAL_OSS_CTL_CONNECT_H_

#include "virtual_oss_ctl.h"

class VOssConnect;

class VOssDevConnections : public QWidget
{
public:
	VOssDevConnections(VOssConnect *);
	~VOssDevConnections();

	enum {
	  MIX_LEFT,
	  MIX_RIGHT
	};
	int getTxRow(int);
	int getRxRow(int);
	void drawNice(QPainter &, int, int, int, int, int, int);
	VOssConnect *parent;
	void paintEvent(QPaintEvent *);
};

class VOssLoopConnections : public QWidget
{
public:
	VOssLoopConnections(VOssConnect *);
	~VOssLoopConnections();

	enum {
	  MIX_LEFT,
	  MIX_RIGHT
	};
	void drawNice(QPainter &, int, int, int, int, int, int);
	VOssConnect *parent;
	void paintEvent(QPaintEvent *);
};

class VOssConnect : public QGroupBox
{
	Q_OBJECT;

public:
	VOssConnect(VOssMainWindow *);
	~VOssConnect();

	VOssMainWindow *parent;
	QGridLayout *gl;
	VOssDevConnections *devconn;
	VOssLoopConnections *loopconn;

	uint32_t n_master_input;
	uint32_t n_master_output;
};

#endif		/* _VIRTUAL_OSS_CTL_CONNECT_H_ */
