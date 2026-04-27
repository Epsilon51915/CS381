******************************************
**
**	Tyler Carroll
**	CS 381 AS10 
**
******************************************

How to run program:
	Open the provided project file (as-10) within the Godot editor, set the number of instances to any number (if you would like to test the multiplayer
	aspect, choose 2 or more instances,) and run the program.

Controls:
	The controls for this assignment are simple:
		For one instance, use the mouse to select the "Host Game" button. This sets up a server for other instances to connect to.
		To all connecting instances, select "Join Game." This connects these instances to the host instance.
	
		For all instances, once you are in the "game", press increment to update the shown value by 1, and decrement to decrease
		the shown value by 1.

Readme Question:
	The RPC updates values by first checking that the server itself (sender_id 0) is not doing any incrementing. The function then increments the 
	value or decrements it as necessary, and then sends out a message to all users. 
