extends Label

var sync_value = 0

func update_value(new_val, sender_id = 1):
	if !(is_multiplayer_authority() or sender_id == 0):
		update_value_rpc.rpc_id(1, new_val, multiplayer.get_unique_id())
		return
	
	sync_value = new_val
	text = "Value: " + String.num_int64(sync_value)
	
	if is_multiplayer_authority() and sender_id != 0:
		print(sender_id, " has set the value to: ", sync_value)
		update_value_rpc.rpc(new_val, 0)

@rpc("any_peer")
func update_value_rpc(new_val, sender_id):
	if is_multiplayer_authority() or sender_id == 0:
		update_value(new_val, sender_id)

func _ready():
	if(is_multiplayer_authority()):
		update_value(0)


func _on_increment_pressed() -> void:
	update_value(sync_value + 1)

func _on_decrement_pressed() -> void:
	update_value(sync_value - 1)
