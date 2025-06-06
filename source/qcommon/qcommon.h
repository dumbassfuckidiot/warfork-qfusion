/*
Copyright (C) 1997-2001 Id Software, Inc.

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

// qcommon.h -- definitions common between client and server, but not game.dll

#ifndef __QCOMMON_H
#define __QCOMMON_H

#include "../gameshared/q_arch.h"
#include "../gameshared/q_math.h"
#include "../gameshared/q_shared.h"
#include "../gameshared/q_cvar.h"
#include "../gameshared/q_dynvar.h"
#include "../gameshared/q_comref.h"
#include "../gameshared/q_collision.h"

#include "qfiles.h"
#include "cmodel.h"
#include "version.h"
#include "bsp.h"
#include "l10n.h"

//#define	PARANOID			// speed sapping error checking

//============================================================================

struct mempool_s;

struct snapshot_s;

struct ginfo_s;
struct client_s;
struct cmodel_state_s;
struct client_entities_s;
struct fatvis_s;

//============================================================================

typedef struct
{
	uint8_t *data;
	size_t maxsize;
	size_t cursize;
	size_t readcount;
	bool compressed;
} msg_t;

// msg.c
void MSG_Init( msg_t *buf, uint8_t *data, size_t length );
void MSG_Clear( msg_t *buf );
void *MSG_GetSpace( msg_t *buf, size_t length );
void MSG_WriteData( msg_t *msg, const void *data, size_t length );
void MSG_CopyData( msg_t *buf, const void *data, size_t length );

//============================================================================

struct usercmd_s;
struct entity_state_s;

void MSG_WriteChar( msg_t *sb, int c );
void MSG_WriteByte( msg_t *sb, int c );
void MSG_WriteShort( msg_t *sb, int c );
void MSG_WriteInt3( msg_t *sb, int c );
void MSG_WriteLong( msg_t *sb, int c );
void MSG_WriteLongLong( msg_t *sb, long long c );
void MSG_WriteFloat( msg_t *sb, float f );
void MSG_WriteString( msg_t *sb, const char *s );
#define MSG_WriteCoord( sb, f ) ( MSG_WriteInt3( ( sb ), Q_rint( ( f*PM_VECTOR_SNAP ) ) ) )
#define MSG_WritePos( sb, pos ) ( MSG_WriteCoord( ( sb ), ( pos )[0] ), MSG_WriteCoord( sb, ( pos )[1] ), MSG_WriteCoord( sb, ( pos )[2] ) )
#define MSG_WriteAngle( sb, f ) ( MSG_WriteByte( ( sb ), ANGLE2BYTE( ( f ) ) ) )
#define MSG_WriteAngle16( sb, f ) ( MSG_WriteShort( ( sb ), ANGLE2SHORT( ( f ) ) ) )
void MSG_WriteDeltaUsercmd( msg_t *sb, struct usercmd_s *from, struct usercmd_s *cmd );
void MSG_WriteDeltaEntity( struct entity_state_s *from, struct entity_state_s *to, msg_t *msg, bool force, bool newentity );
void MSG_WriteDir( msg_t *sb, vec3_t vector );


void MSG_BeginReading( msg_t *sb );

// returns -1 if no more characters are available
int MSG_ReadChar( msg_t *msg );
int MSG_ReadByte( msg_t *msg );
int MSG_ReadShort( msg_t *sb );
int MSG_ReadInt3( msg_t *sb );
int MSG_ReadLong( msg_t *sb );
float MSG_ReadFloat( msg_t *sb );
char *MSG_ReadString( msg_t *sb );
char *MSG_ReadStringLine( msg_t *sb );
#define MSG_ReadCoord( sb ) ( (float)MSG_ReadInt3( ( sb ) )*( 1.0/PM_VECTOR_SNAP ) )
#define MSG_ReadPos( sb, pos ) ( ( pos )[0] = MSG_ReadCoord( ( sb ) ), ( pos )[1] = MSG_ReadCoord( ( sb ) ), ( pos )[2] = MSG_ReadCoord( ( sb ) ) )
#define MSG_ReadAngle( sb ) ( BYTE2ANGLE( MSG_ReadByte( ( sb ) ) ) )
#define MSG_ReadAngle16( sb ) ( SHORT2ANGLE( MSG_ReadShort( ( sb ) ) ) )
void MSG_ReadDeltaUsercmd( msg_t *sb, struct usercmd_s *from, struct usercmd_s *cmd );

int MSG_ReadEntityBits( msg_t *msg, unsigned *bits );
void MSG_ReadDeltaEntity( msg_t *msg, entity_state_t *from, entity_state_t *to, int number, unsigned bits );

void MSG_ReadDir( msg_t *sb, vec3_t vector );
void MSG_ReadData( msg_t *sb, void *buffer, size_t length );
int MSG_SkipData( msg_t *sb, size_t length );

//============================================================================

typedef struct purelist_s
{
	char *filename;
	unsigned checksum;
	struct purelist_s *next;
} purelist_t;

void Com_AddPakToPureList( purelist_t **purelist, const char *pakname, const unsigned checksum, struct mempool_s *mempool );
unsigned Com_CountPureListFiles( purelist_t *purelist );
purelist_t *Com_FindPakInPureList( purelist_t *purelist, const char *pakname );
void Com_FreePureList( purelist_t **purelist );

//============================================================================

#define SNAP_INVENTORY_LONGS			((MAX_ITEMS + 31) / 32)
#define SNAP_STATS_LONGS				((PS_MAX_STATS + 31) / 32)

#define SNAP_MAX_DEMO_META_DATA_SIZE	4*1024

void SNAP_ParseBaseline( msg_t *msg, entity_state_t *baselines );
void SNAP_SkipFrame( msg_t *msg, struct snapshot_s *header );
struct snapshot_s *SNAP_ParseFrame( msg_t *msg, struct snapshot_s *lastFrame, int *suppressCount, struct snapshot_s *backup, entity_state_t *baselines, int showNet );

void SNAP_WriteFrameSnapToClient( struct ginfo_s *gi, struct client_s *client, msg_t *msg, unsigned int frameNum, unsigned int gameTime,
								 entity_state_t *baselines, struct client_entities_s *client_entities,
								 int numcmds, gcommand_t *commands, const char *commandsData );

void SNAP_BuildClientFrameSnap( struct cmodel_state_s *cms, struct ginfo_s *gi, unsigned int frameNum, unsigned int timeStamp,
							   struct fatvis_s *fatvis, struct client_s *client, 
							   game_state_t *gameState, struct client_entities_s *client_entities,
							   bool relay, struct mempool_s *mempool );

void SNAP_FreeClientFrames( struct client_s *client );

void SNAP_RecordDemoMessage( int demofile, msg_t *msg, int offset );
int SNAP_ReadDemoMessage( int demofile, msg_t *msg );
void SNAP_BeginDemoRecording( int demofile, unsigned int spawncount, unsigned int snapFrameTime, 
								const char *sv_name, unsigned int sv_bitflags, purelist_t *purelist, 
								char *configstrings, entity_state_t *baselines );
void SNAP_StopDemoRecording( int demofile );
void SNAP_WriteDemoMetaData( const char *filename, const char *meta_data, size_t meta_data_realsize );
size_t SNAP_ClearDemoMeta( char *meta_data, size_t meta_data_max_size );
size_t SNAP_SetDemoMetaKeyValue( char *meta_data, size_t meta_data_max_size, size_t meta_data_realsize,
							  const char *key, const char *value );
size_t SNAP_ReadDemoMetaData( int demofile, char *meta_data, size_t meta_data_size );

//============================================================================

int COM_Argc( void );
const char *COM_Argv( int arg );  // range and null checked
void COM_ClearArgv( int arg );
int COM_CheckParm( char *parm );
void COM_AddParm( char *parm );

void COM_Init( void );
void COM_InitArgv( int argc, char **argv );

// some hax, because we want to save the file and line where the copy was called
// from, not the file and line from ZoneCopyString function
char *_ZoneCopyString( const char *str, const char *filename, int fileline );
#define ZoneCopyString( str ) _ZoneCopyString( str, __FILE__, __LINE__ )

char *_TempCopyString( const char *str, const char *filename, int fileline );
#define TempCopyString( str ) _TempCopyString( str, __FILE__, __LINE__ )

int Com_GlobMatch( const char *pattern, const char *text, const bool casecmp );

void Info_Print( char *s );

//============================================================================

/* crc.h */
void CRC_Init( unsigned short *crcvalue );
void CRC_ProcessByte( unsigned short *crcvalue, uint8_t data );
unsigned short CRC_Value( unsigned short crcvalue );
unsigned short CRC_Block( uint8_t *start, int count );

/*
==============================================================

PROTOCOL

==============================================================
*/

// protocol.h -- communications protocols

//=========================================

#define	PORT_MASTER			27950
#define	PORT_MASTER_STEAM	27011
#define	PORT_MASTER_WARFORK			27951
#define	PORT_SERVER			44400
#define	PORT_HTTP_SERVER	44444
#define PORT_TV_SERVER		44440
#define PORT_MATCHMAKER		46002
#define	NUM_BROADCAST_PORTS 5

//=========================================

#define	UPDATE_BACKUP	32  // copies of entity_state_t to keep buffered
                            // must be power of two

#define	UPDATE_MASK	( UPDATE_BACKUP-1 )

//==================
// the svc_strings[] array in snapshot.c should mirror this
//==================
extern const char * const svc_strings[256];
void _SHOWNET( msg_t *msg, const char *s, int shownet );

//
// server to client
//
enum svc_ops_e
{
	svc_bad,

	// the rest are private to the client and server
	svc_nop,
	svc_servercmd,          // [string] string
	svc_serverdata,         // [int] protocol ...
	svc_spawnbaseline,
	svc_download,           // [short] size [size bytes]
	svc_playerinfo,         // variable
	svc_packetentities,     // [...]
	svc_gamecommands,
	svc_match,
	svc_clcack,
	svc_servercs,			//tmp jalfixme : send reliable commands as unreliable
	svc_frame,
	svc_demoinfo,
	svc_voice,
	svc_extension			// for future expansion
};

//==============================================

//
// client to server
//
enum clc_ops_e
{
	clc_bad,
	clc_nop,
	clc_move,				// [[usercmd_t]
	clc_svcack,
	clc_clientcommand,      // [string] message
	clc_extension,
	clc_voice,
	clc_steamauth,
};

//==============================================

// serverdata flags
#define SV_BITFLAGS_PURE			( 1<<0 )
#define SV_BITFLAGS_RELIABLE		( 1<<1 )
#define SV_BITFLAGS_TVSERVER		( 1<<2 )
#define SV_BITFLAGS_HTTP			( 1<<3 )
#define SV_BITFLAGS_HTTP_BASEURL	( 1<<4 )

// framesnap flags
#define FRAMESNAP_FLAG_DELTA		( 1<<0 )
#define FRAMESNAP_FLAG_ALLENTITIES	( 1<<1 )
#define FRAMESNAP_FLAG_MULTIPOV		( 1<<2 )

// plyer_state_t communication

#define	PS_M_TYPE	    ( 1<<0 )
#define	PS_M_ORIGIN0	( 1<<1 )
#define	PS_M_ORIGIN1	( 1<<2 )
#define	PS_M_ORIGIN2	( 1<<3 )
#define	PS_M_VELOCITY0	( 1<<4 )
#define	PS_M_VELOCITY1	( 1<<5 )
#define	PS_M_VELOCITY2	( 1<<6 )
#define PS_MOREBITS1	( 1<<7 )

#define	PS_M_TIME	    ( 1<<8 )
#define	PS_EVENT	    ( 1<<9 )
#define	PS_EVENT2	    ( 1<<10 )
#define	PS_WEAPONSTATE	( 1<<11 )
#define PS_INVENTORY	( 1<<12 )
#define	PS_FOV		    ( 1<<13 )
#define	PS_VIEWANGLES	( 1<<14 )
#define PS_MOREBITS2	( 1<<15 )

#define	PS_POVNUM	    ( 1<<16 )
#define	PS_VIEWHEIGHT	( 1<<17 )
#define PS_PMOVESTATS	( 1<<18 )
#define	PS_M_FLAGS	    ( 1<<19 )
#define PS_PLRKEYS	    ( 1<<20 )
//...
#define PS_MOREBITS3	( 1<<23 )

#define	PS_M_GRAVITY	    ( 1<<24 )
#define	PS_M_DELTA_ANGLES0  ( 1<<25 )
#define	PS_M_DELTA_ANGLES1  ( 1<<26 )
#define	PS_M_DELTA_ANGLES2  ( 1<<27 )
#define	PS_PLAYERNUM	    ( 1<<28 )



//==============================================

// user_cmd_t communication

//#define	CMD_BACKUP		64	// allow a lot of command backups for very fast systems
//#define CMD_MASK		(CMD_BACKUP-1)

// ms and light always sent, the others are optional
#define	CM_ANGLE1   ( 1<<0 )
#define	CM_ANGLE2   ( 1<<1 )
#define	CM_ANGLE3   ( 1<<2 )
#define	CM_FORWARD  ( 1<<3 )
#define	CM_SIDE	    ( 1<<4 )
#define	CM_UP	    ( 1<<5 )
#define	CM_BUTTONS  ( 1<<6 )

//==============================================

// entity_state_t communication

// try to pack the common update flags into the first byte
#define	U_ORIGIN1	( 1<<0 )
#define	U_ORIGIN2	( 1<<1 )
#define	U_ORIGIN3	( 1<<2 )
#define	U_ANGLE1	( 1<<3 )
#define	U_ANGLE2	( 1<<4 )
#define	U_EVENT		( 1<<5 )
#define	U_REMOVE	( 1<<6 )      // REMOVE this entity, don't add it
#define	U_MOREBITS1	( 1<<7 )      // read one additional byte

// second byte
#define	U_NUMBER16	( 1<<8 )      // NUMBER8 is implicit if not set
#define	U_FRAME8	( 1<<9 )      // frame is a byte
#define	U_SVFLAGS	( 1<<10 )
#define	U_MODEL		( 1<<11 )
#define U_TYPE		( 1<<12 )
#define	U_OTHERORIGIN	( 1<<13 )     // FIXME: get rid of this
#define U_SKIN8		( 1<<14 )
#define	U_MOREBITS2	( 1<<15 )     // read one additional byte

// third byte
#define	U_EFFECTS8	( 1<<16 )     // autorotate, trails, etc
#define U_WEAPON	( 1<<17 )
#define	U_SOUND		( 1<<18 )
#define	U_MODEL2	( 1<<19 )     // weapons, flags, etc
#define U_LIGHT		( 1<<20 )
#define	U_SOLID		( 1<<21 )     // angles are short if bmodel (precise)
#define	U_EVENT2	( 1<<22 )
#define	U_MOREBITS3	( 1<<23 )     // read one additional byte

// fourth byte
#define	U_SKIN16	( 1<<24 )
#define	U_ANGLE3	( 1<<25 )     // for multiview, info need for culling
#define	U_ATTENUATION	( 1<<26 )
#define	U_EFFECTS16	( 1<<27 )
#define U_____UNUSED2	( 1<<28 )
#define	U_FRAME16	( 1<<29 )     // frame is a short
#define	U_TEAM		( 1<<30 )     // gameteam. Will rarely change

/*
==============================================================

Library

Dynamic library loading

==============================================================
*/

#ifdef __cplusplus
#define EXTERN_API_FUNC	   extern "C"
#else
#define EXTERN_API_FUNC	   extern
#endif

// qcommon/library.c
typedef struct { const char *name; void **funcPointer; } dllfunc_t;

void Com_UnloadLibrary( void **lib );
void *Com_LoadLibrary( const char *name, dllfunc_t *funcs ); // NULL-terminated array of functions
void *Com_LoadSysLibrary( const char *name, dllfunc_t *funcs ); // NULL-terminated array of functions
void *Com_LibraryProcAddress( void *lib, const char *name );

void *Com_LoadGameLibrary( const char *basename, const char *apifuncname, void **handle, void *parms,
                           bool pure, char *manifest );
void Com_UnloadGameLibrary( void **handle );

/*
==============================================================

CMD

Command text buffering and command execution

==============================================================
*/

/*

Any number of commands can be added in a frame, from several different sources.
Most commands come from either keybindings or console line input, but remote
servers can also send across commands and entire text files can be execed.

The + command line options are also added to the command buffer.
*/

void	    Cbuf_Init( void );
void	    Cbuf_Shutdown( void );
void	    Cbuf_AddText( const char *text );
void	    Cbuf_InsertText( const char *text );
void	    Cbuf_ExecuteText( int exec_when, const char *text );
void	    Cbuf_AddEarlyCommands( bool clear );
bool    Cbuf_AddLateCommands( void );
void	    Cbuf_Execute( void );


//===========================================================================

/*

Command execution takes a null terminated string, breaks it into tokens,
then searches for a command or variable that matches the first token.

*/


//typedef void ( *xcommand_t )( void );
//typedef char ** ( *xcompletionf_t )( const char *partial );
//
//void	    Cmd_AddCommand( const char *cmd_name, xcommand_t function );
//void	    Cmd_RemoveCommand( const char *cmd_name );
//bool    Cmd_Exists( const char *cmd_name );
//bool	Cmd_CheckForCommand( char *text );
//void	    Cmd_WriteAliases( int file );
//int			Cmd_CompleteAliasCountPossible( const char *partial );
//char		**Cmd_CompleteAliasBuildList( const char *partial );
//int			Cmd_CompleteCountPossible( const char *partial );
//char		**Cmd_CompleteBuildList( const char *partial );
//char		**Cmd_CompleteBuildArgList( const char *partial );
//char		**Cmd_CompleteBuildArgListExt( const char *command, const char *arguments );
//char		**Cmd_CompleteFileList( const char *partial, const char *basedir, const char *extension, bool subdirectories );
//int			Cmd_Argc( void );
//char		*Cmd_Argv( int arg );
//char		*Cmd_Args( void );
//void	    Cmd_TokenizeString( const char *text );
//void	    Cmd_ExecuteString( const char *text );
//void		Cmd_SetCompletionFunc( const char *cmd_name, xcompletionf_t completion_func );

#include "mod_cmd.h"
void	    Cmd_PreInit( void );
void	    Cmd_Init( void );
void	    Cmd_Shutdown( void );

/*
==============================================================

CVAR

==============================================================
*/

#include "cvar.h"

/*
==========================================================

DYNVAR

==========================================================
*/

#include "dynvar.h"

/*
==============================================================

NET

==============================================================
*/

// net.h -- quake's interface to the networking layer

#define	PACKET_HEADER			10          // two ints, and a short

#define	MAX_RELIABLE_COMMANDS	256 // max string commands buffered for restransmit
#define	MAX_PACKETLEN			1400        // max size of a network packet
#define	MAX_MSGLEN				32768       // max length of a message, which may be fragmented into multiple packets

// wsw: Medar: doubled the MSGLEN as a temporary solution for multiview on bigger servers
#define	FRAGMENT_SIZE			( MAX_PACKETLEN - 96 )
#define	FRAGMENT_LAST		(	 1<<14 )
#define	FRAGMENT_BIT			( 1<<31 )

typedef enum
{
	NA_NOTRANSMIT,      // wsw : jal : fakeclients
	NA_LOOPBACK,
	NA_IP,
	NA_IP6,
	NA_SDR,
} netadrtype_t;

typedef struct netadr_ipv4_s
{
	uint8_t ip [4];
	unsigned short port;
} netadr_ipv4_t;

typedef struct netadr_ipv6_s
{
	uint8_t ip [16];
	unsigned short port;
	unsigned long scope_id;
} netadr_ipv6_t;

typedef struct netadr_s
{
	netadrtype_t type;
	union
	{
		netadr_ipv4_t ipv4;
		netadr_ipv6_t ipv6;
		uint64_t steamid;
	} address;
} netadr_t;

typedef enum
{
	SOCKET_LOOPBACK,
	SOCKET_UDP,
	SOCKET_SDR
#ifdef TCP_SUPPORT
	, SOCKET_TCP
#endif
} socket_type_t;

typedef struct
{
	bool open;

	socket_type_t type;
	netadr_t address;
	bool server;

#ifdef TCP_SUPPORT
	bool connected;
#endif
	netadr_t remoteAddress;

	socket_handle_t handle;
} socket_t;

typedef enum
{
	CONNECTION_FAILED = -1,
	CONNECTION_INPROGRESS = 0,
	CONNECTION_SUCCEEDED = 1
} connection_status_t;

typedef enum
{
	NET_ERR_UNKNOWN = -1,
	NET_ERR_NONE = 0,

	NET_ERR_CONNRESET,
	NET_ERR_INPROGRESS,
	NET_ERR_MSGSIZE,
	NET_ERR_WOULDBLOCK,
	NET_ERR_UNSUPPORTED,
} net_error_t;

void	    NET_Init( void );
void	    NET_Shutdown( void );

bool		NET_OpenSocket( socket_t *socket, socket_type_t type, const netadr_t *address, bool server );
void	    NET_CloseSocket( socket_t *socket );

#ifdef TCP_SUPPORT
connection_status_t		NET_Connect( socket_t *socket, const netadr_t *address );
connection_status_t		NET_CheckConnect( socket_t *socket );
bool		NET_Listen( const socket_t *socket );
int			NET_Accept( const socket_t *socket, socket_t *newsocket, netadr_t *address );
#endif

int			NET_GetPacket( const socket_t *socket, netadr_t *address, msg_t *message );
bool		NET_SendPacket( const socket_t *socket, const void *data, size_t length, const netadr_t *address );

int			NET_Get( const socket_t *socket, netadr_t *address, void *data, size_t length );
int         NET_Send( const socket_t *socket, const void *data, size_t length, const netadr_t *address );
int64_t		NET_SendFile( const socket_t *socket, int file, size_t offset, size_t count, const netadr_t *address );

void	    NET_Sleep( int msec, socket_t *sockets[] );
int         NET_Monitor( int msec, socket_t *sockets[], 
				void (*read_cb)(socket_t *socket, void*), 
				void (*write_cb)(socket_t *socket, void*), 
				void (*exception_cb)(socket_t *socket, void*), void *privatep[] );
const char *NET_ErrorString( void );
void	    NET_SetErrorString( const char *format, ... );
void		NET_SetErrorStringFromLastError( const char *function );
void	    NET_ShowIP( void );
int			NET_SetSocketNoDelay( socket_t *socket, int nodelay );

const char *NET_SocketTypeToString( socket_type_t type );
const char *NET_SocketToString( const socket_t *socket );
char	   *NET_AddressToString( const netadr_t *address );
bool		NET_StringToAddress( const char *s, netadr_t *address );

unsigned short	NET_GetAddressPort( const netadr_t *address );
void			NET_SetAddressPort( netadr_t *address, unsigned short port );

bool    NET_CompareAddress( const netadr_t *a, const netadr_t *b );
bool    NET_CompareBaseAddress( const netadr_t *a, const netadr_t *b );
bool    NET_IsLANAddress( const netadr_t *address );
bool    NET_IsLocalAddress( const netadr_t *address );
bool    NET_IsAnyAddress( const netadr_t *address );
void	NET_InitAddress( netadr_t *address, netadrtype_t type );
void	NET_BroadcastAddress( netadr_t *address, int port );

//============================================================================

typedef struct
{
	const socket_t *socket;

	int dropped;                // between last packet and previous

	netadr_t remoteAddress;
	int game_port;              // game port value to write when transmitting

	// sequencing variables
	int incomingSequence;
	int incoming_acknowledged;
	int outgoingSequence;

	// incoming fragment assembly buffer
	int fragmentSequence;
	size_t fragmentLength;
	uint8_t fragmentBuffer[MAX_MSGLEN];

	// outgoing fragment buffer
	// we need to space out the sending of large fragmented messages
	bool unsentFragments;
	size_t unsentFragmentStart;
	size_t unsentLength;
	uint8_t unsentBuffer[MAX_MSGLEN];
	bool unsentIsCompressed;

	bool fatal_error;
} netchan_t;

extern netadr_t	net_from;


void Netchan_Init( void );
void Netchan_Shutdown( void );
void Netchan_Setup( netchan_t *chan, const socket_t *socket, const netadr_t *address, int qport );
bool Netchan_Process( netchan_t *chan, msg_t *msg );
bool Netchan_Transmit( netchan_t *chan, msg_t *msg );
bool Netchan_PushAllFragments( netchan_t *chan );
bool Netchan_TransmitNextFragment( netchan_t *chan );
int Netchan_CompressMessage( msg_t *msg );
int Netchan_DecompressMessage( msg_t *msg );
void Netchan_OutOfBand( const socket_t *socket, const netadr_t *address, size_t length, const uint8_t *data );
void Netchan_OutOfBandPrint( const socket_t *socket, const netadr_t *address, const char *format, ... );
int Netchan_GamePort( void );

/*
==============================================================

FILESYSTEM

==============================================================
*/

#define FS_NOTIFY_NEWPAKS	0x01

typedef void (*fs_read_cb)(int filenum, const void *buf, size_t numb, float progress, void *customp);
typedef void (*fs_done_cb)(int filenum, int status, void *customp);

void	    FS_Init( void );
int			FS_Rescan( void );
void	    FS_Frame( void );
void	    FS_Shutdown( void );

#include "mod_fs.h"

static const struct fs_import_s default_fs_imports_s = {
	.FS_GameDirectory = FS_GameDirectory,
	.FS_BaseGameDirectory = FS_BaseGameDirectory,
	.FS_WriteDirectory = FS_WriteDirectory,
	.FS_CacheDirectory = FS_CacheDirectory,
	.FS_SecureDirectory = FS_SecureDirectory,
	.FS_MediaDirectory = FS_MediaDirectory,
	.FS_DownloadsDirectory = FS_DownloadsDirectory,
	.FS_RuntimeDirectory = FS_RuntimeDirectory,
	.FS_GetGameDirectoryList = FS_GetGameDirectoryList,
	.FS_GetExplicitPurePakList = FS_GetExplicitPurePakList,
	.FS_IsExplicitPurePak = FS_IsExplicitPurePak,
	.FS_Read = FS_Read,
	.FS_Print = FS_Print,
	.FS_vPrintf = FS_vPrintf,
	.FS_Write = FS_Write,
	.FS_Tell = FS_Tell,
	.FS_Seek = FS_Seek,
	.FS_Eof = FS_Eof,
	.FS_Flush = FS_Flush,
	.FS_IsUrl = FS_IsUrl,
	.FS_FileNo = FS_FileNo,
	.FS_FOpenFile = FS_FOpenFile,
	.FS_FOpenFileGroup = FS_FOpenFileGroup,
	.FS_CreateAbsolutePath = FS_CreateAbsolutePath,
	.FS_AbsoluteNameForFile = FS_AbsoluteNameForFile,
	.FS_AbsoluteNameForBaseFile = FS_AbsoluteNameForBaseFile,
	.FS_AddExtraPK3Directory = FS_AddExtraPK3Directory,
	.FS_LoadFileExt = FS_LoadFileExt,
	.FS_LoadBaseFileExt = FS_LoadBaseFileExt,
	.FS_FreeFile = FS_FreeFile,
	.FS_FreeBaseFile = FS_FreeBaseFile,
	.FS_CopyFile = FS_CopyFile,
	.FS_CopyBaseFile = FS_CopyBaseFile,
	.FS_ExtractFile = FS_ExtractFile,
	.FS_MoveFile = FS_MoveFile,
	.FS_MoveBaseFile = FS_MoveBaseFile,
	.FS_MoveCacheFile = FS_MoveCacheFile,
	.FS_RemoveFile = FS_RemoveFile,
	.FS_RemoveBaseFile = FS_RemoveBaseFile,
	.FS_RemoveAbsoluteFile = FS_RemoveAbsoluteFile,
	.FS_RemoveDirectory = FS_RemoveDirectory,
	.FS_RemoveBaseDirectory = FS_RemoveBaseDirectory,
	.FS_RemoveAbsoluteDirectory = FS_RemoveAbsoluteDirectory,
	.FS_ChecksumAbsoluteFile = FS_ChecksumAbsoluteFile,
	.FS_ChecksumBaseFile = FS_ChecksumBaseFile,
	.FS_CheckPakExtension = FS_CheckPakExtension,
	.FS_PakFileExists = FS_PakFileExists,
	.FS_FileMTime = FS_FileMTime,
	.FS_BaseFileMTime = FS_BaseFileMTime,
	.FS_FirstExtension2 = FS_FirstExtension2,
	.FS_FirstExtension = FS_FirstExtension,
	.FS_PakNameForFile = FS_PakNameForFile,
	.FS_IsPureFile = FS_IsPureFile,
	.FS_FileManifest = FS_FileManifest,
	.FS_BaseNameForFile = FS_BaseNameForFile,
	.FS_GetFileList = FS_GetFileList,
	.FS_GetFileListExt = FS_GetFileListExt,
	.FS_IsPakValid = FS_IsPakValid,
	.FS_AddPurePak = FS_AddPurePak,
	.FS_RemovePurePaks = FS_RemovePurePaks,
	.FS_AddFileToMedia = FS_AddFileToMedia,
	.FS_FOpenAbsoluteFile = FS_FOpenAbsoluteFile,
	.FS_FCloseFile = FS_FCloseFile,
	.FS_FOpenBaseFile = FS_FOpenBaseFile,
};


bool		FS_SetGameDirectory( const char *dir, bool force );
int			FS_GetGameDirectoryList( char *buf, size_t bufsize );
int			FS_GetExplicitPurePakList( char ***paknames );
bool		FS_IsExplicitPurePak( const char *pakname, bool *wrongver );

/**
* Maps an existing file on disk for reading. 
* Does *not* work for compressed virtual files.
*
* @return mapped pointer to data on disk or NULL if mapping failed or passed size is 0.
*/
void	*FS_MMapBaseFile( int file, size_t size, size_t offset );
void	FS_UnMMapBaseFile( int file, void *data );

int		FS_GetNotifications( void );
int		FS_RemoveNotifications( int bitmask );

bool FS_SetGameDirectory( const char *dir, bool force );

/*
==============================================================

MISC

==============================================================
*/

#define MAX_PRINTMSG	3072

void	    Com_BeginRedirect( int target, char *buffer, int buffersize, 
				void ( *flush )(int, const char*, const void*), const void *extra );
void	    Com_EndRedirect( void );
void 	    Com_DeferConsoleLogReopen( void );
void	    Com_Printf( const char *format, ... );
void	    Com_DPrintf( const char *format, ... );
void	    Com_Error( com_error_code_t code, const char *format, ... );
void		Com_DeferQuit( void );
void	    Com_Quit( void );

int			Com_ClientState( void );        // this should have just been a cvar...
void	    Com_SetClientState( int state );

bool    Com_DemoPlaying( void );
void	    Com_SetDemoPlaying( bool state );

int			Com_ServerState( void );        // this should have just been a cvar...
void	    Com_SetServerState( int state );
struct cmodel_state_s *Com_ServerCM( unsigned *checksum );
void		Com_SetServerCM( struct cmodel_state_s *cms, unsigned checksum );

unsigned int Com_DaysSince1900( void );

extern cvar_t *developer;
extern cvar_t *dedicated;
extern cvar_t *autohelp;
extern cvar_t *host_speeds;
extern cvar_t *versioncvar;
extern cvar_t *revisioncvar;

// host_speeds times
extern unsigned int time_before_game;
extern unsigned int time_after_game;
extern unsigned int time_before_ref;
extern unsigned int time_after_ref;

/*
==============================================================

MEMORY MANAGEMENT

==============================================================
*/

struct mempool_s;
typedef struct mempool_s mempool_t;

#define MEMPOOL_TEMPORARY			1
#define MEMPOOL_GAMEPROGS			2
#define MEMPOOL_USERINTERFACE		4
#define MEMPOOL_CLIENTGAME			8
#define MEMPOOL_SOUND				16
#define MEMPOOL_DB					32
#define MEMPOOL_ANGELSCRIPT			64
#define MEMPOOL_CINMODULE			128
#define MEMPOOL_REFMODULE			256

void Memory_Init( void );
void Memory_InitCommands( void );
void Memory_Shutdown( void );
void Memory_ShutdownCommands( void );

void *_Mem_AllocExt( mempool_t *pool, size_t size, size_t aligment, int z, int musthave, int canthave, const char *filename, int fileline );
void *_Mem_Alloc( mempool_t *pool, size_t size, int musthave, int canthave, const char *filename, int fileline );
void *_Mem_Realloc( void *data, size_t size, const char *filename, int fileline );
void _Mem_Free( void *data, int musthave, int canthave, const char *filename, int fileline );
mempool_t *_Mem_AllocPool( mempool_t *parent, const char *name, int flags, const char *filename, int fileline );
mempool_t *_Mem_AllocTempPool( const char *name, const char *filename, int fileline );
void _Mem_FreePool( mempool_t **pool, int musthave, int canthave, const char *filename, int fileline );
void _Mem_EmptyPool( mempool_t *pool, int musthave, int canthave, const char *filename, int fileline );
char *_Mem_CopyString( mempool_t *pool, const char *in, const char *filename, int fileline );

void _Mem_CheckSentinelsGlobal( const char *filename, int fileline );

size_t Mem_PoolTotalSize( mempool_t *pool );

#define Mem_AllocExt( pool, size, z ) _Mem_AllocExt( pool, size, 0, z, 0, 0, __FILE__, __LINE__ )
#define Mem_Alloc( pool, size ) _Mem_Alloc( pool, size, 0, 0, __FILE__, __LINE__ )
#define Mem_Realloc( data, size ) _Mem_Realloc( data, size, __FILE__, __LINE__ )
#define Mem_Free( mem ) _Mem_Free( mem, 0, 0, __FILE__, __LINE__ )
#define Mem_AllocPool( parent, name ) _Mem_AllocPool( parent, name, 0, __FILE__, __LINE__ )
#define Mem_AllocTempPool( name ) _Mem_AllocTempPool( name, __FILE__, __LINE__ )
#define Mem_FreePool( pool ) _Mem_FreePool( pool, 0, 0, __FILE__, __LINE__ )
#define Mem_EmptyPool( pool ) _Mem_EmptyPool( pool, 0, 0, __FILE__, __LINE__ )
#define Mem_CopyString( pool, str ) _Mem_CopyString( pool, str, __FILE__, __LINE__ )

#define Mem_CheckSentinelsGlobal() _Mem_CheckSentinelsGlobal( __FILE__, __LINE__ )
#ifdef NDEBUG
#define Mem_DebugCheckSentinelsGlobal()
#else
#define Mem_DebugCheckSentinelsGlobal() _Mem_CheckSentinelsGlobal( __FILE__, __LINE__ )
#endif

// used for temporary allocations
extern mempool_t *tempMemPool;
extern mempool_t *zoneMemPool;

#define Mem_ZoneMallocExt( size, z ) Mem_AllocExt( zoneMemPool, size, z )
#define Mem_ZoneMalloc( size ) Mem_Alloc( zoneMemPool, size )
#define Mem_ZoneFree( data ) Mem_Free( data )

#define Mem_TempMallocExt( size, z ) Mem_AllocExt( tempMemPool, size, z )
#define Mem_TempMalloc( size ) Mem_Alloc( tempMemPool, size )
#define Mem_TempFree( data ) Mem_Free( data )

void Qcommon_InitCvarDescriptions( void );

void Qcommon_Init( int argc, char **argv );
void Qcommon_Frame( unsigned int realmsec );
void Qcommon_Shutdown( void );

/*
==============================================================

NON-PORTABLE SYSTEM SERVICES

==============================================================
*/

// directory searching
#define SFF_ARCH    0x01
#define SFF_HIDDEN  0x02
#define SFF_RDONLY  0x04
#define SFF_SUBDIR  0x08
#define SFF_SYSTEM  0x10

void	Sys_Init( void );
void	Sys_InitDynvars( void );

void	Sys_AppActivate( void );

unsigned int	Sys_Milliseconds( void );
uint64_t		Sys_Microseconds( void );
void		Sys_Sleep( unsigned int millis );

char	*Sys_ConsoleInput( void );
void	Sys_ConsoleOutput( char *string );
void	Sys_SendKeyEvents( void );
void	Sys_Error( const char *error, ... );
void	Sys_Quit( void );
char	*Sys_GetClipboardData( bool primary );
bool Sys_SetClipboardData( const char *data );
void	Sys_FreeClipboardData( char *data );
const char *Sys_GetPreferredLanguage( void );

bool	Sys_IsBrowserAvailable( void );
void	Sys_OpenURLInBrowser( const char *url );

void	*Sys_AcquireWakeLock( void );
void	Sys_ReleaseWakeLock( void *wl );

int 	Sys_GetCurrentProcessId( void );

/*
==============================================================

CLIENT / SERVER SYSTEMS

==============================================================
*/

void CL_Init( void );
void CL_InitDynvars( void );
void CL_Disconnect( const char *message );
void CL_Shutdown( void );
void CL_Frame( int realmsec, int gamemsec );
void CL_ParseServerMessage( msg_t *msg );
void CL_Netchan_Transmit( msg_t *msg );
void Con_Print( const char *text );
void SCR_BeginLoadingPlaque( void );

void SV_Init( void );
void SV_Shutdown( const char *finalmsg );
void SV_ShutdownGame( const char *finalmsg, bool reconnect );
void SV_Frame( int realmsec, int gamemsec );
bool SV_SendMessageToClient( struct client_s *client, msg_t *msg );
void SV_ParseClientMessage( struct client_s *client, msg_t *msg );

/*
==============================================================

ANTICHEAT

==============================================================
*/

#define ANTICHEAT_CLIENT	0x01
#define ANTICHEAT_SERVER	0x02

bool AC_LoadLibrary( void *imports, void *exports, unsigned int flags );

/*
==============================================================

WSW ANGEL SCRIPT SYSTEMS

==============================================================
*/

void Com_ScriptModule_Init( void );
void Com_ScriptModule_Shutdown( void );
struct angelwrap_api_s *Com_asGetAngelExport( void );

/*
==============================================================

MAPLIST SUBSYSTEM

==============================================================
*/
void ML_Init( void );
void ML_Shutdown( void );
void ML_Restart( bool forcemaps );
bool ML_Update( void );

const char *ML_GetFilenameExt( const char *fullname, bool recursive );
const char *ML_GetFilename( const char *fullname );
const char *ML_GetFullname( const char *filename );
size_t ML_GetMapByNum( int num, char *out, size_t size );

bool ML_FilenameExists( const char *filename );

bool ML_ValidateFilename( const char *filename );
bool ML_ValidateFullname( const char *fullname );

char **ML_CompleteBuildList( const char *partial );

/*
==============================================================

MULTITHREADING

==============================================================
*/
#include "qthreads.h"

#endif // __QCOMMON_H
