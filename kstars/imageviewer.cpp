/***************************************************************************
                          imageviewer.cpp  -  An ImageViewer for KStars
                             -------------------
    begin                : Mon Aug 27 2001
    copyright            : (C) 2001 by Thomas Kabelmann
    email                : tk78@gmx.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include <qpainter.h>
#include <qfile.h>

#include <klocale.h>
#include <kurl.h>
#include <ktempfile.h>
#include <kmessagebox.h>
#include <kfiledialog.h>
#include <kio/netaccess.h>
#include <ktoolbar.h>
#include <kaction.h>
#include <kaccel.h>
#include <kapp.h>
#include <kiconloader.h>
#include "imageviewer.h"

ImageViewer::ImageViewer (const KURL *url, QWidget *parent, const char *name)
	: KMainWindow (parent, name), imageURL (*url), downloadComplete (false), fileIsImage (false),
	  ctrl (false), key_s (false), key_q (false)
{
// the parent-widget has to implement a new signal closeAllWindows() and must emit this signal in closeEvent()
	connect (this, SIGNAL (StartDownload()), parent, SLOT (setUpDownloads()));
	connect (this, SIGNAL (DownloadComplete()), parent, SLOT (setDownDownloads()));

// toolbar can dock only on top-position and can't be minimized
// JH: easier to just disable its mobility
	toolBar()->enableMoving( false );

//JH: don't need a new KToolbar...KMainWindow has one automatically at toolBar()
//	KToolBar *toolbar = new KToolBar (this, "toolbar");
//	new KToolBar (this, "toolbar");
	KAction *action = new KAction (i18n ("Close Window"), BarIcon( "fileclose" ), KAccel::stringToKey( "Ctrl+Q" ), this, SLOT (close()), actionCollection());
	action->plug (toolBar());
	action = new KAction (i18n ("Save Image"), BarIcon( "filesave" ), KAccel::stringToKey( "Ctrl+S" ), this, SLOT (saveFileToDisc()), actionCollection());
	action->plug (toolBar());

	if (imageURL.isMalformed())		//check URL
		qDebug ("URL is malformed");
	setCaption (imageURL.filename()); // the title of the window
	loadImageFromURL();
}

ImageViewer::~ImageViewer(){
//	qDebug ("destructor");
// remove the tempfile
	if (!file->remove())		// if the file was not complete downloaded the suffix is  ".part"
	{
		qDebug ("remove of %s failed", file->name().local8Bit().data());
		file->setName (file->name() + ".part");		// set new suffix to filename
		qDebug ("try to remove %s", file->name().local8Bit().data());
		if (file->remove())
			qDebug ("file removed");
		else
			qDebug ("file not removed");	// this should never happens
	}
}

void ImageViewer::paintEvent (QPaintEvent *ev)
{
	bitBlt (this, 0, toolBar()->height() + 1, &pix);
}

void ImageViewer::resizeEvent (QResizeEvent *ev)
{
	if (downloadComplete)
		pix = kpix.convertToPixmap ( image.smoothScale ( size().width() , size().height() - toolBar()->height() ) );	// convert QImage to QPixmap (fastest method)
}

void ImageViewer::closeEvent (QCloseEvent *ev)
{
	if (ev)	// not if closeEvent (0) is called, because segfault
		ev->accept();	// parent-widgets should not get this event, so it will be filtered
	this->~ImageViewer();	// destroy the object, so the object can only allocated with operator new, not as a global/local variable
}


void ImageViewer::keyPressEvent (QKeyEvent *ev)
{
	ev->accept();
	switch (ev->key())
	{
		case Key_Control : ctrl = true; break;
		case Key_Q : key_q = true; break;
		case Key_S : key_s = true; break;
		default : ev->ignore();
	}
	if (ctrl && key_q)
		close();
	if (ctrl && key_s)
	{
		ctrl = false;
		key_s = false;
		saveFileToDisc();
	}
}

void ImageViewer::keyReleaseEvent (QKeyEvent *ev)
{
	ev->accept();
	switch (ev->key())
	{
		case Key_Control : ctrl = false; break;
		case Key_Q : key_q = false; break;
		case Key_S : key_s = false; break;
		default : ev->ignore();
	}
}

void ImageViewer::loadImageFromURL()
{
	file = tempfile.file();
	tempfile.unlink();		// we just need the name and delete the tempfile from disc; if we don't do it, a dialog will be shown
	KURL saveURL (file->name());
	if (saveURL.isMalformed())
		qDebug ("tempfile-URL is malformed");

	emit StartDownload();
	KIO::Job *downloadJob = KIO::copy (imageURL, saveURL);	// starts the download asynchron
	connect (downloadJob, SIGNAL (result (KIO::Job *)), SLOT (downloadReady (KIO::Job *)));
}

void ImageViewer::downloadReady (KIO::Job *job)
{
	emit DownloadComplete();
	if (job->error())
	{
		job->showErrorDialog();
		closeEvent (0);
		return;		// exit this function
	}

	file->close(); // to get the newest informations of the file and not any informations from opening of the file

	if (file->exists())
	{
		downloadComplete = true;
		showImage();
		return;
	}
	closeEvent (0);
}

void ImageViewer::showImage()
{
	if (!image.load (file->name()))		// if loading failed
	{
		QString text = i18n ("Loading of the image %1 failed!");
		KMessageBox::error (this, text.arg (imageURL.prettyURL() ));
		closeEvent (0);
		return;
	}
	fileIsImage = true;	// we loaded the file and know now, that it is an image

	int w = kapp->desktop()->width();	// screen width
	int h = 0.9*kapp->desktop()->height();	// 90% of screen height (accounts for taskbar)
	if (image.width() <= w && image.height() <= h)
		resize ( image.width(), image.height() + toolBar()->height());	// the 32 pixel are the space of the toolbar

//If the image is larger than screen width and/or screen height, shrink it to fit the screen
//while preserving the original aspect ratio

	else if (image.width() <= w) //only the height is too large
		resize ( int( image.width()*h/image.height() ), h );
	else if (image.height() <= h) //only the width is too large
		resize ( w, int( image.height()*w/image.width() ) );
	else { //uh-oh...both width and height are too large.  which needs to be shrunk least?
		float fx = float(w)/float(image.width());
		float fy = float(h)/float(image.height());
    if (fx > fy) //width needs to be shrunk less, so shrink to fit in height
			resize ( int( image.width()*fy ), h );
		else //vice versa
			resize ( w, int( image.height()*fx ) );
	}

	show();	// hide is default
// pix will be initialized in resizeEvent(), which will automatically called first time
}

void ImageViewer::saveFileToDisc()
{
	KURL newURL = KFileDialog::getSaveURL(imageURL.filename());  // save-dialog with default filename
	if (!newURL.isEmpty())
	{
		QFile f (newURL.directory() + "/" +  newURL.filename());
		qDebug ("Saving to %s", f.name().local8Bit().data());
		if (f.exists())
		{
			qDebug ("Warning! Remove existing file %s", f.name().local8Bit().data());
			f.remove();
		}
		saveFile (newURL);
	}
}

void ImageViewer::saveFile (KURL &url)
{
// synchronous Access to prevent segfaults
	if (!KIO::NetAccess::copy (KURL (file->name()), url))
	{
		QString text = i18n ("Saving of the image %1 failed!");
		KMessageBox::error (this, text.arg (url.prettyURL() ));
	}
}

void ImageViewer::close()
{
	closeEvent (0);
}
#include "imageviewer.moc"
