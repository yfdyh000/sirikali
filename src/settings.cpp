/*
 *
 *  Copyright (c) 2017
 *  name : Francis Banyikwa
 *  email: mhogomchungu@gmail.com
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <QSettings>
#include <QApplication>
#include <QWidget>
#include <QDialog>

#include "settings.h"
#include "utility.h"
#include "readonlywarning.h"
#include "locale_path.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <cstdio>
#include <memory>
#include <iostream>

#include <QByteArray>
#include <QTranslator>

#include <QCoreApplication>

QString settings::homePath()
{
	return QDir::homePath() ;
}

settings::settings() : m_settings( "SiriKali","SiriKali" )
{
}

int settings::pollForUpdatesInterval()
{
	if( !m_settings.contains( "WinFSPpollingInterval" ) ){

		m_settings.setValue( "WinFSPpollingInterval",2 ) ;
	}

	return m_settings.value( "WinFSPpollingInterval" ).toInt() ;
}

void settings::replaceFavorite( const favorites::entry& e,const favorites::entry& f )
{
	QStringList l ;

	for( const auto& it : this->readFavorites() ){

		if( it == e ){

			l.append( f.configString() ) ;
		}else{
			l.append( it.configString() ) ;
		}
	}

	if( !l.isEmpty() ){

		m_settings.setValue( "FavoritesVolumes",l ) ;
	}
}

int settings::favoritesEntrySize()
{
	auto s = m_settings.value( "FavoritesVolumes" ).toStringList() ;

	if( s.size() == 0 ){

		return 0 ;
	}else{
		return utility::split( s.first(),'\t' ).size() ;
	}
}

void settings::setWindowsExecutableSearchPath( const QString& e )
{
	if( e.isEmpty() ){

		m_settings.setValue( "WindowsExecutableSearchPath",settings::homePath() + "/bin" ) ;
	}else{
		m_settings.setValue( "WindowsExecutableSearchPath",e ) ;
	}
}

QString settings::windowsExecutableSearchPath()
{
	if( !m_settings.contains( "WindowsExecutableSearchPath" ) ){

		m_settings.setValue( "WindowsExecutableSearchPath",settings::homePath() + "/bin" ) ;
	}

	return m_settings.value( "WindowsExecutableSearchPath" ).toString() ;
}

QString settings::externalPluginExecutable()
{
	if( m_settings.contains( "ExternalPluginExecutable" ) ){

		return m_settings.value( "ExternalPluginExecutable" ).toString() ;
	}else{
		settings::setExternalPluginExecutable( QString() ) ;

		return m_settings.value( "ExternalPluginExecutable" ).toString() ;
	}
}

void settings::setExternalPluginExecutable( const QString& e )
{
	if( e.isEmpty() ){

		QString s = "gpg --no-tty --yes --no-mdc-warning --no-verbose --passphrase-fd 0 -d" ;
		m_settings.setValue( "ExternalPluginExecutable",s ) ;
	}else{
		m_settings.setValue( "ExternalPluginExecutable",e ) ;
	}
}

bool settings::enableRevealingPasswords()
{
	if( m_settings.contains( "EnableRevealingPasswords" ) ){

		return m_settings.value( "EnableRevealingPasswords" ).toBool() ;
	}else{
		bool e = true ;

		m_settings.setValue( "EnableRevealingPasswords",e ) ;

		return e ;
	}
}

void settings::clearFavorites()
{
	m_settings.setValue( "FavoritesVolumes",QStringList() ) ;
}

static QString _file_manager()
{
	QString s ;
	QString e ;

	if( utility::platformIsLinux() ){

		s = "xdg-open" ;
		e = utility::executableFullPath( s ) ;

	}else if( utility::platformIsOSX() ){

		s = "open" ;
		e = utility::executableFullPath( s ) ;
	}else{
		s = "explorer.exe" ;
	}

	if( e.isEmpty() ){

		return s ;
	}else{
		return e ;
	}
}

void settings::setParent( QWidget * parent,QWidget ** localParent,QDialog * dialog )
{
	auto _default_parent = [ this ](){

		if( m_settings.contains( "UseDefaultWidgetRelationship" ) ){

			return m_settings.value( "UseDefaultWidgetRelationship" ).toBool() ;
		}else{
			bool e = true ;
			m_settings.setValue( "UseDefaultWidgetRelationship",e ) ;
			return e ;
		}
	}() ;

	if( _default_parent ){

		*localParent = dialog ;
	}else{
		*localParent = parent ;
	}
}

void settings::scaleGUI()
{
#if QT_VERSION >= 0x050600

	bool e = [ this ](){

		if( m_settings.contains( "EnableHighDpiScaling" ) ){

			return m_settings.value( "EnableHighDpiScaling" ).toBool() ;
		}else{
			bool s = false ;
			m_settings.setValue( "EnableHighDpiScaling",s ) ;
			m_settings.setValue( "EnabledHighDpiScalingFactor",QString( "1" ) ) ;
			return s ;
		}
	}() ;

	if( e ){

		QApplication::setAttribute( Qt::AA_EnableHighDpiScaling ) ;

		qputenv( "QT_SCALE_FACTOR",[ this ](){

			if( m_settings.contains( "EnabledHighDpiScalingFactor" ) ){

				return m_settings.value( "EnabledHighDpiScalingFactor" ).toString().toLatin1() ;
			}else{
				m_settings.setValue( "EnabledHighDpiScalingFactor",QString( "1" ) ) ;
				return QByteArray( "1" ) ;
			}
		}() ) ;
	}

#endif
}

QString settings::fileManager()
{
	if( m_settings.contains( "FileManagerOpener" ) ){

		auto e = m_settings.value( "FileManagerOpener" ).toString() ;

		if( e.isEmpty() ){

			settings::setFileManager( QString() ) ;

			return m_settings.value( "FileManagerOpener" ).toString() ; ;
		}else{
			return e ;
		}
	}else{
		settings::setFileManager( QString() ) ;
		return m_settings.value( "FileManagerOpener" ).toString() ;
	}
}

static void _set_mount_default( settings& s )
{
	QSettings& m = s.backend() ;

	if( m.contains( "MountPrefix" ) ){

		if( !m.value( "MountPrefix" ).toString().isEmpty() ){

			return ;
		}
	}

	if( utility::platformIsWindows() ){

		m.setValue( "MountPrefix",s.homePath() + "/Desktop" ) ;
	}else{
		m.setValue( "MountPrefix",s.homePath() + "/.SiriKali" ) ;
	}
}

QString settings::mountPath()
{
	_set_mount_default( *this ) ;

	return m_settings.value( "MountPrefix" ).toString() ;
}

QString settings::mountPath( const QString& path )
{
	_set_mount_default( *this ) ;

	return m_settings.value( "MountPrefix" ).toString() + "/" + path ;
}

QString settings::walletName()
{
	return "SiriKali" ;
}

QString settings::applicationName()
{
	return "SiriKali" ;
}

int settings::readPasswordMaximumLength()
{
	if( !m_settings.contains( "ReadPasswordMaximumLength" ) ){

		m_settings.setValue( "ReadPasswordMaximumLength",1024 ) ;
	}

	return m_settings.value( "ReadPasswordMaximumLength" ).toInt() ;
}

bool settings::unMountVolumesOnLogout()
{
	if( !m_settings.contains( "UnMountVolumesOnLogout" ) ){

		m_settings.setValue( "UnMountVolumesOnLogout",false ) ;
	}

	return m_settings.value( "UnMountVolumesOnLogout" ).toBool() ;
}

favorites::entry settings::readFavorite( const QString& e )
{
	for( const auto& it : this->readFavorites() ){

		if( it.volumePath == e ){

			return it ;
		}
	}

	return {} ;
}

void settings::readFavorites( QMenu * m )
{
	m->clear() ;

	auto _add_action = [ m ]( const QString& e,const QString& s ){

		auto ac = new QAction( m ) ;

		ac->setText( e ) ;
		ac->setObjectName( s ) ;

		return ac ;
	} ;

	m->addAction( _add_action( QObject::tr( "Manage Favorites" ),"Manage Favorites" ) ) ;
	m->addAction( _add_action( QObject::tr( "Mount All" ),"Mount All" ) ) ;

	m->addSeparator() ;

	for( const auto& it : this->readFavorites() ){

		const auto& e = it.volumePath ;

		m->addAction( _add_action( e,e ) ) ;
	}
}

void settings::setDefaultMountPointPrefix( const QString& path )
{
	m_settings.setValue( "MountPrefix",path ) ;
}

template< typename T >
static void _selectOption( QMenu * m,const T& opt )
{
	for( const auto& it : m->actions() ){

		it->setChecked( it->objectName() == opt ) ;
	}
}

void settings::setLocalizationLanguage( bool translate,
					QMenu * m,
					settings::translator& translator )
{
	auto r = settings::instance().localizationLanguage().toLatin1() ;

	if( translate ){

		translator.setLanguage( r ) ;
	}else{
		const auto e = utility::directoryList( settings::instance().localizationLanguagePath() ) ;

		for( const auto& it : e ){

			if( !it.startsWith( "qt_" ) && it.endsWith( ".qm" ) ){

				auto name = it ;
				name.remove( ".qm" ) ;

				auto uiName = translator.UIName( name ) ;

				if( !uiName.isEmpty() ){

					auto ac = m->addAction( uiName ) ;

					ac->setCheckable( true ) ;
					ac->setObjectName( name ) ;
					ac->setText( translator.translate( name ) ) ;
				}
			}
		}

		_selectOption( m,r ) ;
	}
}

void settings::languageMenu( QMenu * m,QAction * ac,settings::translator& s )
{
	auto e = ac->objectName() ;

	this->setLocalizationLanguage( e ) ;

	this->setLocalizationLanguage( true,m,s ) ;

	_selectOption( m,e ) ;
}

QString settings::localizationLanguagePath()
{
	if( utility::platformIsWindows() ){

		return QDir().currentPath() + "/translations" ;
	}else{
		return TRANSLATION_PATH ;
	}
}

void settings::setLocalizationLanguage( const QString& language )
{
	m_settings.setValue( "Language",language ) ;
}

bool settings::startMinimized()
{
	if( m_settings.contains( "StartMinimized" ) ){

		return m_settings.value( "StartMinimized" ).toBool() ;
	}else{
		bool s = false  ;

		m_settings.setValue( "StartMinimized",s ) ;

		return s ;
	}
}

void settings::setStartMinimized( bool e )
{
	m_settings.setValue( "StartMinimized",e ) ;
}

void settings::setFileManager( const QString& e )
{
	if( e.isEmpty() ){

		m_settings.setValue( "FileManagerOpener",_file_manager() ) ;
	}else{
		m_settings.setValue( "FileManagerOpener",e ) ;
	}
}

QString settings::preUnMountCommand()
{
	if( !m_settings.contains( "PreUnMountCommand" ) ){

		m_settings.setValue( "PreUnMountCommand",QString() ) ;
	}

	return m_settings.value( "PreUnMountCommand" ).toString() ;
}

void settings::preUnMountCommand( const QString& e )
{
	m_settings.setValue( "PreUnMountCommand",e ) ;
}

void settings::runCommandOnMount( const QString& e )
{
	m_settings.setValue( "RunCommandOnMount",e ) ;
}

QString settings::runCommandOnMount()
{
	if( m_settings.contains( "RunCommandOnMount" ) ){

		return m_settings.value( "RunCommandOnMount" ).toString() ;
	}else{
		QString s ;
		m_settings.setValue( "RunCommandOnMount",s ) ;

		return s ;
	}
}

bool settings::reUseMountPoint()
{
	if( m_settings.contains( "ReUseMountPoint" ) ){

		return m_settings.value( "ReUseMountPoint" ).toBool() ;
	}else{
		bool e = false ;

		m_settings.setValue( "ReUseMountPoint",e ) ;

		return e ;
	}
}

void settings::reUseMountPoint( bool e )
{
	m_settings.setValue( "ReUseMountPoint",e ) ;
}

bool settings::autoOpenFolderOnMount()
{
	if( m_settings.contains( "AutoOpenFolderOnMount" ) ){

		return m_settings.value( "AutoOpenFolderOnMount" ).toBool() ;
	}else{
		bool s = true ;
		settings::autoOpenFolderOnMount( s ) ;
		return s ;
	}
}

void settings::autoOpenFolderOnMount( bool e )
{
	m_settings.setValue( "AutoOpenFolderOnMount",e ) ;
}

bool settings::autoCheck()
{
	if( m_settings.contains( "AutoCheckForUpdates" ) ){

		return m_settings.value( "AutoCheckForUpdates" ).toBool() ;
	}else{
		bool s = false ;
		settings::autoCheck( s ) ;
		return s ;
	}
}

bool settings::getOpenVolumeReadOnlyOption()
{
	return readOnlyWarning::getOpenVolumeReadOnlyOption() ;
}

bool settings::setOpenVolumeReadOnly( QWidget * parent,bool checked )
{
	return readOnlyWarning::showWarning( parent,checked ) ;
}

void settings::autoCheck( bool e )
{
	m_settings.setValue( "AutoCheckForUpdates",e ) ;
}

bool settings::readOnlyWarning()
{
	if( m_settings.contains( "ReadOnlyWarning" ) ){

		return m_settings.value( "ReadOnlyWarning" ).toBool() ;
	}else{
		bool s = false ;
		settings::readOnlyWarning( s ) ;
		return s ;
	}
}

void settings::readOnlyWarning( bool e )
{
	m_settings.setValue( "ReadOnlyWarning",e ) ;
}

bool settings::doNotShowReadOnlyWarning()
{
	if( m_settings.contains( "DoNotShowReadOnlyWarning" ) ){

		return m_settings.value( "DoNotShowReadOnlyWarning" ).toBool() ;
	}else{
		bool s = false ;
		settings::doNotShowReadOnlyWarning( s ) ;
		return s ;
	}
}

void settings::doNotShowReadOnlyWarning( bool e )
{
	m_settings.setValue( "DoNotShowReadOnlyWarning",e ) ;
}

bool settings::autoMountFavoritesOnStartUp()
{
	if( m_settings.contains( "AutoMountFavoritesOnStartUp" ) ){

		return m_settings.value( "AutoMountFavoritesOnStartUp" ).toBool() ;
	}else{
		bool s = false ;
		settings::autoMountFavoritesOnStartUp( s ) ;
		return s ;
	}
}

void settings::autoMountFavoritesOnStartUp( bool e )
{
	m_settings.setValue( "AutoMountFavoritesOnStartUp",e ) ;
}

void settings::autoMountBackEnd( const settings::walletBackEnd& e )
{
	m_settings.setValue( "AutoMountPassWordBackEnd",[ & ]()->QString{

		if( e.isInvalid() ){

			return "none" ;

		}else if( e == LXQt::Wallet::BackEnd::internal ){

			return "internal" ;

		}else if( e == LXQt::Wallet::BackEnd::libsecret ){

			return "libsecret" ;

		}else if( e == LXQt::Wallet::BackEnd::kwallet ){

			return "kwallet" ;

		}else if( e == LXQt::Wallet::BackEnd::osxkeychain ){

			return "osxkeychain" ;
		}else{
			return "none" ;
		}
			     }() ) ;
}

QSettings &settings::backend()
{
	return m_settings ;
}

settings::walletBackEnd settings::autoMountBackEnd()
{
	if( m_settings.contains( "AutoMountPassWordBackEnd" ) ){

		auto e = m_settings.value( "AutoMountPassWordBackEnd" ).toString() ;

		if( e == "libsecret" ){

			return LXQt::Wallet::BackEnd::libsecret ;

		}else if( e == "kwallet" ){

			return LXQt::Wallet::BackEnd::kwallet ;

		}else if( e == "internal" ){

			return LXQt::Wallet::BackEnd::internal ;

		}else if( e == "osxkeychain" ){

			return LXQt::Wallet::BackEnd::osxkeychain ;
		}else{
			return settings::walletBackEnd() ;
		}
	}else{
		m_settings.setValue( "AutoMountPassWordBackEnd",QString( "none" ) ) ;
		return settings::walletBackEnd() ;
	}
}

void settings::autoMountFavoritesOnAvailable( bool e )
{
	m_settings.setValue( "AutoMountFavoritesOnAvailable",e ) ;
}

bool settings::autoMountFavoritesOnAvailable()
{
	if( m_settings.contains( "AutoMountFavoritesOnAvailable" ) ){

		return m_settings.value( "AutoMountFavoritesOnAvailable" ).toBool() ;
	}else{
		settings::autoMountFavoritesOnAvailable( false ) ;
		return false ;
	}
}

int settings::networkTimeOut()
{
	if( m_settings.contains( "NetworkTimeOut" ) ){

		return m_settings.value( "NetworkTimeOut" ).toInt() ;
	}else{
		int s = 5 ;
		m_settings.setValue( "NetworkTimeOut",s ) ;
		return s ;
	}
}

bool settings::showMountDialogWhenAutoMounting()
{
	if( m_settings.contains( "ShowMountDialogWhenAutoMounting" ) ){

		return m_settings.value( "ShowMountDialogWhenAutoMounting" ).toBool() ;
	}else{
		settings::showMountDialogWhenAutoMounting( false ) ;
		return false ;
	}
}

void settings::showMountDialogWhenAutoMounting( bool e )
{
	m_settings.setValue( "ShowMountDialogWhenAutoMounting",e ) ;
}

int settings::checkForUpdateInterval()
{
	if( m_settings.contains( "CheckForUpdateInterval" ) ){

		return m_settings.value( "CheckForUpdateInterval" ).toInt() * 1000 ;
	}else{
		int s = 10 ;
		m_settings.setValue( "CheckForUpdateInterval",s ) ;
		return s * 1000 ;
	}
}

bool settings::ecryptfsAllowNotEncryptingFileNames()
{
	if( !m_settings.contains( "EcryptfsAllowNotEncryptingFileNames" ) ){

		m_settings.setValue( "EcryptfsAllowNotEncryptingFileNames",false ) ;
	}

	return m_settings.value( "EcryptfsAllowNotEncryptingFileNames" ).toBool() ;
}

void settings::addToFavorite( const QStringList& e )
{
	if( !e.isEmpty() ){

		m_settings.setValue( "FavoritesVolumes",[ & ](){

			auto q = this->readFavorites() ;

			q.emplace_back( e ) ;

			QStringList l ;

			for( const auto& it : q ){

				l.append( it.configString() ) ;
			}

			return l ;
		}() ) ;
	}
}

std::vector< favorites::entry > settings::readFavorites()
{
	if( m_settings.contains( "FavoritesVolumes" ) ){

		std::vector< favorites::entry > e ;

		for( const auto& it : m_settings.value( "FavoritesVolumes" ).toStringList() ){

			e.emplace_back( it ) ;
		}

		return e ;
	}else{
		return {} ;
	}
}

void settings::removeFavoriteEntry( const favorites::entry& e )
{
	m_settings.setValue( "FavoritesVolumes",[ & ](){

		QStringList l ;

		for( const auto& it : this->readFavorites() ){

			if( it != e ){

				l.append( it.configString() ) ;
			}
		}

		return l ;
	}() ) ;
}

QString settings::localizationLanguage()
{
	if( m_settings.contains( "Language" ) ){

		return m_settings.value( "Language" ).toString() ;
	}else{
		QString s = "en_US" ;
		m_settings.setValue( "Language",s ) ;
		return s ;
	}
}

QString settings::walletName( LXQt::Wallet::BackEnd s )
{
	if( s == LXQt::Wallet::BackEnd::kwallet ){

		if( m_settings.contains( "KWalletName" ) ){

			return m_settings.value( "KWalletName" ).toString() ;
		}else{
			QString s = "default" ;
			m_settings.setValue( "KWalletName",s ) ;
			return s ;
		}
	}else{
		return settings::instance().walletName() ;
	}
}

void settings::windowDimensions::setDimensions( const QStringList& e )
{
	m_ok = int( e.size() ) == int( m_array.size() ) ;

	if( m_ok ){

		for( size_t i = 0 ; i < m_array.size() ; i++ ){

			m_array[ i ] = static_cast< int >( e.at( static_cast< int >( i ) ).toInt( &m_ok ) ) ;

			if( !m_ok ){

				break ;
			}
		}
	}else{
		utility::debug() << "window dimensions do not match data structure size" ;
	}
}

settings::windowDimensions::windowDimensions( const QStringList& e )
{
	this->setDimensions( e ) ;
}

settings::windowDimensions::windowDimensions( const QString& e )
{
	this->setDimensions( utility::split( e,' ' ) ) ;
}

settings::windowDimensions::windowDimensions( const std::array< int,size >& e )
	: m_array( e ),m_ok( true )
{
}

settings::windowDimensions::operator bool()
{
	return m_ok ;
}

int settings::windowDimensions::columnWidthAt( std::array< int,size >::size_type s ) const
{
	auto e = s + 4 ;

	if( e < m_array.size() ){

		return m_array[ e ] ;
	}else{
		utility::debug() << "window dimension index out of range" ;
		return 0 ;
	}
}

QRect settings::windowDimensions::geometry() const
{
	auto e = m_array.data() ;

	return { *( e + 0 ),*( e + 1 ),*( e + 2 ),*( e + 3 ) } ;
}

QString settings::windowDimensions::dimensions() const
{
	auto _number = []( const int * s,size_t n ){ return QString::number( *( s + n ) ) ; } ;

	auto s = m_array.data() ;

	auto e = _number( s,0 ) ;

	using tp = decltype( m_array.size() ) ;

	for( tp i = 1 ; i < m_array.size() ; i++ ){

		e += " " + _number( s,i ) ;
	}

	return e ;
}

settings::windowDimensions settings::getWindowDimensions()
{
	QString defaults = "205 149 861 466 326 320 101 76" ;

	QSettings& s = settings::instance().backend() ;

	if( s.contains( "Dimensions" ) ){

		settings::windowDimensions e( s.value( "Dimensions" ).toString() ) ;

		if( e ){

			return e ;
		}else{
			utility::debug() << "failed to parse config option" ;
			return defaults ;
		}
	}else{
		s.setValue( "Dimensions",defaults ) ;
		return defaults ;
	}
}

void settings::setWindowDimensions( const settings::windowDimensions& e )
{
	settings::instance().backend().setValue( "Dimensions",e.dimensions() ) ;
}

settings::translator::translator()
{
	m_languages.emplace_back( QObject::tr( "Russian (RU)" ),"Russian (RU)","ru_RU" ) ;
	m_languages.emplace_back( QObject::tr( "French (FR)" ),"French (FR)","fr_FR" ) ;
	m_languages.emplace_back( QObject::tr( "German (DE)" ),"German (DE)","de_DE" ) ;
	m_languages.emplace_back( QObject::tr( "English (US)" ),"English (US)","en_US" ) ;
	m_languages.emplace_back( QObject::tr( "Swedish (SE)" ),"Swedish (SE)","sv_SE" ) ;
	m_languages.emplace_back( QObject::tr( "Arabic (SA)" ) ,"Arabic (SA)","ar_SA" ) ;
	m_languages.emplace_back( QObject::tr( "Spanish (MX)" ),"Spanish (MX)","es_MX" ) ;
}

void settings::translator::setLanguage( const QByteArray& e )
{
	QCoreApplication::installTranslator( [ & ](){

		this->clear() ;

		m_translator = new QTranslator() ;

		m_translator->load( e.constData(),settings::instance().localizationLanguagePath() ) ;

		return m_translator ;
	}() ) ;
}

settings::translator::~translator()
{
	this->clear() ;
}

const QString& settings::translator::UIName( const QString& internalName )
{
	for( const auto& it : m_languages ){

		if( it.internalName == internalName ){

			return it.UINameTranslated ;
		}
	}

	static QString s ;
	return s ;
}

const QString& settings::translator::name( const QString& UIName )
{
	for( const auto& it : m_languages ){

		if( it.UINameTranslated == UIName ){

			return it.internalName ;
		}
	}

	static QString s ;
	return s ;
}

QString settings::translator::translate( const QString& internalName )
{
	return QObject::tr( this->UINameUnTranslated( internalName ) ) ;
}

const char * settings::translator::UINameUnTranslated( const QString& internalName )
{
	for( const auto& it : m_languages ){

		if( it.internalName == internalName ){

			return it.UINameUnTranslated ;
		}
	}

	return "" ;
}

void settings::translator::clear()
{
	if( m_translator ){

		QCoreApplication::removeTranslator( m_translator ) ;
		delete m_translator ;
	}
}

settings::translator::entry::entry( const QString& a,const char * b,const QString& c ) :
	UINameTranslated( a ),UINameUnTranslated( b ),internalName( c )
{
}
