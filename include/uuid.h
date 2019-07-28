
//UUID object management

#pragma once

//include "C:\Program Files (x86)\Windows Kits\8.1\Include\shared\rpcdce.h"
#include <Windows.h>
#include <Rpcdce.h>

//.................................
//.................................
class HE_UUID
{
	UUID id;
	bool bValid;

public:
	HE_UUID( UUID& in )
		:id( in )
		,bValid(true)//assume as is assighnment constructor
	{ }
	HE_UUID( std::string& in )
	{ bValid= SetID( in.c_str( ) );
	}
	HE_UUID( const char* in )
	{ bValid= SetID( in );
	}
	HE_UUID( ) :bValid(false) { ZeroMemory( &id, sizeof( UUID ) ); }

	bool IsValid()const{return bValid;}
	HE_UUID Generate( ) { UuidCreate( &id ); bValid= true; return *this; }
	void SetID( UUID* inId ) { id= *inId; }
	bool SetID( const char* psId )
	{
		UUID tid;
		if( UuidFromStringA( (RPC_CSTR)psId, &tid ) == RPC_S_OK )
		{
			id= tid;
			return bValid= true;
		}
		else
		{
			id={0,0,0,0};
			return bValid= false;
		}
	}
	operator std::string  ( ) const
	{
		//ASSERT( id.Data1 && id.Data2 );
		std::string strT;
        RPC_CSTR us = NULL;
		if( RPC_S_OK == UuidToStringA( &id, &us ) && us && *us )
		{
			if( id == GUID_NULL )
				strT= "GUID_NULL";
			else
				strT= (char*)us;
			RpcStringFreeA( &us );
		}
		return strT;
	}
	std::string string() const { return this->operator std::string(); }
	operator bool ( ) const //return false if uuid is null
	{
		RPC_STATUS stat;
		UUID temp= id;
		return ! UuidIsNil( &temp, &stat );
	}

	bool IsNull( ) const { return ! *this;  }

	bool operator < ( const HE_UUID& inuuid ) const
	{
		// comparison logic goes here
		return memcmp(&inuuid.id , &id, sizeof( UUID ) ) < 0;
	}

	bool operator == ( const HE_UUID& inuuid ) const
	{
		RPC_STATUS stat;
		UUID temp= id;
		UUID temp2= inuuid.id;
		return !! UuidEqual( &temp2, &temp, &stat );
	}
	operator UUID ( ) { return id; }
};

inline std::ostream& operator << ( std::ostream& s, const HE_UUID& y )
{
	s << std::string(y);
	return s;
}


//  ---------------------------------------------------------------------------
#pragma comment(lib, "rpcrt4")

/*
	HE_UUID testid;
	testid.Generate( );
	CString strTest( testid );
	HE_UUID testid2;
	testid2.SetID( strTest );
	CString strTest2( testid2 );
*/
