// Copyright (c) 2010-2011 Zipline Games, Inc. All Rights Reserved.
// http://getmoai.com

#ifndef MOAITOUCHSENSOR_H
#define MOAITOUCHSENSOR_H

#include <moai-sim/MOAISensor.h>

//================================================================//
// MOAITouchSensor
//================================================================//
/**	@name	MOAITouchSensor
	@text	Multitouch sensor. Tracks up to 16 simultaneous touches.

	@const	TOUCH_DOWN
	@const	TOUCH_MOVE
	@const	TOUCH_UP
	@const	TOUCH_CANCEL
*/
class MOAITouchSensor :
	public MOAISensor {
private:

	static const u32 MAX_TOUCHES		= 16;
	
	bool				mAcceptCancel;
	
	MOAILuaStrongRef	mCallback;
	
	bool			mDown[MAX_TOUCHES];
	
	//----------------------------------------------------------------//
	static int		_setAcceptCancel		( lua_State* L );
	static int		_setCallback			( lua_State* L );

	//----------------------------------------------------------------//
	void			Clear					();

public:

	enum {
		TOUCH_DOWN,
		TOUCH_MOVE,
		TOUCH_UP,
		TOUCH_CANCEL,
	};

	DECL_LUA_FACTORY ( MOAITouchSensor )

	//----------------------------------------------------------------//
	void			HandleEvent				( ZLStream& eventStream );
					MOAITouchSensor			();
					~MOAITouchSensor		();
	void			RegisterLuaClass		( MOAILuaState& state );
	void			RegisterLuaFuncs		( MOAILuaState& state );
	static void		WriteEvent				( ZLStream& eventStream, u32 touchID, bool down, float x, float y, float time );
	static void		WriteEventCancel		( ZLStream& eventStream );
};

#endif
