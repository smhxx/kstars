/***************************************************************************
                          kstarsdata.cpp  -  K Desktop Planetarium
                             -------------------
    begin                : Sun Jul 29 2001
    copyright            : (C) 2001 by Heiko Evermann
    email                : heiko@evermann.de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <qregexp.h>
#include <kmessagebox.h>
#include <kdebug.h>

#include "kstars.h"
#include "ksutils.h"
#include "kstarsdata.h"
#include "kstarsmessagebox.h"
#include "filesource.h"
#include "stardatasink.h"

#include <kapplication.h>

QList<GeoLocation> KStarsData::geoList = QList<GeoLocation>();
QMap<QString, TimeZoneRule> KStarsData::Rulebook = QMap<QString, TimeZoneRule>();
int KStarsData::objects = 0;

KStarsData::KStarsData( KStars *ks ) : Moon(0), kstars( ks ), initTimer(0), inited(false),
	source(0), loader(0), pump(0) {
	objects++;
	stdDirs = new KStandardDirs();
	options = new KStarsOptions();
	LSTh = new dms();
	HourAngle = new dms();
	
	locale = new KLocale( "kstars" );
	oldOptions = 0;

	PC = new PlanetCatalog(ks);

	//REVERTED...remove comments after 1/1/2003
	//ARRAY: setting static array limit for now
	//starArray = new StarObject[40000];
	starList.setAutoDelete( TRUE );
	
	geoList.setAutoDelete( TRUE );
	deepSkyList.setAutoDelete( TRUE );
	clineList.setAutoDelete( TRUE );
	clineModeList.setAutoDelete( TRUE );
	cnameList.setAutoDelete( TRUE );
	Equator.setAutoDelete( TRUE );
	Ecliptic.setAutoDelete( TRUE );
	Horizon.setAutoDelete( TRUE );
	PlanetTrail.setAutoDelete( TRUE );
	for ( unsigned int i=0; i<11; ++i ) MilkyWay[i].setAutoDelete( TRUE );
    VariableStarsList.setAutoDelete(TRUE);

	//Initialize object type strings
	//(type==-1 is a constellation)
	TypeName[0] = i18n( "star" );
	TypeName[1] = i18n( "multiple star" );
	TypeName[2] = i18n( "planet" );
	TypeName[3] = i18n( "open cluster" );
	TypeName[4] = i18n( "globular cluster" );
	TypeName[5] = i18n( "gaseous nebula" );
	TypeName[6] = i18n( "planetary nebula" );
	TypeName[7] = i18n( "supernova remnant" );
	TypeName[8] = i18n( "galaxy" );
	TypeName[9] = i18n( "Users should never see this", "UNUSED_TYPE" );
	
	// at startup times run forward
	setTimeDirection( 0.0 );
}

KStarsData::~KStarsData() {
	objects--;
	checkDataPumpAction();
	
	if ( 0 != oldOptions ) {
  	delete oldOptions;
		oldOptions = 0;
	}

	if ( 0 != options ) {
		delete options;
		options = 0;
	}
	// the list items do not need to be removed by hand.
	// all lists are set to AutoDelete = true

	//REVERTED...remove comments after 1/1/2003
	//ARRAY
	//delete[] starArray;

	if (stdDirs) delete stdDirs;
	if (Moon) delete Moon;
	if (locale) delete locale;
	if (LSTh) delete LSTh;
	if (HourAngle) delete HourAngle;
	if (PC) delete PC;
}

bool KStarsData::readMWData( void ) {
	QFile file;

	for ( unsigned int i=0; i<11; ++i ) {
		QString snum, fname, szero;
		snum = snum.setNum( i+1 );
		if ( i+1 < 10 ) szero = "0"; else szero = "";
		fname = "mw" + szero + snum + ".dat";

		if ( KSUtils::openDataFile( file, fname ) ) {
		  QTextStream stream( &file );

	  	while ( !stream.eof() ) {
		  	QString line;
			  double ra, dec;

			  line = stream.readLine();

			  ra = line.mid( 0, 8 ).toDouble();
	  		dec = line.mid( 9, 8 ).toDouble();

			  SkyPoint *o = new SkyPoint( ra, dec );
			  MilkyWay[i].append( o );
	  	}
			file.close();
		} else {
			return false;
		}
	}
	return true;
}

bool KStarsData::readADVTreeData(void)
{

  QFile file;
  QString Interface;

  // !!!! for some reason, I cannot load any new data file?!! I'm using existing one for the sake of development, then I'll check it out.
  if (!KSUtils::openDataFile(file, "advinterface.dat"))
   return false;

   QTextStream stream(&file);
   QString Line;

  while  (!stream.atEnd())
  {
    QString Name, Link, subName;
    int Type, interfaceIndex;

    Line = stream.readLine();

    if (Line.startsWith("[KSLABEL]"))
    {
       Name = Line.mid(9);
       Type = 0;
    }
    else if (Line.startsWith("[END]"))
       Type = 1;
    else if (Line.startsWith("[KSINTERFACE]"))
    {
      Interface = Line.mid(13);
      continue;
    }
      
    else
    {
      Name = Line.mid(0, Line.find(":"));
      Link = Line.mid(Line.find(":") + 1);

      // Link is empty, using Interface instead
      if (Link.isEmpty())
      {
         Link = Interface;
         subName = Name;
         interfaceIndex = Link.find("KSINTERFACE");
         Link.remove(interfaceIndex, 11);
         Link = Link.insert(interfaceIndex, subName.replace( QRegExp(" "), "+"));

       }
      
      Type = 2;
    }
      
    ADVTreeData *ADVData = new ADVTreeData;

    ADVData->Name = Name;
    ADVData->Link = Link;
    ADVData->Type = Type;

    ADVtreeList.append(ADVData);
   }

   return true;



}
bool KStarsData::readVARData(void)
{
  QFile file;

  if (!KSUtils::openDataFile(file, "var.dat"))
   return false;
  
   QTextStream stream(&file);

  while  (!stream.atEnd())
  {
    QString Name;
    QString Designation;
    QString Line;

    Line = stream.readLine();

    Designation = Line.mid(0,8).stripWhiteSpace();
    Name          = Line.mid(10,20).simplifyWhiteSpace();

    VariableStarInfo *VInfo = new VariableStarInfo;

    VInfo->Designation = Designation;
    VInfo->Name          = Name;
    VariableStarsList.append(VInfo);
   }

   return true;

}

bool KStarsData::readCLineData( void ) {
	QFile file;
	if ( KSUtils::openDataFile( file, "clines.dat" ) ) {
	  QTextStream stream( &file );

  	while ( !stream.eof() ) {
	  	QString line, name;
		  int rah, ram, ras, dd, dm, ds;
		  QChar sgn;
			QChar *mode;

		  line = stream.readLine();

		  rah = line.mid( 0, 2 ).toInt();
	  	ram = line.mid( 2, 2 ).toInt();
		  ras = line.mid( 4, 2 ).toInt();

		  sgn = line.at( 6 );
	  	dd = line.mid( 7, 2 ).toInt();
		  dm = line.mid( 9, 2 ).toInt();
		  ds = line.mid( 11, 2 ).toInt();
	    mode = new QChar( line.at( 13 ) );
		  dms r; r.setH( rah, ram, ras );
	  	dms d( dd, dm,  ds );

		  if ( sgn == "-" ) { d.setD( -1.0*d.Degrees() ); }

		  SkyPoint *o = new SkyPoint( r.Hours(), d.Degrees() );
		  clineList.append( o );
			clineModeList.append( mode );
  	}
		file.close();

		return true;
	} else {
		return false;
	}
}

bool KStarsData::readCNameData( void ) {
  QFile file;
	cnameFile = "cnames.dat";
//	if ( locale->language()=="fr" ) cnameFile = "fr_cnames.dat";

	if ( KSUtils::openDataFile( file, cnameFile ) ) {
	  QTextStream stream( &file );

  	while ( !stream.eof() ) {
	  	QString line, name, abbrev;
		  int rah, ram, ras, dd, dm, ds;
		  QChar sgn;

	  	line = stream.readLine();

		  rah = line.mid( 0, 2 ).toInt();
		  ram = line.mid( 2, 2 ).toInt();
	  	ras = line.mid( 4, 2 ).toInt();

		  sgn = line.at( 6 );
		  dd = line.mid( 7, 2 ).toInt();
	  	dm = line.mid( 9, 2 ).toInt();
		  ds = line.mid( 11, 2 ).toInt();

			abbrev = line.mid( 13, 3 );
		  name  = line.mid( 17 ).stripWhiteSpace();

		  dms r; r.setH( rah, ram, ras );
		  dms d( dd, dm,  ds );

	  	if ( sgn == "-" ) { d.setD( -1.0*d.Degrees() ); }

		  SkyObject *o = new SkyObject( -1, r, d, 0.0, name, abbrev, "" );
			cnameList.append( o );
			ObjNames.append( o );
	  }
		file.close();

		return true;
	} else {
		return false;
	}
}

bool KStarsData::readStarData( void ) {
	QFile file;
	if ( KSUtils::openDataFile( file, "sao.dat" ) ) {
		QTextStream stream( &file );
		
		//REVERTED...remove comments after 1/1/2003
		//ARRAY:
		//StarCount = 0;
		
		while ( !stream.eof() ) {
			QString line, name, gname, SpType;
			float mag;
			int rah, ram, ras, dd, dm, ds;
			QChar sgn;

			line = stream.readLine();

			mag = line.mid( 33, 4 ).toFloat();	// star magnitude
			if ( mag > options->magLimitDrawStar )
				break;	// break reading file if magnitude is higher than needed
			else
				lastFileIndex = file.at();	// stores current file index - needed by reloading
			
			name = ""; gname = "";

			rah = line.mid( 0, 2 ).toInt();
			ram = line.mid( 2, 2 ).toInt();
			ras = int( line.mid( 4, 6 ).toDouble() + 0.5 ); //add 0.5 to make int() pick nearest integer

			sgn = line.at( 17 );
			dd = line.mid( 18, 2 ).toInt();
			dm = line.mid( 20, 2 ).toInt();
			ds = int( line.mid( 22, 5 ).toDouble() + 0.5 ); //add 0.5 to make int() pick nearest integer

			SpType = line.mid( 37, 2 );
			name = line.mid( 40 ).stripWhiteSpace(); //the rest of the line
			if ( name.contains( ':' ) ) { //genetive form exists
				gname = name.mid( name.find(':')+1 );
				name = name.mid( 0, name.find(':') ).stripWhiteSpace();
			}
			if ( name.isEmpty() ) name = "star";

			dms r;
			r.setH( rah, ram, ras );
			dms d( dd, dm,  ds );

			if ( sgn == "-" ) { d.setD( -1.0*d.Degrees() ); }

			//REVERTED...remove comments after 1/1/2003
			//ARRAY:
			StarObject *o = new StarObject( r, d, mag, name, gname, SpType );
			starList.append( o );
			//starArray[StarCount++] = StarObject( r, d, mag, name, gname, SpType );
			//StarObject *o = &starArray[StarCount-1];
			
			if ( o->name() != "star" ) {		// just add to name list if a name is given
				ObjNames.append( o );
			}

  	}  // end of while
		file.close();

/*
	* Store the max set magnitude of current session. Will increased in KStarsData::appendNewData()
	*/
		maxSetMagnitude = options->magLimitDrawStar;
		
		return true;
	} else {
		maxSetMagnitude = 0;  // nothing loaded
		return false;
	}
}

bool KStarsData::readDeepSkyData( void ) {
	QFile file;
	if ( KSUtils::openDataFile( file, "ngcic.dat" ) ) {
	  QTextStream stream( &file );

		while ( !stream.eof() ) {
			QString line, con, ss, name, name2, longname;
			QString cat, cat2;
			float mag(1000.0), ras, a, b;
			int type, ingc, imess(-1), rah, ram, dd, dm, ds, pa;
			int pgc, ugc;
			QChar sgn, iflag;

			line = stream.readLine();

			iflag = line.at( 0 ); //check for IC catalog flag
			if ( iflag == 'I' ) cat = "IC";
			else if ( iflag == ' ' ) cat = "NGC"; // if blank, set catalog to NGC

			ingc = line.mid( 1, 4 ).toInt();  // NGC/IC catalog number
			if ( ingc==0 ) cat = ""; //object is not in NGC or IC catalogs

			con = line.mid( 6, 3 );     // constellation
			rah = line.mid( 10, 2 ).toInt();
			ram = line.mid( 13, 2 ).toInt();
			ras = line.mid( 16, 4 ).toFloat();
			sgn = line.at( 21 );
			dd = line.mid( 22, 2 ).toInt();
			dm = line.mid( 25, 2 ).toInt();
			ds = line.mid( 28, 2 ).toInt();
			type = line.mid( 31, 1 ).toInt();
			//Skipping detailed type string for now (it's mid(33,4) )
			ss = line.mid( 38, 4 );
		  if (ss == "    " ) { mag = 99.9; } else { mag = ss.toFloat(); }
			ss = line.mid( 43, 6 );
			if (ss == "      " ) { a = 0.0; } else { a = ss.toFloat(); }
			ss = line.mid( 50, 5 );
			if (ss == "     " ) { b = 0.0; } else { b = ss.toFloat(); }
			ss = line.mid( 56, 3 );
			if (ss == "   " ) { pa = 0; } else { pa = ss.toInt(); }
			ss = line.mid( 60, 3 );
			if (ss != "   " ) { //Messier object
				cat2 = cat;
				if ( ingc==0 ) cat2 = "";
				cat = "M";
				imess = ss.toInt();
			}
			ss = line.mid( 64, 6 );
			if (ss == "      " ) { pgc = 0; } else { pgc = ss.toInt(); }
			ss = line.mid( 71, 5 );
			if (ss == "     " ) { ugc = 0; } else { ugc = ss.toInt(); }
			longname = line.mid( 77, line.length() ).stripWhiteSpace();

			dms r;
			r.setH( rah, ram, int(ras) );
			dms d( dd, dm, ds );

			if ( sgn == "-" ) { d.setD( -1.0*d.Degrees() ); }

			QString snum;
			if ( cat=="IC" || cat=="NGC" ) {
				snum.setNum( ingc );
				name = cat + " " + snum;
			} else if ( cat=="M" ) {
				snum.setNum( imess );
				name = cat + " " + snum;
				if ( cat2=="NGC" ) {
					snum.setNum( ingc );
					name2 = cat2 + " " + snum;
				} else if ( cat2=="IC" ) {
					snum.setNum( ingc );
					name2 = cat2 + " " + snum;
				} else {
					name2 = "";
				}
			} else {
				if ( longname.isEmpty() ) name = i18n( "Unnamed Object" );
				else name = longname;
			}
			
			// create new starobject or skyobject
			// type 0 and 1 are starobjects
			// all other are skyobjects
			SkyObject *o = 0;
			if (type < 2) {
				o = new StarObject( type, r, d, mag, name, name2, longname, cat, a, b, pa, pgc, ugc );
			} else {
				o = new SkyObject( type, r, d, mag, name, name2, longname, cat, a, b, pa, pgc, ugc );
			}

			deepSkyList.append( o );
			ObjNames.append( o );

			//Add longname to objList, unless longname is the same as name
			if ( !o->longname().isEmpty() && o->name() != o->longname() && o->name() != i18n( "star" ) ) {
				ObjNames.append( o, true );  // append with longname
			}
		}
		file.close();

		return true;
	} else {
		return false;
	}
}

bool KStarsData::openURLFile(QString urlfile, QFile & file)
{

	//QFile file;
	QString localFile;
	bool fileFound = false;
    QFile localeFile;

	if ( locale->language() != "en_US" )
		localFile = locale->language() + "/" + urlfile;

	if ( ! localFile.isEmpty() && KSUtils::openDataFile( file, localFile ) )
   {
		fileFound = true;
	} else
   // Try to load locale file, if not successful, load regular urlfile and then copy it to locale.
   {
    	file.setName( locateLocal( "appdata", urlfile ) );
		if ( file.open( IO_ReadOnly ) ) fileFound = true;
       else if ( KSUtils::openDataFile( file, urlfile ) )
                {
           	        if ( locale->language() != "en_US" ) kdDebug() << i18n( "No localized URL file; using defaul English file." ) << endl;
                      // we found urlfile, we need to copy it to locale
                      localeFile.setName( locateLocal( "appdata", urlfile ) );
                      if (localeFile.open(IO_WriteOnly)) {
                        QTextStream readStream(&file);
                        QTextStream writeStream(&localeFile);
                        writeStream <<  readStream.read();
                        localeFile.close();
                        file.reset(); }
                      else
                         kdDebug() << i18n( "Failed to copy default URL file to locale directory, modifying default object links is not possible" ) << endl;
                      fileFound = true;
           	 } 
	
	}

	return fileFound;
}

bool KStarsData::readUserLog(void)
{

  QFile file;
  QString buffer;
  QString sub, name, data;
  
    if (!KSUtils::openDataFile( file, "userlog.dat" ))
     return false;

    QTextStream stream(&file);

  if (!stream.eof())
    buffer = stream.read();

  while (!buffer.isEmpty())
  {
     int startIndex, endIndex;

    startIndex = buffer.find("[KSLABEL:");
    sub = buffer.mid(startIndex);
    endIndex = sub.find("[KSLogEnd]");

    // Read name after KSLABEL identifer
    name = sub.mid(startIndex + 9, sub.find("]") - (startIndex + 9));
    // Read data and skip new line
    data   = sub.mid(sub.find("]") + 2, endIndex - (sub.find("]") + 2));
    buffer = buffer.mid(endIndex + 11);

  for ( SkyObjectName *sonm = ObjNames.first(); sonm; sonm = ObjNames.next() )
       {
          if ( sonm->text() == name )
          {
    		  sonm->skyObject()->userLog = data;
             break;
			}
        }
        
   } // end while

  file.close();

     return true;
}
bool KStarsData::readURLData( QString urlfile, int type ) {

    QFile file;
    if (!openURLFile(urlfile, file))
      return false;
    
    QTextStream stream(&file);
    
  	while ( !stream.eof() ) {
   	QString line = stream.readLine();

			QString name = line.mid( 0, line.find(':') );
			QString sub = line.mid( line.find(':')+1 );
			QString title = sub.mid( 0, sub.find(':') );
			QString url = sub.mid( sub.find(':')+1 );

			bool bMatched = false;
			for ( SkyObjectName *sonm = ObjNames.first(); sonm; sonm = ObjNames.next() ) {
				if ( sonm->text() == name ) {
             
					bMatched = true;

					if ( type==0 ) { //image URL
					  sonm->skyObject()->ImageList.append( url );
						sonm->skyObject()->ImageTitle.append( title );
					} else if ( type==1 ) { //info URL
					  sonm->skyObject()->InfoList.append( url );
						sonm->skyObject()->InfoTitle.append( title );
					}
	        break;
				}
			}

//			if ( ! bMatched ) kdWarning() << i18n( "Could not find match to object named " ) << name << endl;
		}
		file.close();

     return true;

}

bool KStarsData::readCustomData( QString filename, QList<SkyObject> &objList, bool showerrs ) {
	bool checkValue;
	bool badLine(false);
	int countValidLines(0);
	int countLine(0);

	int iType(0);
	double RA(0.0), Dec(0.0), mag(0.0);
	QString name(""); QString lname(""); QString spType("");

//If the filename begins with "~", replace the "~" with the user's home directory
//(otherwise, the file will not successfully open)
	if ( filename.at(0)=='~' ) filename = QDir::homeDirPath() + filename.mid( 1, filename.length() );

	QFile test( filename );
	QStringList errs;

	if ( test.open( IO_ReadOnly ) ) {
		QTextStream stream( &test );
		while ( !stream.eof() ) {
			QString sub, msg;
			QString line = stream.readLine();
			++countLine;
			iType = -1; //initialize to invalid values
			RA = 0.0; Dec = 0.0; mag = 0.0;
			name = ""; lname = ""; spType = "";

			if ( line.left(1) != "#" ) { //ignore commented lines
				QStringList fields = QStringList::split( " ", line ); //parse the line
				if ( fields.count() < 5 ) { //every valid line has at least 5 fields
					badLine = true;
					if (showerrs) errs.append( i18n( "Line " ) + QString( "%1" ).arg( countLine ) +
						i18n( " does not have enough fields." ) );
				} else {
					iType = (*fields.at(0)).toInt( &checkValue );
					if ( !checkValue ) {
						badLine = true;
						iType = -1;
						if (showerrs) errs.append( i18n( "Line " ) + QString( "%1" ).arg( countLine ) +
							i18n( ": field 1 is not an integer." ) );
					}
					RA = (*fields.at(1)).toDouble( &checkValue );
					if ( !checkValue ) {
						badLine = true;
						if (showerrs) errs.append( i18n( "Line " ) + QString( "%1" ).arg( countLine ) +
							i18n( ": field 2 is not a float (right ascension)." ) );
					}
					Dec = (*fields.at(2)).toDouble( &checkValue );
					if ( !checkValue ) {
						badLine = true;
						if (showerrs) errs.append( i18n( "Line " ) + QString( "%1" ).arg( countLine ) +
							i18n( ": field 3 is not a float (declination)." ) );
					}
					mag = (*fields.at(3)).toDouble( &checkValue );
					if ( !checkValue ) {
						badLine = true;
						if (showerrs) errs.append( i18n( "Line " ) + QString( "%1" ).arg( countLine ) +
							i18n( ": field 4 is not a float (magnitude)." ) );
					}

					if ( iType==0 || iType==1 ) {  //Star
						spType = (*fields.at(4));
						if ( !( spType.left(1)=="O" || spType.left(1)=="B" ||
								spType.left(1)=="A" || spType.left(1)=="F" ||
								spType.left(1)=="G" || spType.left(1)=="K" ||
								spType.left(1)=="M" ) ) { //invalid spType
							badLine = true;
							if (showerrs) errs.append( i18n( "Line " ) + QString( "%1" ).arg( countLine ) +
								i18n( ", field 5: invalid spectral type (must start with O,B,A,F,G,K or M)" ) );
						}
					} else if ( iType < 3 || iType > 8 ) { //invalid object type
						badLine = true;
						if (showerrs) errs.append( i18n( "Line " ) + QString( "%1" ).arg( countLine ) +
							i18n( ", field 1: invalid object type (must be 0,1,3,4,5,6,7 or 8)" ) );
					}
				}

				if ( !badLine ) { //valid data found!  add to objList
					++countValidLines;
					unsigned int Mark(4); //field marker; 5 for stars, 4 for deep-sky

					//Stars:
					if ( iType==0 || iType==1 ) Mark = 5;

					//First, check for name:
					if ( fields.count() > Mark && (*fields.at(Mark)).left(1)=="\"" ) {
						//The name is (probably) more than one word...
						name = (*fields.at(Mark)).mid(1, (*fields.at(Mark)).length()); //remove leading quote mark
						if (name.right(1)=="\"") { //name was one word in quotes...
							name = name.left( name.length() -1 ); //remove trailing quote mark
						} else {
							for ( unsigned int i=Mark+1; i<fields.count(); ++i ) {
								name = name + " " + (*fields.at(i));
								if (name.right(1)=="\"") {
									name = name.left( name.length() -1 ); //remove trailing quote mark
									break;
								}
							}
						}
					} else if ( fields.count() > Mark ) { //one-word name
						name = (*fields.at(Mark));
					}

					if ( Mark==5 ) { //Stars...
						if ( name == "" ) name = i18n( "star" );

						StarObject *o = new StarObject( RA, Dec, mag, name, "", spType );
						objList.append( o );
					} else if ( iType > 2 && iType <= 8 ) { //Deep-sky objects...
						if ( name == "" ) name = i18n( "unnamed object" );

						SkyObject *o = new SkyObject( iType, RA, Dec, mag, name, "", "" );
						objList.append( o );
					}
				}
			}
		}
	} else {
		if (showerrs) errs.append( i18n( "Could not open custom data file: " ) + filename );
		kdWarning() << i18n( "Could not open custom data file: " ) << filename;
	}

	if ( countValidLines==countLine && countValidLines ) { //all lines were valid!
		return true;
	} else if ( countValidLines ) { //found some valid lines, some invalid lines.
		if ( showerrs ) {
			QString message( i18n( "Some lines could not be parsed in the specified file, see error messages below." ) + "\n" +
											i18n( "To reject the file, press Cancel. " ) +
											i18n( "To accept the file (ignoring unparsed lines), press Accept." ) );
			if ( KMessageBox::warningContinueCancelList( 0, message, errs,
					i18n( "Some lines in file were invalid" ), i18n( "Accept" ) )==KMessageBox::Continue ) {
				return true;
			} else {
				return false;
			}
			return true; //not showing errs; return true for parsed lines
		}
	} else {
		if ( showerrs ) {
			QString message( i18n( "No lines could be parsed from the specified file, see error messages below." ) );
			KStarsMessageBox::badCatalog( 0, message, errs,
					i18n( "No Valid Data Found in File" ) );
//			KMessageBox::warningContinueCancelList( 0, message, errs,
//							 i18n( "No valid data found in file" ), i18n( "Close" ) );
		}
		return false;
	}
	// all test passed
	return false;
}


bool KStarsData::processCity( QString line ) {
	QString totalLine;
	QString name, province, country;
	QStringList fields;
	TimeZoneRule *TZrule;
	bool intCheck = true;
	char latsgn, lngsgn;
	int lngD, lngM, lngS;
	int latD, latM, latS;
	double TZ;
	float lng, lat;

	totalLine = line;

	// separate fields
	fields = QStringList::split( ":", line );

	for ( unsigned int i=0; i< fields.count(); ++i )
		fields[i] = fields[i].stripWhiteSpace();

	if ( fields.count() < 11 ) {
		kdDebug()<< i18n( "Cities.dat: Ran out of fields.  Line was:" ) <<endl;
		kdDebug()<< totalLine.local8Bit() <<endl;
		return false;
	} else if ( fields.count() < 12 ) {
		// allow old format (without TZ) for mycities.dat
		fields.append("");
		fields.append("--");
	} else if ( fields.count() < 13 ) {
		// Set TZrule to "--"
		fields.append("--");
	}

	name = fields[0];
	province = fields[1];
	country = fields[2];

	latD = fields[3].toInt( &intCheck );
	if ( !intCheck ) {
		kdDebug() << fields[3] << i18n( "\nCities.dat: Bad integer.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	latM = fields[4].toInt( &intCheck );
	if ( !intCheck ) {
		kdDebug() << fields[4] << i18n( "\nCities.dat: Bad integer.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	latS = fields[5].toInt( &intCheck );
	if ( !intCheck ) {
		kdDebug() << fields[5] << i18n( "\nCities.dat: Bad integer.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	QChar ctemp = fields[6].at(0);
	latsgn = ctemp;
	if (latsgn != 'N' && latsgn != 'S') {
		kdDebug() << latsgn << i18n( "\nCities.dat: Invalid latitude sign.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	lngD = fields[7].toInt( &intCheck );
	if ( !intCheck ) {
		kdDebug() << fields[7] << i18n( "\nCities.dat: Bad integer.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	lngM = fields[8].toInt( &intCheck );
	if ( !intCheck ) {
		kdDebug() << fields[8] << i18n( "\nCities.dat: Bad integer.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	lngS = fields[9].toInt( &intCheck );
	if ( !intCheck ) {
		kdDebug() << fields[9] << i18n( "\nCities.dat: Bad integer.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	ctemp = fields[10].at(0);
	lngsgn = ctemp;
	if (lngsgn != 'E' && lngsgn != 'W') {
		kdDebug() << latsgn << i18n( "\nCities.dat: Invalid longitude sign.  Line was:\n" ) << totalLine << endl;
		return false;
	}

	lat = (float)latD + ((float)latM + (float)latS/60.0)/60.0;
	lng = (float)lngD + ((float)lngM + (float)lngS/60.0)/60.0;

	if ( latsgn == 'S' ) lat *= -1.0;
	if ( lngsgn == 'W' ) lng *= -1.0;

	// find time zone. Use value from Cities.dat if available.
	// otherwise use the old approximation: int(lng/15.0);
	if ( fields[11].isEmpty() || ('x' == fields[11].at(0)) ) {
		TZ = int(lng/15.0);
	} else {
		bool doubleCheck = true;
		TZ = fields[11].toDouble( &doubleCheck);
		if ( !doubleCheck ) {
			kdDebug() << fields[11] << i18n( "\nCities.dat: Bad time zone.  Line was:\n" ) << totalLine << endl;
			return false;
		}
	}

	//last field is the TimeZone Rule ID.
	TZrule = &( Rulebook[ fields[12] ] );

//	if ( fields[12]=="--" )
//		kdDebug() << "Empty rule start month: " << TZrule->StartMonth << endl;
	geoList.append ( new GeoLocation( lng, lat, name, province, country, TZ, TZrule ));  // appends city names to list
	return true;
}

bool KStarsData::readTimeZoneRulebook( void ) {
	QFile file;
	QString id;

	if ( KSUtils::openDataFile( file, "TZrules.dat" ) ) {
		QTextStream stream( &file );

		while ( !stream.eof() ) {
			QString line = stream.readLine().stripWhiteSpace();
			if ( line.left(1) != "#" && line.length() ) { //ignore commented and blank lines
				QStringList fields = QStringList::split( " ", line );
				id = fields[0];
				QTime stime = QTime( fields[3].left( fields[3].find(':')).toInt() ,
								fields[3].mid( fields[3].find(':')+1, fields[3].length()).toInt() );
				QTime rtime = QTime( fields[6].left( fields[6].find(':')).toInt(),
								fields[6].mid( fields[6].find(':')+1, fields[6].length()).toInt() );

				Rulebook[ id ] = TimeZoneRule( fields[1], fields[2], stime, fields[4], fields[5], rtime );
			}
		}
		return true;
	} else {
		return false;
	}
}

bool KStarsData::readCityData( void ) {
	QFile file;
	bool citiesFound = false;

	if ( KSUtils::openDataFile( file, "Cities.dat" ) ) {
		QTextStream stream( &file );

  	while ( !stream.eof() ) {
			QString line = stream.readLine();
			citiesFound |= processCity( line );
		}	// while ( !stream.eof() )
		file.close();
	}	// if ( KSUtils::openDataFile() )

	//check for local cities database, but don't require it.
	file.setName( locateLocal( "appdata", "mycities.dat" ) ); //determine filename in local user KDE directory tree.
	if ( file.open( IO_ReadOnly ) ) {
		QTextStream stream( &file );

  	while ( !stream.eof() ) {
			QString line = stream.readLine();
			citiesFound |= processCity( line );
		}	// while ( !stream.eof() )
		file.close();
	}	// if ( fileopen() )

	return citiesFound;
}

// Save and restore options
void KStarsData::saveOptions()
{
	if ( 0 == oldOptions) {
		oldOptions = new KStarsOptions(false);
	}
	*oldOptions = *options;
}

void KStarsData::restoreOptions()
{
	if ( 0 == oldOptions ) {
		// this should not happen
		return;
	}
	*options = *oldOptions;
	delete oldOptions;
	oldOptions = 0;
}

void KStarsData::setMagnitude( float newMagnitude, bool forceReload ) {
/*
	* Only reload data if not loaded yet or if it is forced by parameter (checkDataPumpAction).
	*/
	if ( newMagnitude > maxSetMagnitude || forceReload ) {

		maxSetMagnitude = newMagnitude;  // store new highest magnitude level
		
		if ( !reloadingData() ) {  // if not allready reloading data
			// create source object
			source = new FileSource( this, "sao.dat" , newMagnitude );
			// create dataSink object
			loader = new StarDataSink( this );
			// make connections
			connect( loader, SIGNAL( done() ), this, SLOT( checkDataPumpAction() ) );
			connect( loader, SIGNAL( updateSkymap() ), this, SLOT( updateSkymap() ) );
			connect( loader, SIGNAL( clearCache() ), this, SLOT( sendClearCache() ) );
			// create QDataPump object and start reloading
			pump = new QDataPump ( source, (QDataSink *) loader );
		}
	}
// change current magnitude level in KStarsOptions
	options->setMagLimitDrawStar( newMagnitude );
	
}

void KStarsData::checkDataPumpAction() {
	bool reloadMoreData = false;  // default is false, just if new datas are needed it will set to true
	float sourceMag(0.0);  // init with 0.0 if source doesn't exist
	if ( source ) {  // if source exists
		sourceMag = source->magnitude();
		if ( sourceMag < maxSetMagnitude ) reloadMoreData = true;
		delete source;
		source = 0;
	}
	if ( pump ) {  // if pump exists
		delete pump;
		pump = 0;
	}
	if ( loader ) {  // if loader exists
		delete loader;
		loader = 0;
	}
/**
	*If magnitude was changed while reloading data start a new reload of data.
	*/
	if ( reloadMoreData ) {
		setMagnitude( maxSetMagnitude, true );
	}
	else
		if ( sourceMag <= 6.0 ) updateSkymap();  // update Skymap
}

bool KStarsData::reloadingData() {
	return ( pump || loader || source );  // true if variables != 0
}

void KStarsData::updateSkymap() {
	emit update();
}

void KStarsData::sendClearCache() {
	emit clearCache();
}

void KStarsData::addCatalog( QString name, QList<SkyObject> oList ) {
	CustomCatalogs[ name ] = oList;
	CustomCatalogs[ name ].setAutoDelete( TRUE );
}

void KStarsData::initialize() {
	if (inited) return;

	initTimer = new QTimer;
	QObject::connect(initTimer, SIGNAL(timeout()), this, SLOT( slotInitialize() ) );
	initTimer->start(1);
	initCounter = 0;
}

void KStarsData::initError(QString s, bool required = false) {
	QString message, caption;
	initTimer->stop();
	if (required) {
		message = i18n( "The file %1 could not be found.\n"
				"Shutting down, because KStars cannot run properly without this file.\n"
				"Place it in one of the following locations, then try again:\n\n"
				"\t$(KDEDIR)/share/apps/kstars/%1\n"
				"\t~/.kde/share/apps/kstars/%1\n"
				"\t[KSTARS_SOURCE_DIR]/data/%1").arg(s);
		caption = i18n( "Critical File Not Found: %1" ).arg(s);
	} else {
		message = i18n( "The file %1 could not be found.\n"
				"KStars can still run without this file, so will continue.\n"
				"However, to avoid seeing this message in the future, you may want to\n"
				"place the file in one of the following locations:\n\n"
				"\t$(KDEDIR)/share/apps/kstars/%1"
				"\n\t~/.kde/share/apps/kstars/%1"
				"\n\t[KSTARS_SOURCE_DIR]/data/%1").arg(s);
		caption = i18n( "Non-Critical File Not Found: %1" ).arg(s);
	}
	
	KMessageBox::information( 0, message, caption );

	if (required) {
		delete initTimer;
		emit initFinished(false);
	} else {
		initTimer->start(1);
	}

}

void KStarsData::slotInitialize() {
	QFile imFile;
	QString ImageName;

	kapp->flush(); // flush all paint events before loading data

	switch ( initCounter )
	{
		case 0: //Load Options//
			emit progressText( i18n("Loading Options") );

			if (objects==1) {
				// timezone rules
				if ( !readTimeZoneRulebook( ) )
					initError( "TZrules.dat", true );
			}

				kstars->loadOptions();
			break;

		case 1: //Load Cities//
			if (objects>1) break;

			emit progressText( i18n("Loading City Data") );

			if ( !readCityData( ) )
				initError( "Cities.dat", true );
			break;

		case 2: //Load SAO stellar database//

			emit progressText(i18n("Loading Star Data" ) );
			if ( !readStarData( ) )
				initError( "sao.dat", true );
			if (!readVARData())
				initError( "var.dat", false);
			if (!readADVTreeData())
				initError( "advinterface.dat", false);
			break;

		case 3: //Load NGC 2000 database//

			emit progressText( i18n("Loading NGC/IC Data" ) );
			if ( !readDeepSkyData( ) )
				initError( "ngcic.dat", true );
			break;

		case 4: //Load Constellation lines//

			emit progressText( i18n("Loading Constellations" ) );
			if ( !readCLineData( ) )
				initError( "clines.dat", true );
			break;

		case 5: //Load Constellation names//

			emit progressText( i18n("Loading Constellation Names" ) );
			if ( !readCNameData( ) )
				initError( cnameFile, true );
			break;

		case 6: //Load Milky Way//

			emit progressText( i18n("Loading Milky Way" ) );
			if ( !readMWData( ) )
				initError( "mw*.dat", true );
			break;

		case 7: //Start the Clock//

			emit progressText( i18n("Starting Clock" ) );
			setFullTimeUpdate();
			break;

		case 8: //Initialize the Planets//

			emit progressText( i18n("Creating Planets" ) );
			if (PC->initialize())
				PC->addObject( ObjNames );
			
			jmoons = new JupiterMoons();
			break;

		case 9: //Initialize the Moon//

			emit progressText( i18n("Creating Moon" ) );
			Moon = new KSMoon(kstars);
			ObjNames.append( Moon );
			Moon->loadData();
			break;

		case 10: //Load Image URLs//

			emit progressText( i18n("Loading Image URLs" ) );
			if ( !readURLData( "image_url.dat", 0 ) ) {
				initError( "image_url.dat", false );
			}
			if ( !readURLData( "myimage_url.dat", 0 ) ) {
			//Don't do anything if the local file is not found.
			}
          // doesn't belong here, only temp
             readUserLog();

			break;

		case 11: //Load Information URLs//

			emit progressText( i18n("Loading Information URLs" ) );
			if ( !readURLData( "info_url.dat", 1 ) ) {
				initError( "info_url.dat", false );
			}
			if ( !readURLData( "myinfo_url.dat", 1 ) ) {
			//Don't do anything if the local file is not found.
			}
			break;

		default:
			initTimer->stop();
			delete initTimer;
			inited = true;
			emit initFinished(true);
			break;
	} // switch ( initCounter )

	initCounter++;
}

void KStarsData::resetToNewDST(const GeoLocation *geo, const bool automaticDSTchange) {
	// reset tzrules data with local time, timezone offset and time direction (forward or backward)
	// force a DST change with option true for 3. parameter
	geo->tzrule()->reset_with_ltime( LTime, geo->TZ0(), TimeRunsForward, automaticDSTchange );
	// reset next DST change time
	setNextDSTChange( KSUtils::UTtoJulian( geo->tzrule()->nextDSTChange() ) );
	//reset LTime, because TZoffset has changed
	LTime = UTime.addSecs( int( 3600*geo->TZ() ) );
}

void KStarsData::updateTime( SimClock *clock, GeoLocation *geo, SkyMap *skymap, const bool automaticDSTchange ) {
	// sync times with clock
	UTime = clock->UTC();
	LTime = UTime.addSecs( int( 3600*geo->TZ() ) );
	CurrentDate = clock->JD();

	//Only check DST if (1) TZrule is not the empty rule, and (2) if we have crossed
	//the DST change date/time.
	if ( !geo->tzrule()->isEmptyRule() ) {
		if ( TimeRunsForward ) {
			// timedirection is forward
			// DST change happens if current date is bigger than next calculated dst change
			if ( CurrentDate > NextDSTChange ) resetToNewDST(geo, automaticDSTchange);
		} else {
			// timedirection is backward
			// DST change happens if current date is smaller than next calculated dst change
			if ( CurrentDate < NextDSTChange ) resetToNewDST(geo, automaticDSTchange);
		}
	}

	KSNumbers num(CurrentDate);

	bool needNewCoords = false;
	if ( fabs( CurrentDate - LastNumUpdate ) > 1.0 ) { 
		//update time-dependent variables once per day
		needNewCoords = true;
		LastNumUpdate = CurrentDate;
	}

	LST = KSUtils::UTtoLST( UTime, geo->lng() );
	LSTh->setH( LST.hour(), LST.minute(), LST.second() );

	// Update positions of objects, if necessary
	if ( fabs( CurrentDate - LastPlanetUpdate ) > 0.01 ) {
		LastPlanetUpdate = CurrentDate;

		PC->findPosition(&num);

		//Add a point to the planet trail if the centered object is a planet.
		if ( skymap->foundObject() == Moon || 
				PC->isPlanet( skymap->foundObject() ) ) {
			PlanetTrail.append( new SkyPoint(skymap->foundObject()->ra(), skymap->foundObject()->dec()) );
			
			//Allow no more than 500 points in the trail
			while ( PlanetTrail.count() > 500 ) 
				PlanetTrail.removeFirst();
		}

		//Recompute the Ecliptic
		if ( options->drawEcliptic ) {
			Ecliptic.clear();
			
			dms temp(0.0);
			for ( unsigned int i=0; i<Equator.count(); ++i ) {
				SkyPoint *o = new SkyPoint( 0.0, 0.0 );
				o->setFromEcliptic( num.obliquity(), Equator.at(i)->ra(), &temp );
				Ecliptic.append( o );
			}
		}
	}

	// Moon moves ~30 arcmin/hr, so update its position every minute.
	if ( fabs( CurrentDate - LastMoonUpdate ) > 0.00069444 ) {
		LastMoonUpdate = CurrentDate;
		if ( options->drawMoon ) {
			Moon->findPosition( &num, geo->lat(), LSTh );
			Moon->findPhase( PC->planetSun() );
		}
	
		//for now, update positions of Jupiter's moons here also
		jmoons->findPosition( &num, (const KSPlanet*)PC->findByName("Jupiter"), PC->planetSun() );
	}

	//Update Alt/Az coordinates.  Timescale varies with zoom level
	//If clock is in Manual Mode, always update. (?)
	if ( fabs( CurrentDate - LastSkyUpdate) > 0.25/skymap->getPixelScale() || clock->isManualMode() ) {
		LastSkyUpdate = CurrentDate;

		//Recompute Alt, Az coords for all objects
		//Planets
		PC->EquatorialToHorizontal( LSTh, geo->lat() );
		jmoons->EquatorialToHorizontal( LSTh, geo->lat() );
		if ( options->drawMoon ) Moon->EquatorialToHorizontal( LSTh, geo->lat() );

		//Planet Trails
		for( SkyPoint *p = PlanetTrail.first(); p; p = PlanetTrail.next() ) 
			p->EquatorialToHorizontal( LSTh, geo->lat() );
		
		//Stars
		if ( options->drawSAO ) {
			//REVERTED...remove comments after 1/1/2003
			//ARRAY:
			for ( StarObject *star = starList.first(); star; star = starList.next() ) {
			//StarObject *star;
			//for ( unsigned int i=0; i<StarCount; ++i ) {
			//	star = &starArray[i];
				
				if ( star->mag() > options->magLimitDrawStar ) break;
				if (needNewCoords) star->updateCoords( &num );
				star->EquatorialToHorizontal( LSTh, geo->lat() );
			}
		}

		//Deep-sky
		for ( SkyObject *o = deepSkyList.first(); o; o = deepSkyList.next() ) {
			bool update = false;
			if ( o->catalog()=="M" &&
					( options->drawMessier || options->drawMessImages ) ) update = true;
			else if ( o->catalog()== "NGC" && options->drawNGC ) update = true;
			else if ( o->catalog()== "IC" && options->drawIC ) update = true;
			else if ( options->drawOther ) update = true;

			if ( update ) {
				if (needNewCoords) o->updateCoords( &num );
				o->EquatorialToHorizontal( LSTh, geo->lat() );
			}
		}

		//Custom Catalogs
		for ( unsigned int j=0; j<options->CatalogCount; ++j ) {
			QList<SkyObject> cat = CustomCatalogs[ options->CatalogName[j] ];
			if ( options->drawCatalog[j] ) {
				for ( SkyObject *o = cat.first(); o; o = cat.next() ) {
					if (needNewCoords) o->updateCoords( &num );
					o->EquatorialToHorizontal( LSTh, geo->lat() );
				}
			}
		}

		//Milky Way
		if ( options->drawMilkyWay ) {
			for ( unsigned int j=0; j<11; ++j ) {
				for ( SkyPoint *p = MilkyWay[j].first(); p; p = MilkyWay[j].next() ) {
					if (needNewCoords) p->updateCoords( &num );
					p->EquatorialToHorizontal( LSTh, geo->lat() );
				}
			}
		}

		//CLines
		if ( options->drawConstellLines ) {
			for ( SkyPoint *p = clineList.first(); p; p = clineList.next() ) {
				if (needNewCoords) p->updateCoords( &num );
				p->EquatorialToHorizontal( LSTh, geo->lat() );
			}
		}

		//CNames
		if ( options->drawConstellNames ) {
			for ( SkyPoint *p = cnameList.first(); p; p = cnameList.next() ) {
				if (needNewCoords) p->updateCoords( &num );
				p->EquatorialToHorizontal( LSTh, geo->lat() );
			}
		}

		//Celestial Equator
		if ( options->drawEquator ) {
			for ( SkyPoint *p = Equator.first(); p; p = Equator.next() ) {
				p->EquatorialToHorizontal( LSTh, geo->lat() );
			}
		}

		//Ecliptic
		if ( options->drawEcliptic ) {
			for ( SkyPoint *p = Ecliptic.first(); p; p = Ecliptic.next() ) {
				p->EquatorialToHorizontal( LSTh, geo->lat() );
			}
		}

		//Horizon: different than the others; Alt & Az remain constant, RA, Dec must keep up
		if ( options->drawHorizon || options->drawGround ) {
			for ( SkyPoint *p = Horizon.first(); p; p = Horizon.next() ) {
				p->HorizontalToEquatorial( LSTh, geo->lat() );
			}
		}

		//Update focus
		if ( options->isTracking && skymap->foundObject() != NULL ) {
			if ( options->useAltAz ) {
				//Tracking any object in Alt/Az mode requires focus updates
				skymap->setDestinationAltAz(
						skymap->refract( skymap->foundObject()->alt(), true ).Degrees(),
						skymap->foundObject()->az()->Degrees() );
				skymap->destination()->HorizontalToEquatorial( LSTh, geo->lat() );
				skymap->setFocus( skymap->destination() );

			} else if (skymap->foundObject()==Moon || PC->isPlanet(skymap->foundObject()) ) {
				//Tracking on the Moon or Planet requires focus updates in both coord systems
				skymap->setDestination( skymap->foundObject() );
				skymap->setFocus( skymap->destination() );

			} else { //tracking non-solar system object in equatorial; update alt/az
				skymap->focus()->EquatorialToHorizontal( LSTh, geo->lat() );
			}
		} else if ( options->isTracking ) {
			if ( options->useAltAz ) {
				//Tracking on empty sky in Alt/Az mode
				skymap->setDestination( skymap->clickedPoint() );
				skymap->destination()->EquatorialToHorizontal( LSTh, geo->lat() );
				skymap->setFocus( skymap->destination() );
			}
		} else if ( ! skymap->isSlewing() ) {
			//Not tracking and not slewing, let sky drift by
			skymap->focus()->setRA( LSTh->Hours() - HourAngle->Hours() );
			skymap->setDestination( skymap->focus() );
			skymap->focus()->EquatorialToHorizontal( LSTh, geo->lat() );
			skymap->destination()->EquatorialToHorizontal( LSTh, geo->lat() );
		}

		skymap->setOldFocus( skymap->focus() );
		skymap->oldfocus()->EquatorialToHorizontal( LSTh, geo->lat() );

		if (clock->isManualMode() )
			QTimer::singleShot( 0, skymap, SLOT( UpdateNow() ) );
		else skymap->Update();
	}
}

void KStarsData::setTimeDirection( float scale ) {
	TimeRunsForward = ( scale < 0 ? false : true );
}

void KStarsData::setFullTimeUpdate() {
			LastSkyUpdate = -1000000.0;
			LastPlanetUpdate = -1000000.0;
			LastMoonUpdate   = -1000000.0;
			LastNumUpdate = -1000000.0;
}

#include "kstarsdata.moc"
