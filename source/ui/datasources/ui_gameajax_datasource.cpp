/*
Copyright (C) 2013 Victor Luchits

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
#include "ui_precompiled.h"
#include "kernel/ui_common.h"
#include "kernel/ui_utils.h"
#include "datasources/ui_gameajax_datasource.h"

#define GAMEAJAX_SOURCE	"gameajax"

using namespace Rocket::Core;
using namespace Rocket::Controls;

namespace WSWUI
{

typedef std::map<std::string, std::string> Row;

class Table
{
public:
	Table( const std::string &name ) : name( name ) {
	}

	~Table() {
		rows.clear();
	}

	std::string GetName( void ) const {
		return name;
	}

	size_t GetNumRows( void ) const {
		return rows.size();
	}

	void AddRow( const Row &row ) {
		rows.push_back( row );
	}

	void GetRocketRow( StringList &rocketRow, int row_index, const StringList& cols ) const {
		RowsList::const_iterator r_it = rows.begin();
		std::advance( r_it, row_index );
		if( r_it == rows.end() ) {;
			return;
		}

		const Row &row = *r_it;
		for( StringList::const_iterator it = cols.begin(); it != cols.end(); ++it ) {
			Row::const_iterator v = row.find( (*it).CString() );
			rocketRow.push_back( v == row.end() ? "" : v->second.c_str() );
		}
	}

	private:
		std::string name;
		typedef std::vector<Row> RowsList;
		RowsList rows;
};

class DynTable: public Table
{
public:
	DynTable( const std::string &name, unsigned int updateTime, const std::string &baseURL ) 
		: Table( name ), updateTime( updateTime ), baseURL( baseURL )
	{
	}

	unsigned int GetUpdateTime() const {
		return updateTime;
	}

	const std::string GetBaseURL() const {
		return baseURL;
	}

private:
	unsigned int updateTime;
	std::string baseURL;
};

// ============================================================================

GameAjaxDataSource::GameAjaxDataSource() : DataSource( GAMEAJAX_SOURCE )
{
}

GameAjaxDataSource::~GameAjaxDataSource( void )
{
	for( DynTableList::iterator it = tableList.begin(); it != tableList.end(); ++it ) {
		__delete__( it->second->table );
		__delete__( it->second );
	}
}

void GameAjaxDataSource::GetRow( StringList &row, const String &table, int row_index, const StringList& cols )
{
	DynTableList::const_iterator it = tableList.find( table.CString() );
	if( it == tableList.end() ) {
		return;
	}
	it->second->table->GetRocketRow( row, row_index, cols );
}

int GameAjaxDataSource::GetNumRows( const String &tableName )
{
	unsigned int now = trap::Milliseconds();

	char baseURL[1024];
	trap::GetBaseServerURL( baseURL, sizeof( baseURL ) );
	DynTable *table, *oldTable = NULL;

	DynTableList::iterator t_it = tableList.find( tableName.CString() );

	if( t_it != tableList.end() ) {
		oldTable = t_it->second->table;
		
		// return cached counter
		if( oldTable->GetBaseURL() == baseURL ) {
			if( oldTable->GetUpdateTime() + UPDATE_INTERVAL > now ) {
				return oldTable->GetNumRows();
			}
		}

		//tableList.erase( t_it );
	}
	
	// trigger AJAX-style query to server

	std::string stdTableName = tableName.CString();
	table = __new__( DynTable )( stdTableName, now, baseURL );
	tableList.insert({ stdTableName, __new__(DynTableFetcher)(table) });

	trap::Cmd_ExecuteText( EXEC_APPEND, va("ajax \"%s\"", stdTableName.c_str()) );

	return oldTable != NULL ? oldTable->GetNumRows() : 0;
}

void GameAjaxDataSource::FlushCache( void )
{
	// do nothing
}

void GameAjaxDataSource::AjaxResponse( const char *resource, const char *resp )
{
	
	DynTable *table = this->tableList[resource]->table;

	char *data = strdup(resp);
	std::string tableName = table->GetName();
	String rocketTableName = tableName.c_str();

	if ( !strncmp( resp, "DONE", 4 ) ) {
		this->NotifyRowAdd( rocketTableName, 0, table->GetNumRows() );
		return;
	}

	// parse server response:
	// key\value\key\value\key\value\key\value\ ...
	std::string key, value;

	char *pch = strtok( data, "\\" );
	Row row;
	while( pch != NULL ) {
		key = pch;
		pch = strtok( NULL, "\\" );
		if ( pch == NULL ) {
			break;
		}
		value = pch;
		pch = strtok( NULL, "\\" );
		row.insert({ key, value });
	}
	table->AddRow( row );

}

}
