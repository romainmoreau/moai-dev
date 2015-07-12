// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#include "pch.h"
#include <moai-sim/MOAIGfxBuffer.h>
#include <moai-sim/MOAIGfxDevice.h>
#include <moai-sim/MOAIGfxResourceMgr.h>
#include <moai-sim/MOAIGrid.h>
#include <moai-sim/MOAIMesh.h>
#include <moai-sim/MOAIProp.h>
#include <moai-sim/MOAISelectionMesh.h>
#include <moai-sim/MOAIShader.h>
#include <moai-sim/MOAIShaderMgr.h>
#include <moai-sim/MOAITextureBase.h>
#include <moai-sim/MOAIVertexFormat.h>

//================================================================//
// local
//================================================================//

//----------------------------------------------------------------//
// TODO: doxygen
int MOAISelectionMesh::_addSelection ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAISelectionMesh, "UNNN" )

	u32 set			= state.GetValue < u32 >( 2, 1 ) - 1;
	u32 base		= state.GetValue < u32 >( 3, 1 ) - 1;
	u32 top			= state.GetValue < u32 >( 4, 0 ) + base;

	self->AddSelection ( set, base, top );

	return 0;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAISelectionMesh::_clearSelection ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAISelectionMesh, "UN" )

	u32 set			= state.GetValue < u32 >( 2, 1 ) - 1;
	
	self->ClearSelection ( set );

	return 0;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAISelectionMesh::_mergeSelections ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAISelectionMesh, "UNN" )

	u32 set			= state.GetValue < u32 >( 2, 1 ) - 1;
	u32 merge		= state.GetValue < u32 >( 3, 1 ) - 1;

	self->MergeSelections ( set, merge );

	return 0;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAISelectionMesh::_printSelection ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAISelectionMesh, "U" )

	if ( state.IsType ( 2, LUA_TNUMBER )) {
		self->PrintSelection ( state.GetValue < u32 >( 2, 1 ) - 1 );
	}
	else {
		self->PrintSelections ();
	}
	return 0;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAISelectionMesh::_reserveSelections ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAISelectionMesh, "U" )

	u32 total = state.GetValue < u32 >( 2, 0 );
	self->ReserveSelections ( total );

	return 0;
}

//----------------------------------------------------------------//
// TODO: doxygen
int MOAISelectionMesh::_setMesh ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAISelectionMesh, "U" )

	MOAIMesh* mesh = state.GetLuaObject < MOAIMesh >( 2, true );

	self->mDeck.Set ( *self, mesh );
	self->mMesh = mesh;

	return 0;
}

//================================================================//
// MOAISelectionMesh
//================================================================//

//----------------------------------------------------------------//
void MOAISelectionMesh::AddSelection ( u32 set, size_t base, size_t top ) {

	if ( set >= this->mSets.Size ()) return;

	MOAISelectionSpan* cursor = this->mSpanListHead;
	
	MOAISelectionSpan* prevInSet = 0;
	MOAISelectionSpan* prevAdjacentInSet = 0;
	MOAISelectionSpan* spanListTail = 0;
	
	for ( ; cursor && ( cursor->mTop < base ); cursor = cursor->mNextInMaster ) {
	
		spanListTail = cursor;
	
		if ( cursor->mSetID == set ) {
		
			prevInSet = cursor;
		
			if ( cursor->mTop == base ) {
				prevAdjacentInSet = cursor;
			}
		}
	}

	MOAISelectionSpan* span = 0;

	if ( cursor ) {
	
		if ( prevAdjacentInSet ) {
			cursor = prevAdjacentInSet;
			prevAdjacentInSet = 0;
		}
		else if ( cursor->mBase == base ) {
			this->ChangeSpanSet ( cursor, set );
		}
		
		if ( cursor->mSetID == set ) {
			
			if ( cursor->mTop <= top ) {
			
				MOAISelectionSpan* next = cursor->mNextInMaster;
				
				while ( next && ( next->mTop <= top )) {
				
					if ( cursor->mNext == next ) {
						cursor->mNext = next->mNext;
					}
					next = this->FreeSpanAndGetNext ( next );
				}
				
				if ( next && ( next->mSetID == set ) && ( next->mBase <= top )) {
					top = next->mTop;
					cursor->mNext = next->mNext;
					next = this->FreeSpanAndGetNext ( next );
				}
				
				cursor->mNextInMaster = next;
				cursor->mTop = top;
				
				if ( next ) {
					next->mBase = top;
				}
			}
		}
		else {
		
			if ( cursor->mTop <= top ) {
			
				cursor->mTop = base;
				MOAISelectionSpan* next = cursor->mNextInMaster;
				
				if ( next && ( next->mSetID == set )) {
					
					// next span is already in the set, so just move the boundary
					next->mBase = base;
				}
				else {
				
					// create a new span and add it
					span = this->AllocSpan ( set, base, top );
				
					span->mNextInMaster = cursor->mNextInMaster;
					cursor->mNextInMaster = span;
				}
			}
			else {
			
				// we need to split the span
				span = this->AllocSpan ( set, base, top );
				MOAISelectionSpan* cap = this->AllocSpan ( cursor->mSetID, top, cursor->mTop );
				
				cap->mNextInMaster = cursor->mNextInMaster;
				span->mNextInMaster = cap;
				cursor->mNextInMaster = span;
				
				cap->mNext = cursor->mNext;
				cursor->mNext = cap;
				
				cursor->mTop = base;
			}
		}
	}
	else {
	
		if ( prevAdjacentInSet ) {
			prevAdjacentInSet->mTop = top;
		}
		else {
	
			span = this->AllocSpan ( set, base, top );
			
			if ( spanListTail ) {
				spanListTail->mNextInMaster = span;
			}
			else {
				this->mSpanListHead = span;
			}
		}
	}
	
	// if there's a new span
	if ( span ) {
		if ( prevInSet ) {
			span->mNext = prevInSet->mNext;
			prevInSet->mNext = span;
		}
	}
}

//----------------------------------------------------------------//
MOAISelectionSpan* MOAISelectionMesh::AllocSpan ( u32 set, size_t base, size_t top ) {

	MOAISelectionSpan* span = this->mSpanPool.Alloc ();
	assert ( span );
	
	span->mSetID = set;
			
	span->mBase = base;
	span->mTop = top;
	
	span->mNextInMaster = 0;
	span->mNext = 0;
	
	if ( !this->mSets [ set ]) {
		this->mSets [ set ] = span;
	}
	return span;
}

//----------------------------------------------------------------//
void MOAISelectionMesh::ChangeSpanSet ( MOAISelectionSpan* span, u32 set ) {

	if ( span && span->mSetID != set ) {

		if ( this->mSets [ span->mSetID ] == span ) {
			this->mSets [ span->mSetID ] = span->mNext;
		}
		
		MOAIMeshSpan* firstInSet = this->mSets [ set ];
		
		if (( firstInSet && ( span->mBase <= firstInSet->mBase )) || !firstInSet ) {
			span->mNext = firstInSet;
			this->mSets [ set ] = span;
		}
		
		span->mSetID = set;
	}
}

//----------------------------------------------------------------//
void MOAISelectionMesh::Clear () {

	MOAISelectionSpan* next = this->mSpanListHead;
	while ( next ) {
		next = FreeSpanAndGetNext ( next );
	}
	
	this->mSets.Clear ();
	
	this->mSpanListHead = 0;
	//this->mSpanListTail = 0;
}

//----------------------------------------------------------------//
void MOAISelectionMesh::ClearSelection ( u32 set ) {
}

//----------------------------------------------------------------//
void MOAISelectionMesh::DrawIndex ( u32 idx, MOAIMaterialBatch& materials, ZLVec3D offset, ZLVec3D scale ) {
	UNUSED ( offset );
	UNUSED ( scale );
	
	if ( !this->mMesh ) return;
	
	size_t size = this->mSets.Size ();
	if ( !size ) return;

	idx = idx - 1;
	u32 itemIdx = idx % size;
	
	MOAIMeshSpan* span = this->mSets [ itemIdx ];
	if ( !span ) return;

	this->mMesh->DrawIndex ( idx, span, materials, offset, scale );
}

//----------------------------------------------------------------//
MOAISelectionSpan* MOAISelectionMesh::FreeSpanAndGetNext ( MOAISelectionSpan* span ) {

	MOAISelectionSpan* next = 0;

	if ( span ) {
	
		if ( this->mSets [ span->mSetID ] == span ) {
			this->mSets [ span->mSetID ] = span->mNext;
		}
		
		next = span->mNextInMaster;
		this->mSpanPool.Free ( span );
	}
	return next;
}

//----------------------------------------------------------------//
void MOAISelectionMesh::MergeSelections ( u32 set, u32 merge ) {
}

//----------------------------------------------------------------//
MOAISelectionMesh::MOAISelectionMesh () :
	mSpanListHead ( 0 ),
	mMesh ( 0 ) {

	RTTI_BEGIN
		RTTI_EXTEND ( MOAIDeckProxy )
	RTTI_END
}

//----------------------------------------------------------------//
MOAISelectionMesh::~MOAISelectionMesh () {
}

//----------------------------------------------------------------//
void MOAISelectionMesh::PrintSelection ( u32 idx ) {

	if ( idx < this->mSets.Size ()) {
	
		printf ( "set %d - ", idx );
	
		MOAISelectionSpan* span = ( MOAISelectionSpan* )this->mSets [ idx ];
		for ( ; span; span = ( MOAISelectionSpan* )span->mNext ) {
		
			printf ( "%d:[%d-%d]", span->mSetID + 1, span->mBase, span->mTop );
			
			if ( span->mNext ) {
				printf ( ", " );
			}
		}
		printf ( "\n" );
	}
}

//----------------------------------------------------------------//
void MOAISelectionMesh::PrintSelections () {

	MOAISelectionSpan* span = this->mSpanListHead;
	for ( ; span; span = span->mNextInMaster ) {
	
		printf ( "%d:[%d-%d]", span->mSetID + 1, span->mBase, span->mTop );
		
		if ( span->mNextInMaster ) {
			printf ( ", " );
		}
	}
	printf ( "\n" );
}

//----------------------------------------------------------------//
void MOAISelectionMesh::RegisterLuaClass ( MOAILuaState& state ) {

	MOAIDeckProxy::RegisterLuaClass ( state );
}

//----------------------------------------------------------------//
void MOAISelectionMesh::RegisterLuaFuncs ( MOAILuaState& state ) {

	MOAIDeckProxy::RegisterLuaFuncs ( state );

	luaL_Reg regTable [] = {
		{ "addSelection",			_addSelection },
		{ "clearSelection",			_clearSelection },
		{ "mergeSelections",		_mergeSelections },
		{ "printSelection",			_printSelection },
		{ "reserveSelections",		_reserveSelections },
		{ "setDeck",				_setMesh }, // override
		{ "setMesh",				_setMesh },
		{ NULL, NULL }
	};
	
	luaL_register ( state, 0, regTable );
}

//----------------------------------------------------------------//
void MOAISelectionMesh::ReserveSelections ( u32 total ) {

	this->Clear ();

	this->mSets.Init ( total );
	this->mSets.Fill ( 0 );
}

//----------------------------------------------------------------//
void MOAISelectionMesh::SerializeIn ( MOAILuaState& state, MOAIDeserializer& serializer ) {

	MOAIDeckProxy::SerializeIn ( state, serializer );
}

//----------------------------------------------------------------//
void MOAISelectionMesh::SerializeOut ( MOAILuaState& state, MOAISerializer& serializer ) {

	MOAIDeckProxy::SerializeOut ( state, serializer );
}
