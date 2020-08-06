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

#include "virtual_oss_ctl_connect.h"
#include "virtual_oss_ctl_mainwindow.h"

VOSSLoopConnections :: VOSSLoopConnections(VOSSConnect *voc)
{
	parent = voc;
	setMinimumWidth(128);
}

VOSSLoopConnections :: ~VOSSLoopConnections()
{
}

void
VOSSLoopConnections :: drawNice(QPainter &paint, int src_flag, int src_row,
    int dst_flag, int dst_row, int muted, int draw_fg)
{
	enum { MARGIN = 16 };
	QPoint points[2];
	QPoint margin[2];
	int w = width();

	if (muted == draw_fg)
		return;

	QColor color = Qt::black;
	if (muted)
		color = QColor(192,192,192);

	paint.setBrush(color);
	paint.setPen(QPen(color, (src_flag == dst_flag) ? 8 : 4,
	    muted ? Qt::DotLine : Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

	QRect nul_rec = parent->gl->cellRect(1,0);
	QPoint nul_point = QPoint(0, nul_rec.y());

	QRect src_rect = parent->gl->cellRect(src_row,0);
	QRect dst_rect = parent->gl->cellRect(dst_row,0);

	if (src_flag == MIX_LEFT) {
		margin[0] = QPoint(MARGIN, 0);
		points[0] = QPoint(0,src_rect.y() + (src_rect.height() / 2)) - nul_point;
	} else {
		margin[0] = QPoint(-MARGIN, 0);
		points[0] = QPoint(w,src_rect.y() + (src_rect.height() / 2)) - nul_point;
	}
	if (dst_flag == MIX_LEFT) {
		margin[1] = QPoint(MARGIN, 0);
		points[1] = QPoint(0,dst_rect.y() + (dst_rect.height() / 2)) - nul_point;
	} else {
		margin[1] = QPoint(-MARGIN, 0);
		points[1] = QPoint(w,dst_rect.y() + (dst_rect.height() / 2)) - nul_point;
	}
	QPoint poly[4] = {points[0], points[0] + margin[0], points[1] + margin[1], points[1]};
	paint.drawPolyline(poly, 4);

	paint.setPen(QPen(color, 1));
	paint.drawEllipse(QRect(poly[3].x() - (MARGIN/2), poly[3].y() - (MARGIN/2), MARGIN, MARGIN));
	paint.drawEllipse(QRect(poly[0].x() - (MARGIN/2), poly[0].y() - (MARGIN/2), MARGIN, MARGIN));
}

void
VOSSLoopConnections :: paintEvent(QPaintEvent *event)
{
	VOSSController *pc;
	QPainter paint(this);
	int w = width();
	int h = height();
	int x;
	int y;

	paint.setRenderHints(QPainter::Antialiasing, 1);

	paint.fillRect(QRectF(0,0,w,h), Qt::white);

	for (y = 0; y != 2; y++) {
		for (x = 0; x != MAX_VOLUME_BAR && ((pc = parent->parent->vb[x]) != 0); x++) {
			switch(pc->type) {
			case VOSS_TYPE_LOOPBACK:
				drawNice(paint, MIX_RIGHT, pc->connect_row,
				    MIX_LEFT, pc->io_info.rx_chan + 1,
				    pc->io_info.rx_mute, y);
				break;
			default:
				break;
			}
		}
	}
}

VOSSDevConnections :: VOSSDevConnections(VOSSConnect *voc)
{
	parent = voc;
	setMinimumWidth(128);
}

VOSSDevConnections :: ~VOSSDevConnections()
{
}

void
VOSSDevConnections :: drawNice(QPainter &paint, int src_flag, int src_row,
    int dst_flag, int dst_row, int muted, int draw_fg)
{
	enum { MARGIN = 16 };
	QPoint points[2];
	QPoint margin[2];
	int w = width();

	if (muted == draw_fg)
		return;

	QColor color = Qt::black;
	if (muted)
		color = QColor(192,192,192);

	paint.setBrush(color);
	paint.setPen(QPen(color, (src_flag == dst_flag) ? 8 : 4,
	    muted ? Qt::DotLine : Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));

	QRect nul_rec = parent->gl->cellRect(1,0);
	QPoint nul_point = QPoint(0, nul_rec.y());
	QRect src_rect;
	QRect dst_rect;

	if (src_row > -1)
		src_rect = parent->gl->cellRect(src_row,0);
	if (dst_row > -1)
		dst_rect = parent->gl->cellRect(dst_row,0);

	if (src_flag == MIX_LEFT) {
		margin[0] = QPoint(MARGIN, 0);
		points[0] = QPoint(0,src_rect.y() + (src_rect.height() / 2)) - nul_point;
	} else {
		margin[0] = QPoint(-MARGIN, 0);
		points[0] = QPoint(w,src_rect.y() + (src_rect.height() / 2)) - nul_point;
	}
	if (dst_flag == MIX_LEFT) {
		margin[1] = QPoint(MARGIN, 0);
		points[1] = QPoint(0,dst_rect.y() + (dst_rect.height() / 2)) - nul_point;
	} else {
		margin[1] = QPoint(-MARGIN, 0);
		points[1] = QPoint(w,dst_rect.y() + (dst_rect.height() / 2)) - nul_point;
	}
	QPoint poly[4] = {points[0], points[0] + margin[0], points[1] + margin[1], points[1]};

	if (src_row > -1 && dst_row > -1)
		paint.drawPolyline(poly, 4);

	paint.setPen(QPen(color, 1));

	if (src_row > -1)
		paint.drawEllipse(QRect(poly[0].x() - (MARGIN/2), poly[0].y() - (MARGIN/2), MARGIN, MARGIN));
	if (dst_row > -1)
		paint.drawEllipse(QRect(poly[3].x() - (MARGIN/2), poly[3].y() - (MARGIN/2), MARGIN, MARGIN));
}

int
VOSSDevConnections :: getRxRow(int which)
{
	if (which < 0 || which >= (int)parent->n_master_input)
		return (-1);
	else
		return (which + 1);
}

int
VOSSDevConnections :: getTxRow(int which)
{
	if (which < 0 || which >= (int)parent->n_master_output)
		return (-1);
	else
		return (which + 1);
}

void
VOSSDevConnections :: paintEvent(QPaintEvent *event)
{
	VOSSController *pc;
	QPainter paint(this);
	int w = width();
	int h = height();
	int x;
	int y;

	paint.setRenderHints(QPainter::Antialiasing, 1);

	paint.fillRect(QRectF(0,0,w,h), Qt::white);

	for (y = 0; y != 2; y++) {
		for (x = 0; x != MAX_VOLUME_BAR && ((pc = parent->parent->vb[x]) != 0); x++) {
			switch(pc->type) {
			case VOSS_TYPE_DEVICE:
				drawNice(paint, MIX_LEFT, pc->connect_row,
				    MIX_RIGHT, getTxRow(pc->io_info.tx_chan), pc->io_info.tx_mute, y);
				drawNice(paint, MIX_LEFT, getRxRow(pc->io_info.rx_chan),
				    MIX_RIGHT, pc->connect_row, pc->io_info.rx_mute, y);
				break;
			case VOSS_TYPE_INPUT_MON:
				drawNice(paint, MIX_LEFT, getRxRow(pc->mon_info.src_chan),
				    MIX_RIGHT, getTxRow(pc->mon_info.dst_chan), pc->mon_info.mute, y);
				break;
			case VOSS_TYPE_OUTPUT_MON:
				drawNice(paint, MIX_RIGHT, getTxRow(pc->mon_info.src_chan),
				    MIX_RIGHT, getTxRow(pc->mon_info.dst_chan), pc->mon_info.mute, y);
				break;
			default:
				break;
			}
		}
	}
}

VOSSConnect :: VOSSConnect(VOSSMainWindow *mw)
{
	VOSSController *pc;
	uint32_t x;
	uint32_t n_row = 1;
	uint32_t n_loopback = 0;
	QLabel *lbl;

	parent = mw;

	n_master_input = 0;
	n_master_output = 0;

	gl = new QGridLayout(this);

	lbl = new QLabel(tr("Main Device Input"));
	gl->addWidget(lbl, 0, 0, 1, 1);

	lbl = new QLabel(tr("Main Device Output"));
	gl->addWidget(lbl, 0, 2, 1, 1);

	for (x = 0; x != MAX_VOLUME_BAR && ((pc = parent->vb[x]) != 0); x++) {
		switch(pc->type) {
		case VOSS_TYPE_MAIN_INPUT:
			pc->connect_row = n_row + n_master_input;

			pc->connect_input_label = new QLineEdit(QString("Channel %1").arg(pc->channel));
			gl->addWidget(pc->connect_input_label, pc->connect_row, 0, 1, 1);
			n_master_input++;
			break;
		case VOSS_TYPE_MAIN_OUTPUT:
			pc->connect_row = n_row + n_master_output;

			pc->connect_output_label = new QLineEdit(QString("Channel %1").arg(pc->channel));
			gl->addWidget(pc->connect_output_label, pc->connect_row, 2, 1, 1);
			n_master_output++;
			break;
		default:
			break;
		}
	}

	if (n_master_input < n_master_output)
		n_row += n_master_output;
	else
		n_row += n_master_input;

	for (x = 0; x != MAX_VOLUME_BAR && ((pc = parent->vb[x]) != 0); x++) {
		switch(pc->type) {
		case VOSS_TYPE_DEVICE:
			if (pc->channel == 0) {
				gl->addWidget(new QLabel(tr(pc->io_info.name) +
				    tr(" - input")), n_row, 0, 1, 1);
				gl->addWidget(new QLabel(tr(pc->io_info.name) +
				    tr(" - output")), n_row, 2, 1, 1);
				n_row++;
			}
			pc->connect_row = n_row;
			pc->connect_input_label = new QLineEdit(QString("Channel %1").arg(pc->channel));
			gl->addWidget(pc->connect_input_label, pc->connect_row, 0, 1, 1);
			pc->connect_output_label = new QLineEdit(QString("Channel %1").arg(pc->channel));
			gl->addWidget(pc->connect_output_label, pc->connect_row, 2, 1, 1);
			n_row++;
			break;
		case VOSS_TYPE_LOOPBACK:
			if (pc->channel == 0) {
				gl->addWidget(new QLabel(tr(pc->io_info.name) +
				    tr(" - input")), n_loopback, 4, 1, 1);
				n_loopback ++;
			}

			pc->connect_row = n_loopback;
			pc->connect_input_label = new QLineEdit(QString("Channel %1").arg(pc->channel));
			gl->addWidget(pc->connect_input_label, pc->connect_row, 4, 1, 1);
			n_loopback++;
			break;
		default:
			break;
		}
	}

	if (n_row < n_loopback)
		n_row = n_loopback;

	devconn = new VOSSDevConnections(this);
	gl->addWidget(devconn, 1, 1, n_row - 1, 1);

	loopconn = new VOSSLoopConnections(this);
	gl->addWidget(loopconn, 1, 3, n_row - 1, 1);

	gl->setColumnStretch(1, 1);
	gl->setColumnStretch(2, 1);
	gl->setRowStretch(n_row + 1, 1);

	setTitle(tr("Connection diagram"));
}

VOSSConnect :: ~VOSSConnect()
{

}
