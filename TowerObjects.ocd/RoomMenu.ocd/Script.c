/**
	Room Menu
	Let's the player select the next room to attempt.
	
	@author Maikel
*/

// Background colors for hovering and bars and description.
static const ROOMMENU_BackgroundColor = 0x77000000;
static const ROOMMENU_HoverColor = 0x99888888;
static const ROOMMENU_BarColor = 0x99888888;


// Variables to keep track of whom is controlling the menu.
local menu, menu_id, menu_controller;


// Creates the room menu for the given player.
public func Create(int plr)
{
	// Safety: is a definition call.
	if (this != RoomMenu)
		return;	
	var menu_obj = CreateObject(RoomMenu, 0, 0, plr);
	menu_obj->OpenRoomMenu(plr, "rooms");
	return menu_obj;
}


/*-- Room Menu --*/

public func OpenRoomMenu(int plr, string menu_type)
{
	// Needs the cursor as command object.
	var clonk = GetCursor(plr);
	
	// If the menu is already open, don't open another instance.
	if (clonk->GetMenu() && clonk->GetMenu().ID == menu_id)
		return;
		
	// This object functions as menu target and for visibility.
	this.Visibility = VIS_Owner;
	menu_controller = clonk;
	
	// Make the room/credits/tablet menu.
	if (menu_type == "rooms")
		MakeRoomMenu(plr);
	else if (menu_type == "credits")
		MakeCreditsMenu(plr);
	else if	(menu_type == "tablets")
		MakeTabletsMenu(plr);
	
	// Open the menu and store the menu ID.
	menu_id = GuiOpen(menu);
	// Notify the clonk and set the menu.
	clonk->SetMenu(this);
	return;
}

public func MakeRoomMenu(int plr)
{
	// Room menu proplist.
	menu =
	{
		Target = this,
		Decoration = GUI_MenuDeco,
		Margin = ["1em", "2em"],
		BackgroundColor = {Std = ROOMMENU_BackgroundColor},
	};
	// The four basic panels for the menu.
	menu.roominfo = 
	{
		Target = this,
		ID = 1,
		Right = "70%-0.5em",
		Bottom = "40%-0.5em",
		Margin = ["0.1em"]
	};
	menu.buttons =
	{
		Target = this,
		ID = 2,
		Left = "70%",
		Bottom = "1.5em"
	};
	menu.roomsel = 
	{
		Target = this,
		ID = 3,
		Left = "70%",
		Top = "2em",
		Margin = ["0.1em"]
	};
	menu.selinfo = 
	{
		Target = this,
		ID = 4,
		Right = "70%-0.5em",
		Top = "40%",
		Margin = ["0.1em"]
	};
	// Make the borders between the submenus.
	menu.vert_border = CreateBarMenuEntry("70%-0.5em", "70%", nil, nil);
	menu.hor_border = CreateBarMenuEntry(nil, "70%-0.5em", "40%-0.5em", "40%");
	menu.bar_border = CreateBarMenuEntry("70%", nil, "1.5em", "2em");
	
	// Display info on current room.
	GetCurrentRoomInfo(menu.roominfo, plr);
	
	// Room selection: header.
	menu.roomsel.header = 
	{
		Target = this,
		ID = 31,
		Bottom = "1.5em",
		header_text = 
		{
			Target = this,
			ID = 32,
			Text = "$RoomMenuSelectRoom$",
			Style = GUI_TextVCenter
		}
	};
	// Buttons.
	menu.buttons.close = 
	{
		Target = this,
		ID = 21,
		Left = "100%-1.5em",
		Symbol = Icon_Cancel,
		Tooltip = "$RoomMenuClose$",
		BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
		OnMouseIn = GuiAction_SetTag("Hover"),
		OnMouseOut = GuiAction_SetTag("Std"),
		OnClick = GuiAction_Call(this, "CloseRoomMenu")
	};
	menu.buttons.credits = 
	{
		Target = this,
		ID = 22,
		Left = "100%-3em",
		Right = "100%-1.5em",
		Symbol = Icon_Wealth,
		Tooltip = "$RoomMenuCredits$",
		BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
		OnMouseIn = GuiAction_SetTag("Hover"),
		OnMouseOut = GuiAction_SetTag("Std"),
		OnClick = GuiAction_Call(this, "ShowRoomMenuCredits", plr)
	};
	menu.buttons.tablets = 
	{
		Target = this,
		ID = 23,
		Left = "100%-4.5em",
		Right = "100%-3em",
		Symbol = AncientTablet,
		Tooltip = "$RoomMenuTablets$",
		BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
		OnMouseIn = GuiAction_SetTag("Hover"),
		OnMouseOut = GuiAction_SetTag("Std"),
		OnClick = GuiAction_Call(this, "ShowRoomMenuTablets", plr)
	};
	
	// Room selection: add a list of available rooms.
	var rooms = 
	{
		Top = "1.5em",
		Style = GUI_VerticalLayout
	};
	menu.roomsel.rooms = MenuShowRooms(rooms, plr);
	return;
}

public func GetCurrentRoomInfo(proplist roominfo, int plr)
{
	// Get lobby or room text.
	var current_room = GetCurrentRoom();
	var room_info = "$RoomMenuInLobby$";
	if (current_room)
		room_info = Format("$RoomMenuInRoom$", current_room->GetRoomName());
	// Get rooms completed and jokers found.
	var nr_rooms =GetRoomCount();
	var completed = GetLength(GetPlayerCompletedRooms(plr));
	var total_jokers = GetRoomCount(true);
	var found_jokers = GetLength(GetPlayerFoundJokers(plr));
	var used_jokers = GetLength(GetPlayerUsedJokers(plr));
	room_info = Format("%s\n\n$RoomMenuRoomsCompleted$", room_info, completed - used_jokers, nr_rooms);
	room_info = Format("%s\n$RoomMenuJokersFound$", room_info, found_jokers, total_jokers, found_jokers - used_jokers);
	// Add the text to the menu.
	roominfo.text =
	{
		Text = room_info,
	};
	return;
}

public func MenuShowRooms(proplist rooms, int plr)
{
	// Show all the active rooms in this round.
	var room_list = SortRoomList(GetPlayerCompletedRooms(nil));
	var next_room = GetPlayerNextOpenRoom(nil);
	if (next_room)
		PushBack(room_list, next_room);
	SortRoomList(room_list);
	// Put all rooms into the selection menu.
	var cnt = 0;
	for (var room_id in room_list)
	{
		var room_completed = nil;
		if (HasPlayerCompletedRoom(plr, room_id))
		{
			room_completed = Icon_Ok;
			if (HasPlayerUsedJoker(plr, room_id))
				room_completed = Joker;
		}
		var room =
		{
			Target = this,
			ID = cnt + 1000,
			Priority = cnt,
			Bottom = "1.5em",
			BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
			OnClick = GuiAction_Call(this, "UpdateRoomSelectionInformation", {plr = plr, room_id = room_id}),
			symbol = 
			{
				Target = this,
				ID = cnt + 2000,
				Right = "1.5em",
				Symbol = room_id,
				completed = 
				{
					Target = this,
					ID = cnt + 3000,
					Left = "50%",
					Top = "50%",
					Symbol = room_completed				
				}
			},
			text = 
			{
				Target = this,
				ID = cnt + 4000,
				Left = "1.5em",
				Text = room_id->GetRoomName(),
				Style = GUI_TextVCenter
			}
		};
		MakeNumberMenuEntry(room.symbol, GetRoomNumber(room_id));
		rooms[Format("room%d", cnt)] = room;		
		cnt++;
	}
	return rooms;
}

public func MakeNumberMenuEntry(proplist parent, int number, int height)
{
	var hundreds = (number % 1000) / 100;
	var tens = (number % 100) / 10;
	var ones = number % 10;
	if (height == nil)
		height = 33;
	
	var separator = 0;
	if (hundreds)
	{
		parent.hundreds = 
		{
			Right = Format("%d%%", separator + 33),
			Bottom = Format("%d%%", height),
			Symbol = Icon_Number,
			GraphicsName = Format("%d", hundreds)
		};
		separator += 33;
	}
	if (tens)
	{
		parent.tens = 
		{
			Left = Format("%d%%", separator),
			Right = Format("%d%%", separator + 33),
			Bottom = Format("%d%%", height),
			Symbol = Icon_Number,
			GraphicsName = Format("%d", tens)
		};
		separator += 33;
	}
	parent.ones = 
	{
		Left = Format("%d%%", separator),
		Right = Format("%d%%", separator + 33),
		Bottom = Format("%d%%", height),
		Symbol = Icon_Number,
		GraphicsName = Format("%d", ones)
	};
	return;
}

public func UpdateRoomSelectionInformation(proplist pars)
{
	var plr = pars.plr;
	var room_id = pars.room_id;
	// Reset the menu selection info.
	if (menu.selinfo.room != nil)
	{
		GuiClose(menu_id, menu.selinfo.room.ID, menu.selinfo.room.Target);
		menu.selinfo.room = nil;
	}
	// If no room is specified stop here.
	if (room_id == nil)
		return;

	menu.selinfo.room_id = room_id;	
	var room_completed = nil;
	if (HasPlayerCompletedRoom(plr, room_id))
	{
		room_completed = Icon_Ok;
		if (HasPlayerUsedJoker(plr, room_id))
			room_completed = Joker;
	}
	menu.selinfo.room =
	{
		Target = this,
		ID = 41,
		icon =
		{
			Target = this,
			ID = 42,
			Right = "4em",
			Bottom = "4em",
			Symbol = room_id,
			completed = 
			{
				Target = this,
				ID = 43,
				Left = "50%",
				Top = "50%",
				Symbol = room_completed				
			}
		},
		name = 
		{
			Target = this,
			ID = 44,
			Left = "4em",
			Bottom = "1em",
			Style = GUI_TextVCenter,
			Text = Format("$RoomMenuInfoRoom$", room_id->GetRoomName())
		},
		author = 
		{
			Target = this,
			ID = 45,
			Left = "4em",
			Top = "1em",
			Bottom = "2em",
			Style = GUI_TextVCenter,
			Text = Format("$RoomMenuInfoAuthor$", room_id->GetRoomAuthor())
		},
		desc =
		{
			Target = this,
			ID = 46,
			Left = "4em",
			Top = "2em",
			Bottom = "4em",
			Text = Format("$RoomMenuInfoDescription$", room_id->GetRoomDescription())
		},
		options =
		{
			Target = this,
			ID = 47,
			Top = "4em",
			Style = GUI_VerticalLayout
		}
	};
	MakeNumberMenuEntry(menu.selinfo.room.icon, GetRoomNumber(room_id));
	
	// Add a play button.
	menu.selinfo.room.options.play = 
	{
		Target = this,
		ID = 48,
		Bottom = "2em",
		icon = 
		{
			Right = "2em",
			Symbol = Icon_Play,
			BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
			OnClick = GuiAction_Call(this, "OnRoomClickPlay", room_id),
		},
		text =
		{
			Left = "2em",
			Style = GUI_TextVCenter,
			Text = "$RoomMenuInfoPlayRoom$"				
		}
	};	

	// Add tablet information to the room if available.
	if (room_id->HasTablet())
	{
		var tablet_text = "$RoomMenuInfoRoomHasTablet$";
		var tablet_completed = nil;
		if (HasPlayerFoundTablet(plr, room_id))
		{
			tablet_text = "$RoomMenuInfoRoomFoundTablet$";
			tablet_completed = Icon_Ok;
		}
		menu.selinfo.room.options.tablet = 
		{
			Target = this,
			ID = 49,
			Bottom = "2em",
			icon = 
			{
				Right = "2em",
				Symbol = AncientTablet,
				completed = 
				{
					Left = "50%",
					Top = "50%",
					Symbol = tablet_completed				
				}
			},
			text =
			{
				Left = "2em",
				Style = GUI_TextVCenter,
				Text = tablet_text				
			}		
		};
	}
	
	// Add joker information to the room if available.
	if (room_id->HasJoker())
	{
		var joker_text = "$RoomMenuInfoRoomHasJoker$";
		var joker_completed = nil;
		if (HasPlayerFoundJoker(plr, room_id))
		{
			joker_text = "$RoomMenuInfoRoomFoundJoker$";
			joker_completed = Icon_Ok;
		}
		menu.selinfo.room.options.joker = 
		{
			Target = this,
			ID = 50,
			Bottom = "2em",
			icon = 
			{
				Right = "2em",
				Symbol = Joker,
				completed = 
				{
					Left = "50%",
					Top = "50%",
					Symbol = joker_completed				
				}
			},
			text =
			{
				Left = "2em",
				Style = GUI_TextVCenter,
				Text = joker_text				
			}		
		};
	}
	
	// Add skip button if jokers are available.
	if (GetPlayerAvailableJokers(plr) > 0 && !HasPlayerCompletedRoom(plr, room_id))
	{
		menu.selinfo.room.options.skip = 
		{
			Target = this,
			ID = 51,
			Bottom = "2em",
			icon = 
			{
				Right = "2em",
				Symbol = Joker,
				BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
				OnMouseIn = GuiAction_SetTag("Hover"),
				OnMouseOut = GuiAction_SetTag("Std"),
				OnClick = GuiAction_Call(this, "OnRoomClickSkip", {plr = plr, room_id = room_id}),
				completed = 
				{
					Left = "50%",
					Top = "50%",
					Symbol = Icon_Grab	
				}
			},
			text =
			{
				Left = "2em",
				Style = GUI_TextVCenter,
				Text = "$RoomMenuInfoSkipRoom$"				
			}		
		};
	}
	
	// Update menu.
	GuiUpdate(menu.selinfo, menu_id, menu.selinfo.ID, this);
	return;
}

public func OnRoomClickPlay(id room_id)
{
	CloseRoomMenu();
	LoadRoom(room_id);
	return;
}

public func OnRoomClickSkip(proplist pars)
{
	var plr = pars.plr;
	var room_id = pars.room_id;
	AddPlayerUsedJoker(plr, room_id);
	AddPlayerCompletedRoom(plr, room_id);
	// Save the progress.
	SavePlayerRoomData(plr);
	// Update selection.
	UpdateRoomSelectionInformation(pars);
	return;
}


/*-- Credits --*/

public func ShowRoomMenuCredits(int plr)
{
	CloseRoomMenu();
	var menu_obj = CreateObject(RoomMenu, 0, 0, plr);
	menu_obj->OpenRoomMenu(plr, "credits");
	return;
}

public func MakeCreditsMenu(int plr)
{
	// Room menu proplist.
	menu =
	{
		Target = this,
		Decoration = GUI_MenuDeco,
		Margin = ["1em", "2em"],
		BackgroundColor = {Std = ROOMMENU_BackgroundColor},
	};
	
	// The basic panels for the menu.
	menu.authordesc = 
	{
		Target = this,
		ID = 1,
		Right = "70%-0.5em",
		Bottom = "4em",
		Margin = ["0.1em"],
		Text = "$RoomMenuCreditsText$"
	};
	menu.authorlist = 
	{
		Target = this,
		ID = 2,
		Top = "4em",
		Right = "70%-0.5em",
		Style = GUI_VerticalLayout
	};
	menu.buttons =
	{
		Target = this,
		ID = 3,
		Left = "70%",
		Bottom = "1.5em"
	};
	menu.authorinfo = 
	{
		Target = this,
		ID = 4,
		Left = "70%",
		Top = "1.5em",
		Margin = ["0.1em"]
	};
	
	// Create the list of authors.
	menu.authorlist = MenuShowRoomAuthors(menu.authorlist);
	
	// Room selection: header with close button.
	menu.buttons.close = 
	{
		Target = this,
		ID = 31,
		Left = "100%-1.5em",
		Symbol = Icon_Cancel,
		Tooltip = "$RoomMenuClose$",
		BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
		OnMouseIn = GuiAction_SetTag("Hover"),
		OnMouseOut = GuiAction_SetTag("Std"),
		OnClick = GuiAction_Call(this, "CloseRoomMenu")
	};
	menu.buttons.rooms = 
	{
		Target = this,
		ID = 32,
		Left = "100%-3em",
		Right = "100%-1.5em",
		Symbol = Icon_Enter,
		Tooltip = "$RoomMenuBackToRooms$",
		BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
		OnMouseIn = GuiAction_SetTag("Hover"),
		OnMouseOut = GuiAction_SetTag("Std"),
		OnClick = GuiAction_Call(this, "SwitchToRoomMenuSelection", plr)
	};
	return;
}

public func MenuShowRoomAuthors(proplist authors)
{
	// Put all authors into the selection menu.
	var cnt = 0;
	for (var author in GetAuthorList())
	{
		var author =
		{
			Target = this,
			ID = cnt + 1000,
			Priority = cnt,
			Bottom = "1.5em",
			BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
			OnMouseIn = [GuiAction_SetTag("Hover"), GuiAction_Call(this, "UpdateAuthorSelectionInformation", {author = author[0], rooms = author[2]})],
			OnMouseOut = [GuiAction_SetTag("Std"), GuiAction_Call(this, "UpdateAuthorSelectionInformation")],
			symbol = 
			{
				Target = this,
				ID = cnt + 2000,
				Right = "1.5em",
			},
			text = 
			{
				Target = this,
				ID = cnt + 4000,
				Left = "1.5em",
				Text = Format("%s: %d $RoomMenuCreditsRooms$\n", author[0], author[1]),
				Style = GUI_TextVCenter
			}
		};
		MakeNumberMenuEntry(author.symbol, cnt + 1, 100);
		authors[Format("room%d", cnt)] = author;		
		cnt++;
	}
	return authors;
}

public func UpdateAuthorSelectionInformation(proplist pars)
{
	if (pars == nil)
	{
		if (menu.authorinfo.author != nil)
		{
			GuiClose(menu_id, menu.authorinfo.author.ID, menu.authorinfo.author.Target);
			menu.authorinfo.author = nil;
		}
		return;	
	}
	
	var author_rooms = Format("$RoomMenuCreditsAuthor$", pars.author);
	for (var room in pars.rooms)
		author_rooms = Format("%s %s,", author_rooms, room->GetRoomName());
	
	menu.authorinfo.author =
	{
		Target = this,
		ID = 41,
		Style = GUI_TextVCenter,
		Text = author_rooms
	};	
	GuiUpdate(menu.authorinfo, menu_id, menu.authorinfo.ID, this);
	return;
}


/*-- Tablets --*/

public func ShowRoomMenuTablets(int plr)
{
	CloseRoomMenu();
	var menu_obj = CreateObject(RoomMenu, 0, 0, plr);
	menu_obj->OpenRoomMenu(plr, "tablets");
	return;
}

public func MakeTabletsMenu(int plr)
{
	// Room menu proplist.
	menu =
	{
		Target = this,
		Decoration = GUI_MenuDeco,
		Margin = ["1em", "2em"],
		BackgroundColor = {Std = ROOMMENU_BackgroundColor},
	};
	
	// The basic panels for the menu.
	menu.tabletdesc = 
	{
		Target = this,
		ID = 1,
		Right = "70%-0.5em",
		Bottom = "4em",
		Margin = ["0.1em"],
		Text = "$RoomMenuTabletsText$"
	};
	menu.tabletlist = 
	{
		Target = this,
		ID = 2,
		Top = "2em",
		Bottom = "80%+0.5em",
		Style = GUI_GridLayout
	};
	menu.buttons =
	{
		Target = this,
		ID = 3,
		Left = "70%",
		Bottom = "1.5em"
	};
	menu.tabletinfo = 
	{
		Target = this,
		ID = 4,
		Top = "80%+0.5em",
		Margin = ["0.1em"]
	};
	
	menu.border1 = CreateBarMenuEntry(nil, nil, "1.5em", "2em");
	menu.border2 = CreateBarMenuEntry(nil, nil, "80%", "80%+0.5em");
	
	menu.tabletlist = MenuShowRoomTablets(menu.tabletlist, plr);
	
	// Room selection: header with close button.
	menu.buttons.close = 
	{
		Target = this,
		ID = 31,
		Left = "100%-1.5em",
		Symbol = Icon_Cancel,
		Tooltip = "$RoomMenuClose$",
		BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
		OnMouseIn = GuiAction_SetTag("Hover"),
		OnMouseOut = GuiAction_SetTag("Std"),
		OnClick = GuiAction_Call(this, "CloseRoomMenu")
	};
	menu.buttons.rooms = 
	{
		Target = this,
		ID = 32,
		Left = "100%-3em",
		Right = "100%-1.5em",
		Symbol = Icon_Enter,
		Tooltip = "$RoomMenuBackToRooms$",
		BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
		OnMouseIn = GuiAction_SetTag("Hover"),
		OnMouseOut = GuiAction_SetTag("Std"),
		OnClick = GuiAction_Call(this, "SwitchToRoomMenuSelection", plr)
	};
	return;
}

public func MenuShowRoomTablets(proplist tablets, int plr)
{
	var cnt = 0;
	for (var room in GetRoomList())
	{
		var tablet_symbol = nil;
		var room_part = nil;
		var tablet_found = false;
		if (HasPlayerFoundTablet(plr, room))
		{
			tablet_symbol = AncientTablet;
			room_part = 
			{
				Left = "1.5em",
				Top = "1.5em",
				Symbol = room
			};
			tablet_found = true;
		}
		var tablet =
		{
			Target = this,
			ID = cnt + 1000,
			Priority = cnt,
			Right = "3em",
			Bottom = "3em",
			Symbol = tablet_symbol,
			BackgroundColor = {Std = 0, Hover = ROOMMENU_HoverColor},
			OnMouseIn = GuiAction_SetTag("Hover"),
			OnMouseOut = GuiAction_SetTag("Std"),
			OnClick = GuiAction_Call(this, "UpdateTabletSelectionInformation", [room, tablet_found]),
			room_symbol = room_part
		};
		MakeNumberMenuEntry(tablet, cnt + 1);
		tablets[Format("room%d", cnt)] = tablet;		
		cnt++;
	}
	return tablets;
}

public func UpdateTabletSelectionInformation(array pars)
{
	var room = pars[0];
	var found = pars[1];
	var text = Format("$RoomMenuTabletsNotFound$", room->GetRoomName());
	if (found)
		text = Format("$RoomMenuTabletsFound$", room->GetRoomName(), AncientTablet->GetTabletCode(room->GetRoomID()));

	menu.tabletinfo.tablet =
	{
		Target = this,
		ID = 41,
		Text = text
	};	
	GuiUpdate(menu.tabletinfo, menu_id, menu.tabletinfo.ID, this);
	return;
}

/*-- Closing & Switching --*/

public func CloseRoomMenu()
{
	// Close the menu and inform the controller.
	GuiClose(menu_id, nil, this);
	menu_id = nil;
	if (menu_controller)
		menu_controller->MenuClosed();
	menu_controller = nil;
	return;
}

public func Close() { return RemoveObject(); }

public func SwitchToRoomMenuSelection(int plr)
{
	CloseRoomMenu();
	var menu_obj = CreateObject(RoomMenu, 0, 0, plr);
	menu_obj->OpenRoomMenu(plr, "rooms");
	return;
}


/*-- Saving --*/

// UI not saved.
public func SaveScenarioObject() { return false; }


/*-- Misc Functionality --*/

private func CreateBarMenuEntry(string left, string right, string top, string bottom)
{
	return
	{
		Left = left,
		Right = right,
		Top = top,
		Bottom = bottom,
		BackgroundColor = {Std = ROOMMENU_BarColor}
	};
}


/*-- Proplist --*/

local Name = "$Name$";
local Description = "$Description$";