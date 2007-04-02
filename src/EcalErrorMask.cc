// $Id: EcalErrorMask.cc,v 1.17 2007/03/26 17:35:05 dellaric Exp $

/*!
  \file EcalErrorMask.cc
  \brief Error mask from text file or database
  \author B. Gobbo
  \version $Revision: 1.17 $
  \date $Date: 2007/03/26 17:35:05 $
*/

#include "DQM/EcalEndcapMonitorClient/interface/EcalErrorMask.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <regex.h>
#include <CondTools/Ecal/interface/EcalErrorDictionary.h>

bool EcalErrorMask::done_ = false;
int  EcalErrorMask::runNb_ = -1;
std::map<EcalLogicID, RunCrystalErrorsDat> EcalErrorMask::mapCrystalErrors_;
std::map<EcalLogicID, RunTTErrorsDat>      EcalErrorMask::mapTTErrors_;
std::map<EcalLogicID, RunPNErrorsDat>      EcalErrorMask::mapPNErrors_;
std::map<EcalLogicID, RunMemChErrorsDat>   EcalErrorMask::mapMemChErrors_;
std::map<EcalLogicID, RunMemTTErrorsDat>   EcalErrorMask::mapMemTTErrors_;

//---------------------------------------------------------------------------------------------

void EcalErrorMask::readFile( std::string inFile, bool verbose, bool verifySyntax ) throw( std::runtime_error ) {

  if( done_ ) {
      throw( std::runtime_error( "Input File already read." ) );
      return;
  }

  done_ = true;

  if( verifySyntax ) {
    std::cout << "----------------------------------------------------------------" << std::endl
	      << "---> Verifying syntax in " << inFile << std::endl
	      << "----------------------------------------------------------------" << std::endl;
  }

  const unsigned int lineSize = 512;
  char line[lineSize];

  std::fstream f( inFile.c_str(), std::ios::in );
  if( f.fail() ) {
    std::string s = "Error accessing input file " + inFile;
    throw( std::runtime_error( s ) );
    return;
  }

  int linecount = 0;
  int nerrors = 0;

  // Local copy of error dictionary
  std::vector<EcalErrorDictionary::errorDef_t> errors;
  EcalErrorDictionary::getDictionary( errors );

  if( verbose ) std::cout << std::endl
			  << "--------- Input Mask File Dump ----------"
			  << std::endl;

  while( f.getline( line, lineSize ) ) {

    linecount++;

    EcalErrorMask::clearComments_( line );
    EcalErrorMask::clearFinalBlanks_( line );

    std::istringstream is( line );
    std::string s;
    is >> s;
    if( s == "" ) continue;

    if( verbose ) std::cout << is.str() << std::endl;

    int sm; is >> sm;
    if( sm < 1 || sm > 36 ) {
      std::ostringstream os;
      os << "line " << linecount << " --> SM must be a number between 1 and 36: " << sm << std::ends;
      if( verifySyntax ) {
	std::cerr << os.str() << std::endl;
	nerrors++;
      }
      else {
	f.close();
	throw( std::runtime_error( os.str() ) );
	return;
      }
    }

    if( s == "Crystal" ) {
      int ic; is >> ic;
      if( ic < 1 || ic > 1700 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> IC must be a number between 1 and 1700: " << ic << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      std::string shortDesc; is >> shortDesc;
      uint64_t bitmask; bitmask = 0;

      for( unsigned int i=0; i<errors.size(); i++ ) {
	if( shortDesc == errors[i].shortDesc ) {
	  bitmask = errors[i].bitmask;
	}
      }
      if( bitmask == 0 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> This Short Description was not found in the Dictionary: " << shortDesc << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      if( !verifySyntax ) {
	EcalLogicID id = EcalLogicID( "EE_crystal_number", 1011000000+10000*sm+ic, sm, ic, 0 );
	std::map<EcalLogicID, RunCrystalErrorsDat>::iterator i = EcalErrorMask::mapCrystalErrors_.find( id );
	if( i != mapCrystalErrors_.end() ) {
	  uint64_t oldBitmask = (i->second).getErrorBits();
	  oldBitmask |= bitmask;
	  (i->second).setErrorBits( oldBitmask );
	}
	else {
	  RunCrystalErrorsDat error;
	  error.setErrorBits(bitmask);
	  EcalErrorMask::mapCrystalErrors_[ id ] = error;
	}
      }
    }
    else if( s == "TT" ) {
      int it; is >> it;
      if( it < 1 || it > 68 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> IT must be a number between 1 and 68: " << it << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      std::string shortDesc; is >> shortDesc;
      uint64_t bitmask; bitmask = 0;

      for( unsigned int i=0; i<errors.size(); i++ ) {
	if( shortDesc == errors[i].shortDesc ) {
	  bitmask = errors[i].bitmask;
	}
      }
      if( bitmask == 0 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> This Short Description was not found in the Dictionary:" << shortDesc << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      if( !verifySyntax ) {
	EcalLogicID id = EcalLogicID( "EE_trigger_tower", 1021000000+10000*sm+it, sm, it, 0 );
	std::map<EcalLogicID, RunTTErrorsDat>::iterator i = EcalErrorMask::mapTTErrors_.find( id );
	if( i != mapTTErrors_.end() ) {
	  uint64_t oldBitmask = (i->second).getErrorBits();
	  oldBitmask |= bitmask;
	  (i->second).setErrorBits( oldBitmask );
	}
	else {
	  RunTTErrorsDat error;
	  error.setErrorBits(bitmask);
	  EcalErrorMask::mapTTErrors_[ id ] = error;
	}
      }
    }
    else if( s == "PN" ) {
      int ic; is >> ic;
      if( ic < 1 || ic > 10 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> IC must be a number between 1 and 10: " << ic << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      std::string shortDesc; is >> shortDesc;
      uint64_t bitmask; bitmask = 0;

      for( unsigned int i=0; i<errors.size(); i++ ) {
	if( shortDesc == errors[i].shortDesc ) {
	  bitmask = errors[i].bitmask;
	}
      }
      if( bitmask == 0 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> This Short Description was not found in the Dictionary: " << shortDesc << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      if( !verifySyntax ) {
	EcalLogicID id = EcalLogicID( "EE_LM_PN", 1131000000+10000*sm+(ic-1), sm, (ic-1), 0 );
	std::map<EcalLogicID, RunPNErrorsDat>::iterator i = EcalErrorMask::mapPNErrors_.find( id );
	if( i != mapPNErrors_.end() ) {
	  uint64_t oldBitmask = (i->second).getErrorBits();
	  oldBitmask |= bitmask;
	  (i->second).setErrorBits( oldBitmask );
	}
	else {
	  RunPNErrorsDat error;
	  error.setErrorBits(bitmask);
	  EcalErrorMask::mapPNErrors_[ id ] = error;
	}
      }
    }
    else if( s == "MemCh" ) {
      int ic; is >> ic;
      if( ic < 1 || ic > 50 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> IC must be a number between 1 and 50: " << ic << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      std::string shortDesc; is >> shortDesc;
      uint64_t bitmask; bitmask = 0;

      for( unsigned int i=0; i<errors.size(); i++ ) {
	if( shortDesc == errors[i].shortDesc ) {
	  bitmask = errors[i].bitmask;
	}
      }
      if( bitmask == 0 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> This Short Description was not found in the Dictionary: " << shortDesc << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      if( !verifySyntax ) {
	EcalLogicID id = EcalLogicID( "EE_mem_channel", 1191000000+10000*sm+ic, sm, ic, 0 );
	std::map<EcalLogicID, RunMemChErrorsDat>::iterator i = EcalErrorMask::mapMemChErrors_.find( id );
	if( i != mapMemChErrors_.end() ) {
	  uint64_t oldBitmask = (i->second).getErrorBits();
	  oldBitmask |= bitmask;
	  (i->second).setErrorBits( oldBitmask );
	}
	else {
	  RunMemChErrorsDat error;
	  error.setErrorBits(bitmask);
	  EcalErrorMask::mapMemChErrors_[ id ] = error;
	}
      }
    }
    else if( s == "MemTT" ) {
      int it; is >> it;
      if( it < 69 || it > 70 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> IT must be 69 or 70: " << it << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      std::string shortDesc; is >> shortDesc;
      uint64_t bitmask; bitmask = 0;

      for( unsigned int i=0; i<errors.size(); i++ ) {
	if( shortDesc == errors[i].shortDesc ) {
	  bitmask = errors[i].bitmask;
	}
      }
      if( bitmask == 0 ) {
	std::ostringstream os;
	os << "line " << linecount << " --> This Short Description was not found in the Dictionary: " << shortDesc << std::ends;
	if( verifySyntax ) {
	  std::cerr << os.str() << std::endl;
	  nerrors++;
	}
	else {
	  f.close();
	  throw( std::runtime_error( os.str() ) );
	  return;
	}
      }
      if( !verifySyntax ) {
	EcalLogicID id = EcalLogicID( "EE_mem_TT", 1181000000+10000*sm+it, sm, it, 0 );
	std::map<EcalLogicID, RunMemTTErrorsDat>::iterator i = EcalErrorMask::mapMemTTErrors_.find( id );
	if( i != mapMemTTErrors_.end() ) {
	  uint64_t oldBitmask = (i->second).getErrorBits();
	  oldBitmask |= bitmask;
	  (i->second).setErrorBits( oldBitmask );
	}
	else {
	  RunMemTTErrorsDat error;
	  error.setErrorBits(bitmask);
	  EcalErrorMask::mapMemTTErrors_[ id ] = error;
	}
      }
    }
    else {
      std::ostringstream os;
      os << "line " << linecount << " --> Wrong Table Name: " << s << std::ends;
      if( verifySyntax ) {
	std::cerr << os.str() << std::endl;
	nerrors++;
      }
      else {
	f.close();
	throw( std::runtime_error( os.str() ) );
	return;
      }
    }
  }

  if( verifySyntax ) {
    std::cout << "----------------------------------------------------------------" << std::endl;
    if( nerrors > 0 ) {
      if( nerrors == 1 ) {
	std::cerr << "---> " << inFile << " contains a syntax error, please fix it..." << std::endl;
      }
      else {
	std::cerr << "---> " << inFile << " contains " << nerrors << " syntax errors, please fix them..." << std::endl;
      }
    }
    else {
      std::cout << "---> " << inFile << " syntax sounds correct... Good!" << std::endl;
    }
    std::cout << "----------------------------------------------------------------" << std::endl;
  }

  if( verbose ) std::cout << "------- End Input Mask File Dump --------"
			  << std::endl;

  f.close();
  return;

}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::writeFile( std::string outFile ) throw( std::runtime_error ) {

  std::ifstream inf( outFile.c_str() );
  inf.close();
  if( !inf.fail() ) {
    std::cout << std::endl;
    std::cout << "File ";
    std::cout << outFile << " already exists. Should I replace it? [y/n] ";
    std::string yesno; std::cin >> yesno;
    if( yesno == "n" ) {
      std::string s = outFile + " left unchanged.";
      throw( std::runtime_error( s ) );
      return;
    }
  }

  std::fstream f( outFile.c_str(), std::ios::out );
  if( f.fail() ) {
    std::string s = "Error accessing output file " + outFile;
    throw( std::runtime_error( s ) );
    return;
  }

  if( EcalErrorMask::runNb_ == -1 ) {
    f << "# -------------- Run Number Unknown --------------" << std::endl;
  }
  else {
    f << "# -------------- Run Number " << EcalErrorMask::runNb_ << " --------------" << std::endl;
  }

  f << "# Errors on Crystals masks" << std::endl;
  for( std::map<EcalLogicID, RunCrystalErrorsDat>::iterator i = EcalErrorMask::mapCrystalErrors_.begin();
       i != EcalErrorMask::mapCrystalErrors_.end(); i++ ) {
    string type = "Crystal";
    int sm = (i->first).getID1();
    int ic = (i->first).getID2();
    std::vector<EcalErrorDictionary::errorDef_t> errors;
    EcalErrorDictionary::getErrors( errors, (i->second).getErrorBits() );
    for( unsigned int j=0; j<errors.size(); j++ ) {
      f << type << " " << sm << " " << ic << " " << errors[j].shortDesc << std::endl;
    }
  }

  f << "# Errors on Trigger Towers masks" << std::endl;
  for( std::map<EcalLogicID, RunTTErrorsDat>::iterator i = EcalErrorMask::mapTTErrors_.begin();
       i != EcalErrorMask::mapTTErrors_.end(); i++ ) {
    string type = "TT";
    int sm = (i->first).getID1();
    int it = (i->first).getID2();
    std::vector<EcalErrorDictionary::errorDef_t> errors;
    EcalErrorDictionary::getErrors( errors, (i->second).getErrorBits() );
    for( unsigned int j=0; j<errors.size(); j++ ) {
      f << type << " " << sm << " " << it << " " << errors[j].shortDesc << std::endl;
    }
  }

  f << "# Errors on PN masks" << std::endl;
  for( std::map<EcalLogicID, RunPNErrorsDat>::iterator i = EcalErrorMask::mapPNErrors_.begin();
       i != EcalErrorMask::mapPNErrors_.end(); i++ ) {
    string type = "PN";
    int sm = (i->first).getID1();
    int ic = 1+(i->first).getID2();
    std::vector<EcalErrorDictionary::errorDef_t> errors;
    EcalErrorDictionary::getErrors( errors, (i->second).getErrorBits() );
    for( unsigned int j=0; j<errors.size(); j++ ) {
      f << type << " " << sm << " " << ic << " " << errors[j].shortDesc << std::endl;
    }
  }

  f << "# Errors on MemCh masks" << std::endl;
  for( std::map<EcalLogicID, RunMemChErrorsDat>::iterator i = EcalErrorMask::mapMemChErrors_.begin();
       i != EcalErrorMask::mapMemChErrors_.end(); i++ ) {
    string type = "MemCh";
    int sm = (i->first).getID1();
    int ic = (i->first).getID2();
    std::vector<EcalErrorDictionary::errorDef_t> errors;
    EcalErrorDictionary::getErrors( errors, (i->second).getErrorBits() );
    for( unsigned int j=0; j<errors.size(); j++ ) {
      f << type << " " << sm << " " << ic << " " << errors[j].shortDesc << std::endl;
    }
  }

  f << "# Errors on MemTT masks" << std::endl;
  for( std::map<EcalLogicID, RunMemTTErrorsDat>::iterator i = EcalErrorMask::mapMemTTErrors_.begin();
       i != EcalErrorMask::mapMemTTErrors_.end(); i++ ) {
    string type = "Crystal ";
    int sm = (i->first).getID1();
    int it = (i->first).getID2();
    std::vector<EcalErrorDictionary::errorDef_t> errors;
    EcalErrorDictionary::getErrors( errors, (i->second).getErrorBits() );
    for( unsigned int j=0; j<errors.size(); j++ ) {
      f << type << " " << sm << " " << it << " " << errors[j].shortDesc << std::endl;
    }
  }

  f.close();
}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::readDB( EcalCondDBInterface* eConn, RunIOV* runIOV ) throw( std::runtime_error ) {

  if( eConn ) {

    try {
      RunIOV validIOV;
      RunTag runTag = runIOV->getRunTag();
      eConn->fetchValidDataSet( &EcalErrorMask::mapCrystalErrors_, &validIOV, &runTag, runIOV->getRunNumber() );

      // use the IOV for CrystalErrors as reference
      EcalErrorMask::runNb_ = validIOV.getRunNumber();

      eConn->fetchValidDataSet( &EcalErrorMask::mapTTErrors_,      &validIOV, &runTag, runIOV->getRunNumber() );
      eConn->fetchValidDataSet( &EcalErrorMask::mapPNErrors_,      &validIOV, &runTag, runIOV->getRunNumber() );
      eConn->fetchValidDataSet( &EcalErrorMask::mapMemChErrors_,   &validIOV, &runTag, runIOV->getRunNumber() );
      eConn->fetchValidDataSet( &EcalErrorMask::mapMemTTErrors_,   &validIOV, &runTag, runIOV->getRunNumber() );
    } catch ( std::runtime_error & e ) {
      throw( std::runtime_error( e.what() ) );
    }

#if 0
    try {
      RunIOV validIOV;
      RunTag runTag = runIOV->getRunTag();
      string location = runTag.getLocationDef().getLocation();
      eConn->fetchValidDataSet( &EcalErrorMask::mapCrystalErrors_, &validIOV, location, runIOV->getRunNumber() );

      // use the IOV for CrystalErrors as reference
      EcalErrorMask::runNb_ = validIOV.getRunNumber();

      eConn->fetchValidDataSet( &EcalErrorMask::mapTTErrors_,      &validIOV, location, runIOV->getRunNumber() );
      eConn->fetchValidDataSet( &EcalErrorMask::mapPNErrors_,      &validIOV, location, runIOV->getRunNumber() );
      eConn->fetchValidDataSet( &EcalErrorMask::mapMemChErrors_,   &validIOV, location, runIOV->getRunNumber() );
      eConn->fetchValidDataSet( &EcalErrorMask::mapMemTTErrors_,   &validIOV, location, runIOV->getRunNumber() );
    } catch ( std::runtime_error & e ) {
      throw( std::runtime_error( e.what() ) );
    }
#endif

  }

}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::writeDB( EcalCondDBInterface* eConn, RunIOV* runIOV ) throw( std::runtime_error ) {

  if( eConn ) {

    try {
      if (EcalErrorMask::mapCrystalErrors_.size() != 0 )
        eConn->insertDataSet( &EcalErrorMask::mapCrystalErrors_, runIOV );
      if (EcalErrorMask::mapTTErrors_.size() != 0 )
        eConn->insertDataSet( &EcalErrorMask::mapTTErrors_,      runIOV );
      if (EcalErrorMask::mapPNErrors_.size() != 0 )
        eConn->insertDataSet( &EcalErrorMask::mapPNErrors_,      runIOV );
      if (EcalErrorMask::mapMemChErrors_.size() != 0 )
        eConn->insertDataSet( &EcalErrorMask::mapMemChErrors_,   runIOV );
      if (EcalErrorMask::mapMemTTErrors_.size() != 0 )
        eConn->insertDataSet( &EcalErrorMask::mapMemTTErrors_,   runIOV );
    } catch ( std::runtime_error & e ) {
      throw( std::runtime_error( e.what() ) );
    }

  }

}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::fetchDataSet( std::map< EcalLogicID, RunCrystalErrorsDat>* fillMap ) throw( std::runtime_error ) {

  //if( !done_ ) {
  //throw( std::runtime_error( "Input file not read" ) );
  //return;
  //}
  fillMap->clear();
  *fillMap = EcalErrorMask::mapCrystalErrors_;
  return;
}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::fetchDataSet( std::map< EcalLogicID, RunTTErrorsDat>* fillMap ) throw( std::runtime_error ) {

  //if( !done_ ) {
  //throw( std::runtime_error( "Input file not read" ) );
  //return;
  //}
  fillMap->clear();
  *fillMap = EcalErrorMask::mapTTErrors_;
  return;
}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::fetchDataSet( std::map< EcalLogicID, RunPNErrorsDat>* fillMap ) throw( std::runtime_error ) {

  //if( !done_ ) {
  //throw( std::runtime_error( "Input file not read" ) );
  //return;
  //}
  fillMap->clear();
  *fillMap = EcalErrorMask::mapPNErrors_;
  return;
}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::fetchDataSet( std::map< EcalLogicID, RunMemChErrorsDat>* fillMap ) throw( std::runtime_error ) {

  //if( !done_ ) {
  //throw( std::runtime_error( "Input file not read" ) );
  //return;
  //}
  fillMap->clear();
  *fillMap = EcalErrorMask::mapMemChErrors_;
  return;
}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::fetchDataSet( std::map< EcalLogicID, RunMemTTErrorsDat>* fillMap ) throw( std::runtime_error ) {

  //if( !done_ ) {
  //throw( std::runtime_error( "Input file not read" ) );
  //return;
  //}
  fillMap->clear();
  *fillMap = EcalErrorMask::mapMemTTErrors_;
  return;
}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::clearComments_( char* line ) {
  // It looks for "#" and replaces it with "\0"...
  regex_t rec;
  regmatch_t pmc;

  (void) regcomp( &rec, "#", REG_EXTENDED );

  int i = regexec( &rec, line, (size_t) 1, &pmc, 0 );

  if( i == 0 ) {
    line[pmc.rm_so] = '\0';
  }

  regfree( &rec );
}

//---------------------------------------------------------------------------------------------

void EcalErrorMask::clearFinalBlanks_( char* line ) {
  // From end of string, find last ' ' or '\t' (tab) and replace it with '\0'
  int i;
  for( i=strlen(line)-1; i>=0 && (line[i]==' '||line[i]=='\t'); i-- );
  if( line[i+1]  == ' ' || line[i+1] == '\t' ) line[i+1] = '\0';
}
