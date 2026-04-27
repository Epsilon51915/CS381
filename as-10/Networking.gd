extends Control

const PLAYER_SCENE = preload("res://Player.tscn")
const PORT = 9999
var peer : ENetMultiplayerPeer = ENetMultiplayerPeer.new()
@onready var IP_FIELD = $Menu/PanelContainer/VBoxContainer/IP
@onready var player_ids = $Label/PlayerIDs
func button_common():
	$Label.show()
	$Menu.hide()
	

func _on_host_pressed() -> void:
	button_common()
	peer.create_server(PORT)
	multiplayer.multiplayer_peer = peer
	multiplayer.peer_connected.connect(add_player)
	multiplayer.peer_disconnected.connect(remove_player)
	add_player(multiplayer.get_unique_id())


func _on_join_pressed() -> void:
	button_common()
	peer.create_client(IP_FIELD.text, PORT)
	multiplayer.multiplayer_peer = peer

func add_player(peer_id):
	var player = PLAYER_SCENE.instantiate()
	player.name = "Player " + String.num_int64(peer_id)
	player.text = player.name
	player_ids.add_child(player)
	
func remove_player(peer_id):
	for child in player_ids.get_children():
		if child.name == "Player " + String.num_int64(peer_id):
			child.queue_free()
			break
