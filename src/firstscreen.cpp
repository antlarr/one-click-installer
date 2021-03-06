//      Copyright 2012 Saurabh Sood <saurabh@saurabh.geeko>
//      
//      This program is free software; you can redistribute it and/or modify
//      it under the terms of the GNU General Public License as published by
//      the Free Software Foundation; either version 2 of the License, or
//      (at your option) any later version.
//      
//      This program is distributed in the hope that it will be useful,
//      but WITHOUT ANY WARRANTY; without even the implied warranty of
//      MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//      GNU General Public License for more details.
//      
//      You should have received a copy of the GNU General Public License
//      along with this program; if not, write to the Free Software
//      Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,
//      MA 02110-1301, USA.
//      
//      


#include "firstscreen.h"

FirstScreen::FirstScreen( PackageBackend *backend, QString *tmpFileName, const QString& filename, QObject *parent )
{
    m_tmpFileName = tmpFileName;
    setStyleSheet( "background-color : white;" );

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->setSpacing( 0 );

    QFile dataFile( m_tmpFileName->toAscii() );
    if( !dataFile.open( QIODevice::Truncate | QIODevice::WriteOnly ) ) {
        qDebug() << "Could not open Data File";
    }

    QTextStream outData( &dataFile );

    setLayout( mainLayout );

    //Parse YMP File
    m_backend = backend;
    OCI::YmpParser parser( filename );
    parser.parse();
    m_packages = parser.packages();
    m_repos = parser.repositories();

    //Add Repository
    int i = 0;
    QVBoxLayout *repoDetails;

    m_untrustedSources = 0;

    foreach( OCI::Repository *iter, m_repos) {
        m_backend->addRepository( QUrl( iter->url() ) );
        RepositoryWidget *repositoryDetails = new RepositoryWidget( m_backend, i, m_repos.at( i ) );
        mainLayout->addWidget( repositoryDetails );

        if( !m_backend->exists( iter->url() ) )
            m_untrustedSources++;
        static int j = 0;
        foreach( OCI::Package *iter, m_packages ) {
            m_backend->addPackage( iter->name() );
            mainLayout->addSpacing( -10 );
            PackageDetails *packDetails = new PackageDetails( iter, j, m_packages.count() );
            QObject::connect( packDetails, SIGNAL( sizeUpdated( QString ) ), this, SIGNAL( sizeUpdated( QString ) ) );
            mainLayout->addWidget( packDetails );
            j++;
        }
        i++;
        mainLayout->addSpacing( -8 );
    }

    if( m_untrustedSources > 0 ) {
        m_warning = new	QLabel( "<b>Be careful!</b> Some Sources are not currently known. Installing<br />software requires trusting these sources" );
        m_warning->setStyleSheet( "border : 1px solid rgb(196,181,147); background-color: rgb(253, 227, 187); border-radius : 10px" );
        m_warning->setContentsMargins( 10,10,10,10 );
        mainLayout->addSpacing( 20 );
        mainLayout->addWidget( m_warning );
    }

    foreach( QUrl iter, m_backend->repositories()) {
        outData << "R " << iter.toString() << "\n";
    }

    foreach( QString iter, m_backend->packages() ) {
        outData << "P " << iter << "\n";
    }

    dataFile.close();
}

void FirstScreen::showEvent( QShowEvent *s )
{
    emit countChanged( m_repos.count(), m_packages.count() );
    qDebug() << "number of untrusted sources is " << m_untrustedSources;
}
