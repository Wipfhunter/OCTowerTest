/**
	Room Base
	Base object for the room which contains the room's settings and scripts.

	@author Maikel
*/


/*-- Room Properties (Adjustable) --*/

public func GetRoomName() { return nil; }

public func GetRoomDescription() { return nil; }

public func GetRoomAuthor() { return nil; }

public func GetRoomSection() { return nil; }

public func GetRoomID() { return "__"; }

public func GetRoomDifficulty() { return nil; }

public func HasTablet() { return false; }

public func HasJoker() { return false; }


/*-- Room Properties (Fixed) --*/

public func IsRoom() { return true; }

// Returns a list with the authors of the room.
public func GetRoomAuthorList()
{
	var author_list = [];
	var authors = GetRoomAuthor();
	// Split author names at , and & with arbitrary number of whitespaces.
	if (GetType(GetRoomAuthor()) == C4V_String)
		author_list = RegexSplit(authors, "\\s*[,&]\\s*");
	return author_list;
}


/*-- Room Control --*/

public func LoadRoom(bool reload)
{
	var fx = AddEffect("IntScheduleLoadRoom", nil, 1, 1, nil, this);
	fx.reload = reload;
	return;
}

public func FxIntScheduleLoadRoomStop(object target, proplist fx, int reason, bool temp)
{
	if (temp || reason != FX_Call_Normal)
		return FX_OK;
	DoLoadRoom(fx.reload);
	return FX_OK;
}

public func DoLoadRoom(bool reload)
{
	var sect = GetRoomSection();
	if (!sect)
		return;
		
	// Load the room from its scenario section.
	if (reload)
	{
		//Log("[%d]Load section Empty", FrameCounter());
		//LogCallStack();
		LoadScenarioSection("Empty");
	}
	//Log("[%d]Load section %v", FrameCounter(), sect);
	//LogCallStack();
	LoadScenarioSection(sect);
	
	// Create the room control object and init.
	var room_control = CreateObject(this);
	room_control->InitRoom();
	return;
}

local playing_plr;
local waiting_container;

public func InitRoom()
{
	// Create basic rules.
	if (!GameCall("IsTemplateRoom"))
		CreateObject(Rule_Restart);	
	// Call to the specific room object to init objects.
	OnRoomInit();
	// Determine which player is playing the room.
	playing_plr = GetNextPlayerInQueue();
	// Create a container for waiting players.
	waiting_container = CreateObject(RelaunchContainer, LandscapeWidth() / 2, LandscapeHeight() / 2);
	// Initialize players in this room.
	for (var plr in GetPlayers(C4PT_User))
		InitializePlayer(plr);
	return;
}

public func OnRoomInit()
{
	// Implemented in the specific room control object.
	return;
}

public func OnPlayerInit(int plr)
{
	// Implemented in the specific room control object.
	return;
}


/*-- Player Control --*/

public func InitializePlayer(int plr)
{
	// Join the player with its crew.
	JoinPlayer(plr);
	return;
}

public func RelaunchPlayer(int plr)
{
	// Add the playing player to the playing queue again.
	if (plr == playing_plr)
		AppendPlayerToQueue(plr);
	
	// Reset the room if not already scheduled and if not in template.
	if (!GameCall("IsTemplateRoom"))
	{
		if (!GetEffect("IntScheduleLoad*"))
			GetID()->LoadRoom(true);
	}
	else
	{
		// Just rejoin the player again, the room can be tested in template mode with F12.
		JoinPlayer(plr);
	}
	return;
}

protected func JoinPlayer(int plr)
{
	// Get crew member or create new one.
	var crew = GetCrew(plr);
	if (!crew)
	{
		var crew = CreateObjectAbove(Clonk, 0, 0, plr);
		crew->MakeCrewMember(plr);
		SetCursor(plr, crew);
	}
	
	// Give clonk its maximum energy.
	crew->DoEnergy(crew.MaxEnergy / 1000);
	
	// Determine if the player is the one playing. If so move crew to
	// room entrance, otherwise into container object.
	if (plr == playing_plr || GameCall("IsTemplateRoom"))
	{
		// Log that a new player attempts the room.
		if (!GameCall("IsTemplateRoom"))
			Log("$MsgPlayerAttemptsRoom$", GetTaggedPlayerName(plr), GetRoomName());
		
		// Also initialize via the player start object.
		if (!GameCall("IsTemplateRoom"))
			for (var plr_start in FindObjects(Find_ID(PlayerStart)))
				plr_start->InitializePlayer(plr);
			
		// Move the crew to the room entrance if available.	
		var room_entrance = FindObject(Find_ID(RoomEntrance));
		if (room_entrance)
			crew->SetPosition(room_entrance->GetX(), room_entrance->GetY());
			
		// Call to the specific room object to init the players.
		OnPlayerInit(plr);
		
		// Set the view of the other players to the playing player.
		if (!GameCall("IsTemplateRoom"))
		{
			for (var other_plr in GetPlayers(C4PT_User))
				if (other_plr != playing_plr)
					SetViewCursor(other_plr, crew);
		}
	}
	else
	{
		// Observing players move into the waiting container.
		crew->Enter(waiting_container);
		// Set player view to the playing player.
		SetViewCursor(plr, GetCrew(playing_plr));
	}
	return;
}


/*-- Goal Control --*/

public func OnRoomExitEntered(object crew)
{
	// Notify the scenario script the room has been completed.
	GameCall("OnRoomCompleted", crew, this->GetID());
	return;
}

public func OnJokerCollected(object crew, object joker)
{
	// Remove the joker.
	joker->RemoveObject();	
	// Notify the scenario script the joker has been collected.
	GameCall("OnRoomJokerCompleted", crew, this->GetID());
	return;
}

public func OnTabletCollected(object crew, object tablet)
{
	// Remove the tablet.
	tablet->RemoveObject();	
	// Notify the scenario script the joker has been collected.
	GameCall("OnRoomTabletCompleted", crew, this->GetID());
	return;
}


/*-- Saving --*/

// Do not save this object as it is recreated on the loading of a new room.
public func SaveScenarioObject() { return false; }


/*-- Properties --*/

local Name = "$Name$";
local Description = "$Description$";