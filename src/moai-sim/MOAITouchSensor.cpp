// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#include "pch.h"
#include <moai-sim/MOAITouchSensor.h>

//================================================================//
// lua
//================================================================//

//----------------------------------------------------------------//
/**	@name	setAcceptCancel
	@text	Sets whether or not to accept cancel events ( these happen on iOS backgrounding ), default value is false
 
	@in		MOAITouchSensor self
	@in		boolean accept	true then touch cancel events will be sent 
	@out	nil
*/
int MOAITouchSensor::_setAcceptCancel ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAITouchSensor, "UB" )
	
	self->mAcceptCancel = state.GetValue < bool >( 2, self->mAcceptCancel );
	
	return 0;
}

//----------------------------------------------------------------//
/**	@name	setCallback
	@text	Sets or clears the callback to be issued when the pointer location changes.

	@in		MOAITouchSensor self
	@opt	function callback		Default value is nil.
	@out	nil
*/
int MOAITouchSensor::_setCallback ( lua_State* L ) {
	MOAI_LUA_SETUP ( MOAITouchSensor, "U" )
	
	self->mCallback.SetRef ( state, 2 );
	
	return 0;
}

//================================================================//
// MOAITouchSensor
//================================================================//

//----------------------------------------------------------------//
void MOAITouchSensor::Clear () {

	for ( u32 i = 0; i < MAX_TOUCHES; ++i ) {
		this->mDown [ i ] = false;
	}
}

//----------------------------------------------------------------//
void MOAITouchSensor::HandleEvent ( ZLStream& eventStream ) {

	u32 eventType = eventStream.Read < u32 >( 0 );

	if ( eventType == TOUCH_CANCEL ) {
		
		this->Clear ();
		
		if ( this->mCallback && this->mAcceptCancel ) {
			MOAIScopedLuaState state = this->mCallback.GetSelf ();
			lua_pushnumber ( state, eventType );
			state.DebugCall ( 1, 0 );
		}
	}
	else {
		
		u32 touchID			= eventStream.Read < u32 >( 0 );
		float x				= eventStream.Read < float >( 0.0f );
		float y				= eventStream.Read < float >( 0.0f );
		
		if (touchID < MAX_TOUCHES)
		{
			if (eventType == TOUCH_DOWN)
			{
				if (this->mDown[touchID])
				{
					eventType = TOUCH_MOVE;
				}
				else
				{
					this->mDown[touchID] = true;
				}
			}
			else if (eventType == TOUCH_UP)
			{
				this->mDown[touchID] = false;
			}
			
			if (this->mCallback) {
				
				MOAIScopedLuaState state = this->mCallback.GetSelf ();
				lua_pushnumber ( state, eventType );
				lua_pushnumber ( state, idx );
				lua_pushnumber ( state, touch.mX );
				lua_pushnumber ( state, touch.mY );
				lua_pushnumber ( state, touch.mTapCount );
				state.DebugCall ( 5, 0 );
			}
		}
	}
}

//----------------------------------------------------------------//
MOAITouchSensor::MOAITouchSensor () {

	RTTI_SINGLE ( MOAISensor )
	
	mAcceptCancel = false;
	
	this->Clear ();
}

//----------------------------------------------------------------//
MOAITouchSensor::~MOAITouchSensor () {
}

//----------------------------------------------------------------//
void MOAITouchSensor::RegisterLuaClass ( MOAILuaState& state ) {

	MOAISensor::RegisterLuaClass ( state );

	state.SetField ( -1, "TOUCH_DOWN", ( u32 )TOUCH_DOWN );
	state.SetField ( -1, "TOUCH_MOVE", ( u32 )TOUCH_MOVE );
	state.SetField ( -1, "TOUCH_UP", ( u32 )TOUCH_UP );
	state.SetField ( -1, "TOUCH_CANCEL", ( u32 )TOUCH_CANCEL );
}

//----------------------------------------------------------------//
void MOAITouchSensor::RegisterLuaFuncs ( MOAILuaState& state ) {

	luaL_Reg regTable [] = {
		{ "setAcceptCancel",	_setAcceptCancel },
		{ "setCallback",		_setCallback },
		{ NULL, NULL }
	};

	luaL_register ( state, 0, regTable );
}

//----------------------------------------------------------------//
void MOAITouchSensor::WriteEvent ( ZLStream& eventStream, u32 touchID, bool down, float x, float y, float time ) {

	u32 eventType = down ? TOUCH_DOWN : TOUCH_UP;

	eventStream.Write < u32 >( eventType );
	eventStream.Write < u32 >( touchID );
	eventStream.Write < float >( x );
	eventStream.Write < float >( y );
}

//----------------------------------------------------------------//
void MOAITouchSensor::WriteEventCancel ( ZLStream& eventStream ) {

	eventStream.Write < u32 >( TOUCH_CANCEL );
}
