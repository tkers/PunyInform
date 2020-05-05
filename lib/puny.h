! Puny: implemented a small stdlib and parser for interactive fiction
! suitable for old-school computers such as the Commodore 64.
! Designed to be similar, but not identical, to Inform 6.

! Always use grammar version 2 which is easier to parse and more economical
! See: section 8.6 in https://www.inform-fiction.org/source/tm/TechMan.txt
Constant Grammar__Version = 2;
Constant INDIV_PROP_START 64;
Constant NULL         = $ffff;

!Constant WORDSIZE 2; ! set by the compiler from Inform 6.30

Constant ALL_WORD     = 'all';
Constant AND_WORD     = 'and';
Constant comma_word   = 'comma,';  ! An "untypeable word" used to substitute
                                   ! for commas in parse buffers

Attribute light;
Attribute supporter;
Attribute container;
Attribute enterable;
Attribute transparent;
Attribute open;
Attribute openable;
Attribute concealed;
Attribute moved;
Attribute visited alias moved;
Attribute proper;
Attribute scenery;
Attribute static;
Attribute animate;
Attribute worn;

! when you order a NPC, but the command isn't understood
! See: http://www.inform-fiction.org/manual/html/s18.html
Fake_Action Order; 
Fake_Action ThrownAt;
Fake_Action NotUnderstood; 

! Property name; ! This seems to be hardcoded in the Inform compiler
Property initial;
Property description;
Property short_name;
Property add_to_scope;
Property react_after;
Property react_before;
Property after;
Property before;
Property parse_name;
Property capacity;

! Daemons and timers

Property additive time_out NULL;
Property daemon alias time_out;
Property time_left;

! directions
Property n_to;
Property s_to;
Property e_to;
Property w_to;
#IfDef FULL_DIRECTIONS;
Property ne_to;
Property nw_to;
Property se_to;
Property sw_to;
#EndIf;
Property u_to;
Property d_to;
Property in_to;
Property out_to;

Property cant_go;

#IfDef FULL_DIRECTIONS;
Array abbr_directions_array table 'n//' 's//' 'e//' 'w//' 'ne' 'nw' 'se' 'sw' 'u//' 'd//' ',,' ',,';
Array full_directions_array table 'north' 'south' 'east' 'west' 'northeast' 'northwest' 'southeast' 'southwest' 'up' 'down' 'in' 'out';
Array direction_properties_array table n_to s_to e_to w_to ne_to nw_to se_to sw_to u_to d_to in_to out_to;
#IfNot;
Array abbr_directions_array table 'n//' 's//' 'e//' 'w//' 'u//' 'd//' ',,' ',,';
Array full_directions_array table 'north' 'south' 'east' 'west' 'up' 'down' 'in' 'out';
Array direction_properties_array table n_to s_to e_to w_to u_to d_to in_to out_to;
#EndIf;

! Header constants
Constant HEADER_DICTIONARY   = 4;    ! 2*4 = $8
Constant HEADER_STATIC_MEM   = 7;    ! 2*7 = $c

Constant TT_OBJECT           = 1;    ! one or more words referring to an object
                                     ! it is one of NOUN_TOKEN etc. below
Constant TT_PREPOSITION      = 2;    ! e.g. 'into'
Constant TT_ROUTINE_FILTER   = 3;    ! e.g. noun=CagedCreature
Constant TT_ATTR_FILTER      = 4;    ! e.g. edible
Constant TT_SCOPE            = 5;    ! e.g. scope=Spells
Constant TT_PARSE_ROUTIME    = 6;    ! a parse routine
Constant TT_END              = 15;   ! End of grammar line

Constant NOUN_TOKEN         = 0;    ! The elementary grammar tokens, and
Constant HELD_TOKEN         = 1;    ! the numbers compiled by Inform to
Constant MULTI_TOKEN        = 2;    ! encode them
Constant MULTIHELD_TOKEN    = 3;
Constant MULTIEXCEPT_TOKEN  = 4;
Constant MULTIINSIDE_TOKEN  = 5;
Constant CREATURE_TOKEN     = 6;
Constant SPECIAL_TOKEN      = 7;
Constant NUMBER_TOKEN       = 8;
Constant TOPIC_TOKEN        = 9;

! $42 = Single prep, $62 = Beginning of list of alternatives, $72 = middle of list, $52 = end of list
Constant TOKEN_SINGLE_PREP   = $42;
Constant TOKEN_FIRST_PREP    = $62;
Constant TOKEN_MIDDLE_PREP   = $72;
Constant TOKEN_LAST_PREP     = $52;

Constant GS_PLAYING          = 1;
Constant GS_QUIT             = 2;
Constant GS_DEAD             = 3;
Constant GS_WIN              = 4;
Constant GS_DEATHMESSAGE     = 5;

Constant FORM_CDEF           = 1;
Constant FORM_DEF            = 2;
Constant FORM_INDEF          = 3;

#IfV3;
Constant DICT_BYTES_FOR_WORD = 4;
#IfNot;
Constant DICT_BYTES_FOR_WORD = 6;
Constant HDR_SCREENHCHARS    = $20;
Constant HDR_SCREENWCHARS    = $21;
Constant MOVES__TX = "Moves: ";
Constant SCORE__TX = "Score: ";
#EndIf;

Default MAX_CARRIED         100;
Default SACK_OBJECT         0;

Global location = 1; ! Must be first global
Global status_field_1 = 0; ! Must be second global. Is used to show score or hours
Global status_field_2 = 0; ! Must be third global. Is used to show turns or minutes
Global score;
Global turns;
Global player;
Global actor;
Global action;
Global meta;      ! if the verb has the meta attribute or not
Global verb_word;
Global verb_wordnum;
Global noun;
Global second;
Global inp1;
Global inp2;
Global game_state;
Global wn;
Global scope_objects;
Global reverse;
Global keep_silent;
Global consult_from;  !TODO remove? ! Word that a "consult" topic starts on
Global consult_words; !TODO remove? ! ...and number of words in topic
#IfV5;
Global statusline_current_height = 0;
Global statusline_height     = 1;
Global statuswin_current     = false;
Global clr_on                = false;
Global clr_bg                = 2;
Global clr_fg                = 8;
Global clr_bgstatus          = 2;
Global clr_fgstatus          = 8;
#endif;

#ifdef DEBUG;
Global debug_flag            = 0;
#endif;

Constant MAX_INPUT_CHARS     = 78;
Constant MAX_INPUT_WORDS     = 20;

Constant WORD_HIGHBIT = $8000;
#Ifndef MAX_TIMERS;
Constant MAX_TIMERS  32;            ! Max number timers/daemons active at once
#Endif; ! MAX_TIMERS
Array  the_timers  --> MAX_TIMERS;
Global active_timers;               ! Number of timers/daemons active
Global current_timer;               ! Index of the timer which is currently being executed

Include "messages.h";

Array which_object-->10;      ! options for "which do you mean?"
Array multiple_objects-->32;  ! holds nouns when multi* grammar used

Array player_input_array->(MAX_INPUT_CHARS + 3);
Array parse_array->(2 + 4 * (MAX_INPUT_WORDS + 1)); ! + 1 to make room for an extra word which is set to 0

! extra arrays to be able to ask for additional info (do you mean X or Y?)
Array temp_player_input_array->(MAX_INPUT_CHARS + 3);
Array temp_parse_array->(2 + 4 * (MAX_INPUT_WORDS + 1)); 

Object Directions
	with
		selected_dir 0,
		selected_dir_prop 0,
#IfV5;
		parse_name [_len _i _w _arr;
#IfNot;
		parse_name [_len _i _w;
#EndIf;
			_w = (parse_array+4*wn-2)-->0;
			_len = abbr_directions_array-->0;
#IfV5;
			_arr = abbr_directions_array + 2;
			@scan_table _w _arr _len -> _i ?success;
			! not found in abbr, try full
			_arr = full_directions_array + 2;
			@scan_table _w _arr _len -> _i ?success;
			! no match
			self.selected_dir = 0;
			self.selected_dir_prop = 0;
			return 0;
.success;
			self.selected_dir = (_i - _arr)/2;
			self.selected_dir_prop = direction_properties_array --> (self.selected_dir + 1);
			return 1;
#IfNot;
			for(_i = 1 : _i <= _len : _i++) {
				if(_w == abbr_directions_array --> _i or full_directions_array --> _i) {
					self.selected_dir = _i;
					self.selected_dir_prop = direction_properties_array --> _i;
					return 1;
				}
			}
			! failure
			self.selected_dir = 0;
			self.selected_dir_prop = 0;
			return 0;
#EndIf;
		]
has scenery;

! ######################### Include utility files

#IfDef FLAG_COUNT;
Include "flags.h";
#EndIf;

Include "scope.h";
Include "grammar.h";

! ######################### Helper routines

[ PrintVerb p_v;
#IfV3;
	switch(p_v) {
		'examine': p_v = "examine";
	}
#EndIf;
	switch(p_v) {
		'i', 'inventory': p_v = "inventory";
		'l': p_v = "look";
		'z': p_v = "wait";
	}

	print (address) p_v;
];

[ GetVisibilityCeiling p_actor _parent;
	for(:: p_actor = _parent) {
		_parent = parent(p_actor);
		!   print "Examining ", p_actor, "(", (object) p_actor, ") whose parent is ", _parent, "(", (object) _parent, ")...^";
		if(_parent == 0 || (p_actor has container && p_actor hasnt transparent or open)) {
			return p_actor;
		}
	}
];

[ IndirectlyContains p_o1 p_o2;
	! Does o1 indirectly contain o2?  (Same as testing if o1 is one of the ancestors of o2.)
	while (p_o2 ~= 0) {
		if (p_o1 == p_o2) rtrue;
		!        if (p_o2 ofclass Class) rfalse;
		p_o2 = parent(p_o2);
	}
	rfalse;
];

[ CommonAncestor p_o1 p_o2 _i _j;
    ! Find the nearest object indirectly containing o1 and o2,
    ! or return 0 if there is no common ancestor.
    _i = p_o1;
    while (_i) {
        _j = p_o2;
        while (_j) {
            if (_j == _i) return _i;
            _j = parent(_j);
        }
        _i = parent(_i);
    }
    return 0;
];


#IfV5;

Array cursor_pos --> 2;

[ StatusLineHeight p_height;
	if (statusline_current_height ~= p_height) {
		@split_window p_height;
		statusline_current_height = p_height;
	}
];


[ MoveCursor line column;  ! 1-based postion on text grid
	if (~~statuswin_current) {
		@set_window 1;
		if (clr_on && clr_bgstatus > 1) {
			@set_colour clr_fgstatus clr_bgstatus;
		} else {
			style reverse;
		}
	}
	if (line == 0) {
		line = 1;
		column = 1;
	}
	@set_cursor line column;
	statuswin_current = true;
];

[ MainWindow;
	if (statuswin_current) {
		if (clr_on && clr_bgstatus > 1) {
			@set_colour clr_fg clr_bg;
		} else {
			style roman;
		}
		@set_window 0;
	}
	statuswin_current = false;
];

#IfNot; !IfV5

! [ MoveCursor line column;  ! 1-based postion on text grid
!   if (~~statuswin_current) {
!     @set_window 1;
!     style reverse;
!   }
!   if (line == 0) {
!     line = 1;
!     column = 1;
!   }
!   @set_cursor line column;
!   statuswin_current = true;
! ];

! [ MainWindow;
!   if (statuswin_current) {
!     style roman;
!     @set_window 0;
!   }
!   statuswin_current = false;
! ];
#EndIf;


#IfV3;
#IfNot;

Array TenSpaces -> "          ";

[ PrintSpacesOrMoveBack p_col p_space_before _current_col _go_to_col;
	@get_cursor cursor_pos;
	_current_col = cursor_pos --> 1;
	_go_to_col = p_col - p_space_before;

	if(_current_col > _go_to_col || cursor_pos --> 0 > 1) {
		MoveCursor(1, _go_to_col);
		_current_col = _go_to_col;
	}

	p_col = p_col - _current_col;
	while(p_col >= 10) {
		@print_table TenSpaces 10;
		p_col = p_col - 10;
	}
	@print_table TenSpaces p_col;
];

[ DrawStatusLine _width _visibility_ceiling;
	! For wide screens (67+ columns):
	! * print a space before room name, and "Score: xxx  Moves: xxxx" to the right.
	! * Room names up to 39 characters are never truncated.
	! For narrow screens:
	! * No space before room name
	! * Print "Score: xxx/yyyy", "xxx/yyyy", "xxx" or nothing, depending on screen width
	! * Room names up to 21 characters are never truncated. On a 40 column screen, room names up to 24 characters are never truncated.

	! If there is no player location, we shouldn't try to draw status window
	if (location == nothing || parent(player) == nothing)
		return;

	_width = HDR_SCREENWCHARS->0;

	StatusLineHeight(statusline_height);
	MoveCursor(1, 1); ! This also sets the upper window as active.
	if(_width > 66) @print_char ' ';

!     MoveCursor(1, 2);
!     if (location == thedark) {
!         print (name) location;
!     }
!     else {
!         FindVisibilityLevels();
!         if (visibility_ceiling == location)
	_visibility_ceiling = GetVisibilityCeiling(player);
! print (object) _visibility_ceiling;
	if (_visibility_ceiling == location) {
		PrintObjName(location);
!   print (name) _visibility_ceiling;
	} else {
		print (The) _visibility_ceiling;
	}

!     if (sys_statusline_flag && width > 53) {
!         MoveCursor(1, posa);
!         print (string) TIME__TX;
!         LanguageTimeOfDay(sline1, sline2);
!     }
!     else {
	if (_width > 24) {
		if (_width < 30) {
			! Width is 25-29, only print score as "0", no moves
			PrintSpacesOrMoveBack(_width - 2, 1);
			print status_field_1;
		} else {
			if (_width > 66) {
				! Width is 67-, print "Score: 0 Moves: 0"
				PrintSpacesOrMoveBack(_width - 26, 1);
				print (string) SCORE__TX, status_field_1;
				PrintSpacesOrMoveBack(_width - 13);
				print (string) MOVES__TX;
			} else {
				if (_width > 36) {
					! Width is 37-66, print "Score: 0/0"
					PrintSpacesOrMoveBack(_width - 14, 1);
					print (string) SCORE__TX;
				} else {
					! Width is 29-35, print "0/0"
					PrintSpacesOrMoveBack(_width - 7, 1);
				}
				print status_field_1;
				@print_char '/';
			}
			print status_field_2;
		}
	}
	! Regardless of what kind of status line we have printed, print spaces to the end.
	PrintSpacesOrMoveBack(_width + 1);
	MainWindow(); ! set_window
];
#Endif;

[ AtFullCapacity p_s _obj _k;
    if (p_s.&capacity == 0) rfalse; ! We will consider that no capacity specified implies infinite capacity.
    if (p_s == player) {
        objectloop (_obj in p_s)
            if (_obj hasnt worn) _k++;
    } else
        _k = children(p_s);
    if (_k < RunRoutines(p_s, capacity) || (p_s == player && RoomInSack())) rfalse;
];

[ RoomInSack _obj _ks;
    if (SACK_OBJECT && SACK_OBJECT in player) {
        for (_obj=youngest(player) : _obj : _obj=elder(_obj))
            if (_obj ~= SACK_OBJECT && _obj hasnt worn or light) {
                _ks = keep_silent;
                Keep_silent = 1;
                PerformAction(##Insert, _obj, SACK_OBJECT);
                keep_silent = _ks;
                if (keep_silent) return;
                if (_obj in SACK_OBJECT) {
                    print "(putting ", (the) _obj, " into ", (the) SACK_OBJECT, " to make room) ";
                    rtrue;
                }
            }
    }
    rfalse;
];

[ PrintObjName p_obj p_form _done;
	if(p_obj hasnt proper) {
		if(p_form == FORM_CDEF) {
			print "The ";
		} else if(p_form == FORM_DEF) {
			print "the ";
		} else if(p_form == FORM_INDEF) {
			print "a ";
		}
	}
	if(p_obj.short_name) {
		_done = PrintOrRun(p_obj, short_name, true);
	}
	if(_done == 0) {
		print (object) p_obj;
	}
];

[ PrintContents p_first_text p_last_text p_obj _obj _printed_first_text _printed_any_objects _last_obj;
!   print "Objectlooping...^";
	objectloop(_obj in p_obj) {
!   print "Considering ", (object) _obj, "...^";
!   if(_obj has concealed) print "Is concealed."; else print "Isnt concealed.";
		if(_obj hasnt concealed && (_obj has moved || _obj.initial == 0)) {
			if(_printed_first_text == 0) {
				print (string) p_first_text;
				_printed_first_text = 1;
			}
			! Push obj onto queue, printing the object that is shifted out, if any
			if(_last_obj) {
				if(_printed_any_objects) print ", ";
				PrintObjName(_last_obj, FORM_INDEF);
				_printed_any_objects = 1;
			}
			_last_obj = _obj;
		}
	}
	if(_last_obj) {
		if(_printed_any_objects) print " and ";
		PrintObjName(_last_obj, FORM_INDEF);
		print (string) p_last_text;
	}

!
!   for(_i = 0: _i < scope_objects: _i++) {
!     _obj = scope-->_i;
!     if(_obj ~= nothing) {
!       @new_line;
!       _text = _obj.initial;
!       if(_text) {
!         PrintOrRun(_text);
!       } else {
!         print "There is a ",(name) scope-->_i, " here. ";
!       }
!       @new_line;
!     }
!   }
	!print "(",_printed_first_text, ")";
	return _printed_first_text;
];

[ RunRoutines p_obj p_prop;
	if (p_obj.&p_prop == 0 && p_prop >= INDIV_PROP_START) rfalse;
	return p_obj.p_prop();
];


[ PrintOrRun p_obj p_prop p_no_string_newline;
	if (p_obj.#p_prop > WORDSIZE) return RunRoutines(p_obj,p_prop);
	if(p_obj.p_prop ofclass String) {
		print (string) p_obj.p_prop;
		if(p_no_string_newline == 0) @new_line;
		rtrue;
	} else if(p_obj.p_prop ofclass Routine) {
		return RunRoutines(p_obj, p_prop);
	}
];

[ PlayerTo p_loc _p;
	move Player to p_loc;
	location = p_loc;
	while(true) {
		_p = parent(location);
		if(_p == 0) rtrue;
		location = _p;
	}
];

Include "parser.h";

[ ActionPrimitive; indirect(#actions_table-->action); ];

[ PerformPreparedAction;
	!print "Performing action ", action, "^";
	sw__var = action;
	if ((BeforeRoutines() == false) && action < 4096) {
		ActionPrimitive();
	}
];

[ BeforeRoutines _i _obj;
	! react_before - Loops over the scope to find possible react_before routines
	! to run in each object, if it's found stop the action by returning true
	for(_i = 0: _i < scope_objects: _i++) {
		_obj = scope-->_i;
		if (_obj provides react_before) {
			if(PrintOrRun(_obj, react_before)) {
				rtrue;
			}
		}
	}
	if(location provides before && RunRoutines(location, before)) {
		rtrue;
	}
	if(inp1 provides before && RunRoutines(inp1, before)) {
		rtrue;
	}
	rfalse;
];

[ AfterRoutines _i _obj;
	! react_after - Loops over the scope to find possible react_before routines
	! to run in each object, if it's found stop the action by returning true
	for(_i = 0: _i < scope_objects: _i++) {
		_obj = scope-->_i;
		if (_obj provides react_after) {
			if(PrintOrRun(_obj, react_after)) {
				rtrue;
			}
		}
	}
	if(location provides after && RunRoutines(location, after)) {
		rtrue;
	}
	if(inp1 provides after && RunRoutines(inp1, after)) {
		rtrue;
	}
	rfalse;
];

[ RunLife p_actor p_reason;
	sw__var = p_reason;
    return RunRoutines(p_actor,life);
];

[ PerformAction p_action p_noun p_second _sa _sn _ss;
	_sa = action; _sn = noun; _ss = second;
	action = p_action; noun = p_noun; second = p_second;
	PerformPreparedAction();
	action = _sa; noun = _sn; second = _ss;
];

[ R_Process p_action p_noun p_second _s1 _s2;
	_s1 = inp1; _s2 = inp2;
	inp1 = p_noun; inp2 = p_second;
	PerformAction(p_action, p_noun, p_second);
	inp1 = _s1; inp2 = _s2;
];

[ CDefArt p_obj _result;
	_result = PrintObjName(p_obj, FORM_CDEF);
];

[ DefArt p_obj _result;
	_result = PrintObjName(p_obj, FORM_DEF);
];

[ IndefArt p_obj _result;
	_result = PrintObjName(p_obj, FORM_INDEF);
];

[ StartTimer p_obj p_timer;
	StartDaemon(p_obj, p_obj, p_timer);
];

[ StartDaemon p_obj p_array_val p_timer _i;
	if(p_array_val == 0)
		p_array_val = WORD_HIGHBIT + p_obj;
	for (_i=0 : _i<active_timers : _i++)
		if (the_timers-->_i == p_array_val) rfalse;
	_i = active_timers++;
	if (_i >= MAX_TIMERS) RunTimeError(ERR_TOO_MANY_TIMERS_DAEMONS);
	if (p_timer > 0) {
		if (p_obj.&time_left == 0) { RunTimeError(ERR_OBJECT_HASNT_PROPERTY); return; }
		else p_obj.time_left = p_timer;
	}
	the_timers-->_i = p_array_val;
];

[ StopTimer p_obj;
	StopDaemon(p_obj, p_obj);
];

[ StopDaemon p_obj p_array_val _i;
	if(p_array_val == 0)
		p_array_val = WORD_HIGHBIT + p_obj;
	for (_i=0 : _i<active_timers : _i++)
		if (the_timers-->_i == p_array_val) jump FoundTSlot4;
	rfalse;
.FoundTSlot4;
	if (p_obj == p_array_val) { ! This is a timer, not a daemon
		if (p_obj.&time_left == 0) { RunTimeError(ERR_OBJECT_HASNT_PROPERTY); return; }
		else p_obj.time_left = 0;
	}
	if(_i <= current_timer)
		current_timer--;
	for (_i=_i + 1: _i < active_timers : _i++)
		the_timers-->(_i - 1) = the_timers-->_i;
	active_timers--;
];

[ RunTimersAndDaemons _j _t;
	for (current_timer=0 : current_timer<active_timers : current_timer++) {
		if (game_state == GS_DEAD) return;
		_j = the_timers-->current_timer;
		if (_j < 0) RunRoutines(_j & ~WORD_HIGHBIT, daemon);
		else {
			_t = _j.time_left;
			if (_t == 0) {
				StopTimer(_j);
				RunRoutines(_j, time_out);
			} else {
				_t--;
				_j.time_left = _t;
			}
		}
	}
];

#IfV3;
! These routines are implemented by Veneer, but the default implementations give compile errors for z3

[ FindIndivPropValue p_obj p_property _x _prop_id;
	_x = p_obj.3;
	if (_x == 0) rfalse;
	!  print "Table for ", (object) obj, " is at ", (hex) x, "^";
	while (true) {
		_prop_id = _x-->0;
		if(_prop_id == 0) break;
		if(_prop_id & 32767 == p_property) break;
		! print (hex) x-->0, " (", (property) x-->0, ")  length ", x->2, ": ";
		!       for (n = 0: n< (x->2)/2: n++)
		!           print (hex) (x+3)-->n, " ";
		!       new_line;
		_x = _x + _x->2 + 3;
	}
	return _x;
];

[ RV__Pr p_object p_property _value _address;
	if(p_object == 0) {
		print "*Error: Tried to read property ";
		Print__PName(p_property);
		" of nothing.*";
	}
	if(p_object provides p_property) {
		if(p_property >= INDIV_PROP_START) {
			_address = FindIndivPropValue(p_object, p_property);
			return (_address + 3)-->0;
		}
		@get_prop p_object p_property -> _value;
		return _value;
	}
	print "[Error: Tried to read undefined property ";
	Print__PName(p_property);
	print " of ";
	PrintShortName(p_object);
	print "]";
	rfalse;
];

!      CA__Pr:  call, that is, print-or-run-or-read, a property:
!                      this exactly implements obj..prop(...).  Note that
!                      classes (members of Class) have 5 built-in properties
!                      inherited from Class: create, recreate, destroy,
!                      remaining and copy.  Implementing these here prevents
!                      the need for a full metaclass inheritance scheme.      */

[ CA__Pr obj id a    x z s s2 n m;
! print "CA_Pr obj = ", obj,", id = ", id,", a = ", a, "^";
	if (obj < 1 || obj > #largest_object-255) {
		switch(Z__Region(obj)) {
			2:
			if (id == call) {
				s = sender; sender = self; self = obj;
				#ifdef action;sw__var=action;#endif;
				x = indirect(obj, a);
				self = sender; sender = s; return x;
			}
			jump Call__Error;
			3:
			if (id == print) { @print_paddr obj; rtrue; }
			if (id == print_to_array) {
				@output_stream 3 a; @print_paddr obj; @output_stream -3;
				return a-->0;
			}
			jump Call__Error;
		}
		jump Call__Error;
	}
! print "CA_Pr(2) obj = ", obj,", id = ", id,", a = ", a, "^";
!   @check_arg_count 3 ?~A__x;y++;@check_arg_count 4 ?~A__x;y++;
!   @check_arg_count 5 ?~A__x;y++;@check_arg_count 6 ?~A__x;y++;
!   @check_arg_count 7 ?~A__x;y++;@check_arg_count 8 ?~A__x;y++;.A__x;
!   #ifdef INFIX;if (obj has infix__watching) n=1;#endif;
	#ifdef DEBUG;if (debug_flag & 1 ~= 0) n=1;#endif;
!   if (n==1) {
!     n=debug_flag & 1; debug_flag=debug_flag-n;
!     print "[ ~", (name) obj, "~.", (property) id, "(";
!     switch(y) {
!     1:
!       print a; 2: print a,",",b; 3: print a,",",b,",",c;
!     4:
!       print a,",",b,",",c,",",d;
!     5:
!       print a,",",b,",",c,",",d,",",e;
!     6:
!       print a,",",b,",",c,",",d,",",e,",",f;
!     }
!     print ") ]^"; debug_flag = debug_flag + n;
!   }
	if (id > 0 && id < INDIV_PROP_START) {
!   print "CA_Pr OK obj = ", obj,", id = ", id,", a = ", a, "^";
		x = obj.&id;
		if (x==0) {
			x=$000a-->0 + 2*(id-1); n=2;
		} else n = obj.#id;
	} else {
		if (id>=64 && id<69 && obj in Class) {
!     print "CA_Pr ERROR0 obj = ", obj,", id = ", id,", a = ", a, "^";
			RT__Err("Class create etc", obj, id); return;
!     return Cl__Ms(obj,id,y,a,b,c);
		}
!   print "CA_Pr(2.1) obj = ", obj,", id = ", id,", n = ", n, "^";
		x = obj..&id;
!   print "CA_Pr(2.2) obj = ", obj,", id = ", id,", x = ", x, "^";
		if (x == 0) {
!     print "CA_Pr ERROR1 obj = ", obj,", id = ", id,", a = ", a, "^";
			.Call__Error;
			RT__Err("send message", obj, id); return;
		}
!   print "Reading n at ", x-1,": ", 0->(x-1), "^";
		n = 0->(x-1);
		if (id&$C000==$4000)
		switch (n&$C0) { 0: n=1; $40: n=2; $80: n=n&$3F; }
	}
! print "CA_Pr(3) obj = ", obj,", id = ", id,", a = ", a, "^";
	for (:2*m<n:m++) {
!   print "Considering routine at ", x+2*m,": ", x-->m, "^";
		if (x-->m==$ffff) rfalse;
		switch(Z__Region(x-->m)) {
		2:
			s = sender; sender = self; self = obj; s2 = sw__var;
!       switch(y) {
!       0:
!         z = indirect(x-->m);
!       1:
			z = indirect(x-->m, a);
!       2:
!         z = indirect(x-->m, a, b);
!       3:
!       z = indirect(x-->m, a, b, c);
!       4:
!         z = indirect(x-->m, a, b, c, d);
!       5:
!         z = indirect(x-->m, a, b, c, d, e);
!       6:
!         z = indirect(x-->m, a, b, c, d, e, f);
!       }
			self = sender; sender = s; sw__var = s2;
			if (z ~= 0) return z;
		3:
			print_ret (string) x-->m;
		default:
			return x-->m;
		}
	}
	rfalse;
];

[ Cl__Ms;
	rfalse;
];

#EndIf;

Object DefaultPlayer "you"
	with
		name 'me',
		short_name  "yourself",
		description "As good-looking as ever.",
		before NULL,
		after NULL,
		life NULL,
		each_turn NULL,
		time_out NULL,
		describe NULL,
		add_to_scope 0,
		capacity MAX_CARRIED,
		!parse_name 0, ! TODO: uncommenting causes error (open box)
		orders 0,
		number 0,
		before_implicit NULL,
	has concealed animate proper transparent;

[ main _i _j _copylength _sentencelength _parsearraylength;
	print "PunyInform 0.0^^";

	player = DefaultPlayer;
	game_state = GS_PLAYING;
	Initialise();
	if(parent(player) == 0) PlayerTo(location);
	<Look>; ! Equivalent to PerformAction(##Look);

	while(game_state == GS_PLAYING) {
		status_field_1 = score;
		status_field_2 = turns;
		if(parse_array->1 == 0) {
			ReadPlayerInput(player_input_array, parse_array);
		}
		_sentencelength = ParseAndPerformAction();
		if(_sentencelength < 0) _sentencelength = -_sentencelength;
		else _sentencelength = parse_array->1;
#IfDef DEBUG;
		print "ParseAndPerformAction returned ", _sentencelength, "^";
#Endif;
		if(action >= 0 && meta == false) {
            RunTimersAndDaemons();
            turns++;
        }

		_parsearraylength = parse_array->1;
		if(_parsearraylength > _sentencelength) {
			! the first sentence in the input  has been parsed
			! and executed. Now remove it from parse_array so that
			! the next sentence can be parsed
#IfDef DEBUG;
			PrintParseArray(parse_array);
#Endif;
			_copylength = 2 * _parsearraylength + 1;
			for(_i = 1, _j = 2 * _sentencelength + 1: _j < _copylength: _i++, _j++)
				parse_array-->_i = parse_array-->_j;

			parse_array->1 = _parsearraylength - _sentencelength;
#IfDef DEBUG;
			PrintParseArray(parse_array);
			print "^";
#Endif;
		} else {
			! the input was just one sentence
			parse_array->1 = 0;
		}
	}
	if(game_state == GS_QUIT) @quit;
	if(game_state == GS_WIN) PrintMsg(MSG_YOU_HAVE_WON);
	else if(game_state == GS_DEAD) PrintMsg(MSG_YOU_HAVE_DIED);
	else if(game_state == GS_DEATHMESSAGE) DeathMessage();
	print ".^";
	for (::)
	{
		PrintMsg(MSG_RESTART_RESTORE_OR_QUIT);
#IFV3;
		read player_input_array parse_array;
#IFNOT;
		read player_input_array parse_array DrawStatusLine;
#ENDIF;
		_i=parse_array-->1;
		if (_i=='restart') @restart;
		if (_i=='restore') RestoreSub();
		if (_i=='quit') @quit;
	}
];

! todo: This has to be at the end of the file in case no DeathMessage was
! defined. Do we need to specify two includes (puny_begin.h and puny_end.h)?
#Stub DeathMessage      0;

