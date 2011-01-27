/***************************************************************************
*   Copyright (C) 2005 by Oleksandr Shneyder   *
*   oleksandr.shneyder@obviously-nice.de   *
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  F*
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
*   This program is distributed in the hope that it will be useful,       *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
*   GNU General Public License for more details.                          *
*                                                                         *
*   You should have received a copy of the GNU General Public License     *
*   along with this program; if not, write to the                         *
*   Free Software Foundation, Inc.,                                       *
*   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
***************************************************************************/
#include <QTextStream>
#include "version.h"
#include "x2goclientconfig.h"
#include "onmainwindow.h"
#include "userbutton.h"
#include "exportdialog.h"
#include "printprocess.h"
#include <QDesktopServices>
#include <QApplication>
#include <QScrollBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFile>


#include <QTimer>
#include <QComboBox>
#include <QMessageBox>
#include <QProcess>
#include <QGroupBox>
#include <QMessageBox>
#include <QTextEdit>
#include <QDesktopWidget>
#include <QLineEdit>
#include <QLabel>
#include <QScrollArea>
#include <QAction>
#include <QMenu>
#include <QMenuBar>
#include <QToolBar>
#include <QShortcut>
#include <QSettings>
#include <QStatusBar>
#include <QInputDialog>
#include <QDir>
#include <unistd.h>
#include <QTreeView>
#include <QStandardItem>
#include <QStandardItemModel>
#include <QHeaderView>
#include <QCheckBox>
#include <QTemporaryFile>
#include <QFileDialog>
#include <QtNetwork/QTcpSocket>
#include <QPlastiqueStyle>
#include "sshprocess.h"
#include "imgframe.h"
#include <QToolTip>
#include "clicklineedit.h"

#if !defined Q_OS_WIN
#include <sys/mount.h>
#ifdef Q_OS_LINUX
#include <linux/fs.h>
#endif // Q_OS_LINUX
#endif // !defined Q_OS_WIN

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <QCoreApplication>

#include <QDesktopWidget>
#include "embedwidget.h"

#define ldap_SUCCESS 0
#define ldap_INITERROR 1
#define ldap_OPTERROR 2
#define ldap_BINDERROR 3
#define ldap_SEARCHERROR 4
#define ldap_NOBASE 5



//LDAP attributes
#define SESSIONID "sn"
#define USERNAME  "cn"
#define CLIENT    "registeredAddress"
#define SERVER    "postalAddress"
#define RES       "title"
#define DISPLAY   "street"
#define STATUS    "st"
#define STARTTIME "telephoneNumber"
#define CREATTIME "telexNumber"
#define SUSPTIME  "internationaliSDNNumber"

#define SESSIONCMD "o"
#define FIRSTUID "ou"
#define LASTUID "l"

#define SNDSUPPORT "sn"
#define NETSOUNDSYSTEM "o"
#define SNDSUPPORT "sn"
#define SNDPORT   "ou"
#define STARTSNDSERVER "title"



#include <QDateTime>

#include "SVGFrame.h"
#include "configdialog.h"
#include "editconnectiondialog.h"
#include "sessionbutton.h"
#include "sessionmanagedialog.h"
#include "x2gologdebug.h"
#include <QMouseEvent>

#ifdef Q_OS_WIN
#include "wapi.h"
#endif

void x2goSession::operator = ( const x2goSession& s )
{
	agentPid=s.agentPid;
	clientIp=s.clientIp;
	cookie=s.cookie;
	crTime=s.crTime;
	display=s.display;
	grPort=s.grPort;
	server=s.server;
	sessionId=s.sessionId;
	sndPort=s.sndPort;
	fsPort=s.fsPort;
	status=s.status;
}

ONMainWindow::ONMainWindow ( QWidget *parent ) :QMainWindow ( parent )
{
	drawMenu=true;
	usePGPCard=false;
	extLogin=false;
	startMaximized=false;
	startHidden=false;
	defaultUseSound=true;
	defaultSetKbd=true;
	defaultSetDPI=false;
	defaultDPI=96;
	extStarted=false;
	defaultLink=2;
	defaultFullscreen=false;
	acceptRsa=false;
	cardStarted=false;
	cardReady=false;
	useSshAgent=false;
	miniMode=false;
	embedMode=false;
	proxyWinEmbedded=false;
	proxyWinId=0;
	embedParent=embedChild=0l;
	defaultSession=false;
	defaultUser=false;
	defaultWidth=800;
	defaultHeight=600;
	defaultPack="16m-jpeg";
	defaultQuality=9;
	defaultLayout=tr ( "us" );
	defaultKbdType=tr ( "pc105/us" );
	defaultCmd="KDE";
	defaultSshPort=sshPort=clientSshPort="22";
#ifdef Q_OS_WIN
	clientSshPort="7022";
	pulsePort=4713;
#endif
	appDir=QApplication::applicationDirPath();
	homeDir=QDir::homePath();

#ifdef Q_OS_WIN
	pulseServer=0l;
	xorg=0l;
	xDisplay=0;
#endif
	cleanAskPass();
	setWindowTitle ( tr ( "X2Go client" ) );
	ld=0;
	tunnel=0l;
	sndTunnel=0l;
	fsTunnel=0l;
	nxproxy=0l;
	soundServer=0l;
	scDaemon=0l;
	gpgAgent=0l;
	statusLabel=0;
	gpg=0l;
	restartResume=false;
	isPassShown=true;
	readExportsFrom=QString::null;
	spoolTimer=0l;
	ldapOnly=false;
	embedControlChanged=false;
	statusString=tr ( "connecting" );


	hide();
	kdeIconsPath=getKdeIconsPath();

	addToAppNames ( "WWWBROWSER",tr ( "Internet browser" ) );
	addToAppNames ( "MAILCLIENT",tr ( "Email client" ) );
	addToAppNames ( "OFFICE",tr ( "OpenOffice.org" ) );
	addToAppNames ( "TERMINAL",tr ( "Terminal" ) );





#ifndef Q_OS_LINUX
	widgetExtraStyle =new QPlastiqueStyle();
#endif

	QDesktopWidget wd;
// 	x2goDebug<<"Desktop geometry:"<<wd.screenGeometry();
	if ( wd.screenGeometry().width() <1024 ||
	        wd.screenGeometry().height() <768 )
	{
		miniMode=true;
		x2goDebug<<"Switching to \"mini\" mode";
	}


	agentCheckTimer=new QTimer ( this );
	connect ( agentCheckTimer,SIGNAL ( timeout() ),this,
	          SLOT ( slot_checkAgentProcess() ) );


	loadSettings();
	QStringList args=QCoreApplication::arguments();
	for ( int i=1;i<args.size();++i )
	{
		if ( !parseParam ( args[i] ) )
		{
			close();
			exit ( -1 );
		}
	}


	if ( embedMode )
	{
		miniMode=false;
		useLdap=false;
	}

	if ( readExportsFrom!=QString::null )
	{
		exportTimer=new QTimer ( this );
		connect ( exportTimer,SIGNAL ( timeout() ),this,
		          SLOT ( slot_exportTimer() ) );
	}
	if ( extLogin )
	{
		extTimer=new QTimer ( this );
		extTimer->start ( 2000 );
		connect ( extTimer,SIGNAL ( timeout() ),this,
		          SLOT ( slotExtTimer() ) );
	}

	if ( startMaximized )
	{
		QTimer::singleShot ( 10, this, SLOT ( slot_resize() ) );
	}

	if ( usePGPCard )
	{
		QTimer::singleShot ( 10, this, SLOT ( slot_startPGPAuth() ) );
	}


	//fr=new SVGFrame(QString::null,true,this);
	fr=new IMGFrame ( ( QImage* ) 0l,this );
	setCentralWidget ( fr );

#ifndef Q_WS_HILDON
	bgFrame=new SVGFrame ( ( QString ) ":/svg/bg.svg",true,fr );
#else
	bgFrame=new SVGFrame ( ( QString ) ":/svg/bg_hildon.svg",true,fr );
#endif
	//bgFrame=new SVGFrame((QString)"/home/admin/test.svg",false,fr);


	SVGFrame* x2g=new SVGFrame ( ( QString ) ":/svg/x2gologo.svg",
	                             false,fr );

	QPalette pl=x2g->palette();
	pl.setColor ( QPalette::Base, QColor ( 255,255,255,0 ) );
	pl.setColor ( QPalette::Window, QColor ( 255,255,255,0 ) );
	x2g->setPalette ( pl );

	SVGFrame* on=new SVGFrame ( ( QString ) ":/svg/onlogo.svg",false,fr );
	on->setPalette ( pl );

	if ( !miniMode )
	{
		x2g->setFixedSize ( 100,100 );
		on->setFixedSize ( 100,80 );
	}
	else
	{
		x2g->setFixedSize ( 50,50 );
		on->setFixedSize ( 50,40 );
	}

	mainL=new QHBoxLayout ( fr );
	QVBoxLayout* onlay=new QVBoxLayout();
	onlay->addStretch();
	onlay->addWidget ( on );

	QVBoxLayout* x2golay=new QVBoxLayout();
	x2golay->addStretch();
	x2golay->addWidget ( x2g );


	QHBoxLayout* bgLay=new QHBoxLayout ( bgFrame );
	bgLay->setSpacing ( 0 );
	bgLay->setMargin ( 0 );
	bgLay->addLayout ( onlay );
	bgLay->addStretch();
	username=new QHBoxLayout();
	bgLay->addLayout ( username );
	if ( embedMode )
		bgLay->addStretch();
	bgLay->addLayout ( x2golay );



	act_set=new QAction (
	    QIcon ( iconsPath ( "/32x32/edit_settings.png" ) ),
	    tr ( "&Settings ..." ),this );

	act_abclient=new QAction ( QIcon ( ":icons/32x32/x2goclient.png" ),
	                           tr ( "About X2GO client" ),this );



	connect ( act_set,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slot_config() ) );
	connect ( act_abclient,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slot_about() ) );


#ifdef Q_OS_DARWIN
	embedMode=false;
#endif

#ifdef Q_OS_WIN
	winServersReady=false;
	startWinServers();
#endif

	initPassDlg();
	initSelectSessDlg();
	initStatusDlg();

	if ( !embedMode )
	{
		initWidgetsNormal();
	}
	else
	{
		initWidgetsEmbed();
	}
	mainL->setSpacing ( 0 );
	mainL->setMargin ( 0 );
	mainL->insertWidget ( 0, bgFrame );
	hide();
	QTimer::singleShot ( 1, this, SLOT ( slot_resize() ) );
	connect ( fr,SIGNAL ( resized ( const QSize ) ),this,
	          SLOT ( slot_resize ( const QSize ) ) );

	slot_resize ( fr->size() );
}

ONMainWindow::~ONMainWindow()
{
}

void ONMainWindow::initWidgetsEmbed()
{
	stb=new QToolBar ( this );
	addToolBar ( stb );
	stb->toggleViewAction()->setEnabled ( false );
	stb->toggleViewAction()->setVisible ( false );
	stb->setFloatable ( false );
	stb->setMovable ( false );
	statusBar()->setSizeGripEnabled ( false );
	statusBar()->hide();
	proxyWinTimer=new QTimer ( this );
	connect ( proxyWinTimer, SIGNAL ( timeout() ), this,
	          SLOT ( slotFindProxyWin() ) );

	act_shareFolder=new QAction ( QIcon ( ":icons/32x32/file-open.png" ),
	                              tr ( "Share folder..." ),this );
//  	act_shareFolder->setToolTip ( "Share folder" );

	act_suspend=new QAction ( QIcon ( ":icons/32x32/suspend.png" ),
	                          tr ( "Suspend" ),this );
//  	act_suspend->setToolTip (tr( "Suspend session" ));

	act_terminate=new QAction ( QIcon ( ":icons/32x32/stop.png" ),
	                            tr ( "Terminate" ),this );
//  	act_terminate->setToolTip(tr("Terminate session"))

	act_embedContol=new QAction ( QIcon ( ":icons/32x32/detach.png" ),
	                              tr ( "Detach X2Go window" ),this );

	act_embedToolBar=new QAction ( QIcon ( ":icons/32x32/tbhide.png" ),
	                               tr ( "Minimize toolbar" ),this );


	setEmbedSessionActionsEnabled ( false );

	connect ( act_shareFolder,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slot_exportDirectory() ) );

	connect ( act_suspend,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slotSuspendSessFromSt() ) );

	connect ( act_terminate,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slotTermSessFromSt() ) );

	connect ( act_embedContol,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slotEmbedControlAction() ) );

	connect ( act_embedToolBar,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slotEmbedToolBar() ) );

	processSessionConfig();
	EmbedWidget::initWidgets();
	QTimer::singleShot ( 1, this, SLOT ( slotEmbedIntoParentWindow() ) );
#ifndef Q_OS_WIN
	QSettings st ( homeDir +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	embedTbVisible=!st.value ( "embedded/tbvisible", true ).toBool();
	slotEmbedToolBar();
	showTbTooltip=false;
	if ( !embedTbVisible )
	{
		showTbTooltip=true;
		QTimer::singleShot ( 500, this,
		                     SLOT ( slotEmbedToolBarToolTip() ) );
		QTimer::singleShot ( 3000, this,
		                     SLOT ( slotHideEmbedToolBarToolTip() ) );
	}


	slotSelectedFromList ( ( SessionButton* ) 0 );
#ifdef Q_OS_LINUX
	QTimer::singleShot ( 500, this,
	                     SLOT ( slotActivateWindow() ) );
#endif
#ifdef Q_OS_WIN
	if ( embedMode )
	{
		QRect r;
		wapiWindowRect (
		    stb->widgetForAction ( act_embedToolBar )->winId(),r );
	}
#endif

}

void ONMainWindow::initWidgetsNormal()
{
	username->setSpacing ( 10 );
	username->addStretch();
	username->addStretch();
	ln=new SVGFrame ( ( QString ) ":/svg/line.svg",true,fr );
	ln->setFixedWidth ( ln->sizeHint().width() );
	uname=new QLineEdit ( bgFrame );
	setWidgetStyle ( uname );

	uname->hide();
	uname->setFrame ( false );
	u=new QLabel ( tr ( "Session:" ),bgFrame );
	u->hide();
	QFont fnt=u->font();
	fnt.setPointSize ( 16 );
#ifndef Q_WS_HILDON
	if ( miniMode )
	{
		fnt.setPointSize ( 12 );
	}
#endif

	u->setFont ( fnt );

	connect ( uname,SIGNAL ( returnPressed() ),this,
	          SLOT ( slotUnameEntered() ) );

	QPalette pal=u->palette();
	pal.setColor ( QPalette::WindowText,
	               QColor ( 200,200,200,255 ) );
	u->setPalette ( pal );
	uname->setFont ( fnt );
	pal=uname->palette();
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Text, QColor ( 200,200,200,255 ) );
	uname->setPalette ( pal );

	u->show();
	uname->show();

	users=new QScrollArea ( fr );
	pal=users->verticalScrollBar()->palette();
	pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
	pal.setBrush ( QPalette::Base, QColor ( 110,112,127,255 ) );
	pal.setBrush ( QPalette::Button, QColor ( 110,112,127,255 ) );
	users->verticalScrollBar()->setPalette ( pal );
	users->setFrameStyle ( QFrame::Plain );
	users->setFocusPolicy ( Qt::NoFocus );


	pal=users->palette();
	pal.setBrush ( QPalette::Window, QColor ( 110,112,127,255 ) );
	users->setPalette ( pal );
	users->setWidgetResizable ( true );

	uframe=new QFrame();
	users->setWidget ( uframe );

	mainL->insertWidget ( 1, ln );
	mainL->addWidget ( users );

	QAction *act_exit=new QAction (
	    QIcon ( iconsPath ( "/32x32/exit.png" ) ),
	    tr ( "&Quit" ),this );
	act_exit->setShortcut ( tr ( "Ctrl+Q" ) );
	act_exit->setStatusTip ( tr ( "Quit" ) );

	act_new=new QAction ( QIcon ( iconsPath ( "/32x32/new_file.png" ) ),
	                      tr ( "&New session ..." ),this );
	act_new->setShortcut ( tr ( "Ctrl+N" ) );



	setWindowIcon ( QIcon ( ":icons/128x128/x2go.png" ) );
	act_edit=new QAction ( QIcon ( iconsPath ( "/32x32/edit.png" ) ),
	                       tr ( "Session management..." ),this );
	act_edit->setShortcut ( tr ( "Ctrl+E" ) );

	act_sessicon=new QAction ( QIcon ( iconsPath ( "/32x32/create_file.png" ) ),
	                           tr ( "&Create session icon on desktop..." ),this );


	QAction *act_tb=new QAction ( tr ( "Show toolbar" ),this );
	act_tb->setCheckable ( true );
	act_tb->setChecked ( showToolBar );



	QAction *act_abqt=new QAction ( tr ( "About Qt" ),this );

	connect ( act_abqt,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slot_about_qt() ) );
	connect ( act_new,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slotNewSession() ) );
	connect ( act_sessicon,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slotCreateSessionIcon() ) );
	connect ( act_edit,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( slot_manage() ) );
	connect ( act_exit,SIGNAL ( triggered ( bool ) ),this,
	          SLOT ( close() ) );
	connect ( act_tb,SIGNAL ( toggled ( bool ) ),this,
	          SLOT ( displayToolBar ( bool ) ) );
	stb=addToolBar ( tr ( "Show toolbar" ) );

	QShortcut* ex=new QShortcut ( QKeySequence ( tr ( "Ctrl+Q","exit" ) ),
	                              this );
	connect ( ex,SIGNAL ( activated() ),this,SLOT ( close() ) );

	if ( drawMenu )
	{
		QMenu* menu_sess=menuBar()->addMenu ( tr ( "&Session" ) );
		QMenu* menu_opts=menuBar()->addMenu ( tr ( "&Options" ) );

		menu_sess->addAction ( act_new );
		menu_sess->addAction ( act_edit );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
		menu_sess->addAction ( act_sessicon );
#endif
		menu_sess->addSeparator();
		menu_sess->addAction ( act_exit );
		menu_opts->addAction ( act_set );
		menu_opts->addAction ( act_tb );

		QMenu* menu_help=menuBar()->addMenu ( tr ( "&Help" ) );
		menu_help->addAction ( act_abclient );
		menu_help->addAction ( act_abqt );

		stb->addAction ( act_new );
		stb->addAction ( act_edit );
#if (!defined Q_WS_HILDON) && (!defined Q_OS_DARWIN)
		stb->addAction ( act_sessicon );
#endif
		stb->addSeparator();
		stb->addAction ( act_set );

		if ( !showToolBar )
			stb->hide();
		connect ( act_tb,SIGNAL ( toggled ( bool ) ),stb,
		          SLOT ( setVisible ( bool ) ) );
	}
	else
		stb->hide();

#if !defined USELDAP

	useLdap=false;
#endif

	if ( useLdap )
	{
		act_new->setEnabled ( false );
		act_edit->setEnabled ( false );
		u->setText ( tr ( "Login:" ) );
		QTimer::singleShot ( 1500, this, SLOT ( readUsers() ) );
	}
	else
		QTimer::singleShot ( 1, this, SLOT ( slot_readSessions() ) );
	QTimer* t=new QTimer ( this );
	connect ( t,SIGNAL ( timeout() ),this,SLOT ( slot_rereadUsers() ) );
	t->start ( 20000 );
#ifdef Q_OS_WIN
	proxyWinTimer=new QTimer ( this );
	connect ( proxyWinTimer, SIGNAL ( timeout() ), this,
	          SLOT ( slotFindProxyWin() ) );
#endif

}



QString ONMainWindow::findTheme ( QString /*theme*/ )
{
	return QString::null;
}

QString ONMainWindow::getKdeIconsPath()
{
	return ":/icons";
}

void ONMainWindow::slot_resize ( const QSize sz )
{
	if ( startHidden )
	{
		return;
	}
	int height;
	int usize;
	height=sz.height();
	if ( !embedMode )
	{
		if ( !miniMode )
		{
			usize=sz.width()-800;
			if ( usize<360 )
				usize=360;
			if ( usize>500 )
				usize=500;
		}
		else
		{
			usize=285;
		}

		if ( users->width() !=usize )
		{
			users->setFixedWidth ( usize );
			if ( useLdap )
			{
				QList<UserButton*>::iterator it;
				QList<UserButton*>::iterator end=names.end();
				for ( it=names.begin();it!=end;it++ )
				{
					if ( !miniMode )
						( *it )->move (
						    ( usize-360 ) /2,
						    ( *it )->pos().y() );
					else
						( *it )->move (
						    ( usize-250 ) /2,
						    ( *it )->pos().y() );
				}
			}
			else
			{
				QList<SessionButton*>::iterator it;
				QList<SessionButton*>::iterator end=
				    sessions.end();
				for ( it=sessions.begin();it!=end;it++ )
				{
					if ( !miniMode )
						( *it )->move (
						    ( usize-360 ) /2,
						    ( *it )->pos().y() );
					else
						( *it )->move (
						    ( usize-250 ) /2,
						    ( *it )->pos().y() );
				}
			}
		}
		u->setFixedWidth ( u->sizeHint().width() );

		int bwidth=bgFrame->width();
		int upos= ( bwidth-u->width() ) /2;
		if ( upos<0 )
			upos=0;
		int rwidth=bwidth- ( upos+u->width() +5 );
		if ( rwidth<0 )
			rwidth=1;
		uname->setMinimumWidth ( rwidth );
		u->move ( upos,height/2 );
		uname->move ( u->pos().x() +u->width() +5,u->pos().y() );
	}
}

void ONMainWindow::closeEvent ( QCloseEvent* event )
{
	if ( !startMaximized && !startHidden && !embedMode )
	{
#ifndef Q_OS_WIN
		QSettings st ( homeDir +"/.x2goclient/sizes",
		               QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sizes" );
#endif

		st.setValue ( "mainwindow/size",QVariant ( size() ) );
		st.setValue ( "mainwindow/pos",QVariant ( pos() ) );
		st.setValue ( "mainwindow/maximized",
		              QVariant ( isMaximized() ) );
		st.sync();
	}
	QMainWindow::closeEvent ( event );
	if ( nxproxy!=0l )
	{
		if ( nxproxy->state() ==QProcess::Running )
			nxproxy->terminate();
	}
	if ( tunnel!=0l )
		delete tunnel;
	if ( sndTunnel!=0l )
		delete sndTunnel;
	if ( fsTunnel!=0l )
		delete fsTunnel;
	if ( soundServer )
		delete soundServer;
	if ( gpgAgent!=0l )
	{
		if ( gpgAgent->state() ==QProcess::Running )
			gpgAgent->terminate();
	}
	if ( useSshAgent )
		finishSshAgent();
	if ( agentPid.length() >0 )
	{
		if ( checkAgentProcess() )
		{
			QStringList arg;
			arg<<"-9"<<agentPid;
			QProcess::execute ( "kill",arg );
		}
	}
#ifdef Q_OS_WIN
	if ( pulseServer )
	{
		pulseServer->kill();
		delete pulseServer;
		QDir dr ( homeDir );
		dr.remove ( pulseDir+"/config.pa" );
		dr.remove ( pulseDir+"/pulse-pulseuser/pid" );
		dr.rmdir ( pulseDir+"/pulse-pulseuser" );
		dr.rmdir ( pulseDir );
	}
	if ( xorg )
	{
		xorg->terminate();
		delete xorg;
	}

	TerminateProcess ( sshd.hProcess,0 );
	CloseHandle ( sshd.hProcess );
	CloseHandle ( sshd.hThread );

#endif
	if ( embedMode )
	{
		passForm->close();
		selectSessionDlg->close();
		closeEmbedWidget();
	}
}

void ONMainWindow::loadSettings()
{

#ifndef Q_OS_WIN
	QSettings st ( homeDir +"/.x2goclient/sizes",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sizes" );
#endif

	mwSize=st.value ( "mainwindow/size",
	                  ( QVariant ) QSize ( 800,600 ) ).toSize();
	mwPos=st.value ( "mainwindow/pos",
	                 ( QVariant ) QPoint ( 20,20 ) ).toPoint();
	mwMax=st.value ( "mainwindow/maximized", ( QVariant ) false ).toBool();

#ifndef Q_OS_WIN

	QSettings st1 ( homeDir +"/.x2goclient/settings",
	                QSettings::NativeFormat );
#else

	QSettings st1 ( "Obviously Nice","x2goclient" );
	st1.beginGroup ( "settings" );
#endif

	if ( !ldapOnly )
	{
		useLdap=st1.value ( "LDAP/useldap",
		                    ( QVariant ) false ).toBool();
		ldapServer=st1.value ( "LDAP/server",
		                       ( QVariant ) "localhost" ).toString();
		ldapPort=st1.value ( "LDAP/port", ( QVariant ) 389 ).toInt();
		ldapDn=st1.value ( "LDAP/basedn",
		                   ( QVariant ) QString::null ).toString();
		ldapServer1=st1.value ( "LDAP/server1",
		                        ( QVariant ) QString::null ).toString();
		ldapPort1=st1.value ( "LDAP/port1",
		                      ( QVariant ) 0 ).toInt();
		ldapServer2=st1.value ( "LDAP/server2",
		                        ( QVariant ) QString::null ).toString();
		ldapPort2=st1.value ( "LDAP/port2", ( QVariant ) 0 ).toInt();
	}
#ifndef Q_OS_WIN
	clientSshPort=st1.value ( "clientport", ( QVariant ) 22 ).toString();
#endif
	showToolBar=st1.value ( "toolbar/show", ( QVariant ) true ).toBool();

}
QString ONMainWindow::iconsPath ( QString fname )
{
	/*    QFile fl(this->kdeIconsPath+fname);
		if(fl.exists())
		return kdeIconsPath+fname;*/
	return ( QString ) ":/icons"+fname;
}

void ONMainWindow::displayUsers()
{

	QPixmap pix;
	if ( !miniMode )
		pix=QPixmap ( ":/png/ico.png" );
	else
		pix=QPixmap ( ":/png/ico_mini.png" );
	QPixmap foto=QPixmap ( iconsPath ( "/64x64/personal.png" ) );

	QPalette pal=palette();
	pal.setBrush ( QPalette::Window, QBrush ( pix ) );
	pal.setBrush ( QPalette::Base, QBrush ( pix ) );
	pal.setBrush ( QPalette::Button, QBrush ( pix ) );
	QFont fnt=font();
	fnt.setPointSize ( 12 );
	uframe->setFont ( fnt );
	QList<user>::iterator it;
	QList<user>::iterator end=userList.end();
	int i=0;
	for ( it=userList.begin();it!=end;it++ )
	{
		int val=i+1;
		UserButton* l;
		if ( ( *it ).foto.isNull() )
			l=new UserButton ( this, uframe, ( *it ).uid,
			                   ( *it ).name,foto,pal );
		else
			l=new UserButton ( this, uframe, ( *it ).uid,
			                   ( *it ).name, ( *it ).foto,pal );
		connect ( l,SIGNAL ( userSelected ( UserButton* ) ),this,
		          SLOT ( slotSelectedFromList ( UserButton* ) ) );
		if ( !miniMode )
			l->move ( ( users->width()-360 ) /2,
			          i*120+ ( val-1 ) *25+5 );
		else
			l->move ( ( users->width()-260 ) /2,
			          i*120+ ( val-1 ) *25+5 );
		l->show();
		names.append ( l );
		i++;
	}
	int val=i;
	uframe->setFixedHeight ( val*120+ ( val-1 ) *25 );
	uname->setText ( "" );
	disconnect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
	             SLOT ( slotSnameChanged ( const QString& ) ) );
	connect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
	          SLOT ( slotUnameChanged ( const QString& ) ) );
}

void ONMainWindow::showPass ( UserButton* user )
{
	QPalette pal=users->palette();
	setUsersEnabled ( false );
	QString fullName;
	QPixmap foto;
	if ( user )
	{
		foto=user->foto();
		nick=user->username();
		fullName=user->fullName();
		user->hide();
		lastUser=user;
	}
	else
	{
		lastUser=0;
		foto.load ( iconsPath ( "/64x64/personal.png" ) );
		foto=foto.scaled ( 100,100 );
		nick=uname->text();
		fullName="User Unknown";
	}

	fotoLabel->setPixmap ( foto );

	QString text="<b>"+nick+"</b><br>("+fullName+")";
	nameLabel->setText ( text );
	login->setText ( nick );
	login->hide();

	pass->setEchoMode ( QLineEdit::Password );
	pass->setFocus();
	slot_showPassForm();
}



void ONMainWindow::slotSelectedFromList ( UserButton* user )
{
	showPass ( user );
}

void ONMainWindow::slotClosePass()
{
	passForm->hide();
	if ( !embedMode )
	{
		u->show();
		uname->show();
		if ( useLdap )
		{
			if ( lastUser )
			{
				lastUser->show();
				uname->setText ( lastUser->username() );
			}
		}
		else
		{
			lastSession->show();
			uname->setText ( lastSession->name() );
		}
		uname->setEnabled ( true );
		u->setEnabled ( true );
		setUsersEnabled ( true );
		uname->selectAll();
		uname->setFocus();
	}
}


void ONMainWindow::slotPassEnter()
{
#if defined ( Q_OS_WIN ) || defined (Q_OS_DARWIN )
	QString disp=getXDisplay();
	if ( disp==QString::null )
		return;
#endif
#ifdef USELDAP

	if ( ! initLdapSession() )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "Please check LDAP settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );

		slot_config();
		return;
	}

	passForm->setEnabled ( false );

	x2goServers.clear();

	list<string> attr;
	attr.push_back ( "cn" );
	attr.push_back ( "serialNumber" );
	list<LDAPStringEntry> res;
	QString searchBase="ou=Servers,ou=ON,"+ldapDn;

	try
	{
		ld->stringSearch ( searchBase.toStdString(),attr,
		                   "objectClass=ipHost",res );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}
	if ( res.size() ==0 )
	{
		QString message=tr ( "no X2Go server found in LDAP " );
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}


	list<LDAPStringEntry>::iterator it=res.begin();
	list<LDAPStringEntry>::iterator end=res.end();
	QString freeServer;
	QString firstServer;
	bool isFirstServerSet=false;
	for ( ;it!=end;++it )
	{
		serv server;
		server.name=LDAPSession::getStringAttrValues (
		                *it,"cn" ).front().c_str();
		if ( !isFirstServerSet )
		{
			isFirstServerSet=true;
			firstServer=server.name;
		}
		QString sFactor="1";
		list<string> serialNumber=LDAPSession::getStringAttrValues (
		                              *it,"serialNumber" );
		if ( serialNumber.size() >0 )
		{
			sFactor=serialNumber.front().c_str();
		}
		x2goDebug<<server.name<<": factor is "<<sFactor;
		server.factor=sFactor.toFloat();
		server.sess=0;
		server.connOk=true;
		x2goServers.append ( server );
	}
	if ( ld )
	{
		delete ld;
		ld=0;
	}
	setEnabled ( false );
	passForm->setEnabled ( false );

	QString passwd;
	if ( !extLogin )
		currentKey=QString::null;
	QString user=getCurrentUname();
//      get x2gogetservers not from ldap server, but from first x2goserver
// 	QString host=ldapServer;
	QString host=firstServer;
	passwd=getCurrentPass();
	sshProcess* proc;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2gogetservers",
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_getServers ( false,message,0 );
		return;
	}
	connect ( proc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_getServers ( bool, QString,sshProcess* ) ) );
	connect ( proc,SIGNAL ( sudoConfigError ( QString,sshProcess* ) ),
	          this,SLOT ( slot_sudoErr ( QString,sshProcess* ) ) );
	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( sshEnv );
		if ( cardReady )
			cardStarted=true;
	}

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_getServers ( false,message,0 );
		return;
	}
	return;
#endif
}


void ONMainWindow::slotUnameChanged ( const QString& text )
{
	if ( prevText==text )
		return;
	if ( text=="" )
		return;
	QList<UserButton*>::iterator it;
	QList<UserButton*>::iterator endit=names.end();
	for ( it=names.begin();it!=endit;it++ )
	{
		QString username= ( *it )->username();
		if ( username.indexOf ( text,0,Qt::CaseInsensitive ) ==0 )
		{
			QPoint pos= ( *it )->pos();
			uname->setText ( username );
			QScrollBar* bar=users->verticalScrollBar();
			int docLang=bar->maximum()-bar->minimum() +
			            bar->pageStep();
			double position= ( double ) ( pos.y() ) /
			                 ( double ) ( uframe->height() );
			bar->setValue ( ( int ) ( docLang*position-height() /2+
			                          ( *it )->height() /2 ) );
			uname->setSelection ( username.length(),text.length()-
			                      username.length() );
			break;
		}
	}
	prevText=text;
}

void ONMainWindow::slotUnameEntered()
{
	QString text=uname->text();
	if ( useLdap )
	{
		UserButton* user=NULL;
		QList<UserButton*>::iterator it;
		QList<UserButton*>::iterator endit=names.end();
		for ( it=names.begin();it!=endit;it++ )
		{
			QString username= ( *it )->username();
			if ( username==text )
			{
				user=*it;
				break;
			}
		}
		showPass ( user );
	}
	else
	{
		SessionButton* sess=NULL;
		QList<SessionButton*>::iterator it;
		QList<SessionButton*>::iterator endit=sessions.end();
		for ( it=sessions.begin();it!=endit;it++ )
		{
			QString name= ( *it )->name();
			if ( name==text )
			{
				sess=*it;
				break;
			}
		}
		if ( sess )
			slotSelectedFromList ( sess );
	}
}


void ONMainWindow::readUsers()
{
#ifdef USELDAP
	if ( ! initLdapSession() )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "Please check LDAP settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );

		slot_config();
		return;
	}


	list<string> attr;
	attr.push_back ( "uidNumber" );
	attr.push_back ( "uid" );
	attr.push_back ( "cn" );
	attr.push_back ( "jpegPhoto" );


	list<LDAPBinEntry> result;
	try
	{
		ld->binSearch ( ldapDn.toStdString(),attr,
		                "objectClass=posixAccount",result );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "Please check LDAP settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}

	list<LDAPBinEntry>::iterator it=result.begin();
	list<LDAPBinEntry>::iterator end=result.end();

	for ( ;it!=end;++it )
	{
		user u;
		QString uin=LDAPSession::getBinAttrValues (
		                *it,"uidNumber" ).front().getData();
		u.uin=uin.toUInt();
		if ( u.uin<firstUid || u.uin>lastUid )
		{
			continue;
		}
		u.uid=LDAPSession::getBinAttrValues ( *it,
		                                      "uid" ).front().getData();
		u.name=u.name.fromUtf8 ( LDAPSession::getBinAttrValues ( *it,
		                         "cn" ).front().getData() );
		list<ByteArray> lst=LDAPSession::getBinAttrValues (
		                        *it,"jpegPhoto" );
		if ( lst.size() )
		{
			u.foto.loadFromData ( ( const uchar* ) (
			                          lst.front().getData() ),
			                      lst.front().length() );
		}
		userList.append ( u );
	}
	qSort ( userList.begin(),userList.end(),user::lessThen );
	delete ld;
	ld=0;
	displayUsers();
	if ( defaultUser )
	{
		defaultUser=false;
		for ( int i=0;i<userList.size();++i )
		{
			if ( userList[i].uid ==defaultUserName )
			{
				uname->setText ( defaultUserName );
				slotUnameChanged ( defaultUserName );
				QTimer::singleShot (
				    100, this,
				    SLOT ( slotUnameEntered() ) );
				break;
			}
		}
	}
#endif
}


void ONMainWindow::slot_config()
{
	if ( !startMaximized && !startHidden && !embedMode )
	{
#ifndef Q_OS_WIN
		QSettings st ( homeDir +"/.x2goclient/sizes",
		               QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sizes" );
#endif

		st.setValue ( "mainwindow/size",QVariant ( size() ) );
		st.setValue ( "mainwindow/pos",QVariant ( pos() ) );
		st.sync();
	}
	if ( ld )
		delete ld;
	ld=0;

	ConfigDialog dlg ( this );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		int i;

		if ( passForm->isVisible() && !embedMode )
			slotClosePass();
		if ( sessionStatusDlg->isVisible() || embedMode )
		{
			//if session is running or embed mode, save changes,
			//but not accepted
			//
			return;
		}
		if ( !embedMode )
		{

			for ( i=0;i<names.size();++i )
				names[i]->close();
			for ( i=0;i<sessions.size();++i )
				sessions[i]->close();

			userList.clear();
			sessions.clear();
		}
		loadSettings();
		if ( useLdap )
		{
			act_new->setEnabled ( false );
			act_edit->setEnabled ( false );
			u->setText ( tr ( "Login:" ) );
			QTimer::singleShot ( 1, this, SLOT ( readUsers() ) );
		}
		else
		{
			act_new->setEnabled ( true );
			act_edit->setEnabled ( true );
			u->setText ( tr ( "Session:" ) );
			QTimer::singleShot ( 1, this,
			                     SLOT ( slot_readSessions() ) );
		}
		slot_resize ( fr->size() );
	}
}

void ONMainWindow::slot_edit ( SessionButton* bt )
{
	EditConnectionDialog dlg ( bt->id(),this );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		bt->redraw();
		placeButtons();
		users->ensureVisible ( bt->x(),bt->y(),50,220 );
	}
}

void ONMainWindow::slot_createDesktopIcon ( SessionButton* bt )
{
	bool crHidden= ( QMessageBox::question (
	                     this,
	                     tr ( "Create session icon on desktop" ),
	                     tr ( "Desktop icons can be configured "
	                          "not to show x2goclient (hidden mode). "
	                          "If you like to use this feature you'll "
	                          "need to configure login by a gpg key "
	                          "or gpg smart card.\n\n"
	                          "Use x2goclient hidden mode?" ),
	                     QMessageBox::Yes|QMessageBox::No ) ==
	                 QMessageBox::Yes );
#ifndef Q_OS_WIN
	QSettings st ( homeDir +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else
	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	QString name=st.value ( bt->id() +"/name",
	                        ( QVariant ) tr ( "New Session" ) ).toString() ;
	QString sessIcon=st.value (
	                     bt->id() +"/icon",
	                     ( QVariant )
	                     ":icons/128x128/x2gosession.png"
	                 ).toString();
	if ( sessIcon.startsWith ( ":icons",Qt::CaseInsensitive ) ||
	        !sessIcon.endsWith ( ".png",Qt::CaseInsensitive ) )
	{
		sessIcon="/usr/share/x2goclient/icons/x2gosession.png";
	}
#ifndef Q_OS_WIN
	QFile file (
	    QDesktopServices::storageLocation (
	        QDesktopServices::DesktopLocation ) +"/"+name+".desktop" );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
		return;

	QString cmd="x2goclient";
	if ( crHidden )
		cmd="x2goclient --hide";
	QTextStream out ( &file );
	out << "[Desktop Entry]\n"<<
	"Exec[$e]="<<cmd<<" --sessionid="<<bt->id() <<"\n"<<
	"Icon="<<sessIcon<<"\n"<<
	"Name="<<name<<"\n"<<
	"StartupNotify=true\n"<<
	"Terminal=false\n"<<
	"Type=Application\n"<<
	"X-KDE-SubstituteUID=false\n";
	file.close();
#else
	QString scrname=QDir::tempPath() +"\\mklnk.vbs";
	QFile file ( scrname );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
		return;

	QSettings xst ( "HKEY_LOCAL_MACHINE\\SOFTWARE\\x2goclient",QSettings::NativeFormat );
	QString workDir=xst.value ( "Default" ).toString();
	workDir+="\\bin";
	QString progname=workDir+"\\x2goclient.exe";
	QString args="--sessionid="+bt->id();
	if ( crHidden )
		args+=" --hide";
	QTextStream out ( &file );
	out << "Set Shell = CreateObject(\"WScript.Shell\")\n"<<
	"DesktopPath = Shell.SpecialFolders(\"Desktop\")\n"<<
	"Set link = Shell.CreateShortcut(DesktopPath & \"\\"<<name<<
	".lnk\")\n"<<
	"link.Arguments = \""<<args<<"\"\n"<<
	"link.Description = \""<<tr ( "X2Go Link to session " ) <<
	"--"<<name<<"--"<<"\"\n"<<
	"link.TargetPath = \""<<progname<<"\"\n"<<
	"link.iconLocation = \""<<progname<<"\"\n"<<
	"link.WindowStyle = 1\n"<<
	"link.WorkingDirectory = \""<<workDir<<"\"\n"<<
	"link.Save\n";
	file.close();
	system ( scrname.toAscii() );
	QFile::remove ( scrname );
#endif
}


void ONMainWindow::slot_readSessions()
{
#ifndef Q_OS_WIN
	QSettings st ( homeDir +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	QStringList slst=st.childGroups();
	for ( int i=0;i<slst.size();++i )
	{
		if ( slst[i]!="embedded" )
			createBut ( slst[i] );
	}
	placeButtons();
	if ( slst.size() ==0 )
		slotNewSession();
	uname->setText ( "" );
	disconnect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
	             SLOT ( slotUnameChanged ( const QString& ) ) );
	connect ( uname,SIGNAL ( textEdited ( const QString& ) ),this,
	          SLOT ( slotSnameChanged ( const QString& ) ) );
	if ( !defaultSession&& startHidden )
	{
		startHidden=false;
		slot_resize();
		show();
		activateWindow();
		raise();

	}
	if ( defaultSession )
	{
		bool sfound=false;
		defaultSession=false;
		if ( defaultSessionId.length() >0 )
		{
			for ( int i=0;i<sessions.size();++i )
			{
				if ( sessions[i]->id() ==defaultSessionId )
				{
					sfound=true;
					slotSelectedFromList ( sessions[i] );
					break;
				}
			}
		}
		else
		{
			for ( int i=0;i<sessions.size();++i )
			{
				if ( sessions[i]->name() ==defaultSessionName )
				{
					sfound=true;
					uname->setText ( defaultSessionName );
					QTimer::singleShot (
					    100, this,
					    SLOT ( slotUnameEntered() ) );
					slotSnameChanged ( defaultSessionName );
					break;
				}
			}
		}
		if ( !sfound && startHidden )
		{
			startHidden=false;
			slot_resize();
			show();
			activateWindow();
			raise();
		}
	}
}


void ONMainWindow::slotNewSession()
{
	QString id=QDateTime::currentDateTime().
	           toString ( "yyyyMMddhhmmsszzz" );
	EditConnectionDialog dlg ( id, this );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		SessionButton* bt=createBut ( id );
		placeButtons();
		users->ensureVisible ( bt->x(),bt->y(),50,220 );
	}
}

void ONMainWindow::slot_manage()
{
	SessionManageDialog dlg ( this );
	dlg.exec();
}

void ONMainWindow::slotCreateSessionIcon()
{
	SessionManageDialog dlg ( this,true );
	dlg.exec();
}

SessionButton* ONMainWindow::createBut ( const QString& id )
{
	SessionButton* l;
	l=new SessionButton ( this,uframe,id );
	sessions.append ( l );
	connect ( l,SIGNAL ( signal_edit ( SessionButton* ) ),
	          this,SLOT ( slot_edit ( SessionButton* ) ) );

	connect ( l,SIGNAL ( signal_remove ( SessionButton* ) ),
	          this,SLOT ( slotDeleteButton ( SessionButton* ) ) );

	connect ( l,SIGNAL ( sessionSelected ( SessionButton* ) ),this,
	          SLOT ( slotSelectedFromList ( SessionButton* ) ) );

	return l;
}


void ONMainWindow::placeButtons()
{
	qSort ( sessions.begin(),sessions.end(),SessionButton::lessThen );
	for ( int i=0;i<sessions.size();++i )
	{
		if ( !miniMode )
			sessions[i]->move ( ( users->width()-360 ) /2,
			                    i*220+i*25+5 );
		else
			sessions[i]->move ( ( users->width()-260 ) /2,
			                    i*155+i*20+5 );
		sessions[i]->show();
	}
	if ( sessions.size() )
	{
		if ( !miniMode )
			uframe->setFixedHeight (
			    sessions.size() *220+ ( sessions.size()-1 ) *25 );
		else
			uframe->setFixedHeight (
			    sessions.size() *155+ ( sessions.size()-1 ) *20 );
	}

}

void ONMainWindow::slotDeleteButton ( SessionButton * bt )
{
	if ( QMessageBox::warning (
	            this,bt->name(),
	            tr ( "Are you sure you want to delete this session?" ),
	            QMessageBox::Yes,QMessageBox::No ) !=QMessageBox::Yes )
		return;
#ifndef Q_OS_WIN

	QSettings st ( homeDir +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif

	st.beginGroup ( bt->id() );
	st.remove ( "" );
	st.sync();
	sessions.removeAll ( bt );
	bt->close();
	placeButtons();
	users->ensureVisible ( 0,0,50,220 );
}


void ONMainWindow::displayToolBar ( bool show )
{
#ifndef Q_OS_WIN
	QSettings st1 ( homeDir +"/.x2goclient/settings",
	                QSettings::NativeFormat );
#else

	QSettings st1 ( "Obviously Nice","x2goclient" );
	st1.beginGroup ( "settings" );
#endif

	st1.setValue ( "toolbar/show",show );
	st1.sync();
}


bool ONMainWindow::initLdapSession ( bool showError )
{
#ifdef USELDAP
	x2goDebug<<"initing LDAP Session"<<endl;
	try
	{
		ld=new LDAPSession ( ldapServer.toStdString(),
		                     ldapPort,"","",true,false );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption0 in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		x2goDebug <<message<<endl;
		if ( ldapServer1.length() )
		{
			try
			{
				ld=new LDAPSession ( ldapServer1.toStdString(),
				                     ldapPort1,"","",
				                     true,false );
			}
			catch ( LDAPExeption e )
			{
				QString message="Exeption1 in: ";
				message=message+e.err_type.c_str();
				message=message+" : "+e.err_str.c_str();
				x2goDebug <<message<<endl;
				if ( ldapServer2.length() )
				{
					try
					{
						ld=new LDAPSession (
						    ldapServer2.toStdString(),
						    ldapPort2,"","",
						    true,false );
					}
					catch ( LDAPExeption e )
					{
						QString message=
						    "Exeption2 in: ";
						message=message+
						        e.err_type.c_str();
						message=message+" : "+
						        e.err_str.c_str();
						x2goDebug <<message<<endl;
						x2goDebug<<"return false"<<endl;
						if ( showError )
							QMessageBox::critical (
							    0l,tr ( "Error" ),
							    message,
							    QMessageBox::Ok,
							    QMessageBox::
							    NoButton );

						return false;
					}
				}
				else
				{
					x2goDebug<<"return false"<<endl;
					if ( showError )
						QMessageBox::critical (
						    0l,tr ( "Error" ),
						    message, QMessageBox::Ok,
						    QMessageBox::NoButton );

					return false;
				}

			}
		}
		else
		{
			x2goDebug<<"return false"<<endl;
			if ( showError )
				QMessageBox::critical ( 0l,tr ( "Error" ),
				                        message,QMessageBox::Ok,
				                        QMessageBox::NoButton );

			return false;
		}

	}
	sessionCmd="/usr/bin/startkde";
	LDAPSndSys="ARTS_SERVER";
	LDAPSndStartServer=true;
	LDAPPrintSupport=false;
	startSound=false;
	firstUid=0;
	lastUid=65535;


	list<string> attr;
	attr.push_back ( SESSIONCMD );
	attr.push_back ( FIRSTUID );
	attr.push_back ( LASTUID );

	list<LDAPStringEntry> res;
	QString searchBase="ou=Settings,ou=ON,"+ldapDn;
	QString srch="cn=session settings";
	try
	{
		ld->stringSearch ( searchBase.toStdString(),attr,
		                   srch.toStdString(),res );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		return false;
	}

	if ( res.size() !=0 )
	{
		LDAPStringEntry entry=res.front();
		list<string> str=LDAPSession::getStringAttrValues (
		                     entry,SESSIONCMD );
		if ( str.size() )
		{
			sessionCmd=str.front().c_str();
		}
		str=LDAPSession::getStringAttrValues ( entry,FIRSTUID );
		if ( str.size() )
		{
			firstUid= ( ( QString ) str.front().c_str() ).toInt();
		}
		str=LDAPSession::getStringAttrValues ( entry,LASTUID );
		if ( str.size() )
		{
			lastUid= ( ( QString ) str.front().c_str() ).toInt();
		}
	}
	attr.clear();
	res.clear();
	attr.push_back ( NETSOUNDSYSTEM );
	attr.push_back ( SNDSUPPORT );
	attr.push_back ( SNDPORT );
	attr.push_back ( STARTSNDSERVER );


	srch="cn=sound settings";
	try
	{
		ld->stringSearch ( searchBase.toStdString(),attr,
		                   srch.toStdString(),res );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		return false;
	}

	if ( res.size() !=0 )
	{
		LDAPStringEntry entry=res.front();
		list<string> str=LDAPSession::getStringAttrValues (
		                     entry,NETSOUNDSYSTEM );
		if ( str.size() )
		{
			LDAPSndSys=str.front().c_str();
		}
		if ( LDAPSndSys=="PULSE" )
		{
			LDAPSndSys="pulse";
			LDAPSndStartServer=false;
			LDAPSndPort="4713";
		}
		if ( LDAPSndSys=="ARTS_SERVER" )
		{
			LDAPSndPort="20221";
			LDAPSndSys="arts";
		}
		if ( LDAPSndSys=="ESPEAKER" )
		{
			LDAPSndPort="16001";
			LDAPSndSys="esd";
		}
		str=LDAPSession::getStringAttrValues ( entry,SNDSUPPORT );
		if ( str.size() )
		{
			startSound= ( str.front() =="yes" ) ?true:false;
		}
		str=LDAPSession::getStringAttrValues ( entry,SNDPORT );
		if ( str.size() )
		{
			LDAPSndPort=str.front().c_str();
		}
		str=LDAPSession::getStringAttrValues ( entry,STARTSNDSERVER );
		if ( str.size() )
		{
			LDAPSndStartServer=
			    ( str.front() =="yes" ) ?true:false;
		}
	}
#endif
	return true;

}



void ONMainWindow::slotSnameChanged ( const QString& text )
{
	if ( prevText==text )
		return;
	if ( text=="" )
		return;
	QList<SessionButton*>::iterator it;
	QList<SessionButton*>::iterator endit=sessions.end();
	for ( it=sessions.begin();it!=endit;it++ )
	{
		QString name= ( *it )->name();
		if ( name.indexOf ( text,0,Qt::CaseInsensitive ) ==0 )
		{
			QPoint pos= ( *it )->pos();
			uname->setText ( name );
			QScrollBar* bar=users->verticalScrollBar();
			int docLang=bar->maximum()-bar->minimum() +
			            bar->pageStep();
			double position= ( double ) ( pos.y() ) /
			                 ( double ) ( uframe->height() );
			bar->setValue ( ( int ) ( docLang*position-height() /
			                          2+ ( *it )->height() /2 ) );
			uname->setSelection ( name.length(),
			                      text.length()-name.length() );
			break;
		}
	}
	prevText=text;
}


void ONMainWindow::slotSelectedFromList ( SessionButton* session )
{
	lastSession=session;
	QString command;
	QString server;
	QString userName;
	QString sessIcon;
	QPalette pal;
	QString sessionName;
	if ( !embedMode )
	{
		session->hide();
		pal=users->palette();
		setUsersEnabled ( false );
		sessionName=session->name();

		QString sid=session->id();
#ifndef Q_OS_WIN

		QSettings st ( homeDir +"/.x2goclient/sessions",
		               QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		sessIcon=st.value (
		             sid+"/icon",
		             ( QVariant ) ":icons/128x128/x2gosession.png"
		         ).toString();


		command=st.value ( sid+"/command",
		                   ( QVariant ) tr ( "KDE" ) ).toString();

		server=st.value ( sid+"/host",
		                  ( QVariant ) QString::null
		                ).toString();
		userName=st.value (
		             sid+"/user",
		             ( QVariant ) QString::null ).toString();
		sshPort=st.value ( sid+"/sshport",
		                   ( QVariant ) defaultSshPort ).toString();
		currentKey=st.value ( sid+"/key",
		                      ( QVariant ) QString::null ).toString();
	}
	else
	{
		command=config.command;
		server=config.server;
		userName=config.user;
		sshPort=config.sshport;
		sessIcon=":icons/128x128/x2gosession.png";
		sessionName=config.session;
	}
	selectedCommand=command;
	command=transAppName ( command );
	login->setText ( userName );
	QPixmap pix ( sessIcon );
	if ( !miniMode )
	{
		fotoLabel->setPixmap (
		    pix.scaled ( 64,64,
		                 Qt::IgnoreAspectRatio,
		                 Qt::SmoothTransformation ) );
		fotoLabel->setFixedSize ( 64,64 );
	}
	else
	{
		fotoLabel->setPixmap (
		    pix.scaled ( 48,48,
		                 Qt::IgnoreAspectRatio,
		                 Qt::SmoothTransformation ) );
		fotoLabel->setFixedSize ( 48,48 );
	}


	if ( command=="RDP" )
	{
		command=tr ( "RDP connection" );
	}

	QString text="<b>"+sessionName +"</b><br>"+
	             command+tr ( " on " ) +server;
	nameLabel->setText ( text );
	if ( userName.length() <=0 )
		login->setFocus();

	bool nopass=false;
	if ( !embedMode )
		slot_showPassForm();

	if ( currentKey.length() >0 )
	{
		QFile file ( currentKey );
		if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		{
			nopass=true;
			while ( !file.atEnd() )
			{
				QString line = file.readLine();
				if ( line.indexOf ( "ENCRYPTED" ) !=-1 )
				{
					nopass=false;
					break;
				}

			}
			file.close();
		}
		else
			currentKey=QString::null;
	}
	if ( currentKey != QString::null && currentKey != "" && nopass )
	{
		slotSessEnter();
	}
	if ( cardReady || useSshAgent )
	{
		nopass=true;
		if ( cardReady )
			login->setText ( cardLogin );
		slotSessEnter();
		return;
	}
	if ( startHidden && nopass==false )
	{
		startHidden=false;
		slot_resize();
		show();
		activateWindow();
		raise();
	}
	if ( embedMode )
	{
		QTimer::singleShot ( 50, this,
		                     SLOT ( slot_showPassForm() ) );
	}
}


void ONMainWindow::slotSessEnter()
{
	resumingSession.sessionId=QString::null;
	resumingSession.server=QString::null;
	resumingSession.display=QString::null;
	setStatStatus ( tr ( "connecting" ) );
#ifdef Q_OS_WIN
	//waiting for X
	if ( !winServersReady )
	{
		QTimer::singleShot ( 100, this, SLOT ( slotSessEnter() ) );
		return;
	}
#endif
#if defined ( Q_OS_WIN ) || defined (Q_OS_DARWIN )
	QString disp=getXDisplay();
	if ( disp==QString::null )
		return;
#endif

	QString sid="";
	if ( !embedMode )
		sid=lastSession->id();
	startSession ( sid );
}


bool ONMainWindow::startSession ( const QString& sid )
{
	setEnabled ( false );
	QString passwd;
	QString user;
	QString host;
	user=getCurrentUname();

	if ( !embedMode )
	{
#ifndef Q_OS_WIN

		QSettings st ( homeDir +"/.x2goclient/sessions",
		               QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		passForm->setEnabled ( false );
		host=st.value ( sid+"/host",
		                ( QVariant ) QString::null ).toString();
	}
	else
	{
		host=config.server;
		sshPort=config.sshport;
		selectedCommand=config.command;
	}

	passwd=getCurrentPass();
	sshProcess* proc;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "export HOSTNAME && x2golistsessions",
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_listSessions ( false,message,0 );
		return false;
	}
	connect ( proc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_listSessions ( bool, QString,
	                                          sshProcess* ) ) );
	connect ( proc,SIGNAL ( sudoConfigError ( QString,sshProcess* ) ),
	          this,SLOT ( slot_sudoErr ( QString,sshProcess* ) ) );

	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env=sshEnv+env;
		proc->setEnvironment ( env );
		if ( cardReady )
			cardStarted=true;
	}
	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_listSessions ( false,message,0 );
		return false;
	}
	return true;
}


void ONMainWindow::slot_listSessions ( bool result,QString output,
                                       sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		cardReady=false;
		cardStarted=false;
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong password!</b><br><br>" ) +
			        message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		currentKey=QString::null;
		setEnabled ( true );
		passForm->setEnabled ( true );
		slot_showPassForm();
		pass->setFocus();
		pass->selectAll();
		return;
	}

	passForm->hide();
	if ( !embedMode )
	{
		setUsersEnabled ( false );
		uname->setEnabled ( false );
		u->setEnabled ( false );
	}
	QStringList sessions=output.trimmed().split ( '\n' );
	if ( sessions.size() ==1&&sessions[0].length() <5 )
		startNewSession();
	else if ( sessions.size() ==1 )
	{
		x2goSession s=getSessionFromString ( sessions[0] );
		QDesktopWidget wd;
		if ( s.status=="S" && isColorDepthOk ( wd.depth(),s.colorDepth )
		        &&s.command == selectedCommand )
			resumeSession ( s );
		else
		{
			if ( startHidden )
				startNewSession();
			else
				selectSession ( sessions );
		}
	}
	else
	{
		if ( !startHidden )
			selectSession ( sessions );
		else
		{
			for ( int i=0;i<sessions.size();++i )
			{
				x2goSession s=getSessionFromString (
				                  sessions[i] );
				QDesktopWidget wd;
				if ( s.status=="S" && isColorDepthOk (
				            wd.depth(),s.colorDepth )
				        &&s.command == selectedCommand )
				{
					resumeSession ( s );
					return;
				}
			}
			startNewSession();
		}
	}
}


x2goSession ONMainWindow::getSessionFromString ( const QString& string )
{
	QStringList lst=string.split ( '|' );
	x2goSession s;
	s.agentPid=lst[0];
	s.sessionId=lst[1];
	s.display=lst[2];
	s.server=lst[3];
	s.status=lst[4];
	s.crTime=lst[5];
	s.cookie=lst[6];
	s.clientIp=lst[7];
	s.grPort=lst[8];
	s.sndPort=lst[9];
	if ( lst.count() >13 )
		s.fsPort=lst[13];
	s.colorDepth=0;
	if ( s.sessionId.indexOf ( "_dp" ) !=-1 )
	{
		s.colorDepth=s.sessionId.split ( "_dp" ) [1].toInt();
	}
	s.sessionType=x2goSession::DESKTOP;
	s.command=tr ( "unknown" );
	if ( s.sessionId.indexOf ( "_st" ) !=-1 )
	{
		QString cmdinfo=s.sessionId.split ( "_st" ) [1];
		cmdinfo=cmdinfo.split ( "_" ) [0];
		QChar st=cmdinfo[0];
		if ( st=='R' )
			s.sessionType=x2goSession::ROOTLESS;
		if ( st=='S' )
			s.sessionType=x2goSession::SHADOW;
		QString command=cmdinfo.mid ( 1 );
		if ( command.length() >0 )
			s.command=command;
	}
	return s;
}


void ONMainWindow::startNewSession()
{
	newSession=true;
	QString passwd=getCurrentPass();
	QString user=getCurrentUname();

	QString pack;
	bool fullscreen;
	int height;
	int width;
	int quality;
	int speed;
	bool usekbd;
	bool rootless=false;
	bool setDPI=defaultSetDPI;
	uint dpi=defaultDPI;
	QString layout;
	QString type;
	QString command;
	QString host=QString::null;

	if ( useLdap )
	{
		pack=defaultPack;
		fullscreen=defaultFullscreen;
		height=defaultHeight;
		width=defaultWidth;
		quality=defaultQuality;
		speed=defaultLink;
		usekbd=defaultSetKbd;
		layout=defaultLayout;
		type=defaultKbdType;
		command=defaultCmd;
		for ( int j=0;j<x2goServers.size();++j )
		{
			if ( x2goServers[j].connOk )
			{
				host=x2goServers[j].name;
				break;
			}
		}
		if ( host==QString::null )
		{
			QMessageBox::critical ( 0l,tr ( "Error" ),
			                        tr ( "No server availabel" ),
			                        QMessageBox::Ok,
			                        QMessageBox::NoButton );
			return;
		}
	}
	else
	{
#ifndef Q_OS_WIN
		QSettings st ( homeDir +"/.x2goclient/sessions",
		               QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		QString sid;
		if ( !embedMode )
			sid=lastSession->id();
		else
			sid="embedded";
		pack=st.value ( sid+"/pack",
		                ( QVariant ) defaultPack ).toString();
		fullscreen=st.value ( sid+"/fullscreen",
		                      ( QVariant )
		                      defaultFullscreen ).toBool();
		height=st.value ( sid+"/height",
		                  ( QVariant ) defaultHeight ).toInt();
		width=st.value ( sid+"/width",
		                 ( QVariant ) defaultWidth ).toInt();
		setDPI=st.value ( sid+"/setdpi",
		                  ( QVariant ) defaultSetDPI ).toBool();
		dpi=st.value ( sid+"/dpi",
		               ( QVariant ) defaultDPI ).toUInt();
		quality=st.value (
		            sid+"/quality",
		            ( QVariant ) defaultQuality ).toInt();
		speed=st.value ( sid+"/speed",
		                 ( QVariant ) defaultLink ).toInt();
		usekbd=st.value ( sid+"/usekbd",
		                  ( QVariant ) defaultSetKbd ).toBool();
		layout=st.value ( sid+"/layout",
		                  ( QVariant )
		                  defaultLayout ).toString();
		type=st.value ( sid+"/type",
		                ( QVariant )
		                defaultKbdType ).toString();
		if ( !embedMode )
		{
			command=st.value ( sid+"/command",
			                   ( QVariant ) defaultCmd ).toString();
			host=st.value (
			         sid+"/host",
			         ( QVariant )
			         ( QString ) "localhost" ).toString();

			rootless=st.value ( sid+"/rootless",
			                    ( QVariant ) false ).toBool();
		}
		else
		{
			command=config.command;
			rootless= config.rootless;
			host=config.server;
			startEmbedded=false;
			if ( st.value ( sid+"/startembed",
			                ( QVariant ) true ).toBool() )
			{
				startEmbedded=true;
				fullscreen=false;
				height=bgFrame->size().height()-stb->height();
				width=bgFrame->size().width();

				if ( height<0 ||width<0 )
				{
					height=defaultHeight;
					width=defaultWidth;
				}
			}


		}
		if ( command=="RDP" )
			rootless=true;
	}



	resumingSession.server=host;

	QString geometry;
#ifdef Q_OS_WIN
	maximizeProxyWin=false;
	proxyWinWidth=width;
	proxyWinHeight=height;
#endif
	if ( fullscreen )
	{
		geometry="fullscreen";
#ifdef Q_OS_WIN
		fullscreen=false;
		maximizeProxyWin=true;
#endif
	}
	if ( ! fullscreen )
	{
		geometry=QString::number ( width ) +"x"+
		         QString::number ( height );
		if ( embedMode )
		{
			QPoint position=mapToGlobal ( bgFrame->pos() );
			geometry+="+"+QString::number ( position.x() ) +"+"+
			          QString::number ( position.y() +
			                            stb->height() );
		}

	}
	QString link;
	switch ( speed )
	{
		case 0:
			link="modem";
			break;
		case 1:
			link="isdn";
			break;
		case 2:
			link="adsl";
			break;
		case 3:
			link="wan";
			break;
		case 4:
			link="lan";
			break;
	}

	QFile file ( ":/txt/packs" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;
	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		QString pc=in.readLine();
		if ( pc.indexOf ( "-%" ) !=-1 )
		{
			pc=pc.left ( pc.indexOf ( "-%" ) );
			if ( pc==pack )
			{
				pack+="-"+QString::number ( quality );
				break;
			}
		}
	}
	file.close();


	if ( selectSessionDlg->isVisible() )
	{
		if ( !embedMode )
			slotCloseSelectDlg();
		else
			selectSessionDlg->hide();
	}
	QDesktopWidget wd;
	QString depth=QString::number ( wd.depth() );
#ifdef Q_OS_DARWIN
	usekbd=0;
	type="query";
#endif
	QString sessTypeStr="D ";
	if ( rootless )
		sessTypeStr="R ";
	QString dpiEnv;
	if ( setDPI )
	{
		dpiEnv="X2GODPI="+QString::number ( dpi ) +" ";
	}
	QString cmd=dpiEnv+"x2gostartagent "+
	            geometry+" "+link+" "+pack+
	            " unix-kde-depth_"+depth+" "+layout+" "+type+" ";
	if ( usekbd )
		cmd += "1 ";
	else
		cmd += "0 ";
	QFileInfo f ( command );
	cmd+=sessTypeStr+f.fileName();

	sshProcess* proc=0l;

	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
	}

	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retResumeSess ( bool,
	                                           QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
		return;
	}
	passForm->hide();
}



void ONMainWindow::resumeSession ( const x2goSession& s )
{
	newSession=false;

	QString passwd=getCurrentPass();
	QString user=getCurrentUname();
	QString host=s.server;

	QString pack;
	bool fullscreen;
	int height;
	int width;
	int quality;
	int speed;
	bool usekbd;
	QString layout;
	QString type;

	if ( useLdap )
	{
		pack=defaultPack;
		fullscreen=defaultFullscreen;
		height=defaultHeight;
		width=defaultWidth;
		quality=defaultQuality;
		speed=defaultLink;
		usekbd=defaultSetKbd;
		layout=defaultLayout;
		type=defaultKbdType;

	}
	else
	{

		QString sid;
		if ( !embedMode )
			sid=lastSession->id();
		else
			sid="embedded";
#ifndef Q_OS_WIN
		QSettings st (
		    homeDir+"/.x2goclient/sessions",
		    QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		pack=st.value ( sid+"/pack",
		                ( QVariant ) defaultPack ).toString();

		fullscreen=st.value ( sid+"/fullscreen",
		                      ( QVariant )
		                      defaultFullscreen ).toBool();
		height=st.value ( sid+"/height",
		                  ( QVariant ) defaultHeight ).toInt();
		width=st.value ( sid+"/width",
		                 ( QVariant ) defaultWidth ).toInt();
		quality=st.value ( sid+"/quality",
		                   ( QVariant )
		                   defaultQuality ).toInt();
		speed=st.value ( sid+"/speed",
		                 ( QVariant ) defaultLink ).toInt();
		usekbd=st.value ( sid+"/usekbd",
		                  ( QVariant ) defaultSetKbd ).toBool();
		layout=st.value ( sid+"/layout",
		                  ( QVariant )
		                  defaultLayout ).toString();
		type=st.value ( sid+"/type",
		                ( QVariant )
		                defaultKbdType ).toString();
		if ( !embedMode )
		{
			host=st.value ( sid+"/host",
			                ( QVariant ) s.server ).toString();
		}
		else
		{
			startEmbedded=false;
			if ( st.value ( sid+"/startembed",
			                ( QVariant ) true ).toBool() )
			{
				fullscreen=false;
				startEmbedded=true;
				height=bgFrame->size().height()-stb->height();
				width=bgFrame->size().width();
				if ( height<0 ||width<0 )
				{
					height=defaultHeight;
					width=defaultWidth;
				}
			}
			host=config.server;
		}
	}
	QString geometry;
#ifdef Q_OS_WIN
	maximizeProxyWin=false;
	proxyWinWidth=width;
	proxyWinHeight=height;
#endif
	if ( fullscreen )
	{
		geometry="fullscreen";
#ifdef Q_OS_WIN
		fullscreen=false;
		maximizeProxyWin=true;
#endif
	}
	if ( !fullscreen )
	{
		geometry=QString::number ( width ) +"x"+
		         QString::number ( height );
	}
	QString link;
	switch ( speed )
	{
		case 0:
			link="modem";
			break;
		case 1:
			link="isdn";
			break;
		case 2:
			link="adsl";
			break;
		case 3:
			link="wan";
			break;
		case 4:
			link="lan";
			break;
	}

	QFile file ( ":/txt/packs" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;
	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		QString pc=in.readLine();
		if ( pc.indexOf ( "-%" ) !=-1 )
		{
			pc=pc.left ( pc.indexOf ( "-%" ) );
			if ( pc==pack )
			{
				pack+="-"+QString::number ( quality );
				break;
			}
		}
	}
	file.close();

#ifdef Q_OS_DARWIN
	usekbd=0;
	type="query";
#endif


	if ( selectSessionDlg->isVisible() )
	{
		if ( !embedMode )
			slotCloseSelectDlg();
		else
			selectSessionDlg->hide();
	}
	QString cmd="x2goresume-session "+s.sessionId+" "+geometry+
	            " "+link+" "+pack+" "+layout+
	            " "+type+" ";
	if ( usekbd )
		cmd += "1";
	else
		cmd += "0";

	sshProcess* proc=0l;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retResumeSess ( bool, QString,
	                                           sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retResumeSess ( false,message,0 );
		return;
	}
	resumingSession=s;
	passForm->hide();
}


void ONMainWindow::selectSession ( const QStringList& sessions )
{
	setEnabled ( true );
	sessionStatusDlg->hide();
	passForm->hide();
	selectedSessions.clear();
	QFontMetrics fm ( sessTv->font() );
	for ( int row = 0; row < sessions.size(); ++row )
	{

		x2goSession s=getSessionFromString ( sessions[row] );
		selectedSessions.append ( s );
		QStandardItem *item;

		item= new QStandardItem ( s.display );
		model->setItem ( row,S_DISPLAY,item );

		if ( s.status=="R" )
			item= new QStandardItem ( tr ( "running" ) );
		else
			item= new QStandardItem ( tr ( "suspended" ) );
		model->setItem ( row,S_STATUS,item );

		item= new QStandardItem ( transAppName ( s.command ) );
		model->setItem ( row,S_COMMAND,item );

		QString type=tr ( "Desktop" );
		if ( s.sessionType==x2goSession::ROOTLESS )
			type=tr ( "single application" );
		if ( s.sessionType==x2goSession::SHADOW )
			type=tr ( "shadow session" );

		item= new QStandardItem ( type );
		model->setItem ( row,S_TYPE,item );

		item= new QStandardItem ( s.crTime );
		model->setItem ( row,S_CRTIME,item );
		item= new QStandardItem ( s.server );
		model->setItem ( row,S_SERVER,item );
		item= new QStandardItem ( s.clientIp );
		model->setItem ( row,S_IP,item );
		item= new QStandardItem ( s.sessionId );
		model->setItem ( row,S_ID,item );
		for ( int j=0;j<8;++j )
		{
			QString txt=model->index ( row,j ).data().toString();
			if ( sessTv->header()->sectionSize ( j ) <
			        fm.width ( txt ) +6 )
			{
				sessTv->header()->resizeSection (
				    j,fm.width ( txt ) +6 );
			}
		}
	}
	selectSessionDlg->show();
}

void ONMainWindow::slotCloseSelectDlg()
{
	selectSessionDlg->hide();
	if ( !embedMode )
	{
		u->setEnabled ( true );
		uname->setEnabled ( true );
	}
	slot_showPassForm();
}



void ONMainWindow::slot_activated ( const QModelIndex& index )
{
	QString status=sessTv->model()->index ( index.row(),
	                                        S_STATUS ).data().toString();
	if ( status==tr ( "running" ) )
	{
		bSusp->setEnabled ( true );
		sOk->setEnabled ( false );
	}
	else
	{
		bSusp->setEnabled ( false );
		sOk->setEnabled ( true );
	}
	bTerm->setEnabled ( true );
	if ( status==QString::null )
	{
		sOk->setEnabled ( false );
		bTerm->setEnabled ( false );
	}
}


void ONMainWindow::slotResumeSess()
{
	x2goSession s=getSelectedSession();
	QDesktopWidget wd;
	if ( isColorDepthOk ( wd.depth(),s.colorDepth ) )
		resumeSession ( s );
	else
	{
		QString depth=QString::number ( s.colorDepth );
		int res;
		if ( s.colorDepth==24 || s.colorDepth==32 )
		{
			res=QMessageBox::warning (
			        0l,tr ( "Warning" ),
			        tr (
			            "Your current color depth is "
			            "different to the color depth of your "
			            "x2go-session. This may cause problems "
			            "reconnecting to this session and in most "
			            "cases <b>you will loose the session</b> "
			            "and have to start a new one! It's highly "
			            "recommended to change the color depth of "
			            "your Display to " ) +tr ( "24 or 32" ) +
			        tr (
			            " bit and restart your X-server before you "
			            "reconnect to this x2go-session.<br>Resume "
			            "this session anyway?" ),tr ( "Yes" ),
			        tr ( "No" ) );

		}
		else
		{
			res=QMessageBox::warning (
			        0l,tr ( "Warning" ),
			        tr (
			            "Your current color depth is different to "
			            "the color depth of your x2go-session. "
			            "This may cause problems reconnecting to "
			            "this session and in most cases <b>you "
			            "will loose the session</b> and have to "
			            "start a new one! It's highly recommended "
			            "to change the color depth of your "
			            "Display to " ) +depth+
			        tr (
			            " bit and restart your X-server before you "
			            "reconnect to this x2go-session.<br>Resume "
			            "this session anyway?" ),tr ( "Yes" ),
			        tr ( "No" ) );
		}
		if ( res==0 )
			resumeSession ( s );
	}

}


void ONMainWindow::slotSuspendSess()
{

	QString passwd;
	QString user=getCurrentUname();

	passwd=getCurrentPass();

	selectSessionDlg->setEnabled ( false );


	QString sessId=sessTv->model()->index (
	                   sessTv->currentIndex().row(),
	                   S_ID ).data().toString();
	QString host=sessTv->model()->index (
	                 sessTv->currentIndex().row(),
	                 S_SERVER ).data().toString();
	if ( !useLdap )
	{
		if ( !embedMode )
		{
#ifndef Q_OS_WIN
			QSettings st ( homeDir +
			               "/.x2goclient/sessions",
			               QSettings::NativeFormat );
#else

			QSettings st ( "Obviously Nice","x2goclient" );
			st.beginGroup ( "sessions" );
#endif
			QString sid=lastSession->id();
			host=st.value ( sid+"/host",
			                ( QVariant ) host ).toString();
		}
		else
		{
			host=config.server;
		}
	}
	suspendSession ( user,host,passwd,currentKey,sessId );
}


void ONMainWindow::slotSuspendSessFromSt()
{
	QString passwd;
	QString user=getCurrentUname();
	passwd=getCurrentPass();
	setStatStatus ( tr ( "suspending" ) );

	x2goDebug <<"disconnect export"<<endl;
	disconnect ( sbExp,SIGNAL ( clicked() ),this,
	             SLOT ( slot_exportDirectory() ) );
	sbExp->setEnabled ( false );

	suspendSession ( user,resumingSession.server,
	                 passwd,currentKey,
	                 resumingSession.sessionId );
}

void ONMainWindow::slotTermSessFromSt()
{
	QString passwd;
	QString user=getCurrentUname();
	passwd=getCurrentPass();

	x2goDebug <<"disconnect export"<<endl;
	disconnect ( sbExp,SIGNAL ( clicked() ),this,
	             SLOT ( slot_exportDirectory() ) );
	sbExp->setEnabled ( false );

	if ( termSession ( user,resumingSession.server,passwd,
	                   currentKey,
	                   resumingSession.sessionId ) )
		setStatStatus ( tr ( "terminating" ) );
}


void ONMainWindow::slot_retSuspSess ( bool result, QString output,
                                      sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr (
			            "<b>Wrong password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
	else
	{
		if ( selectSessionDlg->isVisible() )
		{
			( ( QStandardItemModel* )
			  ( sessTv->model() ) )->item (
			      sessTv->currentIndex().row(),
			      S_STATUS )->setData (
			          QVariant ( ( QString ) tr ( "suspended" ) ),
			          Qt::DisplayRole );
			bSusp->setEnabled ( false );
			sOk->setEnabled ( true );

		}
	}
	if ( selectSessionDlg->isVisible() )
		selectSessionDlg->setEnabled ( true );
}



void ONMainWindow::slotTermSess()
{
	QString passwd;
	QString user=getCurrentUname();
	passwd=getCurrentPass();

	selectSessionDlg->setEnabled ( false );


	QString sessId=sessTv->model()->index ( sessTv->currentIndex().row(),
	                                        S_ID ).data().toString();
	QString host=sessTv->model()->index ( sessTv->currentIndex().row(),
	                                      S_SERVER ).data().toString();

	if ( !useLdap )
	{
		if ( !embedMode )
		{
#ifndef Q_OS_WIN
			QSettings st ( homeDir +
			               "/.x2goclient/sessions",
			               QSettings::NativeFormat );
#else

			QSettings st ( "Obviously Nice","x2goclient" );
			st.beginGroup ( "sessions" );
#endif

			QString sid=lastSession->id();
			host=st.value ( sid+"/host",
			                ( QVariant ) host ).toString();
		}
		else
		{
			host=config.server;
		}

	}

	termSession ( user,host,passwd,currentKey,sessId );
}


void ONMainWindow::slotNewSess()
{
	startNewSession();
}


void ONMainWindow::slot_retTermSess ( bool result,  QString output,
                                      sshProcess* proc )
{
	bool nodel= ( proc==0 );
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr (
			            "<b>Wrong password!</b><br><br>" ) +message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
	else
	{
		if ( selectSessionDlg->isVisible() &&!nodel )
		{
			sessTv->model()->removeRow (
			    sessTv->currentIndex().row() );
			slot_activated ( sessTv->currentIndex() );
		}
	}
	if ( selectSessionDlg->isVisible() )
		selectSessionDlg->setEnabled ( true );
}

void ONMainWindow::slot_retResumeSess ( bool result,
                                        QString output,
                                        sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr (
			            "<b>Wrong Password!</b><br><br>" ) +message;
		}
		if ( output.indexOf ( "LIMIT" ) !=-1 )
		{
			QString sessions=output.mid ( output.indexOf ( "LIMIT" ) +6 );

			message="Sessions limit reached:"+sessions;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        message,QMessageBox::Ok,
		                        QMessageBox::NoButton );
		slot_showPassForm();
		return;
	}

	QString passwd=getCurrentPass();
	QString user=getCurrentUname();
	QString host;

	bool sound=true;
	int sndSystem=PULSE;
	QString sndPort;
#ifndef Q_OS_WIN
	sndPort="4713";
#endif
	bool startSoundServer=true;
	bool sshSndTunnel=true;

	if ( useLdap )
	{
		sound=startSound;
		startSoundServer=LDAPSndStartServer;
		if ( LDAPSndSys=="arts" )
			sndSystem=ARTS;
		if ( LDAPSndSys=="esd" )
			sndSystem=ESD;
		sndPort=LDAPSndPort;
	}
	else
	{
		QString sid;
		if ( !embedMode )
			sid=lastSession->id();
		else
			sid="embedded";
#ifndef Q_OS_WIN
		QSettings st ( homeDir +
		               "/.x2goclient/sessions",
		               QSettings::NativeFormat );
#else
		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif

		sound=st.value ( sid+"/sound",
		                 ( QVariant ) true ).toBool();
		QString sndsys=st.value (
		                   sid+"/soundsystem",
		                   ( QVariant ) "pulse" ).toString();
		if ( sndsys=="arts" )
			sndSystem=ARTS;
		if ( sndsys=="esd" )
			sndSystem=ESD;
#ifndef Q_OS_WIN
		sndPort=st.value ( sid+"/sndport" ).toString();
#endif
		startSoundServer=st.value (
		                     sid+"/startsoundsystem",
		                     true ).toBool();
#ifndef Q_OS_WIN
		bool defPort=st.value ( sid+
		                        "/defsndport",true ).toBool();
		if ( defPort )
		{
			switch ( sndSystem )
			{
				case PULSE:
					sndPort="4713";break;
				case ESD:
					sndPort="16001";break;
			}
		}
#endif
		sshSndTunnel=st.value ( sid+"/soundtunnel",
		                        true ).toBool();

#ifdef Q_OS_WIN
		switch ( sndSystem )
		{
			case PULSE:
				sndPort=QString::number ( pulsePort );break;
			case ESD:
				sndPort=QString::number ( esdPort );break;
		}
#endif
	}

	//Will be used in runCommand
	startSessSound=sound;
	startSessSndSystem=sndSystem;

	if ( newSession )
	{
		QString sString=output.trimmed();
		sString.replace ( '\n','|' );
		host=resumingSession.server;
		resumingSession=getNewSessionFromString ( sString );
		resumingSession.server=host;
		resumingSession.crTime=QDateTime::currentDateTime().toString (
		                           "dd.MM.yy HH:mm:ss" );
	}
	else
		host=resumingSession.server;
	if ( !useLdap )
	{
		if ( !embedMode )
		{
#ifndef Q_OS_WIN
			QSettings st ( homeDir +"/.x2goclient/sessions",
			               QSettings::NativeFormat );
#else

			QSettings st ( "Obviously Nice","x2goclient" );
			st.beginGroup ( "sessions" );
#endif

			QString sid=lastSession->id();
			host=st.value ( sid+"/host",
			                ( QVariant ) host ).toString();
		}
		else
			host=config.server;
		resumingSession.server=host;
	}
	try
	{
		tunnel=new sshProcess ( this,user,host,sshPort,
		                        QString::null,
		                        passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_tunnelFailed ( false,message,0 );
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=tunnel->environment();
		env+=sshEnv;
		tunnel->setEnvironment ( env );
	}


	connect ( tunnel,SIGNAL ( sshFinished ( bool,  QString,sshProcess* ) ),
	          this,SLOT ( slot_tunnelFailed ( bool,
	                                          QString,sshProcess* ) ) );
	connect ( tunnel,SIGNAL ( sshTunnelOk() ),
	          this,SLOT ( slot_tunnelOk() ) );

	localGraphicPort=resumingSession.grPort;
	int iport=localGraphicPort.toInt();
	while ( isServerRunning ( iport ) )
		++iport;
	localGraphicPort=QString::number ( iport );

	try
	{
		tunnel->startTunnel ( "localhost",localGraphicPort,
		                      resumingSession.grPort );
	}
	catch ( QString message )
	{
		slot_tunnelFailed ( false,message,0 );
		return;
	}

	if ( sndSystem==PULSE )
	{
		startSoundServer=false;
		QString scmd;
		if ( !sshSndTunnel )
			scmd="echo \"default-server=`echo "
			     "$SSH_CLIENT | awk '{print $1}'`:"+
			     sndPort+
			     "\"> ~/.x2go/C-"+
			     resumingSession.sessionId+
			     "/.pulse-client.conf"
			     ";echo \"cookie-file=.x2go/C-"+
			     resumingSession.sessionId+
			     "/.pulse-cookie"+
			     "\">> ~/.x2go/C-"+
			     resumingSession.sessionId+
			     "/.pulse-client.conf";
		else
			scmd="echo \"default-server=localhost:"+
			     resumingSession.sndPort+
			     "\"> ~/.x2go/C-"+
			     resumingSession.sessionId+
			     "/.pulse-client.conf"
			     ";echo \"cookie-file=.x2go/C-"+
			     resumingSession.sessionId+
			     "/.pulse-cookie"+
			     "\">> ~/.x2go/C-"+
			     resumingSession.sessionId+
			     "/.pulse-client.conf";
		sshProcess* paProc;
		try
		{
			paProc=new sshProcess ( this,user,host,sshPort,
			                        scmd,
			                        passwd,currentKey,acceptRsa );
		}
		catch ( QString message )
		{
			return;
		}

		if ( cardReady || useSshAgent )
		{
			QStringList env=paProc->environment();
			env+=sshEnv;
			paProc->setEnvironment ( env );
		}
		paProc->startNormal();
		try
		{
			paProc=new sshProcess ( this,user,host,sshPort,
			                        scmd,
			                        passwd,currentKey,acceptRsa );
		}
		catch ( QString message )
		{
			return;
		}

		if ( cardReady || useSshAgent )
		{
			QStringList env=paProc->environment();
			env+=sshEnv;
			paProc->setEnvironment ( env );
		}

		bool sysPulse=false;
#ifdef Q_OS_LINUX
		loadPulseModuleNativeProtocol();
		QFile file ( "/etc/default/pulseaudio" );
		if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		{

			while ( !file.atEnd() )
			{
				QByteArray line = file.readLine();
				int pos=line.indexOf (
				            "PULSEAUDIO_SYSTEM_START=1" );
				if ( pos!=-1 )
				{
					int commentPos=line.indexOf ( "#" );
					if ( commentPos==-1 || commentPos>pos )
					{
						sysPulse=true;
						break;
					}
				}
			}
			file.close();
		}
#endif
		if ( sysPulse )
			paProc->start_cp ( "/var/run/pulse/.pulse-cookie",
			                   "~/.x2go/C-"+
			                   resumingSession.sessionId+
			                   "/.pulse-cookie" );
		else
		{
#ifndef Q_OS_WIN
			paProc->start_cp ( homeDir+"/.pulse-cookie",
			                   "~/.x2go/C-"+
			                   resumingSession.sessionId+
			                   "/.pulse-cookie" );
#else
			QString cooFile=
			    wapiShortFileName ( homeDir )  +
			    "/.x2go/pulse/.pulse-cookie";
			QString destFile="~/.x2go/C-"+
			                 resumingSession.sessionId+
			                 "/.pulse-cookie";
			paProc->start_cp ( cooFile,
			                   destFile );

			/*x2goDebug<<"cookie file: "<<cooFile<<" remote:"<<
			destFile<<endl;*/
			connect ( paProc,
			          SIGNAL ( sshFinished ( bool,
			                                 QString,
			                                 sshProcess* ) ),
			          this,
			          SLOT ( slotPCookieReady ( bool,
			                                    QString,
			                                    sshProcess* ) ) );
			parecTunnelOk=true;
#endif
		}
	}
	if ( sndSystem==ESD )
	{
		sshProcess* paProc;
		try
		{
			paProc=new sshProcess ( this,user,host,sshPort,
			                        "",
			                        passwd,currentKey,acceptRsa );
		}
		catch ( QString message )
		{
			return;
		}

		if ( cardReady || useSshAgent )
		{
			QStringList env=paProc->environment();
			env+=sshEnv;
			paProc->setEnvironment ( env );
		}
#ifndef Q_OS_WIN
		paProc->start_cp ( homeDir+"/.esd_auth",
		                   "~/.esd_auth" );
#else
		QString cooFile=
		    wapiShortFileName ( homeDir )  +
		    "/.x2go/pulse/.esd_auth";
		QString destFile="~/.esd_auth";
		paProc->start_cp ( cooFile,
		                   destFile );
#endif
	}
	sndTunnel=0l;
	if ( sound )
	{
#ifndef Q_OS_WIN
		if ( startSoundServer )
		{
			soundServer=new QProcess ( this );
			QString acmd="artsd",ecmd="esd";
#ifdef Q_OS_DARWIN
			QStringList env = soundServer->environment();
			QDir dir ( appDir );
			dir.cdUp();
			dir.cd ( "esd" );
			env.insert ( 0,"DYLD_LIBRARY_PATH="+
			             dir.absolutePath() );
			soundServer->setEnvironment ( env );
			ecmd="\""+dir.absolutePath() +"\"/esd";
#endif //Q_OS_DARWIN
			if ( sndSystem==ESD )
				soundServer->start (
				    ecmd+
				    " -tcp -nobeeps -bind localhost -port "+
				    resumingSession.sndPort );
			if ( sndSystem==ARTS )
				soundServer->start ( acmd+" -u -N -p "+
				                     resumingSession.sndPort );
			sndPort=resumingSession.sndPort;
		}
#endif //Q_OS_WIN
		if ( sshSndTunnel )
		{
			try
			{
				sndTunnel=new sshProcess ( this,user,host,
				                           sshPort,
				                           QString::null,
				                           passwd,currentKey,
				                           acceptRsa );
			}
			catch ( QString message )
			{
				slot_sndTunnelFailed ( false,message,0 );
			}
			if ( cardReady || useSshAgent )
			{
				QStringList env=sndTunnel->environment();
				env+=sshEnv;
				sndTunnel->setEnvironment ( env );
			}

#ifdef Q_OS_WIN
			if ( sndSystem==PULSE )
			{
				parecTunnelOk=false;
				connect ( sndTunnel,SIGNAL ( sshTunnelOk() ),
				          this,SLOT ( slotSndTunOk() ) );
			}
#endif
			connect ( sndTunnel,SIGNAL ( sshFinished ( bool,
			                             QString,
			                             sshProcess* ) ),
			          this,SLOT (
			              slot_sndTunnelFailed ( bool,
			                                     QString,
			                                     sshProcess* ) ) );

			try
			{
				sndTunnel->startTunnel (
				    "localhost",
				    resumingSession.sndPort,
				    sndPort,true );
				/*x2goDebug<<"starting tunnel, local port:"<<
					sndPort<<", remote: "<<
					resumingSession.sndPort<<
					endl;*/
			}
			catch ( QString message )
			{
				slot_sndTunnelFailed ( false,message,0 );
				return;
			}
		}
	}
}



x2goSession ONMainWindow::getSelectedSession()
{
	QString sessId=sessTv->model()->index ( sessTv->currentIndex().row(),
	                                        S_ID ).data().toString();
	for ( int i=0;i<selectedSessions.size();++i )
	{
		if ( selectedSessions[i].sessionId==sessId )
			return selectedSessions[i];
	}
	return selectedSessions[0]; //warning !!!!! undefined session
}


void ONMainWindow::slot_tunnelOk()
{
	showExport=false;
	QString nxroot=homeDir +"/.x2go";
	QString dirpath=nxroot+"/S-"+resumingSession.sessionId;
	QDir d ( dirpath );
	if ( !d.exists() )
		if ( !d.mkpath ( dirpath ) )
		{
			QString message=tr ( "Unable to create folder:" ) +
			                dirpath;
			QMessageBox::critical ( 0l,tr ( "Error" ),message,
			                        QMessageBox::Ok,
			                        QMessageBox::NoButton );
			slot_showPassForm();
			if ( tunnel )
				delete tunnel;
			if ( sndTunnel )
				delete sndTunnel;
			if ( fsTunnel )
				delete fsTunnel;
			if ( soundServer )
				delete soundServer;
			tunnel=sndTunnel=fsTunnel=0l;
			soundServer=0l;
			nxproxy=0l;
			return;
		}
#ifdef Q_OS_WIN
	dirpath=wapiShortFileName ( dirpath );
	nxroot=wapiShortFileName ( nxroot );
#endif
	QFile file ( dirpath+"/options" );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to write file:" ) +
		                dirpath+"/options";
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		slot_showPassForm();
		return;
	}

	QTextStream out ( &file );
#ifdef Q_OS_WIN
	dirpath=cygwinPath ( dirpath );
	nxroot=cygwinPath ( nxroot );
#endif
	out << "nx/nx,root="<<nxroot<<",connect=localhost,cookie="<<
	resumingSession.cookie<<",port="
	<<localGraphicPort/*resumingSession.grPort*/<<
	",errors="<<dirpath<<"/sessions:"<<
	resumingSession.display;
	file.close();
	xmodExecuted=false;
	nxproxy=new QProcess;
	QStringList env = QProcess::systemEnvironment();
	QString x2golibpath="/usr/lib/x2go";
#if defined ( Q_OS_WIN ) || defined ( Q_OS_DARWIN )
	int dispInd=-1;
#endif
	for ( int l=0;l<env.size();++l )
	{
		if ( env[l].indexOf ( "X2GO_LIB" ) ==0 )
		{
			x2golibpath=env[l].split ( "=" ) [1];
		}
#if defined ( Q_OS_WIN ) || defined ( Q_OS_DARWIN )
		if ( env[l].indexOf ( "DISPLAY" ) ==0 )
		{
			dispInd=l;
		}
#endif

	}
	env << "LD_LIBRARY_PATH="+x2golibpath;
	env << "NX_CLIENT="+QCoreApplication::applicationFilePath ();
#ifdef Q_OS_DARWIN
	//setting /usr/X11/bin to find xauth
	env.insert (
	    0,
	    "PATH=/usr/bin:/bin:/usr/sbin:/sbin:/usr/local/bin:/usr/X11/bin" );
#endif

#if defined ( Q_OS_WIN ) || defined ( Q_OS_DARWIN )
	QString disp=getXDisplay();
	if ( disp==QString::null )
	{
		slot_proxyerror ( QProcess::FailedToStart );
		return;
	}
	if ( dispInd==-1 )
	{
		env <<"DISPLAY=localhost:"+disp;
	}
	else
	{
		env[dispInd]="DISPLAY=localhost:"+disp;
	}
#endif
	nxproxy->setEnvironment ( env );

	connect ( nxproxy,SIGNAL ( error ( QProcess::ProcessError ) ),this,
	          SLOT ( slot_proxyerror ( QProcess::ProcessError ) ) );
	connect ( nxproxy,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),this,
	          SLOT ( slot_proxyFinished ( int,QProcess::ExitStatus ) ) );
	connect ( nxproxy,SIGNAL ( readyReadStandardError() ),this,
	          SLOT ( slot_proxyStderr() ) );
	connect ( nxproxy,SIGNAL ( readyReadStandardOutput() ),this,
	          SLOT ( slot_proxyStdout() ) );

	QString proxyCmd="nxproxy -S nx/nx,options="+dirpath+"/options:"+
	                 resumingSession.display;
#ifdef Q_OS_DARWIN
	//run nxproxy from bundle
	QDir dir ( appDir );
	dir.cdUp();
	dir.cd ( "exe" );
	proxyCmd="\""+dir.absolutePath() +"/\""+proxyCmd;
#endif //Q_OS_DARWIN
	x2goDebug<<"starting nxproxy with: "<<proxyCmd<<endl;
	nxproxy->start ( proxyCmd );
	if ( embedMode )
	{
		proxyWinTimer->start ( 300 );
		if ( !startEmbedded )
		{
			act_embedContol->setText (
			    tr ( "Attach X2Go window" ) );
		}
	}
#ifdef Q_OS_WIN
	else
		proxyWinTimer->start ( 300 );
#endif
	showSessionStatus();
	QTimer::singleShot ( 30000,this,SLOT ( slot_restartNxProxy() ) );

}

void ONMainWindow::slot_tunnelFailed ( bool result,  QString output,
                                       sshProcess* )
{
	if ( result==false )
	{
		QString message=tr ( "Unable to create SSL tunnel:\n" ) +output;
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		if ( tunnel )
			delete tunnel;
		if ( sndTunnel )
			delete sndTunnel;
		if ( fsTunnel )
			delete fsTunnel;
		if ( soundServer )
			delete soundServer;
		tunnel=sndTunnel=fsTunnel=0l;
		soundServer=0l;
		nxproxy=0l;
		slot_showPassForm();
	}
}

void ONMainWindow::slot_sndTunnelFailed ( bool result,  QString output,
        sshProcess* )
{
	if ( result==false )
	{
		QString message=tr ( "Unable to create SSL Tunnel:\n" ) +output;
		QMessageBox::warning ( 0l,tr ( "Warning" ),message,
		                       QMessageBox::Ok,
		                       QMessageBox::NoButton );
		if ( sndTunnel )
			delete sndTunnel;
		sndTunnel=0l;
	}
}



void ONMainWindow::slot_proxyerror ( QProcess::ProcessError )
{
	slot_proxyFinished ( -1,QProcess::CrashExit );
}


void ONMainWindow::slot_proxyFinished ( int,QProcess::ExitStatus )
{
	if ( embedMode )
	{
		proxyWinTimer->stop();
		setEmbedSessionActionsEnabled ( false );
	}
#ifdef Q_OS_WIN
	else
		proxyWinTimer->stop();
#endif
	if ( tunnel )
		delete tunnel;
	if ( sndTunnel )
		delete sndTunnel;
	if ( fsTunnel )
		delete fsTunnel;
	if ( soundServer )
		delete soundServer;
	if ( spoolTimer )
		delete spoolTimer;

	if ( nxproxy )
		delete nxproxy;
	spoolTimer=0l;
	tunnel=sndTunnel=fsTunnel=0l;
	soundServer=0l;
	nxproxy=0l;
	proxyWinId=0;

	if ( !usePGPCard )
		check_cmd_status();
	if ( startHidden )
		close();

	if ( readExportsFrom!=QString::null )
	{
		exportTimer->stop();
		if ( extLogin )
		{
			currentKey=QString::null;
		}
	}
	if ( printSupport )
		cleanPrintSpool();
	if ( !restartResume )
	{
		if ( !embedMode )
		{
			pass->setText ( "" );
			QTimer::singleShot ( 2000,this,
			                     SLOT ( slot_showPassForm() ) );
		}
	}
	else
	{
		restartResume=false;
		sessionStatusDlg->hide();
		resumeSession ( resumingSession );
	}
	setStatStatus ( tr ( "Finished" ) );
}


void ONMainWindow::slot_proxyStderr()
{
	QString reserr;
	if ( nxproxy )
		reserr= nxproxy->readAllStandardError();
	stInfo->insertPlainText ( reserr );
	stInfo->ensureCursorVisible();
	if ( stInfo->toPlainText().indexOf (
	            "Connecting to remote host 'localhost:"+
	            /*resumingSession.grPort*/ localGraphicPort ) !=-1 )
		setStatStatus ( tr ( "connecting" ) );

	if ( stInfo->toPlainText().indexOf (
	            "Connection to remote proxy 'localhost:"+
	            /*resumingSession.grPort*/
	            localGraphicPort+"' established" ) !=-1 )
	{
		if ( newSession )
			setStatStatus ( tr ( "starting" ) );
		else
			setStatStatus ( tr ( "resuming" ) );
	}

	if ( stInfo->toPlainText().indexOf (
	            "Established X server connection" ) !=-1 )
	{
		setStatStatus ( tr ( "running" ) );
		if ( embedMode )
			setEmbedSessionActionsEnabled ( true );
		disconnect ( sbSusp,SIGNAL ( clicked() ),this,
		             SLOT ( slot_testSessionStatus() ) );
		disconnect ( sbSusp,SIGNAL ( clicked() ),this,
		             SLOT ( slotSuspendSessFromSt() ) );
		connect ( sbSusp,SIGNAL ( clicked() ),this,
		          SLOT ( slotSuspendSessFromSt() ) );
		if ( !showExport )
		{
			showExport=true;
			connect ( sbExp,SIGNAL ( clicked() ),this,
			          SLOT ( slot_exportDirectory() ) );
			sbExp->setEnabled ( true );
			exportDefaultDirs();
			if ( readExportsFrom!=QString::null )
			{
				exportTimer->start ( 2000 );
			}
		}
		sbSusp->setText ( tr ( "Suspend" ) );
		if ( newSession )
		{
			runCommand();
			newSession=false;
		}
#ifdef 	Q_WS_HILDON
		else
		{
			if ( !xmodExecuted )
			{
				xmodExecuted=true;
				QTimer::singleShot (
				    2000, this,
				    SLOT ( slot_execXmodmap() ) );
			}
		}
#endif
	}
	if ( stInfo->toPlainText().indexOf (
	            tr ( "Connection timeout, aborting" ) ) !=-1 )
		setStatStatus ( tr ( "aborting" ) );

}


void ONMainWindow::slot_proxyStdout()
{
// 	QString resout ( nxproxy->readAllStandardOutput() );
}


void ONMainWindow::slot_showPassForm()
{
	if ( !useLdap )
	{
		loginPrompt->show();
		login->show();
	}
	else
	{
		loginPrompt->hide();
		login->hide();
	}
	setEnabled ( true );
	if ( !embedMode )
	{
		u->hide();
		uname->hide();
	}
	sessionStatusDlg->hide();
	selectSessionDlg->hide();
	setEnabled ( true );
	if ( isPassShown )
	{
		passForm->show();
		passForm->setEnabled ( true );
	}
	isPassShown=true;
	login->setEnabled ( true );
	if ( login->text().length() >0 )
	{
		pass->setFocus();
		pass->selectAll();
	}
	else
		login->setFocus();


	if ( !embedMode )
	{
		u->setEnabled ( true );
	}
	else
	{
		if ( config.user.length() >0 )
			login->setEnabled ( false );
	}
}


void ONMainWindow::showSessionStatus()
{
	setStatStatus();
}


void ONMainWindow::slotShowAdvancedStat()
{
	if ( !miniMode )
	{
		if ( sbAdv->isChecked() )
		{
			sessionStatusDlg->setFixedSize (
			    sessionStatusDlg->width(),
			    sessionStatusDlg->height() *2 );
		}
		else
		{
			sessionStatusDlg->setFixedSize (
			    sessionStatusDlg->sizeHint() );
			stInfo->hide();
		}
	}
	else
	{
		if ( sbAdv->isChecked() )
		{
			sessionStatusDlg->setFixedSize ( 310,300 );
		}
		else
		{
			stInfo->hide();
			sessionStatusDlg->setFixedSize ( 310,200 );
		}
	}


// 	username->invalidate();


	if ( sbAdv->isChecked() )
	{
		stInfo->show();
	}

#ifndef Q_OS_WIN
	QSettings st ( homeDir +"/.x2goclient/settings",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "settings" );
#endif
	st.setValue ( "showStatus", ( QVariant ) sbAdv->isChecked() );
	st.sync();
}




void ONMainWindow::slot_resumeDoubleClick ( const QModelIndex& )
{
	slotResumeSess();
}


void ONMainWindow::suspendSession ( QString user,QString host,QString pass,
                                    QString key,QString sessId )
{
	sshProcess* proc=0l;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2gosuspend-session "+sessId,
		                      pass,key,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retSuspSess ( false,message,0 );
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool,  QString,sshProcess* ) ),
	          this,SLOT ( slot_retSuspSess ( bool,  QString,
	                                         sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retSuspSess ( false,message,0 );
		return;
	}
}


bool ONMainWindow::termSession ( QString user,QString host,QString pass,
                                 QString key, QString sessId )
{
	if ( QMessageBox::warning (
	            this,tr ( "Warning" ),
	            tr ( "Are you sure you want to terminate this session?\n"
	                 "Unsaved documents will be lost" ),
	            QMessageBox::Yes,QMessageBox::No ) !=QMessageBox::Yes )
	{
		slot_retTermSess ( true,QString::null,0 );
		return false;
	}

	sshProcess* proc=0l;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2goterminate-session "+sessId,
		                      pass,key,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retTermSess ( false,message,0 );
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool,  QString,sshProcess* ) ),
	          this,SLOT ( slot_retTermSess ( bool,
	                                         QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retTermSess ( false,message,0 );
		return true;
	}
	return true;
}



void ONMainWindow::setStatStatus ( QString status )
{
	setEnabled ( true );
	passForm->hide();
	selectSessionDlg->hide();
	if ( status == QString::null )
		status=statusString;
	else
		statusString=status;
	QString tstr;
	if ( statusLabel )
		statusLabel->setText ( QString::null );
	if ( resumingSession.sessionId!=QString::null )
	{
		QString f="dd.MM.yy HH:mm:ss";
		QDateTime dt=QDateTime::fromString ( resumingSession.crTime,f );
		dt=dt.addYears ( 100 );
		tstr=dt.toString();
	}
	if ( !embedMode || !proxyWinEmbedded )
	{
		statusBar()->hide();
		QString srv;
		if ( embedMode )
			srv=config.server;
		else
			srv=resumingSession.server;
		slVal->setText ( resumingSession.sessionId+"\n"+
		                 srv+"\n"+
		                 getCurrentUname() +"\n"+resumingSession.display+
		                 "\n"+tstr+"\n"+status );

		slVal->setFixedSize ( slVal->sizeHint() );
		sessionStatusDlg->show();
	}
	else
	{
		QString srv=config.server;
		QString message=getCurrentUname() +"@"+
		                srv+
		                ", "+tr ( "Session" ) +": "+
		                resumingSession.sessionId+", "+
		                tr ( "Display" ) +": "+
		                resumingSession.display+", "+
		                tr ( "Creation time" ) +": "+tstr;
		if ( statusLabel )
		{
			statusLabel->setText ( "   "+message );
		}
		else
		{
			statusBar()->show();
			statusBar()->showMessage ( message );
		}
		sessionStatusDlg->hide();
	}
}


void ONMainWindow::slot_restartNxProxy()
{
	if ( !sessionStatusDlg->isVisible() )
		return;
	if ( stInfo->toPlainText().indexOf (
	            "Established X server connection" ) ==-1 )
	{
		stInfo->insertPlainText (
		    tr (
		        "Connection timeout, aborting" ) );
		if ( nxproxy )
			nxproxy->terminate();
		restartResume=true;
	}
}


void ONMainWindow::slot_testSessionStatus()
{
	if ( !sessionStatusDlg->isVisible() )
		return;
	if ( stInfo->toPlainText().indexOf (
	            "Established X server connection" ) ==-1 )
	{
		stInfo->insertPlainText (
		    tr ( "Connection timeout, aborting" ) );
		if ( nxproxy )
			nxproxy->terminate();
	}
}


x2goSession ONMainWindow::getNewSessionFromString ( const QString& string )
{
	QStringList lst=string.split ( '|' );
	x2goSession s;
	s.display=lst[0];
	s.cookie=lst[1];
	s.agentPid=lst[2];
	s.sessionId=lst[3];
	s.grPort=lst[4];
	s.sndPort=lst[5];
	if ( lst.count() >6 )
		s.fsPort=lst[6];
	return s;
}


void ONMainWindow::runCommand()
{
	QString passwd=getCurrentPass();
	QString user=getCurrentUname();
	QString host=resumingSession.server;
	QString command;
	QString sessionType="D";
	QString rdpOpts,rdpServer;
	bool rdpFS=false;
	QString rdpWidth;
	QString rdpHeight;
	bool rootless=false;
	if ( !embedMode )
	{
#ifndef Q_OS_WIN
		QSettings st ( homeDir +"/.x2goclient/sessions",
		               QSettings::NativeFormat );
#else

		QSettings st ( "Obviously Nice","x2goclient" );
		st.beginGroup ( "sessions" );
#endif


		if ( useLdap )
			command=sessionCmd;
		else
		{
			QString sid=lastSession->id();
			command=st.value (
			            sid+"/command",
			            ( QVariant ) tr ( "KDE" ) ).toString();
			rdpOpts=st.value (
			            sid+"/rdpoptions",
			            ( QVariant ) "" ).toString();
			rdpServer=st.value (
			              sid+"/rdpserver",
			              ( QVariant ) "" ).toString();
			rootless=st.value ( sid+"/rootless",
			                    ( QVariant ) false ).toBool();

			rdpFS=st.value ( sid+"/fullscreen",
			                 ( QVariant ) defaultFullscreen ).toBool();
			rdpHeight=st.value ( sid+"/height",
			                     ( QVariant ) defaultHeight ).toString();
			rdpWidth=st.value ( sid+"/width",
			                    ( QVariant ) defaultWidth ).toString();

		}
	}
	else
	{
		command=config.command;
		rootless=config.rootless;
	}
	if ( rootless )
		sessionType="R";

	if ( command=="KDE" )
	{
		command="startkde";
	}
	else if ( command=="GNOME" )
	{
		command="gnome-session";
	}
	else if ( command=="LXDE" )
	{
		command="startlxde";
	}
	else if ( command=="RDP" )
	{
		command="rdesktop ";
		if ( rdpFS )
			command+=" -f ";
		else
			command+=" -g "+rdpWidth+"x"+rdpHeight;
		command+=" "+rdpOpts+ " "+rdpServer;

		sessionType="R";
	}


	sshProcess *proc=0l;

	QString cmd;

	command.replace ( " ","X2GO_SPACE_CHAR" );

	if ( !startSessSound  || startSessSndSystem==PULSE )
	{
		cmd="setsid x2goruncommand "+resumingSession.display+" "+
		    resumingSession.agentPid + " " +
		    resumingSession.sessionId+" "+
		    resumingSession.sndPort+ " "+ command+" nosnd "+
		    sessionType +">& /dev/null & exit";
		if ( startSessSndSystem ==PULSE )
		{
			cmd="PULSE_CLIENTCONFIG=~/.x2go/C-"+
			    resumingSession.sessionId+
			    "/.pulse-client.conf "+cmd;
		}
	}
	else
	{
		switch ( startSessSndSystem )
		{
			case ESD:
				cmd="setsid x2goruncommand "+
				    resumingSession.display+" "+
				    resumingSession.agentPid + " " +
				    resumingSession.sessionId+" "+
				    resumingSession.sndPort+ " "+
				    command+" esd "+
				    sessionType +">& /dev/null & exit";
				break;
			case ARTS:
				cmd="setsid x2goruncommand "+
				    resumingSession.display+" "+
				    resumingSession.agentPid + " " +
				    resumingSession.sessionId+" "+
				    resumingSession.sndPort+ " "+
				    command+" arts "+
				    sessionType +">& /dev/null & exit";
				break;

		}
	}


	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_retRunCommand ( false,message,0 );
	}
	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retRunCommand ( bool,
	                                           QString,sshProcess* ) ) );

	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retRunCommand ( false,message,0 );
		return;
	}
#ifdef Q_WS_HILDON
	//wait 5 seconds and execute xkbcomp
	QTimer::singleShot ( 5000, this, SLOT ( slot_execXmodmap() ) );
#endif
}

void ONMainWindow::slot_retRunCommand ( bool result, QString output,
                                        sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n:\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong password!</b><br><br>" ) +
			        message;
		}
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
}


bool ONMainWindow::parseParam ( QString param )
{
	if ( param=="--help" )
	{
		showHelp();
		return false;
	}

	if ( param=="--help-pack" )
	{
		showHelpPack();
		return false;
	}

	if ( param=="--no-menu" )
	{
		drawMenu=false;
		return true;
	}

	if ( param=="--maximize" )
	{
		startMaximized=true;
		return true;
	}
	if ( param=="--hide" )
	{
		startHidden=true;
		return true;
	}
	if ( param=="--pgp-card" )
	{
		usePGPCard=true;
		return true;
	}
	if ( param=="--add-to-known-hosts" )
	{
		acceptRsa=true;
		return true;
	}

	QString setting,value;
	QStringList vals=param.split ( "=" );
	if ( vals.size() <2 )
	{
		printError ( param );
		return false;
	}
	setting=vals[0];
	vals.removeFirst();
	value=vals.join ( "=" );
	if ( setting=="--link" )
	{
		return link_par ( value );
	}
	if ( setting=="--sound" )
	{
		return sound_par ( value );
	}
	if ( setting=="--geometry" )
	{
		return geometry_par ( value );
	}
	if ( setting=="--pack" )
	{
		return pack_par ( value );
	}
	if ( setting=="--kbd-layout" )
	{
		defaultKbdType=value;
		return true;
	}
	if ( setting=="--session" )
	{
		defaultSession=true;
		defaultSessionName=value;
		return true;
	}
	if ( setting=="--sessionid" )
	{
		defaultSession=true;
		defaultSessionId=value;
		return true;
	}
	if ( setting=="--user" )
	{
		defaultUser=true;
		defaultUserName=value;
		return true;
	}
	if ( setting=="--kbd-type" )
	{
		defaultKbdType=value;
		return true;
	}
	if ( setting=="--set-kbd" )
	{
		return setKbd_par ( value );
	}
	if ( setting=="--ldap" )
	{
		return ldap_par ( value );
	}
	if ( setting=="--ldap1" )
	{
		return ldap1_par ( value );
	}
	if ( setting=="--ldap2" )
	{
		return ldap2_par ( value );
	}
	if ( setting=="--command" )
	{
		defaultCmd=value;
		return true;
	}
	if ( setting=="--read-exports-from" )
	{
		readExportsFrom=value;
		return true;
	}
	if ( setting=="--external-login" )
	{
		extLogin=true;
		readLoginsFrom=value;
		return true;
	}
	if ( setting=="--ssh-port" )
	{
		defaultSshPort=value;
		return true;
	}
	if ( setting=="--dpi" )
	{
		defaultSetDPI=true;
		defaultDPI=value.toUInt();
		return true;
	}
	if ( setting=="--client-ssh-port" )
	{
		clientSshPort=value;
		return true;
	}
	if ( setting == "--embed-into" )
	{
		embedMode=true;
		embedParent=value.toLong();
		return true;
	}
	if ( setting == "--config" )
	{
		sessionConfigFile=value;
		return true;
	}
	printError ( param );
	return false;
}


bool ONMainWindow::link_par ( QString value )
{
	if ( value=="modem" )
		defaultLink=0;
	else if ( value=="isdn" )
		defaultLink=1;
	else if ( value=="adsl" )
		defaultLink=2;
	else if ( value=="wan" )
		defaultLink=3;
	else if ( value=="lan" )
		defaultLink=4;
	else
	{
		qCritical (
		    "%s",tr (
		        "wrong value for argument\"--link\""
		    ).toLocal8Bit().data() );
		return false;
	}
	return true;

}

bool ONMainWindow::sound_par ( QString val )
{
	if ( val=="1" )
		defaultUseSound=true;
	else if ( val=="0" )
		defaultUseSound=false;
	else
	{
		qCritical (
		    "%s",tr ( "wrong value for "
		              "argument\"--sound\"" ).toLocal8Bit().data() );
		return false;
	}
	return true;
}

bool ONMainWindow::geometry_par ( QString val )
{
	if ( val=="fullscreen" )
		defaultFullscreen=true;
	else
	{
		QStringList res=val.split ( "x" );
		if ( res.size() !=2 )
		{
			qCritical (
			    "%s",tr (
			        "wrong value for argument\"--geometry\"" ).
			    toLocal8Bit().data() );
			return false;
		}
		bool o1,o2;
		defaultWidth=res[0].toInt ( &o1 );
		defaultHeight=res[1].toInt ( &o2 );
		if ( ! ( defaultWidth >0 && defaultHeight >0 && o1 && o2 ) )
		{
			qCritical (
			    "%s",tr (
			        "wrong value for argument\"--geometry\"" ).
			    toLocal8Bit().data() );
			return false;
		}
	}
	return true;
}

bool ONMainWindow::setKbd_par ( QString val )
{
	if ( val=="1" )
		defaultSetKbd=true;
	else if ( val=="0" )
		defaultSetKbd=false;
	else
	{
		qCritical (
		    "%s",tr (
		        "wrong value for argument\"--set-kbd\"" ).
		    toLocal8Bit().data() );
		return false;
	}
	return true;
}

bool ONMainWindow::ldap_par ( QString val )
{
	QString ldapstring=val;
	useLdap=true;
	ldapstring.replace ( "\"","" );
	QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
	if ( lst.size() !=3 )
	{
		qCritical (
		    "%s",tr (
		        "wrong value for argument\"--ldap\"" ).
		    toLocal8Bit().data() );
		return false;
	}
	ldapOnly=true;
	ldapServer=lst[0];
	ldapPort=lst[1].toInt();
	ldapDn=lst[2];


	return true;
}

bool ONMainWindow::ldap1_par ( QString val )
{
	QString ldapstring=val;
	ldapstring.replace ( "\"","" );
	QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
	if ( lst.size() !=2 )
	{
		qCritical (
		    "%s",tr (
		        "wrong value for argument\"--ldap1\"" ).
		    toLocal8Bit().data() );
		return false;
	}
	ldapServer1=lst[0];
	ldapPort1=lst[1].toInt();

	return true;
}

bool ONMainWindow::ldap2_par ( QString val )
{
	QString ldapstring=val;
	ldapstring.replace ( "\"","" );
	QStringList lst=ldapstring.split ( ':',QString::SkipEmptyParts );
	if ( lst.size() !=2 )
	{
		qCritical ( "%s",
		            tr (
		                "wrong value for argument\"--ldap2\"" ).
		            toLocal8Bit().data() );
		return false;
	}
	ldapServer2=lst[0];
	ldapPort2=lst[1].toInt();

	return true;
}


bool ONMainWindow::pack_par ( QString val )
{

	QFile file ( ":/txt/packs" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return true;
	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		QString pc=in.readLine();
		if ( pc.indexOf ( "-%" ) !=-1 )
		{
			pc=pc.left ( pc.indexOf ( "-%" ) );

			QStringList pctails=val.split ( "-" );
			QString pcq=pctails[pctails.size()-1];
			pctails.removeLast();

			if ( pctails.join ( "-" ) ==pc )
			{
				bool ok;
				int v=pcq.toInt ( &ok );
				if ( ok && v>=0 && v<=9 )
				{
					defaultPack=pc;
					defaultQuality=v;
					return true;
				}
				else
					break;
			}
		}
		else
		{
			if ( pc==val )
			{
				defaultPack=val;
				return true;
			}
		}
	}
	file.close();
	qCritical ( "%s",tr ( "wrong value for argument\"--pack\"" ).
	            toLocal8Bit().data() );
	return false;
}


void ONMainWindow::printError ( QString param )
{
	qCritical ( "%s", ( tr ( "wrong parameter: " ) +param ).
	            toLocal8Bit().data() );
#ifdef Q_OS_WIN
	x2goDebug<<tr ( "wrong parameter: " ) +param <<endl;
#endif
}

void ONMainWindow::showHelp()
{
	QString helpMsg=
	    "Usage: x2goclient [Options]\n"
	    "Options:\n"
	    "--help\t\t\t\t show this message\n"
	    "--help-pack\t\t\t show available pack methods\n"
	    "--no-menu\t\t\t hide menu bar\n"
	    "--maximize\t\t\t start maximized\n"
	    "--hide\t\t\t\t start hidden\n"
	    "--pgp-card\t\t\t use openPGP card authentication\n"
	    "--add-to-known-hosts\t\t add RSA key fingerprint to "
	    ".ssh/known_hosts\n"
	    "\t\t\t\t if authenticity of server can't be established\n\n"
	    "--ldap=<host:port:dn> \t\t start with LDAP support. Example:\n"
	    "\t\t\t\t --ldap=ldapserver:389:o=organization,c=de\n\n"
	    "--ldap1=<host:port>\t\t LDAP failover server #1 \n"
	    "--ldap2=<host:port>\t\t LDAP failover server #2 \n"
	    "--ssh-port=<port>\t\t connect to this port, default 22\n"
	    "--client-ssh-port=<port>\t local ssh port (for fs export), "
	    "default 22\n"
	    "--command=<cmd>\t\t\t Set default command, default value 'KDE'\n"
	    "--session=<session>\t\t Start session 'session'\n"
	    "--user=<username>\t\t in LDAP mode, select user 'username'\n"
	    "--geomerty=<W>x<H>|fullscreen\t set default geometry, default "
	    "value '800x600'\n"
	    "--dpi=<dpi>\t\t\t set dpi of x2goagent to dpi, default not set\n"
	    "--link=<modem|isdn|adsl|wan|lan> set default link type, "
	    "default 'adsl'\n"
	    "--pack=<packmethod>\t\t set default pack method, default "
	    "'16m-jpeg-9'\n"
	    "--kbd-layout=<layout>\t\t set default keyboard layout\n"
	    "--kbd-type=<typed>\t\t set default keyboard type\n"
	    "--set-kbd=<0|1>\t\t\t overwrite current keyboard settings\n" ;
	qCritical ( "%s",helpMsg.toLocal8Bit().data() );
	QMessageBox::information ( this,tr ( "Options" ),helpMsg );
}

void ONMainWindow::showHelpPack()
{
	qCritical ( "%s",tr (
	                "Available pack methodes:" ).toLocal8Bit().data() );
	QFile file ( ":/txt/packs" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;
	QTextStream in ( &file );
	QString msg;
	while ( !in.atEnd() )
	{
		QString pc=in.readLine();
		if ( pc.indexOf ( "-%" ) !=-1 )
		{
			pc=pc.left ( pc.indexOf ( "-%" ) );
			pc+="-[0-9]";
		}
		msg+=pc+"\n";
		qCritical ( "%s",pc.toLocal8Bit().data() );
	}
	file.close();
#ifdef Q_OS_WIN

	QMessageBox::information ( this,tr ( "Options" ),msg );
#endif

}

void ONMainWindow::slot_getServers ( bool result, QString output,
                                     sshProcess* proc )
{
	if ( proc )
		delete proc;
	proc=0;
	if ( result==false )
	{
		cardReady=false;
		cardStarted=false;

		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong password!</b><br><br>" ) +
			        message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		currentKey=QString::null;
		setEnabled ( true );
		passForm->setEnabled ( true );
		pass->setFocus();
		pass->selectAll();
		return;
	}

	passForm->hide();
	setUsersEnabled ( false );
	uname->setEnabled ( false );
	u->setEnabled ( false );
	QStringList servers=output.trimmed().split ( '\n' );
	for ( int i=0;i<servers.size();++i )
	{
		QStringList lst=servers[i].simplified().split ( ' ' );
		if ( lst.size() >1 )
		{
			for ( int j=0;j<x2goServers.size();++j )
				if ( x2goServers[j].name==lst[0] )
				{
					x2goServers[j].sess=
					    lst[1].toInt() *
					    x2goServers[j].factor;
					x2goDebug<<x2goServers[j].name<<
					": sessions "<<
					lst[1].toInt() <<
					", multiplied "<<x2goServers[j].sess;
					break;
				}
		}
	}

	qSort ( x2goServers.begin(),x2goServers.end(),serv::lt );

	listedSessions.clear();
	retSessions=0;
// TODO: should use x2golistsessions --all-servers to create less ssh sessions
	for ( int j=0;j<x2goServers.size();++j )
	{
		QString passwd;
		QString user=getCurrentUname();
		QString host=x2goServers[j].name;
		passwd=getCurrentPass();

		sshProcess* lproc;
		try
		{
			lproc=new sshProcess (
			    this,user,host,sshPort,
			    "export HOSTNAME && x2golistsessions",
			    passwd,currentKey,acceptRsa );
		}
		catch ( QString message )
		{
			slot_listAllSessions ( false,message,0 );
			continue;
		}
		if ( cardReady || useSshAgent )
		{
			QStringList env=lproc->environment();
			env+=sshEnv;
			lproc->setEnvironment ( env );
		}

		connect ( lproc,SIGNAL ( sshFinished ( bool,
		                                       QString,sshProcess* ) ),
		          this,SLOT (
		              slot_listAllSessions ( bool,
		                                     QString,sshProcess* ) ) );
		try
		{
			lproc->startNormal();
		}
		catch ( QString message )
		{
			slot_listAllSessions ( false,message,0 );
			continue;
		}
	}
}


void ONMainWindow::slot_listAllSessions ( bool result,QString output,
        sshProcess* proc )
{
	bool last=false;
	++retSessions;
	if ( retSessions == x2goServers.size() )
		last=true;
	if ( proc )
		delete proc;
	proc=0;
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong password!</b><br><br>" ) +
			        message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QString sv=output.split ( ":" ) [0];
		for ( int j=0;j<x2goServers.size();++j )
		{
			if ( x2goServers[j].name==sv )
			{
				x2goServers[j].connOk=false;
			}
		}
	}
	else
		listedSessions+=output.trimmed().split ( '\n',
		                QString::SkipEmptyParts );
	if ( last )
	{
		if ( listedSessions.size() ==0||
		        ( listedSessions.size() ==1 &&
		          listedSessions[0].length() <5 ) )
			startNewSession();
		else if ( listedSessions.size() ==1 )
		{
			x2goSession s=getSessionFromString (
			                  listedSessions[0] );
			QDesktopWidget wd;
			if ( s.status=="S" && isColorDepthOk (
			            wd.depth(),s.colorDepth ) )
				resumeSession ( s );
			else
				selectSession ( listedSessions );
		}
		else
			selectSession ( listedSessions );
	}
}

void ONMainWindow::slot_resize()
{
	if ( startHidden )
	{
		hide();
		return;
	}
	if ( !startMaximized && !mwMax )
	{
		resize ( mwSize );
		move ( mwPos );
		show();
	}
	else
		showMaximized();
}

void ONMainWindow::slot_exportDirectory()
{
	QString path;
	if ( !useLdap && !embedMode )
	{
		ExportDialog dlg ( lastSession->id(),this );
		if ( dlg.exec() ==QDialog::Accepted )
			path=dlg.getExport();
	}
	else
		path= QFileDialog::getExistingDirectory (
		          this,QString::null,
		          homeDir );
#ifdef Q_OS_WIN
	path=cygwinPath ( wapiShortFileName ( path ) );
#endif
	if ( path!=QString::null )
		exportDirs ( path );
}


void ONMainWindow::exportDirs ( QString exports,bool removable )
{
	fsExportKeyReady=false;
	directory dr;
	dr.dirList=exports;
	dr.key=createRSAKey();
	QString passwd;

	passwd=getCurrentPass();

	fsInTun=false;
	if ( !useLdap )
	{
		if ( !embedMode )
		{
#ifndef Q_OS_WIN
			QSettings st ( homeDir +"/.x2goclient/sessions",
			               QSettings::NativeFormat );
#else

			QSettings st ( "Obviously Nice","x2goclient" );
			st.beginGroup ( "sessions" );
#endif

			QString sid=lastSession->id();

			fsInTun=st.value ( sid+"/fstunnel",
			                   ( QVariant ) true ).toBool();
		}
		else
			fsInTun=true;
	}
	if ( fsInTun )
	{
		if ( fsTunnel==0l )
			if ( startSshFsTunnel() )
				return;
	}
	sshProcess* lproc;
	QString uname=getCurrentUname();
	try
	{
		lproc=new sshProcess ( this,uname,
		                       resumingSession.server,
		                       sshPort,"",passwd,
		                       currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_copyKey ( false,message,0 );
		return;
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=lproc->environment();
		env+=sshEnv;
		lproc->setEnvironment ( env );
	}

	connect ( lproc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_copyKey ( bool, QString,sshProcess* ) ) );
	try
	{
		QString dst=dr.key;
		QString dhdir=homeDir+"/.x2go";
#ifdef Q_OS_WIN
		dhdir=wapiShortFileName ( dhdir );
#endif
		dst.replace ( dhdir +"/ssh/gen/","" );
		dst="~"+uname +"/.x2go/ssh/"+dst;
		dr.dstKey=dst;
		dr.isRemovable=removable;
		exportDir.append ( dr );
		QString keyFile=dr.key;
		lproc->start_cp ( keyFile,dst );
	}
	catch ( QString message )
	{
		slot_copyKey ( false,message,0 );
	}

}


void ONMainWindow::exportDefaultDirs()
{
	QStringList dirs;
	bool clientPrinting= ( useLdap && LDAPPrintSupport );

	if ( !useLdap )
	{
		if ( !embedMode )
		{

#ifndef Q_OS_WIN
			QSettings st ( homeDir +"/.x2goclient/sessions",
			               QSettings::NativeFormat );
#else

			QSettings st ( "Obviously Nice","x2goclient" );
			st.beginGroup ( "sessions" );
#endif
			clientPrinting= st.value ( lastSession->id() +
			                           "/print", true ).toBool();

			QString exd=st.value ( lastSession->id() +"/export",
			                       ( QVariant ) QString::null ).toString();
			QStringList lst=exd.split ( ";",QString::SkipEmptyParts );
			for ( int i=0;i<lst.size();++i )
			{
#ifndef Q_OS_WIN
				QStringList tails=lst[i].split (
				                      ":",
				                      QString::SkipEmptyParts );
#else

				QStringList tails=lst[i].split (
				                      "#",
				                      QString::SkipEmptyParts );
#endif

				if ( tails[1]=="1" )
				{
#ifdef Q_OS_WIN
					tails[0]=cygwinPath (
					             wapiShortFileName (
					                 tails[0] ) );
#endif
					dirs+=tails[0];
				}
			}
		}
		else
			clientPrinting=true;
	}

	if ( clientPrinting )
	{
		QString path= homeDir +
		              "/.x2go/S-"+
		              resumingSession.sessionId +"/spool";
		QDir spooldir;
		if ( !spooldir.exists ( path ) )
		{
			if ( !spooldir.mkpath ( path ) )
			{
				QString message=
				    tr (
				        "Unable to create folder:" ) + path;
				QMessageBox::critical ( 0l,tr (
				                            "Error" ),message,
				                        QMessageBox::Ok,
				                        QMessageBox::NoButton );

			}
		}
		spoolDir=path;
#ifdef Q_OS_WIN
		path=cygwinPath (
		         wapiShortFileName (
		             path ) );
#endif
		QFile::setPermissions (
		    path,QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner );

		path+="__PRINT_SPOOL_";
		dirs+=path;
		printSupport=true;
		if ( spoolTimer )
			delete spoolTimer;
		spoolTimer=new QTimer ( this );
		connect ( spoolTimer,SIGNAL ( timeout() ),this,
		          SLOT ( slot_checkPrintSpool() ) );
		spoolTimer->start ( 2000 );
	}
	if ( dirs.size() <=0 )
		return;
	exportDirs ( dirs.join ( ":" ) );
}

QString ONMainWindow::createRSAKey()
{
	QDir dr;
	QString keyPath=homeDir +"/.x2go/ssh/gen";
	dr.mkpath ( keyPath );
#ifdef Q_OS_WIN
	keyPath=wapiShortFileName ( keyPath );
#endif
	QTemporaryFile fl ( keyPath+"/key" );
	fl.open();
	QString keyName=fl.fileName();
	fl.setAutoRemove ( false );
	fl.close();
	fl.remove();

	QStringList args;

	args<<"-t"<<"rsa"<<"-b"<<"1024"<<"-N"<<""<<"-f"<<keyName;
	x2goDebug <<keyName<<endl;

	if ( QProcess::execute ( "ssh-keygen",args ) !=0 )
	{
		x2goDebug <<"ssh-keygen failed" <<endl;
		return QString::null;
	}
	x2goDebug <<"ssh-keygen ok" <<endl;

	QFile rsa ( "/etc/ssh/ssh_host_rsa_key.pub" );
#ifdef Q_OS_WIN
	rsa.setFileName (
	    wapiShortFileName (
	        homeDir+"\\.x2go\\etc\\ssh_host_dsa_key.pub" ) );
#endif
#ifdef Q_OS_DARWIN
	rsa.setFileName ( "/etc/ssh_host_rsa_key.pub" );
#endif

	if ( !rsa.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		printSshDError();
		return QString::null;
	}

	QByteArray rsa_pub;

	if ( !rsa.atEnd() )
		rsa_pub = rsa.readLine();
	else
		return QString::null;

	QFile file ( keyName );
	if ( !file.open (
	            QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append )
	   )
		return keyName;
	QTextStream out ( &file );
	out<<"----BEGIN RSA IDENTITY----"<<rsa_pub;
	file.close();
	return keyName;
}

void ONMainWindow::slot_copyKey ( bool result,QString output,sshProcess* proc )
{
	fsExportKey=proc->getSource();
	if ( proc )
		delete proc;
	proc=0;
	QFile::remove ( fsExportKey );
	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong password!</b><br><br>" ) +
			        message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove ( fsExportKey+".pub" );
		return;
	}
	fsExportKeyReady=true;

	//start reverse mounting if RSA Key and FS tunnel are ready
	//start only once from slot_fsTunnelOk() or slot_copyKey().
	if ( !fsInTun || fsTunReady )
		startX2goMount();

}

directory* ONMainWindow::getExpDir ( QString key )
{
	for ( int i=0;i<exportDir.size();++i )
	{
		if ( exportDir[i].key==key )
			return &exportDir[i];
	}
	return 0l;
}




void ONMainWindow::slot_retExportDir ( bool result,QString output,
                                       sshProcess* proc )
{

	QString key;
	for ( int i=0;i<exportDir.size();++i )
		if ( exportDir[i].proc==proc )
		{
			key=exportDir[i].key;
			exportDir.removeAt ( i );
			break;
		}

	if ( proc )
		delete proc;

	if ( result==false )
	{
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong password!</b><br><br>" ) +
			        message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}
	QFile file ( key+".pub" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		printSshDError();
		QFile::remove
		( key+".pub" );
		return;
	}

	QByteArray line = file.readLine();
	file.close();
	QString authofname=homeDir;
#ifdef Q_OS_WIN
	authofname=wapiShortFileName ( authofname ) +"/.x2go";
#endif
	authofname+="/.ssh/authorized_keys" ;
	file.setFileName ( authofname );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		printSshDError();
		QFile::remove
		( key+".pub" );
		return;
	}


	QTemporaryFile tfile ( authofname );
	tfile.open();
	tfile.setAutoRemove ( true );
	QTextStream out ( &tfile );

	while ( !file.atEnd() )
	{
		QByteArray newline = file.readLine();
		if ( newline!=line )
			out<<newline;
	}
	file.close();
	tfile.close();
	file.remove();
	tfile.copy ( authofname );
	QFile::remove
	( key+".pub" );
}



void ONMainWindow::exportsEdit ( SessionButton* bt )
{
	EditConnectionDialog dlg ( bt->id(),this,3 );
	if ( dlg.exec() ==QDialog::Accepted )
	{
		bt->redraw();
		bool vis=bt->isVisible();
		placeButtons();
		users->ensureVisible ( bt->x(),bt->y(),50,220 );
		bt->setVisible ( vis );
	}
}


void ONMainWindow::slotExtTimer()
{

	if ( QFile::permissions ( readLoginsFrom ) !=
	        ( QFile::ReadUser|QFile::WriteUser|QFile::ExeUser|
	          QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) )
	{
		x2goDebug <<"Wrong permissions on "<<readLoginsFrom <<":"<<endl;
		x2goDebug << ( int ) ( QFile::permissions (
		                           readLoginsFrom+"/." ) )
		<<"must be"<< ( int ) ( QFile::ReadUser|QFile::WriteUser
		                        |QFile::ExeUser|QFile::ReadOwner|
		                        QFile::WriteOwner|
		                        QFile::ExeOwner ) <<endl;
		if ( extLogin )
			extTimer->stop();
		return;
	}
	QString loginDir;
	QString logoutDir;
	QDir dir ( readLoginsFrom );
	QStringList list = dir.entryList ( QDir::Files );
	for ( int i=0;i<list.size();++i )
	{
		QFile file ( readLoginsFrom+"/"+list[i] );
		if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
			continue;
		if ( !file.atEnd() )
		{
			QByteArray line = file.readLine();
			QString ln ( line );
			QStringList args=ln.split ( "=",
			                            QString::SkipEmptyParts );
			if ( args.size() >1 )
			{
				if ( args[0]=="login" )
				{
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						loginDir=args[1];
				}
				if ( args[0]=="logout" )
				{
					x2goDebug <<
					" I HAVE external logout"<<
					endl;
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						logoutDir=args[1];
				}
			}
		}
		file.close();
		file.remove();
	}
	if ( exportTimer->isActive() ) //running session
	{
		if ( logoutDir != QString::null )
		{
			x2goDebug <<"external logout"<<endl;
			externalLogout ( logoutDir );
		}
	}
	else
	{
		if ( loginDir != QString::null )
		{
			x2goDebug <<"external login"<<endl;
			externalLogin ( loginDir );
		}
	}
}


void ONMainWindow::slot_exportTimer()
{

	if ( QFile::permissions ( readExportsFrom ) != ( QFile::ReadUser|
	        QFile::WriteUser|
	        QFile::ExeUser|
	        QFile::ReadOwner|QFile::WriteOwner|QFile::ExeOwner ) )
	{
		x2goDebug <<"Wrong permissions on "<<
		readExportsFrom <<":"<<endl;
		x2goDebug << ( int ) ( QFile::permissions (
		                           readExportsFrom+"/." ) )
		<<"must be"<< ( int ) ( QFile::ReadUser|QFile::WriteUser
		                        |QFile::ExeUser|QFile::ReadOwner|
		                        QFile::WriteOwner|
		                        QFile::ExeOwner ) <<endl;
		exportTimer->stop();
		return;
	}

	QDir dir ( readExportsFrom );
	QStringList list = dir.entryList ( QDir::Files );
	QString expList;
	QString unexpList;
	QString loginDir;
	QString logoutDir;
	for ( int i=0;i<list.size();++i )
	{
		QFile file ( readExportsFrom+"/"+list[i] );
		if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
			continue;
		if ( !file.atEnd() )
		{
			QByteArray line = file.readLine();
			QString ln ( line );
			QStringList args=ln.split ( "=",
			                            QString::SkipEmptyParts );
			if ( args.size() >1 )
			{
				if ( args[0]=="export" )
				{
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						expList+=":"+args[1];
				}
				if ( args[0]=="unexport" )
				{
					args[1].replace ( "\n","" );
					if ( args[1].size() )
						unexpList+=":"+args[1];
				}
			}
		}
		file.close();
		file.remove();
	}
	QStringList args=expList.split ( ":",QString::SkipEmptyParts );
	expList=args.join ( ":" );
	if ( expList.size() >0 )
	{
		exportDirs ( expList,true );
	}
	args.clear();
	args=unexpList.split ( ":",QString::SkipEmptyParts );

	QString passwd=getCurrentPass();
	QString user=getCurrentUname();
	QString host=resumingSession.server;
	QString sessionId=resumingSession.sessionId;

	for ( int i=0;i<args.size();++i )
	{
		sshProcess* sproc=new sshProcess (
		    this,user,host,sshPort,
		    "export HOSTNAME && x2goumount_session "+
		    sessionId+" "+args[i],
		    passwd,currentKey,acceptRsa );
		if ( cardReady || useSshAgent )
		{
			QStringList env=sproc->environment();
			env+=sshEnv;
			sproc->setEnvironment ( env );
		}

		sproc->startNormal();
	}
}

void ONMainWindow::slot_about_qt()
{
	QMessageBox::aboutQt ( this );
}

void ONMainWindow::slot_about()
{
	QMessageBox::about (
	    this,tr ( "About X2GO client" ),
	    tr ( "<b>X2Go client V. " ) +VERSION+
	    " (</b>Qt - "+qVersion() +")"+
	    tr (
	        "</b><br> (C. 2006-2009 <b>obviously nice</b>: "
	        "Oleksandr Shneyder, Heinz-Markus Graesing)<br>"
	        "<br>Client for use with the X2Go network based "
	        "computing environment. This Client will be able "
	        "to connect to X2Go server(s) and start, stop, "
	        "resume and terminate (running) desktop sessions. "
	        "X2Go Client stores different server connections "
	        "and may automatically request authentification "
	        "data from LDAP directories. Furthermore it can be "
	        "used as fullscreen loginscreen (replacement for "
	        "loginmanager like xdm). Please visit x2go.org for "
	        "further information." ) );
}



void ONMainWindow::slot_rereadUsers()
{
	if ( !useLdap )
		return;
#ifdef USELDAP

	if ( ld )
	{
		delete ld;
		ld=0;
	}


	if ( ! initLdapSession ( false ) )
	{
		return;
	}


	list<string> attr;
	attr.push_back ( "uidNumber" );
	attr.push_back ( "uid" );


	list<LDAPBinEntry> result;
	try
	{
		ld->binSearch ( ldapDn.toStdString(),attr,
		                "objectClass=posixAccount",result );
	}
	catch ( LDAPExeption e )
	{
		QString message="Exeption in: ";
		message=message+e.err_type.c_str();
		message=message+" : "+e.err_str.c_str();
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,QMessageBox::NoButton );
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "Please check LDAP Settings" ),
		                        QMessageBox::Ok,QMessageBox::NoButton );
		slot_config();
		return;
	}

	list<LDAPBinEntry>::iterator it=result.begin();
	list<LDAPBinEntry>::iterator end=result.end();

	for ( ;it!=end;++it )
	{
		user u;
		QString uin=LDAPSession::getBinAttrValues (
		                *it,"uidNumber" ).front().getData();
		u.uin=uin.toUInt();
		if ( u.uin<firstUid || u.uin>lastUid )
		{
			continue;
		}
		u.uid=LDAPSession::getBinAttrValues (
		          *it,"uid" ).front().getData();
		if ( !findInList ( u.uid ) )
		{
			reloadUsers();
			return;
		}
	}
#endif
}

void ONMainWindow::reloadUsers()
{
	int i;
	for ( i=0;i<names.size();++i )
		names[i]->close();
	for ( i=0;i<sessions.size();++i )
		sessions[i]->close();

	userList.clear();
	sessions.clear();


	loadSettings();
	if ( useLdap )
	{
		act_new->setEnabled ( false );
		act_edit->setEnabled ( false );
		u->setText ( tr ( "Login:" ) );
		QTimer::singleShot ( 1, this, SLOT ( readUsers() ) );
	}
	else
	{
		act_new->setEnabled ( true );
		act_edit->setEnabled ( true );
		u->setText ( tr ( "Session:" ) );
		QTimer::singleShot ( 1, this, SLOT ( slot_readSessions() ) );
	}
	slot_resize ( fr->size() );
}


bool ONMainWindow::findInList ( const QString& uid )
{
	for ( int i=0;i<userList.size();++i )
	{
		if ( userList[i].uid==uid )
			return true;
	}
	return false;
}

void ONMainWindow::setUsersEnabled ( bool enable )
{

	if ( useLdap )
	{
		QScrollBar* bar=users->verticalScrollBar();
		bar->setEnabled ( enable );
		int upos=bar->value();
		QDesktopWidget dw;
		int height=dw.screenGeometry ( fr ).height();
		QList<UserButton*>::iterator it;
		QList<UserButton*>::iterator endit=names.end();
		if ( !enable )
		{
			for ( it=names.begin();it!=endit;it++ )
			{
				QPoint pos= ( *it )->pos();
				if ( ( pos.y() >upos-height ) &&
				        ( pos.y() <upos+height ) )
					( *it )->setEnabled ( false );
				if ( pos.y() >upos+height )
					break;
			}
		}
		else
		{
			for ( it=names.begin();it!=endit;it++ )
			{
				if ( ! ( *it )->isEnabled() )
					( *it )->setEnabled ( true );
			}
		}
	}
	else
		users->setEnabled ( enable );
}


void ONMainWindow::externalLogin ( const QString& loginDir )
{
	QFile file ( loginDir+"/username" );
	QString user;

	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
		return;
	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		user=in.readLine();
		break;
	}
	file.close();


	if ( passForm->isVisible() )
		slotClosePass();
	uname->setText ( user );
	slotUnameEntered();
	currentKey=loginDir+"/dsa.key";
	extStarted=true;
	slotPassEnter();
}


void ONMainWindow::externalLogout ( const QString& )
{
	if ( extStarted )
	{
		extStarted=false;
		currentKey=QString::null;
		if ( nxproxy )
			if ( nxproxy->state() ==QProcess::Running )
				nxproxy->terminate();
	}
}


void ONMainWindow::slot_startPGPAuth()
{
	scDaemon=new QProcess ( this );
	QStringList arguments;
	arguments<<"--multi-server";
	connect ( scDaemon,SIGNAL ( readyReadStandardError() ),this,
	          SLOT ( slot_scDaemonError() ) );
	connect ( scDaemon,SIGNAL ( readyReadStandardOutput() ),this,
	          SLOT ( slot_scDaemonOut() ) );
	connect ( scDaemon,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),
	          this,
	          SLOT (
	              slot_scDaemonFinished ( int, QProcess::ExitStatus ) ) );
	scDaemon->start ( "scdaemon",arguments );
	QTimer::singleShot ( 3000, this, SLOT ( slot_checkScDaemon() ) );
	isScDaemonOk=false;
}

void ONMainWindow::slot_checkScDaemon()
{
	if ( !isScDaemonOk )
	{
		scDaemon->kill();
	}
}

void ONMainWindow::slot_scDaemonError()
{
	QString stdOut ( scDaemon->readAllStandardError() );
	stdOut=stdOut.simplified();
	x2goDebug<<"SCDAEMON err:"<<stdOut<<endl;
	if ( stdOut.indexOf ( "updating slot" ) !=-1 )
	{
		isScDaemonOk=true;
		if ( ( stdOut.indexOf ( "->0x0002" ) !=-1 ) ||
		        ( stdOut.indexOf ( "->0x0007" ) !=-1 ) ) //USABLE or PRESENT
		{
			scDaemon->kill();
		}
	}
}

void ONMainWindow::slot_scDaemonOut()
{
	QString stdOut ( scDaemon->readAllStandardOutput() );
	stdOut=stdOut.simplified();
	x2goDebug<<"SCDAEMON out:"<<stdOut<<endl;
}

void ONMainWindow::slot_scDaemonFinished ( int , QProcess::ExitStatus )
{
	scDaemon=0l;
	if ( isScDaemonOk )
	{
		x2goDebug<<"scDaemon finished"<<endl;
		gpg=new QProcess ( this );
		QStringList arguments;
		arguments<<"--card-status";
		connect ( gpg,SIGNAL ( readyReadStandardError() ),
		          this,SLOT ( slot_gpgError() ) );
		connect ( gpg,SIGNAL ( finished ( int,
		                                  QProcess::ExitStatus ) ),this,
		          SLOT ( slot_gpgFinished ( int,
		                                    QProcess::ExitStatus ) ) );
		gpg->start ( "gpg",arguments );
	}
	else
		slot_startPGPAuth();
}



void ONMainWindow::slot_gpgError()
{
	QString stdOut ( gpg->readAllStandardError() );
	stdOut=stdOut.simplified();
	x2goDebug<<"GPG err:"<<stdOut<<endl;
	if ( stdOut.indexOf ( "failed" ) !=-1 )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "No valid card found" ),
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		gpg->kill();
	}
}


void ONMainWindow::slot_gpgFinished ( int exitCode,
                                      QProcess::ExitStatus exitStatus )
{
	x2goDebug<<"gpg finished, exit code:"<<exitCode<<" exit status:"<<
	exitStatus<<endl;
	if ( exitStatus==0 )
	{
		QString stdOut ( gpg->readAllStandardOutput() );
		stdOut.chop ( 1 );
		x2goDebug<<"GPG out:"<<stdOut<<endl;
		QStringList lines=stdOut.split ( "\n" );
		QString login;
		QString appId;
		QString authKey;
		for ( int i=0;i<lines.count();++i )
		{
			if ( lines[i].indexOf ( "Application ID" ) !=-1 )
			{
				appId=lines[i].split ( ":" ) [1];
			}
			else if ( lines[i].indexOf ( "Login data" ) !=-1 )
			{
				login=lines[i].split ( ":" ) [1];
			}
			else if ( lines[i].indexOf (
			              "Authentication key" ) !=-1 )
			{
				authKey=lines[i].split ( ":" ) [1];
				break;
			}
		}
		appId=appId.simplified();
		login=login.simplified();
		authKey=authKey.simplified();
		x2goDebug<<"card data: "<<appId<<login<<authKey<<endl;
		if ( login=="[not set]" || authKey == "[none]" )
		{
			x2goDebug<<"Card not configured\n";
			QMessageBox::critical (
			    0l,tr ( "Error" ),
			    tr (
			        "This card is unknown by X2Go system" ),
			    QMessageBox::Ok,
			    QMessageBox::NoButton );
			QTimer::singleShot ( 1000, this,
			                     SLOT ( slot_startPGPAuth() ) );
		}
		else
			startGPGAgent ( login,appId );
	}
	else
		QTimer::singleShot ( 1000, this, SLOT ( slot_startPGPAuth() ) );
	gpg=0l;
}



void ONMainWindow::startGPGAgent ( const QString& login, const QString& appId )
{
	QString gpgPath=homeDir +"/.x2goclient/gnupg";
	QDir d;
	cardLogin=login;
	d.mkpath ( gpgPath );
	QFile file ( gpgPath+"/scd-event" );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
	{
		QMessageBox::critical (
		    0l,tr ( "Error" ),
		    tr (
		        "Unable to create file: " ) +
		    gpgPath+"/scd-event"
		    ,QMessageBox::Ok,
		    QMessageBox::NoButton );
		exit ( -1 );
	}
	QTextStream out ( &file );
	out << "#!/bin/bash\n\n"
	"if [ \"$6\" != \"0x0002\" ] && [ \"$6\" != "
	"\"0x0007\" ]\n\
	then\n\
	kill -9 $_assuan_pipe_connect_pid\n\
	fi"<<endl;
	file.close();
	file.setPermissions ( gpgPath+"/scd-event",
	                      QFile::ReadOwner|
	                      QFile::WriteOwner|
	                      QFile::ExeOwner );

	gpgAgent=new QProcess ( this );
	QStringList arguments;
	arguments<<"--pinentry-program"<<"/usr/bin/pinentry-x2go"<<
	"--enable-ssh-support"<<"--daemon"<<"--no-detach";

	connect ( gpgAgent,SIGNAL ( finished ( int,QProcess::ExitStatus ) ),
	          this,
	          SLOT ( slot_gpgAgentFinished ( int,
	                                         QProcess::ExitStatus ) ) );

	QStringList env=QProcess::systemEnvironment();
	env<<"GNUPGHOME="+gpgPath<<"CARDAPPID="+appId;
	gpgAgent->setEnvironment ( env );
	gpgAgent->start ( "gpg-agent",arguments );
}

void ONMainWindow::slot_gpgAgentFinished ( int , QProcess::ExitStatus )
{
	QString stdOut ( gpgAgent->readAllStandardOutput() );
	stdOut=stdOut.simplified();
	stdOut.replace ( " ","" );
	QStringList envLst=stdOut.split ( ";" );
	QString gpg_agent_info=envLst[0].split ( "=" ) [1];
	QString ssh_auth_sock=envLst[2].split ( "=" ) [1];
	agentPid=envLst[4].split ( "=" ) [1];
	x2goDebug<<gpg_agent_info<<ssh_auth_sock<<agentPid<<endl;
	x2goDebug<<"GPGAGENT out:"<<envLst[0]<<envLst[2]<<envLst[4]<<endl;

	agentCheckTimer->start ( 1000 );
	cardReady=true;

	sshEnv.clear();
	sshEnv<<envLst[0]<<envLst[2]<<envLst[4];
// 	x2goDebug<<"sshenv:"<<sshEnv<<endl;

	if ( !useLdap )
	{
		if ( passForm->isVisible() )
		{
			if ( passForm->isEnabled() )
			{
				if ( login->isEnabled() )
				{
					login->setText ( cardLogin );
					slotSessEnter();
					return;
				}
			}
		}
		QProcess sshadd ( this ); //using it to start scdaemon
		sshadd.setEnvironment ( sshEnv );
		QStringList arguments;
		arguments<<"-l";
		sshadd.start ( "ssh-add",arguments );
		sshadd.waitForFinished ( -1 );
		QString sshout ( sshadd.readAllStandardOutput() );
		sshout=sshout.simplified();
		x2goDebug<<"SSH-ADD out:"<<sshout<<endl;
	}
	else
	{
		if ( selectSessionDlg->isVisible() ||
		        sessionStatusDlg->isVisible() )
		{
			QProcess sshadd ( this ); //using it to start scdaemon
			sshadd.setEnvironment ( sshEnv );
			QStringList arguments;
			arguments<<"-l";
			sshadd.start ( "ssh-add",arguments );
			sshadd.waitForFinished ( -1 );
			QString sshout ( sshadd.readAllStandardOutput() );
			sshout=sshout.simplified();
			x2goDebug<<"SSH-ADD out:"<<sshout<<endl;
			return;
		}
		if ( passForm->isVisible() )
			slotClosePass();
		uname->setText ( cardLogin );
		slotUnameEntered();
		slotPassEnter();
	}
}


void ONMainWindow::slot_checkAgentProcess()
{
	if ( checkAgentProcess() )
		return;
	agentCheckTimer->stop();
	cardReady=false;
	if ( cardStarted )
	{
		cardStarted=false;
		if ( nxproxy )
			if ( nxproxy->state() ==QProcess::Running )
				nxproxy->terminate();
	}

	x2goDebug<<"gpg-agent finished\n";
	slot_startPGPAuth();
}

bool ONMainWindow::checkAgentProcess()
{
	QFile file ( "/proc/"+agentPid+"/cmdline" );
	if ( file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QString line ( file.readLine() );
		file.close();
		if ( line.indexOf ( "gpg-agent" ) !=-1 )
		{
			return true;
		}
	}
	return false;
}

#if defined ( Q_OS_DARWIN )
QString ONMainWindow::getXDisplay()
{
	QTcpSocket tcpSocket ( this );
	uint dispNumber=0;
	QString xname,xdir,xopt;
	dispNumber=0;
	xdir=ConfigDialog::getXDarwinDirectory();
	xname=xdir+"/Contents/MacOS/X11";
	xopt=" -rootless :0";

	//for newer versions of XQuartz start startx instead of X11.app
	xname="/usr/X11/bin/startx";
	xopt="";
	tcpSocket.connectToHost ( "127.0.0.1",6000+dispNumber );

	if ( tcpSocket.waitForConnected ( 3000 ) )
	{
		tcpSocket.close();
		return QString::number ( dispNumber );
	}
	if ( xname==QString::null )
	{
		QMessageBox::critical (
		    this,tr ( "Can't connect to X-Server" ),
		    tr (
		        "Can't connect to X-Server\nPlease check your settings"
		    ) );
		slot_config();
		return QString::null;
	}
	QProcess* pr=new QProcess ( this );
	pr->setWorkingDirectory ( xdir );
	pr->start ( xname+" "+xopt,QIODevice::NotOpen );
	if ( pr->waitForStarted ( 3000 ) )
	{
		QThread::sleep ( 3 );
		tcpSocket.connectToHost ( "127.0.0.1",6000+dispNumber );
		if ( tcpSocket.waitForConnected ( 1000 ) )
		{
			tcpSocket.close();
			return QString::number ( dispNumber );
		}
		QMessageBox::critical (
		    this,tr ( "Can't connect to X-Server" ),
		    tr (
		        "Can't connect to X-Server\nPlease check your settings"
		    ) );
		slot_config();
		return QString::null;
	}
	QMessageBox::critical (
	    this,QString::null,
	    tr (
	        "Can't start X Server\nPlease check your settings" ) );
	slot_config();
	return QString::null;
}
#endif

#ifdef Q_OS_WIN
QString ONMainWindow::getXDisplay()
{
	if ( !isServerRunning ( 6000+xDisplay ) )
	{
		QMessageBox::critical (
		    this,QString::null,
		    tr (
		        "Can't start X Server\nPlease check your installation" )
		);
		close();
	}
	return QString::number ( xDisplay );

}

QString ONMainWindow::cygwinPath ( const QString& winPath )
{
	QString cPath="/cygdrive/"+winPath;
	cPath.replace ( "\\","/" );
	cPath.replace ( ":","" );
	return cPath;
}
#endif

bool ONMainWindow::isColorDepthOk ( int disp, int sess )
{
	if ( sess==0 )
		return true;
	if ( disp==sess )
		return true;
	if ( ( disp == 24 || disp == 32 ) && ( sess == 24 || sess == 32 ) )
		return true;
	return false;
}

#ifndef Q_OS_LINUX
void ONMainWindow::setWidgetStyle ( QWidget* widget )
{
	widget->setStyle ( widgetExtraStyle );
}
#else
void ONMainWindow::setWidgetStyle ( QWidget* )
{
}
#endif

QString ONMainWindow::internAppName ( const QString& transAppName, bool* found )
{
	if ( found )
		*found=false;
	int ind=_transApplicationsNames.indexOf ( transAppName );
	if ( ind!=-1 )
	{
		if ( found )
			*found=true;
		return _internApplicationsNames[ind];
	}
	return transAppName;
}


QString ONMainWindow::transAppName ( const QString& internAppName, bool* found )
{
	if ( found )
		*found=false;
	int ind=_internApplicationsNames.indexOf ( internAppName );
	if ( ind!=-1 )
	{
		if ( found )
			*found=true;
		return _transApplicationsNames[ind];
	}
	return internAppName;
}

void ONMainWindow::addToAppNames ( QString intName, QString transName )
{
	_internApplicationsNames.append ( intName );
	_transApplicationsNames.append ( transName );
}


void ONMainWindow::slot_execXmodmap()
{
#ifdef Q_WS_HILDON
	QString passwd=getCurrentPass();
	QString user=getCurrentUname();
	QString host=resumingSession.server;
	QString cmd;

	cmd="(xmodmap -pke ;"
	    "echo keycode 73= ;"
// 	    "echo clear shift ;"
// 	    "echo clear lock ;"
// 	    "echo clear control ;"
// 	    "echo clear mod1 ;"
// 	    "echo clear mod2 ;"
// 	    "echo clear mod3 ;"
// 	    "echo clear mod4 ;"
// 	    "echo clear mod5 ;"
//  	    "echo add shift = Shift_L ;"
	    "echo add control = Control_R "
//  	    "echo add mod5 = ISO_Level3_Shift"
	    ")| DISPLAY=:"
	    +resumingSession.display+" xmodmap - ";
	x2goDebug<<"cmd:"<<cmd;
	sshProcess* xmodProc;
	try
	{
		xmodProc=new sshProcess ( this,user,host,sshPort,
		                          cmd,
		                          passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		return;
	}

	if ( cardReady || useSshAgent )
	{
		QStringList env=xmodProc->environment();
		env+=sshEnv;
		xmodProc->setEnvironment ( env );
	}
	xmodProc->setFwX ( true );
	xmodProc->startNormal();
#endif
}

void ONMainWindow::slot_sudoErr ( QString, sshProcess* proc )
{
	proc->setErrorString ( tr ( "<br>Sudo configuration error" ) );
	proc->kill();

}

void ONMainWindow::check_cmd_status()
{
	QString passwd;
	QString user=getCurrentUname();
	QString host=resumingSession.server;
	passwd=getCurrentPass();

	x2goDebug<<"check command message"<<endl;
	sshProcess* proc;
	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      "x2gocmdexitmessage "+
		                      resumingSession.sessionId,
		                      passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		return;
	}

	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}
	connect ( proc,SIGNAL ( sshFinished ( bool,QString,sshProcess* ) ),
	          this,SLOT ( slot_cmdMessage ( bool, QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		return;
	}

}

void ONMainWindow::slot_cmdMessage ( bool result,QString output,
                                     sshProcess* proc )
{
	if ( proc )
		delete proc;
	if ( result==false )
	{
		cardReady=false;
		cardStarted=false;
		QString message=tr ( "<b>Connection failed</b>\n" ) +output;
		if ( message.indexOf ( "publickey,password" ) !=-1 )
		{
			message=tr ( "<b>Wrong password!</b><br><br>" ) +
			        message;
		}

		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		currentKey=QString::null;
		setEnabled ( true );
		passForm->setEnabled ( true );
		pass->setFocus();
		pass->selectAll();
		return;
	}
	if ( output.indexOf ( "X2GORUNCOMMAND ERR NOEXEC:" ) !=-1 )
	{
		QString cmd=output;
		cmd.replace ( "X2GORUNCOMMAND ERR NOEXEC:","" );
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "Unable to execute: " ) +
		                        cmd,QMessageBox::Ok,
		                        QMessageBox::NoButton );
	}


}


int ONMainWindow::startSshFsTunnel()
{
	fsTunReady=false;
	x2goDebug<<"starting fs tunnel for:"<<resumingSession.sessionId<<
	"\nfs port: "<<resumingSession.fsPort;

	if ( resumingSession.fsPort.length() <=0 )
	{
		QString message=tr (
		                    "Remote server does not "
		                    "support file system export "
		                    "through SSH Tunnel\n"
		                    "Please update to a newer "
		                    "x2goserver package" );
		slot_fsTunnelFailed ( false,message,0 );
		return 1;
	}
	QString passwd=getCurrentPass();
	QString uname=getCurrentUname();

	try
	{

		fsTunnel=new sshProcess ( this,uname,
		                          resumingSession.server,
		                          sshPort,"",passwd,
		                          currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		slot_fsTunnelFailed ( false,message,0 );
		return 1;
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=fsTunnel->environment();
		env+=sshEnv;
		fsTunnel->setEnvironment ( env );
	}


	connect ( fsTunnel,SIGNAL ( sshFinished ( bool,
	                            QString,sshProcess* ) ),
	          this,SLOT ( slot_fsTunnelFailed ( bool,
	                                            QString,sshProcess* ) ) );
	connect ( fsTunnel,SIGNAL ( sshTunnelOk() ),
	          this,SLOT ( slot_fsTunnelOk() ) );

	try
	{
		fsTunnel->startTunnel ( "localhost",resumingSession.fsPort,
		                        clientSshPort,true );
	}
	catch ( QString message )
	{
		slot_fsTunnelFailed ( false,message,0 );
		return 1;
	}
	return 0;
}

void ONMainWindow::slot_fsTunnelFailed ( bool result,  QString output,
        sshProcess* )
{
	if ( result==false )
	{
		QString message=tr ( "Unable to create SSL tunnel:\n" ) +output;
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		if ( fsTunnel )
			delete fsTunnel;
		fsTunnel=0l;
		fsTunReady=false;
	}
}


void ONMainWindow::slot_fsTunnelOk()
{
	fsTunReady=true;
	//start reverse mounting if RSA Key and FS tunnel are ready
	//start only once from slot_fsTunnelOk() or slot_copyKey().
	if ( fsExportKeyReady )
		startX2goMount();
}

void ONMainWindow::startX2goMount()
{
	QFile file ( fsExportKey+".pub" );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QString message=tr ( "Unable to read :\n" ) +fsExportKey+".pub";
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove
		( fsExportKey+".pub" );
		return;
	}

	QByteArray line = file.readLine();
	file.close();
	QString authofname=homeDir;
#ifdef Q_OS_WIN
	authofname=wapiShortFileName ( authofname ) +"/.x2go";
#endif
	authofname+= "/.ssh/authorized_keys" ;

	QFile file1 ( authofname );

	if ( !file1.open ( QIODevice::WriteOnly | QIODevice::Text |
	                   QIODevice::Append ) )
	{
		QString message=tr ( "Unable to write :\n" ) + authofname;
		QMessageBox::critical ( 0l,tr ( "Error" ),message,
		                        QMessageBox::Ok,
		                        QMessageBox::NoButton );
		QFile::remove
		( fsExportKey+".pub" );
		return;

	}
	QTextStream out ( &file1 );
	out<<line;
	file1.close();
	directory* dir=getExpDir ( fsExportKey );
	bool rem=dir->isRemovable;
	if ( !dir )
		return;

	QString passwd=getCurrentPass();
	QString user=getCurrentUname();
	QString host=resumingSession.server;
	QString sessionId=resumingSession.sessionId;

	QStringList env=QProcess::systemEnvironment();


	QString cuser;
#ifndef Q_WS_HILDON
	for ( int i=0;i<env.size();++i )
	{
		QStringList ls=env[i].split ( "=" );
		if ( ls[0]=="USER" )

		{
			cuser=ls[1];
			break;
		}
	}
#else
	cuser="user";
#endif
#ifdef Q_OS_WIN
	cuser="sshuser";
#endif
	sshProcess* proc=0l;
	QString cmd;
	QString dirs=dir->dirList;

	if ( !fsInTun && clientSshPort!="22" )
	{
		dirs=dirs+"__SSH_PORT__"+clientSshPort;
	}
	if ( fsInTun )
	{
		dirs=dirs+"__REVERSESSH_PORT__"+resumingSession.fsPort;
	}
	if ( !rem )
		cmd="export HOSTNAME && x2gomountdirs dir "+sessionId+" "+cuser+
		    " "+dir->dstKey+" "+dirs;
	else
		cmd="export HOSTNAME && x2gomountdirs rem "+sessionId+" "+cuser+
		    " "+dir->dstKey+" "+dirs;

#ifdef Q_OS_WIN

	cmd="chmod 600 "+dir->dstKey+"&&"+cmd;
#endif

	try
	{
		proc=new sshProcess ( this,user,host,sshPort,
		                      cmd,
		                      passwd,currentKey,acceptRsa );
		dir->proc=proc;
	}
	catch ( QString message )
	{
		slot_retExportDir ( false,message,proc );
	}
	if ( cardReady || useSshAgent )
	{
		QStringList env=proc->environment();
		env+=sshEnv;
		proc->setEnvironment ( env );
	}

	connect ( proc,SIGNAL ( sshFinished ( bool, QString,sshProcess* ) ),
	          this,SLOT ( slot_retExportDir ( bool,
	                                          QString,sshProcess* ) ) );

	try
	{
		proc->startNormal();
	}
	catch ( QString message )
	{
		slot_retExportDir ( false,message,proc );
		return;
	}
}

void ONMainWindow::slot_checkPrintSpool()
{
	QDir dir ( spoolDir );
	QStringList list = dir.entryList ( QDir::Files );
	for ( int i=0;i<list.size();++i )
	{
		if ( !list[i].endsWith ( ".ready" ) )
			continue;
		QFile file ( spoolDir+"/"+list[i] );
		if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
			continue;
		bool startProc=false;
		QString fname,title;
		if ( !file.atEnd() )
		{
			QByteArray line = file.readLine();
			QString fn ( line );
			fn.replace ( "\n","" );
			fname=fn;
			if ( !file.atEnd() )
			{
				line = file.readLine();
				title=line;
				title.replace ( "\n","" );
			}
			startProc=true;
		}
		file.close();
		file.remove();
		if ( startProc )
			new PrintProcess ( spoolDir+"/"+fname,title ,this );

	}
}


void ONMainWindow::cleanPrintSpool()
{
	QDir dir ( spoolDir );
	QStringList list = dir.entryList ( QDir::Files );
	for ( int i=0;i<list.size();++i )
	{
		QFile::remove ( spoolDir+"/"+list[i] );
	}
}


void ONMainWindow::cleanAskPass()
{
	QString path=homeDir +"/.x2go/ssh/";
	QDir dir ( path );
	QStringList list = dir.entryList ( QDir::Files );
	for ( int i=0;i<list.size();++i )
	{
		if ( list[i].startsWith ( "askpass" ) )
			QFile::remove ( path+list[i] );
	}

}


/*
this slot will be connected from EmbedWidget
 */
void ONMainWindow::slotUpdateEmbed()
{
	EmbedWidget::slotUpdateEmbed();
}

bool ONMainWindow::isServerRunning ( int port )
{
	QTcpSocket tcpSocket ( 0 );
	tcpSocket.connectToHost ( "127.0.0.1",port );

	if ( tcpSocket.waitForConnected ( 1000 ) )
	{
		tcpSocket.close();
		return true;
	}
	return false;
}

#ifdef Q_OS_WIN
void ONMainWindow::slotCheckXOrgLog()
{
	xorgLogMutex.lock();
	if ( xorgLogFile.length() <=0 )
	{
		xorgLogMutex.unlock();
		return;
	}
	QFile file ( xorgLogFile );
	if ( !file.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		xorgLogMutex.unlock();
		return;
	}

	QTextStream in ( &file );
	while ( !in.atEnd() )
	{
		QString line = in.readLine();
		if ( line.indexOf ( "successfully opened the display" ) !=-1 )
		{
			xorgLogTimer->stop();
			slotSetWinServersReady();
			xorgLogMutex.unlock();
			return;
		}
	}
	xorgLogMutex.unlock();
}

void ONMainWindow::startXOrg ()
{
	while ( isServerRunning ( 6000+xDisplay ) )
		++xDisplay;
	QString dispString;
	QStringList env=QProcess::systemEnvironment();
	env<<"GLWIN_ENABLE_DEBUG=0";
	QTextStream ( &dispString ) <<":"<<xDisplay;

	xorgLogMutex.lock();
	xorgLogFile=homeDir+"/.x2go/xorg";
	QDir dr ( homeDir );
	dr.mkpath ( xorgLogFile );
	xorgLogFile=wapiShortFileName ( xorgLogFile ) +"\\xorg.log."+
	            QString::number ( xDisplay );
	if ( QFile::exists ( xorgLogFile ) )
		QFile::remove ( xorgLogFile );
	xorgLogMutex.unlock();
	QStringList args;
	args<<dispString<<"-multiwindow"<<"-notrayicon"<<
	"-logfile"<<xorgLogFile;
	xorg=new QProcess ( 0 );
	xorg-> setWorkingDirectory ( appDir+"\\xming" );
	xorg->setEnvironment ( env );
	xorg->start ( appDir+"\\xming\\Xming.exe",args );
	if ( !xorg->waitForStarted ( 3000 ) )
	{
		QMessageBox::critical (
		    0,QString::null,
		    tr ( "Can't start X Server\n"
		         "Please check your installation" ) );
		close();
	}
}

WinServerStarter::WinServerStarter ( daemon server, ONMainWindow * par ) :
		QThread ( 0 )
{
	mode=server;
	parent=par;
}

void WinServerStarter::run()
{
	switch ( mode )
	{
		case SSH:parent->startSshd();break;
		case X:parent->startXOrg();break;
		case PULSE:parent->startPulsed();break;
	}
}


void ONMainWindow::startWinServers()
{
	QString etcDir=homeDir+"/.x2go/etc";
	QDir dr ( homeDir );
	dr.mkpath ( etcDir );
	generateHostDsaKey();
	generateEtcFiles();
	saveCygnusSettings();

	WinServerStarter* xStarter = new WinServerStarter ( WinServerStarter::X,
	        this );
	WinServerStarter* sshStarter = new WinServerStarter (
	    WinServerStarter::SSH, this );

	WinServerStarter* pulseStarter = new WinServerStarter (
	    WinServerStarter::PULSE, this );
	xStarter->start();
	sshStarter->start();
	pulseStarter->start();
	xorgLogTimer=new QTimer ( this );
	connect ( xorgLogTimer,SIGNAL ( timeout() ),this,
	          SLOT ( slotCheckXOrgLog() ) );
	xorgLogTimer->start ( 1000 );
}

void ONMainWindow::generateHostDsaKey()
{
	QString etcDir=homeDir+"/.x2go/etc";
	if ( !QFile::exists ( etcDir+"/ssh_host_dsa_key" ) ||
	        !QFile::exists ( etcDir+"/ssh_host_dsa_key.pub" ) )
	{
		/*		x2goDebug<<"Generating host DSA key\n";*/
		QString fname=cygwinPath ( wapiShortFileName ( etcDir ) ) +
		              "/ssh_host_dsa_key";
		QStringList args;
		args<<"-t"<<"dsa"<<"-N"<<""<<"-C"<<
		"x2goclient DSA host key"<<"-f"<<fname;
		QProcess::execute ( "ssh-keygen",args );
	}
}

void ONMainWindow::generateEtcFiles()
{
	QString etcDir=homeDir+"/.x2go/etc";
	if ( !QFile::exists ( etcDir+"/passwd" ) )
	{
		QString sid, sys, user, grsid, grname;
		if ( !wapiAccountInfo ( &sid,&user,&grsid, &grname, &sys ) )
		{
// 			x2goDebug<<"Get account info failed\n";
			close();
		}

// 		x2goDebug<<"sid: "<<sid <<" system:"<<
// 		sys<< " user: "<<user<<" group sid:"<<grsid<<
// 		"group name: "<<grname<<endl;

		QStringList sidList=sid.split ( '-' );
		QString rid=sidList[sidList.count()-1];
		QStringList grsidList=grsid.split ( '-' );
		QString grid=grsidList[grsidList.count()-1];
		QFile file ( etcDir +"/passwd" );
		if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
			return;
		QTextStream out ( &file );
		out <<"sshuser::"<<rid<<":"<<grid<<":"<<sys<<"\\sshuser,"
		<<sid<<":"<<cygwinPath ( wapiShortFileName ( homeDir ) ) <<
		"/.x2go"<<":/bin/bash\n";
		file.close();
	}
	if ( !QFile::exists ( etcDir+"/sshd_config" ) )
	{
		QFile file ( etcDir +"/sshd_config" );
		if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
			return;
		QTextStream out ( &file );
		out<<"StrictModes no\n"<<
		"UsePrivilegeSeparation no\n"<<
		"Subsystem sftp /bin/sftp-server\n";
		file.close();
	}
}



void ONMainWindow::saveCygnusSettings()
{
	QSettings etcst ( "HKEY_CURRENT_USER\\Software"
	                  "\\Cygnus Solutions\\Cygwin\\mounts v2\\/etc",
	                  QSettings::NativeFormat );
	oldEtcDir=QString::null;
	oldEtcDir=etcst.value ( "native",oldEtcDir ).toString();
// 	x2goDebug<<"old etc:"<<oldEtcDir<<endl;
	QString newEtc=homeDir+"/.x2go/etc";
	newEtc.replace ( "/","\\" );
	etcst.setValue ( "native",wapiShortFileName ( newEtc ) );
	etcst.sync();

	QSettings binst ( "HKEY_CURRENT_USER\\Software"
	                  "\\Cygnus Solutions\\Cygwin\\mounts v2\\/bin",
	                  QSettings::NativeFormat );
	oldBinDir=QString::null;
	oldBinDir=binst.value ( "native",oldBinDir ).toString();
// 	x2goDebug<<"old bin:"<<oldBinDir<<endl;
	QString newBin=appDir;
	newBin.replace ( "/","\\" );
	binst.setValue ( "native",wapiShortFileName ( newBin ) );
	binst.sync();

	QSettings tmpst ( "HKEY_CURRENT_USER\\Software"
	                  "\\Cygnus Solutions\\Cygwin\\mounts v2\\/tmp",
	                  QSettings::NativeFormat );
	oldTmpDir=QString::null;
	oldTmpDir=tmpst.value ( "native",oldTmpDir ).toString();
// 	x2goDebug<<"old tmp:"<<oldTmpDir<<endl;
	QString newTmp=QDir::tempPath();
	newTmp.replace ( "/","\\" );
	tmpst.setValue ( "native",wapiShortFileName ( newTmp ) );
	tmpst.sync();
}

void ONMainWindow::restoreCygnusSettings()
{
	if ( oldEtcDir==QString::null )
	{
		QSettings st ( "HKEY_CURRENT_USER\\Software"
		               "\\Cygnus Solutions\\Cygwin\\mounts v2\\/etc",
		               QSettings::NativeFormat );
// 		x2goDebug<<"Removing /etc from cygwin mounts\n";
		st.remove ( "" );
		st.sync();
	}
	else
	{
		QSettings st ( "HKEY_CURRENT_USER\\Software"
		               "\\Cygnus Solutions\\Cygwin\\mounts v2\\/etc",
		               QSettings::NativeFormat );
		st.setValue ( "native",oldEtcDir );
		st.sync();
// 		x2goDebug<<"Restoring /etc in cygwin mounts\n";
	}
	if ( oldBinDir==QString::null )
	{
		QSettings st ( "HKEY_CURRENT_USER\\Software"
		               "\\Cygnus Solutions\\Cygwin\\mounts v2\\/bin",
		               QSettings::NativeFormat );
// 		x2goDebug<<"Removing /bin from cygwin mounts\n";
		st.remove ( "" );
		st.sync();
	}
	else
	{
		QSettings st ( "HKEY_CURRENT_USER\\Software"
		               "\\Cygnus Solutions\\Cygwin\\mounts v2\\/bin",
		               QSettings::NativeFormat );
		st.setValue ( "native",oldBinDir );
		st.sync();
// 		x2goDebug<<"Restoring /bin in cygwin mounts\n";
	}
	if ( oldTmpDir==QString::null )
	{
		QSettings st ( "HKEY_CURRENT_USER\\Software"
		               "\\Cygnus Solutions\\Cygwin\\mounts v2\\/tmp",
		               QSettings::NativeFormat );
// 		x2goDebug<<"Removing /tmp from cygwin mounts\n";
		st.remove ( "" );
		st.sync();
	}
	else
	{
		QSettings st ( "HKEY_CURRENT_USER\\Software"
		               "\\Cygnus Solutions\\Cygwin\\mounts v2\\/tmp",
		               QSettings::NativeFormat );
		st.setValue ( "native",oldTmpDir );
		st.sync();
// 		x2goDebug<<"Restoring /tmp in cygwin mounts\n";
	}
}

void ONMainWindow::startPulsed()
{
	while ( isServerRunning ( pulsePort ) )
		++pulsePort;
	esdPort=pulsePort+1;
	while ( isServerRunning ( esdPort ) )
		++esdPort;

	pulseDir=homeDir+"/.x2go/pulse";
	QDir dr ( homeDir );
	dr.mkpath ( pulseDir );
	pulseDir=wapiShortFileName ( pulseDir );
// 	x2goDebug<<"template: "<<pulseDir+"/tmp"<<endl;
	QTemporaryFile* fl=new QTemporaryFile ( pulseDir+"/tmp" );
	fl->open();
	pulseDir=fl->fileName();
	fl->close();
	delete fl;
	QFile::remove ( pulseDir );
	dr.mkpath ( pulseDir );
// 	x2goDebug<<"pulse tmp file: "<<pulseDir<<endl;
	QStringList pEnv=QProcess::systemEnvironment();
	for ( int i=0; i<pEnv.size();++i )
	{
		if ( pEnv[i].indexOf ( "USERPROFILE=" ) !=-1 )
			pEnv[i]="USERPROFILE="+
			        wapiShortFileName ( homeDir+"/.x2go/pulse" );
		if ( pEnv[i].indexOf ( "TEMP=" ) !=-1 )
			pEnv[i]="TEMP="+pulseDir;
		if ( pEnv[i].indexOf ( "USERNAME=" ) !=-1 )
			pEnv[i]="USERNAME=pulseuser";
	}

	QFile file ( pulseDir+"/config.pa" );
	if ( !file.open ( QIODevice::WriteOnly | QIODevice::Text ) )
		return;
	QTextStream out ( &file );
	out << "load-module module-native-protocol-tcp port="+
	QString::number ( pulsePort ) <<endl;
	out << "load-module module-esound-protocol-tcp port="+
	QString::number ( esdPort ) <<endl;
	out << "load-module module-waveout"<<endl;
	file.close();
	pulseServer=new QProcess ( 0 );
	pulseServer->setEnvironment ( pEnv );
	QStringList args;
	args<<"-n"<<"-F"<<pulseDir+"/config.pa";
	pulseServer->setWorkingDirectory ( wapiShortFileName ( appDir+"\\pulse" ) );
	pulseServer->start ( "pulse\\pulseaudio.exe",args );
}

#include <windows.h>
#include<sstream>

void ONMainWindow::startSshd()
{
	int port=clientSshPort.toInt();

	//clientSshPort have initvalue
	while ( isServerRunning ( port ) )
		++port;
	clientSshPort=QString::number ( port );
	std::string clientdir=wapiShortFileName ( appDir ).toStdString();
	std::stringstream strm;
	strm<<clientdir<<"\\sshd.exe -D -p"<<clientSshPort.toInt();

	STARTUPINFOA si;
	std::string desktopName="x2go_";
	desktopName+=getenv ( "USERNAME" );
	char* desktop=new char[desktopName.size() +1];
	strcpy ( desktop,desktopName.c_str() );
	x2goDebug<<"Creating desktop: "<<desktop<<endl;
	if ( !CreateDesktopA (
	            desktop,
	            0,
	            0,
	            0,
	            GENERIC_ALL,
	            0
	        ) )
	{
		strcpy ( desktop,"" );
		x2goDebug<<"Desktop creation failed, using default\n";
	}
	ZeroMemory ( &si, sizeof ( si ) );
	ZeroMemory ( &sshd, sizeof ( sshd ) );
	si.lpDesktop=desktop;
	si.cb = sizeof ( si );
	CreateProcessA ( NULL,  // No module name (use command line)
	                 ( LPSTR ) strm.str().c_str(),  // Command line
	                 NULL,           // Process handle not inheritable
	                 NULL,           // Thread handle not inheritable
	                 TRUE,          // Set handle inheritance to FALSE
	                 0/*CREATE_NO_WINDOW|CREATE_NEW_PROCESS_GROUP*/, //creation flags
	                 NULL,           // Use parent's environment block
	                 clientdir.c_str(), // Starting directory
	                 &si,            // Pointer to STARTUPINFO structure
	                 &sshd );           // Pointer to PROCESS_INFORMATION structure
	delete []desktop;
}


void ONMainWindow::slotSetWinServersReady()
{
	winServersReady=true;
	restoreCygnusSettings();
}

#endif
void ONMainWindow::slotFindProxyWin()
{
	proxyWinId=findWindow ( "X2GO-"+resumingSession.sessionId );
	if ( proxyWinId )
	{
		proxyWinTimer->stop();
		if ( embedMode )
		{
			if ( config.rootless )
			{
				act_embedContol->setEnabled ( false );
			}
			else
				slotAttachProxyWindow();
		}
#ifdef Q_OS_WIN
		if ( !startEmbedded )
		{
			if ( maximizeProxyWin )
			{
				wapiShowWindow ( ( HWND ) proxyWinId,
				                 WAPI_SHOWMAXIMIZED );
			}
			else
			{
				wapiMoveWindow ( ( HWND ) proxyWinId,0,0,
				                 proxyWinWidth,
				                 proxyWinHeight,true );
			}
		}
#endif
	}
}


QString ONMainWindow::getCurrentUname()
{
	return login->text();
}

QString ONMainWindow::getCurrentPass()
{
	return pass->text();
}

void ONMainWindow::slotDetachProxyWindow()
{
	proxyWinEmbedded=false;
	bgFrame->show();
	setStatStatus();
	act_embedContol->setText ( tr ( "Attach X2Go window" ) );
	act_embedContol->setIcon ( QIcon ( ":icons/32x32/attach.png" ) );
#ifdef Q_OS_LINUX
	//if QX11EmbedContainer cannot embed window, check if window exists
	//and reconnect
	if ( !embedControlChanged )
	{
		x2goDebug<<"\n";
		slotFindProxyWin();
		x2goDebug<<"proxy win detached, proxywin is:"<<proxyWinId<<endl;
	}
#endif
	embedControlChanged=false;
}


void ONMainWindow::slotAttachProxyWindow()
{
	if ( startEmbedded )
	{
		embedControlChanged=false;
		bgFrame->hide();
		proxyWinEmbedded=true;
		setStatStatus();
		act_embedContol->setText ( tr ( "Detach X2Go window" ) );
		act_embedContol->setIcon ( QIcon ( ":icons/32x32/detach.png" ) );
		QTimer::singleShot ( 100, this, SLOT ( slotEmbedWindow() ) );
	}
	else
		startEmbedded=true;

}

void ONMainWindow::slotEmbedWindow()
{
	embedWindow ( proxyWinId );
}

void ONMainWindow::setEmbedSessionActionsEnabled ( bool enable )
{
	act_shareFolder->setEnabled ( enable );
	act_suspend->setEnabled ( enable );
	act_terminate->setEnabled ( enable );
	act_embedContol->setEnabled ( enable );
}

void ONMainWindow::slotEmbedControlAction()
{
	embedControlChanged=true;
	if ( proxyWinEmbedded )
	{
		detachClient();
	}
	else
		slotAttachProxyWindow();
}

void ONMainWindow::slotEmbedIntoParentWindow()
{
	embedInto ( embedParent );
}



void ONMainWindow::processSessionConfig()
{
	x2goDebug<<"config file: "<<sessionConfigFile<<endl;
	QFile fl ( sessionConfigFile );
	if ( !fl.open ( QIODevice::ReadOnly | QIODevice::Text ) )
	{
		QMessageBox::critical ( 0l,tr ( "Error" ),
		                        tr ( "Can't open config file: " ) +
		                        sessionConfigFile,
		                        QMessageBox::Ok,QMessageBox::NoButton );
		exit ( -1 );
	}
	QTextStream in ( &fl );
	QString key;
	bool haveKey=false;
	config.command="KDE";
	config.sshport="22";
	config.session=tr ( "X2Go Session" );
	while ( !in.atEnd() )
	{
		QString line = in.readLine();
		if ( ( line=="-----BEGIN DSA PRIVATE KEY-----" ) ||
		        ( line=="-----BEGIN RSA PRIVATE KEY-----" ) )
		{
			key=line+"\n";
			while ( !in.atEnd() )
				key+=in.readLine() +"\n";
			haveKey=true;
		}
		else
			processCfgLine ( line );
	}
	fl.close();
	fl.open ( QIODevice::WriteOnly | QIODevice::Text );
	QTextStream out ( &fl );
	out<<key;
	fl.close();
	if ( haveKey )
	{
		startSshAgent();
	}
	else
		QFile::remove ( sessionConfigFile );
}

void ONMainWindow::startSshAgent()
{
	QProcess* sshAgent=new QProcess ( this );
	sshAgent->start ( "ssh-agent" );
	if ( !sshAgent->waitForStarted() )
	{
		QFile::remove ( sessionConfigFile );
		return;
	}

	if ( !sshAgent->waitForFinished ( -1 ) )
	{
		QFile::remove ( sessionConfigFile );
		return;
	}

	QString stdOut ( sshAgent->readAllStandardOutput() );
	// 	x2goDebug<<"SSHAGENT out:"<<stdOut;
	stdOut.replace ( " ","" );
	stdOut.replace ( "\n","" );
	QStringList envLst=stdOut.split ( ";" );
	sshAgentPid=envLst[2].split ( "=" ) [1];

	// 	x2goDebug<<"pid:"<<sshAgentPid;
	// 	x2goDebug<<"env:"<<envLst[0]<<":"<<envLst[2];

	sshEnv.clear();
	sshEnv<<envLst[0]<<envLst[2];
	// 	x2goDebug<<"sshenv:"<<sshEnv<<endl;
	useSshAgent=true;
	addKey2SshAgent();
}

void ONMainWindow::addKey2SshAgent()
{
	QProcess sshadd ( this ); //using it to start scdaemon
	sshadd.setEnvironment ( sshEnv );
	QStringList arguments;
	arguments<<sessionConfigFile;
	sshadd.start ( "ssh-add",arguments );
	if ( !sshadd.waitForStarted() )
	{
		QFile::remove ( sessionConfigFile );
		x2goDebug<<"ssh-add start failed";
		return;
	}

	if ( !sshadd.waitForFinished ( -1 ) )
	{
		QFile::remove ( sessionConfigFile );
		x2goDebug<<"ssh-add finish failed";
		return;
	}
	sshadd.waitForFinished ( -1 );
	QFile::remove ( sessionConfigFile );
}

void ONMainWindow::finishSshAgent()
{
	QProcess sshAgent ( this ); //using it to start scdaemon
	sshAgent.setEnvironment ( sshEnv );
	QStringList arguments;
	arguments<<"-k";
	sshAgent.start ( "ssh-agent",arguments );
	if ( !sshAgent.waitForStarted() )
	{
		x2goDebug<<"ssh-agent start failed";
		return;
	}

	if ( !sshAgent.waitForFinished ( -1 ) )
	{
		x2goDebug<<"ssh-agent finish failed";
		return;
	}
	sshAgent.waitForFinished ( -1 );
}


void ONMainWindow::processCfgLine ( QString line )
{
	QStringList lst=line.split ( "=" );
	if ( lst[0]=="command" )
	{
		config.command=lst[1];
		return;
	}
	if ( lst[0]=="server" )
	{
		config.server=lst[1];
		return;
	}
	if ( lst[0]=="session" )
	{
		config.session=lst[1];
		return;
	}
	if ( lst[0]=="sshport" )
	{
		config.sshport=lst[1];
		return;
	}
	if ( lst[0]=="user" )
	{
		config.user=lst[1];
		return;
	}
	if ( lst[0]=="rootless" )
	{
		if ( lst[1]=="true" )
			config.rootless=true;
		else
			config.rootless=false;
		return;
	}
}


void ONMainWindow::initPassDlg()
{
	passForm = new SVGFrame ( ":/svg/passform.svg",
	                          false,bgFrame );
#ifdef Q_OS_WIN
	passForm->setMainWidget ( ( QWidget* ) this );
#endif
	username->addWidget ( passForm );
	passForm->hide();
	setWidgetStyle ( passForm );
	if ( !miniMode )
		passForm->setFixedSize ( passForm->sizeHint() );
	else
		passForm->setFixedSize ( 310,180 );
	QPalette pal=passForm->palette();
	pal.setBrush ( QPalette::Window, QColor ( 255,255,255,0 ) );
	passForm->setPalette ( pal );

	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

	QFont fnt=passForm->font();
	if ( miniMode )
#ifdef Q_WS_HILDON
		fnt.setPointSize ( 10 );
#else
		fnt.setPointSize ( 9 );
#endif
	passForm->setFont ( fnt );

	fotoLabel=new QLabel ( passForm );
	fotoLabel->hide();

	nameLabel=new QLabel ( "",passForm );
	nameLabel->hide();

	loginPrompt=new QLabel ( tr ( "Login:" ),passForm );
	passPrompt=new QLabel ( tr ( "Password:" ),passForm );

	login=new ClickLineEdit ( passForm );
	setWidgetStyle ( login );
	login->setFrame ( false );
	login->setEnabled ( false );

	login->hide();
	loginPrompt->hide();

	pass=new ClickLineEdit ( passForm );
	setWidgetStyle ( pass );
	pass->setFrame ( false );
	fnt.setBold ( true );
	pass->setFont ( fnt );
	pass->setEchoMode ( QLineEdit::Password );
	pass->setFocus();

#ifdef Q_OS_LINUX
	connect ( login,SIGNAL ( clicked() ),this,
	          SLOT ( slotActivateWindow() ) );
	connect ( pass,SIGNAL ( clicked() ),this,
	          SLOT ( slotActivateWindow() ) );
#endif

	pass->hide();
	passPrompt->hide();


	ok=new QPushButton ( tr ( "Ok" ),passForm );
	setWidgetStyle ( ok );
	cancel=new QPushButton ( tr ( "Cancel" ),passForm );
	setWidgetStyle ( cancel );
	ok->hide();
	cancel->hide();

	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );
	ok->setPalette ( pal );
	cancel->setPalette ( pal );


#ifndef Q_WS_HILDON
	ok->setFixedSize ( ok->sizeHint() );
	cancel->setFixedSize ( cancel->sizeHint() );
#else
	QSize sz=cancel->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	cancel->setFixedSize ( sz );
	sz=ok->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	ok->setFixedSize ( sz );
#endif

	QVBoxLayout *layout=new QVBoxLayout ( passForm );
	QHBoxLayout *labelLay=new QHBoxLayout();
	QHBoxLayout *inputLay=new QHBoxLayout();
	QHBoxLayout *buttonLay=new QHBoxLayout();

	labelLay->setSpacing ( 20 );
	inputLay->setSpacing ( 10 );
	layout->setContentsMargins ( 20,20,10,10 );
	layout->addLayout ( labelLay );
	layout->addStretch();
	layout->addLayout ( inputLay );
	layout->addStretch();
	layout->addLayout ( buttonLay );

	labelLay->addWidget ( fotoLabel );
	labelLay->addWidget ( nameLabel );
	labelLay->addStretch();

	QVBoxLayout* il1=new QVBoxLayout();
	il1->addWidget ( loginPrompt );
	il1->addWidget ( passPrompt );

	QVBoxLayout* il2=new QVBoxLayout();
	il2->addWidget ( login );
	il2->addWidget ( pass );
	inputLay->addLayout ( il1 );
	inputLay->addLayout ( il2 );
	inputLay->addStretch();

	buttonLay->addStretch();
	buttonLay->addWidget ( ok );
	buttonLay->addWidget ( cancel );
	buttonLay->addStretch();

	pal.setColor ( QPalette::Base, QColor ( 239,239,239,255 ) );
	login->setPalette ( pal );
	pass->setPalette ( pal );

	connect ( ok,SIGNAL ( clicked() ),this, SLOT ( slotSessEnter() ) );
	connect ( cancel,SIGNAL ( clicked() ),this, SLOT ( slotClosePass() ) );
	connect ( pass,SIGNAL ( returnPressed() ),this,
	          SLOT ( slotSessEnter() ) );
	connect ( login,SIGNAL ( returnPressed() ),pass, SLOT ( selectAll() ) );
	connect ( login,SIGNAL ( returnPressed() ),pass, SLOT ( setFocus() ) );

	passPrompt->show();
	pass->show();
	ok->show();
	cancel->show();
	fotoLabel->show();
	nameLabel->show();
	if ( !useLdap )
	{
		login->show();
		loginPrompt->show();
	}
	if ( embedMode )
	{
		cancel->setEnabled ( false );
#ifdef Q_OS_WIN
		QRect r;
		wapiWindowRect ( ok->winId(),r );
#endif
	}
}


void ONMainWindow::initStatusDlg()
{
	sessionStatusDlg = new SVGFrame ( ":/svg/passform.svg",
	                                  false,bgFrame );
	sessionStatusDlg->hide();
	if ( !miniMode )
		sessionStatusDlg->setFixedSize (
		    sessionStatusDlg->sizeHint() );
	else
		sessionStatusDlg->setFixedSize ( 310,200 );
	QFont fnt=sessionStatusDlg->font();
	if ( miniMode )
#ifdef Q_WS_HILDON
		fnt.setPointSize ( 10 );
#else
		fnt.setPointSize ( 9 );
#endif
	sessionStatusDlg->setFont ( fnt );
	username->addWidget ( sessionStatusDlg );
	QPalette pal=sessionStatusDlg->palette();
	pal.setBrush ( QPalette::Window, QColor ( 0,0,0,0 ) );
	sessionStatusDlg->setPalette ( pal );

	slName=new QLabel ( sessionStatusDlg );
	slVal=new QLabel ( sessionStatusDlg );

	slName->setText (
	    tr (
	        "<b>Session ID:<br>Server:<br>Username:"
	        "<br>Display:<br>Creation time:<br>Status:</b>" ) );
	slName->setFixedSize ( slName->sizeHint() );
	slName->hide();

	slVal->hide();
	slVal->setFixedHeight ( slName->sizeHint().height() );
	sbSusp=new QPushButton ( tr ( "Abort" ),sessionStatusDlg );
	sbTerm=new QPushButton ( tr ( "Terminate" ),sessionStatusDlg );
	sbExp=new QPushButton ( tr ( "Share folder..." ),
	                        sessionStatusDlg );
	sbAdv=new QCheckBox ( tr ( "Show details" ),sessionStatusDlg );
	setWidgetStyle ( sbTerm );
	setWidgetStyle ( sbExp );
	setWidgetStyle ( sbSusp );
	setWidgetStyle ( sbAdv );

	sbAdv->setFixedSize ( sbAdv->sizeHint() );
#ifndef Q_WS_HILDON
	sbSusp->setFixedSize ( sbSusp->sizeHint() );
	sbTerm->setFixedSize ( sbTerm->sizeHint() );
	sbExp->setFixedSize ( sbExp->sizeHint() );
#else
	QSize sz=sbSusp->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sbSusp->setFixedSize ( sz );
	sz=sbExp->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sbExp->setFixedSize ( sz );
	sz=sbTerm->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sbTerm->setFixedSize ( sz );
#endif
	sbAdv->hide();
	sbSusp->hide();
	sbTerm->hide();
	sbExp->hide();


	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

	sbAdv->setPalette ( pal );
	sbSusp->setPalette ( pal );
	sbTerm->setPalette ( pal );
	sbExp->setPalette ( pal );

	stInfo=new QTextEdit ( sessionStatusDlg );
	setWidgetStyle ( stInfo );
	setWidgetStyle ( stInfo->verticalScrollBar() );
	stInfo->setReadOnly ( true );
	stInfo->hide();
	stInfo->setFrameStyle ( QFrame::StyledPanel|QFrame::Plain );
	stInfo->setPalette ( pal );

	sbExp->setEnabled ( false );

	connect ( sbSusp,SIGNAL ( clicked() ),this,
	          SLOT ( slot_testSessionStatus() ) );
	connect ( sbTerm,SIGNAL ( clicked() ),this,
	          SLOT ( slotTermSessFromSt() ) );
	connect ( sbAdv,SIGNAL ( clicked() ),this,
	          SLOT ( slotShowAdvancedStat() ) );

	QVBoxLayout* layout=new QVBoxLayout ( sessionStatusDlg );
	QHBoxLayout* ll=new QHBoxLayout();
	ll->addWidget ( slName );
	ll->addWidget ( slVal );
	ll->addStretch();
	ll->setSpacing ( 10 );
	if ( !miniMode )
		layout->setContentsMargins ( 25,25,10,10 );
	else
		layout->setContentsMargins ( 10,10,10,10 );

	QHBoxLayout* bl=new QHBoxLayout();
	bl->addStretch();
	bl->addWidget ( sbExp );
	bl->addWidget ( sbSusp );
	bl->addWidget ( sbTerm );
	bl->addStretch();
	layout->addLayout ( ll );
	layout->addStretch();
	layout->addWidget ( stInfo );
	layout->addWidget ( sbAdv );
	layout->addStretch();
	layout->addLayout ( bl );


	slName->show();
	slVal->show();
	sbAdv->show();
	if ( !embedMode )
	{
		sbSusp->show();
		sbTerm->show();
		sbExp->show();
	}
#ifndef Q_OS_WIN

	QSettings st ( homeDir +"/.x2goclient/settings",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "settings" );
#endif


	if ( st.value ( "showStatus", ( QVariant ) false ).toBool() )
	{
		sbAdv->setChecked ( true );
		slotShowAdvancedStat();
	}
#ifdef Q_OS_WIN
	if ( embedMode )
	{
		QRect r;
		wapiWindowRect ( sbAdv->winId(),r );
		wapiWindowRect ( stInfo->verticalScrollBar ()->winId(),r );
	}
#endif

}


void ONMainWindow::initSelectSessDlg()
{
	selectSessionDlg = new SVGFrame ( ":/svg/passform.svg",
	                                  false,bgFrame );
	username->addWidget ( selectSessionDlg );
	setWidgetStyle ( selectSessionDlg );
	if ( !miniMode )
		selectSessionDlg->setFixedSize ( selectSessionDlg->sizeHint() );
	else
		selectSessionDlg->setFixedSize ( 310,180 );
	QPalette pal=selectSessionDlg->palette();
	pal.setBrush ( QPalette::Window, QColor ( 255,255,255,0 ) );
	selectSessionDlg->setPalette ( pal );

	pal.setColor ( QPalette::Button, QColor ( 255,255,255,0 ) );
	pal.setColor ( QPalette::Window, QColor ( 255,255,255,255 ) );
	pal.setColor ( QPalette::Base, QColor ( 255,255,255,255 ) );

	QFont fnt=selectSessionDlg->font();
	if ( miniMode )
#ifdef Q_WS_HILDON
		fnt.setPointSize ( 10 );
#else
		fnt.setPointSize ( 9 );
#endif
	selectSessionDlg->setFont ( fnt );
	selectSessionLabel=new QLabel ( tr ( "Select session:" ),
	                                selectSessionDlg );
	sOk=new QPushButton ( tr ( "Resume" ),selectSessionDlg );
	setWidgetStyle ( sOk );
	sCancel=new QPushButton ( tr ( "Cancel" ),selectSessionDlg );
	setWidgetStyle ( sCancel );

	bSusp=new QPushButton ( tr ( "Suspend" ),selectSessionDlg );
	setWidgetStyle ( bSusp );
	bTerm=new QPushButton ( tr ( "Terminate" ),selectSessionDlg );
	setWidgetStyle ( bTerm );

	bNew=new QPushButton ( tr ( "New" ),selectSessionDlg );
	setWidgetStyle ( bNew );

	sOk->setPalette ( pal );
	sCancel->setPalette ( pal );

	connect ( sCancel,SIGNAL ( clicked() ),this,
	          SLOT ( slotCloseSelectDlg() ) );

	selectSessionDlg->show();
#ifndef Q_WS_HILDON
	sOk->setFixedSize ( ok->sizeHint() );
	sCancel->setFixedSize ( cancel->sizeHint() );
#else
	QSize sz=sCancel->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sCancel->setFixedSize ( sz );
	sz=sOk->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	sOk->setFixedSize ( sz );
	sz=bSusp->sizeHint();
	if ( bTerm->sizeHint().width() > sz.width() )
		sz=bTerm->sizeHint();
	if ( bNew->sizeHint().width() > sz.width() )
		sz=bNew->sizeHint();
	sz.setWidth ( ( int ) ( sz.width() /1.5 ) );
	sz.setHeight ( ( int ) ( sz.height() /1.5 ) );
	bSusp->setFixedSize ( sz );
	bTerm->setFixedSize ( sz );
	bNew->setFixedSize ( sz );
#endif
	int bmaxw=bNew->size().width();
	if ( bSusp->size().width() >bmaxw )
		bmaxw=bSusp->size().width();
	if ( bTerm->size().width() >bmaxw )
		bmaxw=bTerm->size().width();

	bNew->setFixedWidth ( bmaxw );
	bSusp->setFixedWidth ( bmaxw );
	bTerm->setFixedWidth ( bmaxw );


	sOk->setEnabled ( true );
	sCancel->setEnabled ( true );
	selectSessionDlg->setEnabled ( true );
	setEnabled ( true );

	sessTv=new QTreeView ( selectSessionDlg );
	setWidgetStyle ( sessTv );
	setWidgetStyle ( sessTv->horizontalScrollBar() );
	setWidgetStyle ( sessTv->verticalScrollBar() );
	sessTv->setItemsExpandable ( false );
	sessTv->setRootIsDecorated ( false );

	model=new QStandardItemModel ( sessions.size(), 8 );

	model->setHeaderData ( S_DISPLAY,Qt::Horizontal,
	                       QVariant ( ( QString ) tr ( "Display" ) ) );
	model->setHeaderData ( S_STATUS,Qt::Horizontal,
	                       QVariant ( ( QString ) tr ( "Status" ) ) );
	model->setHeaderData ( S_COMMAND,Qt::Horizontal,
	                       QVariant ( ( QString ) tr ( "Command" ) ) );
	model->setHeaderData ( S_TYPE,Qt::Horizontal,
	                       QVariant ( ( QString ) tr ( "Type" ) ) );
	model->setHeaderData ( S_SERVER,Qt::Horizontal,
	                       QVariant ( ( QString ) tr ( "Server" ) ) );
	model->setHeaderData (
	    S_CRTIME,Qt::Horizontal,
	    QVariant ( ( QString ) tr ( "Creation time" ) ) );
	model->setHeaderData ( S_IP,Qt::Horizontal,
	                       QVariant ( ( QString ) tr ( "Client IP" ) ) );
	model->setHeaderData ( S_ID,Qt::Horizontal,
	                       QVariant ( ( QString ) tr ( "Session ID" ) ) );
	sessTv->setModel ( ( QAbstractItemModel* ) model );

	QFontMetrics fm ( sessTv->font() );
	sessTv->setEditTriggers ( QAbstractItemView::NoEditTriggers );
	sessTv->setPalette ( pal );
	bNew->setPalette ( pal );
	bSusp->setPalette ( pal );
	bTerm->setPalette ( pal );
	sessTv->setFrameStyle ( QFrame::StyledPanel|QFrame::Plain );
	sOk->setEnabled ( false );
	bSusp->setEnabled ( false );
	bTerm->setEnabled ( false );
	selectSessionLabel->hide();

	QVBoxLayout* layout=new QVBoxLayout ( selectSessionDlg );
	QHBoxLayout* blay=new QHBoxLayout();
	QVBoxLayout* alay=new QVBoxLayout();
	QHBoxLayout* tvlay=new QHBoxLayout();

	layout->addWidget ( selectSessionLabel );
	layout->addLayout ( tvlay );
	layout->addLayout ( blay );

	alay->addWidget ( bSusp );
	alay->addWidget ( bTerm );
	alay->addStretch();
	alay->addWidget ( bNew );

	tvlay->addWidget ( sessTv );
	tvlay->addLayout ( alay );

	blay->addStretch();
	blay->addWidget ( sOk );
	blay->addWidget ( sCancel );
	blay->addStretch();
	if ( !miniMode )
		layout->setContentsMargins ( 25,25,10,10 );
	else
		layout->setContentsMargins ( 10,10,10,10 );



	sOk->hide();
	sCancel->hide();
	bNew->hide();
	bSusp->hide();
	bTerm->hide();

	connect ( sessTv,SIGNAL ( clicked ( const QModelIndex& ) ),
	          this,SLOT ( slot_activated ( const QModelIndex& ) ) );

	connect ( sessTv,SIGNAL ( doubleClicked ( const QModelIndex& ) ),
	          this,SLOT ( slot_resumeDoubleClick ( const QModelIndex& ) ) );

	connect ( sOk,SIGNAL ( clicked() ),this, SLOT ( slotResumeSess() ) );
	connect ( bSusp,SIGNAL ( clicked() ),this, SLOT ( slotSuspendSess() ) );
	connect ( bTerm,SIGNAL ( clicked() ),this, SLOT ( slotTermSess() ) );
	connect ( bNew,SIGNAL ( clicked() ),this, SLOT ( slotNewSess() ) );

	selectSessionLabel->show();
	sOk->show();
	sCancel->show();
	bNew->show();
	bSusp->show();
	bTerm->show();
	sessTv->show();
	selectSessionDlg->hide();
#ifdef Q_OS_WIN
	if ( embedMode )
	{
		QRect r;
		wapiWindowRect ( sOk->winId(),r );
		wapiWindowRect ( sessTv->verticalScrollBar ()->winId(),r );
		wapiWindowRect ( sessTv->horizontalScrollBar ()->winId(),r );
		wapiWindowRect ( sessTv->header ()->viewport()->winId(),r );
	}
#endif

}



void ONMainWindow::printSshDError()
{
	QMessageBox::critical ( 0l,tr ( "Error" ),
	                        tr ( "sshd not started, "
	                             "you'll need sshd for printing "
	                             "and file sharing\n"
	                             "you can install sshd with\n"
	                             "<b>sudo apt-get install "
	                             "openssh-server</b>" ),
	                        QMessageBox::Ok,QMessageBox::NoButton );
}

void ONMainWindow::slotStartParec ()
{
	if ( !parecTunnelOk )
	{
// 		wait 1 sec and try again
		QTimer::singleShot ( 1000, this, SLOT ( slotStartParec() ) );
		return;
	}
	sshProcess* paProc;
	QString passwd=getCurrentPass();
	QString user=getCurrentUname();
	QString host=resumingSession.server;
	QString scmd="PULSE_CLIENTCONFIG=~/.x2go/C-"+
	             resumingSession.sessionId+
	             "/.pulse-client.conf "+
	             "parec > /dev/null &sleep 1 && kill %1";

	try
	{
		paProc=new sshProcess ( this,user,host,sshPort,
		                        scmd,
		                        passwd,currentKey,acceptRsa );
	}
	catch ( QString message )
	{
		return;
	}

	if ( cardReady || useSshAgent )
	{
		QStringList env=paProc->environment();
		env+=sshEnv;
		paProc->setEnvironment ( env );
	}
	paProc->startNormal();
}


void ONMainWindow::slotSndTunOk()
{
	parecTunnelOk=true;
}


void ONMainWindow::slotPCookieReady (	bool result,
                                      QString ,
                                      sshProcess* )
{
	if ( result )
		slotStartParec();
}


void ONMainWindow::loadPulseModuleNativeProtocol()
{
	QProcess* proc=new QProcess ( this );
	QStringList args;
	args<<"load-module"<<"module-native-protocol-tcp";
	proc->start ( "pactl",args );
	proc->waitForFinished ( 3000 );
}

void ONMainWindow::slotEmbedToolBar()
{
	if ( statusLabel )
	{
		delete statusLabel;
		statusLabel=0;
	}
	if ( embedTbVisible )
	{
		stb->clear();
		act_embedToolBar->setIcon (
		    QIcon ( ":icons/16x16/tbshow.png" ) );
		stb->addAction ( act_embedToolBar );
		stb->setToolButtonStyle ( Qt::ToolButtonIconOnly );
		stb->widgetForAction (
		    act_embedToolBar )->setFixedHeight ( 16 );
		act_embedToolBar->setText ( tr ( "Restore toolbar" ) );
		statusLabel=new QLabel;
		stb->addWidget ( statusLabel );
		statusBar()->hide();
	}
	else
	{
		initEmbedToolBar();
		act_embedToolBar->setIcon (
		    QIcon ( ":icons/32x32/tbhide.png" ) );
		act_embedToolBar->setText ( tr ( "Minimize toolbar" ) );
	}
	embedTbVisible=!embedTbVisible;
	if ( proxyWinEmbedded )
		setStatStatus();
#ifndef Q_OS_WIN
	QSettings st ( homeDir +"/.x2goclient/sessions",
	               QSettings::NativeFormat );
#else

	QSettings st ( "Obviously Nice","x2goclient" );
	st.beginGroup ( "sessions" );
#endif
	st.setValue ( "embedded/tbvisible", embedTbVisible );
	st.sync();
}

void ONMainWindow::initEmbedToolBar()
{
	stb->addAction ( act_embedToolBar );
	stb->addSeparator();
	stb->setToolButtonStyle ( Qt::ToolButtonTextUnderIcon );
	stb->addAction ( act_shareFolder );
	stb->addAction ( act_suspend );
	stb->addAction ( act_terminate );
	stb->addSeparator();
	stb->addAction ( act_embedContol );
	stb->addSeparator();
	stb->addAction ( act_set );
	stb->addAction ( act_abclient );
}

void ONMainWindow::slotEmbedToolBarToolTip()
{
	if ( !showTbTooltip )
		return;
	QWidget* widg=stb->widgetForAction (
	                  act_embedToolBar );
	QToolTip::showText ( this->mapToGlobal ( QPoint ( 6,6 ) ),
	                     tr ( "<br><b>&nbsp;&nbsp;&nbsp;Click this "
	                          "button&nbsp;&nbsp;&nbsp;<br>"
	                          "&nbsp;&nbsp;&nbsp;to restore toolbar"
	                          "&nbsp;&nbsp;&nbsp;</b><br>" ),
	                     widg );
}


void ONMainWindow::slotActivateWindow()
{
	if ( embedMode )
	{
		activateWindow();
		QTimer::singleShot ( 50, this,
		                     SLOT ( slotEmbedToolBarToolTip() ) );
	}
}

#ifndef Q_OS_WIN
void ONMainWindow::mouseReleaseEvent ( QMouseEvent * event )
{
	QMainWindow::mouseReleaseEvent ( event );
	slotActivateWindow();
}
#endif

void ONMainWindow::slotHideEmbedToolBarToolTip()
{
	showTbTooltip=false;
	QToolTip::hideText();
}
