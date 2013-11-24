#include "SQLConnection.h"

#include <stdio.h>
#include <OS.h>

#define DEBUG


SQLiteConnection::SQLiteConnection( const char * filename )
:	m_sqlite(NULL),
	m_last_error(SQLITE_OK),
	m_table(NULL),
	m_rows(0),
	m_cols(0)
{
	int res = sqlite3_open(filename,&m_sqlite);
	
	if ( res != SQLITE_OK )
	{
		char error[1024];
		sprintf(error, "SQLite error opening database.");
//		sqlite3_freemem(errmsg);
		throw string(error);
	}
}

SQLiteConnection::~SQLiteConnection()
{
	Cleanup();
	
	if ( m_sqlite )
		sqlite3_close( m_sqlite );
}

void
SQLiteConnection::Cleanup()
{
	if ( m_table )
	{
		sqlite3_free_table(m_table);
		m_table = NULL;
	}
	
	m_rows = 0;
	m_cols = 0;
}

status_t
SQLiteConnection::Exec( string query )
{
	if ( !m_sqlite )
		throw string("SQLite error: Trying to Exec() with no connection");
	
	Cleanup();
	
	#ifdef DEBUG
	cout << "SQLiteConnection::Exec(" << query << ");" << endl;
	#endif
	
	char * error;
	
	m_last_error = sqlite3_get_table(
		m_sqlite,
		query.c_str(),
		&m_table,
		&m_rows,
		&m_cols,
		&error
	);
	
	#ifdef DEBUG
	cout << "   result: " << GetLastError() << endl;
	#endif
	
	if ( m_last_error == SQLITE_OK )
		return B_OK;
	else
		return B_ERROR;
}

string
SQLiteConnection::GetLastError()
{
	return sqlite3_errmsg(m_sqlite);
/*	switch ( m_last_error )
	{
		case SQLITE_OK:			return "Successful result";
		case SQLITE_ERROR:		return "SQL error or missing database";
		case SQLITE_INTERNAL:	return "An internal logic error in SQLite";
		case SQLITE_PERM:		return "Access permission denied";
		case SQLITE_ABORT:		return "Callback routine requested an abort";
		case SQLITE_BUSY:		return "The database file is locked";
		case SQLITE_LOCKED:		return "A table in the database is locked";
		case SQLITE_NOMEM:		return "A malloc() failed";
		case SQLITE_READONLY:	return "Attempt to write a readonly database";
		case SQLITE_INTERRUPT:	return "Operation terminated by sqlite_interrupt()";
		case SQLITE_IOERR:		return "Some kind of disk I/O error occurred";
		case SQLITE_CORRUPT:	return "The database disk image is malformed";
		case SQLITE_NOTFOUND:	return "(Internal Only) Table or record not found";
		case SQLITE_FULL:		return "Insertion failed because database is full";
		case SQLITE_CANTOPEN:	return "Unable to open the database file";
		case SQLITE_PROTOCOL:	return "Database lock protocol error";
		case SQLITE_EMPTY:		return "(Internal Only) Database table is empty";
		case SQLITE_SCHEMA:		return "The database schema changed";
		case SQLITE_TOOBIG:		return "Too much data for one row of a table";
		case SQLITE_CONSTRAINT:	return "Abort due to contraint violation";
		case SQLITE_MISMATCH:	return "Data type mismatch";
		case SQLITE_MISUSE:		return "Library used incorrectly";
		case SQLITE_NOLFS:		return "Uses OS features not supported on host";
		case SQLITE_AUTH:		return "Authorization denied";
		case SQLITE_FORMAT:		return "Auxiliary database format error";
		case SQLITE_ROW:		return "sqlite_step() has another row ready";
		case SQLITE_DONE:		return "sqlite_step() has finished executing";
		default:				return "Unknown SQLite error";
	}	
*/
}

int
SQLiteConnection::NumRows()
{
	return m_rows;
}

string
SQLiteConnection::GetValue( int row, int value )
{
	if ( row < 0 || value < 0 || row >= m_rows || value >= m_cols )
		throw "SQLite: Out of bounds error in GetValue(int,int)";
	
	return m_table[m_cols*(row+1)+value];
}

string
SQLiteConnection::GetValue( int row, string value )
{
	if ( m_cols == 0 )
		throw "SQLite: Out of bounds error in GetValue(int,string)";
		
	for ( int i=0; i<m_cols; i++ )
		if ( value == m_table[i] )
			return GetValue(row,i);
	
	throw "SQLite: Field not found in GetValue(int,string)";
}

void
SQLiteConnection::PrettyPrintResults()
{
	for ( int y=0; y<m_rows+1; y++ )
	{
		for ( int x=0; x<m_rows; x++ )
			cout << m_table[y*m_cols+x] << " ";
		cout << endl;
	}
}


