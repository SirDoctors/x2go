//
// C++ Interface: printprocess
//
// Description:
//
//
// Author: Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>, (C) 2009
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef PRINTPROCESS_H
#define PRINTPROCESS_H

#include <QString>
#include <QObject>
#include <QProcess>

/**
	@author Oleksandr Shneyder <oleksandr.shneyder@obviously-nice.de>
*/
class PrintProcess: public QObject
{
		Q_OBJECT

	public:
		PrintProcess ( QString pdf, QString title,
		               QObject * parent=0l );
		~PrintProcess();
	private:
		QString pdfFile;
		QString pdfTitle;
		QString psFile;
		bool viewPdf;
		bool customPrintCmd;
		bool printStdIn;
		bool printPs;
		bool pdfOpen;
		QString pdfOpenCmd;
		QWidget* parentWidg;
		QString printCmd;
#ifdef Q_OS_WIN
		QString winX2goPrinter;
		QString winDefaultPrinter;
#endif
	private:
		bool loadSettings();
	private slots:
		void slot_processFinished ( int exitCode,
		                            QProcess::ExitStatus exitStatus );
		void slot_pdf2psError ( QProcess::ProcessError error ) ;
		void slot_error ( QProcess::ProcessError error );
		void openPdf();
		void print();
};

#endif